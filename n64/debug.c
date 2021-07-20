/***************************************************************
A basic debug library that makes use of the USB library for N64
flashcarts.
https://github.com/buu342/N64-UNFLoader
***************************************************************/

#include <stdarg.h>
#include <string.h>

#include <ultra64.h>
#include <PR/os_internal.h> // Needed for Crash's Linux toolchain

#include "unf_usb.h"

#include "debug.h"

#if DEBUG

    #define MAX_COMMANDS     25  // The max amount of user defined commands possible

    // USB thread definitions
    #define USB_THREAD_ID    14
    #define USB_THREAD_PRI   126
    #define USB_THREAD_STACK 0x2000

    /*********************************
               Definitions
    *********************************/

    #define MSG_READ  0x11
    #define MSG_WRITE 0x12

    #define USBERROR_NONE    0
    #define USBERROR_NOTTEXT 1
    #define USBERROR_UNKNOWN 2
    #define USBERROR_TOOMUCH 3
    #define USBERROR_CUSTOM  4

    #define HASHTABLE_SIZE 7
    #define COMMAND_TOKENS 10
    #define BUFFER_SIZE    256

    /*********************************
                 Structs
    *********************************/

    // Register struct
    typedef struct
    {
        u32 mask;
        u32 value;
        char *string;
    } regDesc;

    // Thread message struct
    typedef struct
    {
        int msgtype;
        int datatype;
        void* buff;
        int size;
    } usbMesg;

    // Debug command struct
    typedef struct
    {
        char* command;
        char* description;
        char* (*execute)();
        void* next;
    } debugCommand;

    /*********************************
            Function Prototypes
    *********************************/

    // Threads
    static void debug_thread_usb(void *arg);

    // Other
    static void* debug_osSyncPrintf_implementation(void *unused, const char *str, size_t len);

    /*********************************
                 Globals
    *********************************/

    // Function pointers
    extern int _Printf(void *(*copyfunc)(void *, const char *, size_t), void*, const char*, va_list);
    extern void* __printfunc;

    // Debug globals
    static char  debug_initialized = 0;
    static char  debug_buffer[BUFFER_SIZE];

    // Commands hashtable related
    static debugCommand* debug_commands_hashtable[HASHTABLE_SIZE];
    static debugCommand  debug_commands_elements[MAX_COMMANDS];
    static int debug_commands_count = 0;

    // Command parsing related
    static int debug_command_current = 0;
    static int debug_command_totaltokens = 0;
    static int debug_command_incoming_start[COMMAND_TOKENS];
    static int debug_command_incoming_size[COMMAND_TOKENS];
    static char* debug_command_error;

    // Assertion globals
    static int assert_line = 0;
    static const char* assert_file = NULL;
    static const char* assert_expr = NULL;

    // USB thread globals
    static OSMesgQueue usbMessageQ;
    static OSMesg      usbMessageBuf;
    static OSThread    usbThread;
    static u64         usbThreadStack[USB_THREAD_STACK/sizeof(u64)];

    /*********************************
             Debug functions
    *********************************/

    /*==============================
        debug_initialize
        Initializes the debug library
    ==============================*/

    void debug_init()
    {
        // Initialize the USB functions
        if (!usb_initialize()) {
            return;
        }

        // Initialize the USB thread
        osCreateThread(&usbThread, USB_THREAD_ID, debug_thread_usb, 0,
                        (usbThreadStack+USB_THREAD_STACK/sizeof(u64)),
                        USB_THREAD_PRI);
        osStartThread(&usbThread);

        __printfunc = (void*)debug_osSyncPrintf_implementation;

        // Mark the debug mode as initialized
        debug_initialized = 1;
        debug_printf("debug_init...\n");
    }

    /*==============================
        printf_handler
        Handles printf memory copying
        @param The buffer to copy the partial string to
        @param The string to copy
        @param The length of the string
        @returns The end of the buffer that was written to
    ==============================*/

    static void* printf_handler(void *buf, const char *str, size_t len)
    {
        return ((char *) memcpy(buf, str, len) + len);
    }

    /*==============================
        debug_printf
        Prints a formatted message to the developer's command prompt.
        Supports up to 256 characters.
        @param A string to print
        @param variadic arguments to print as well
    ==============================*/

    void debug_printf(const char* message, ...)
    {
        int len = 0;
        usbMesg msg __attribute__((aligned(8)));
        va_list args;

        if (!debug_initialized) {
            return;
        }

        // use the internal libultra printf function to format the string
        va_start(args, message);
        len = _Printf(&printf_handler, debug_buffer, message, args);
        va_end(args);

        // Attach the '\0' if necessary
        if (0 <= len) {
            debug_buffer[len] = '\0';
        }

        // Send the printf to the usb thread
        msg.msgtype = MSG_WRITE;
        msg.datatype = DATATYPE_TEXT;
        msg.buff = debug_buffer;
        msg.size = len+1;
        osSendMesg(&usbMessageQ, (OSMesg)&msg, OS_MESG_BLOCK);
    }

    /*==============================
        debug_dumpbinary
        Dumps a binary file through USB
        @param The file to dump
        @param The size of the file
    ==============================*/

    void debug_dumpbinary(void* file, int size)
    {
        usbMesg msg;

        // Send the binary file to the usb thread
        msg.msgtype = MSG_WRITE;
        msg.datatype = DATATYPE_RAWBINARY;
        msg.buff = file;
        msg.size = size;
        osSendMesg(&usbMessageQ, (OSMesg)&msg, OS_MESG_BLOCK);
    }

    /*==============================
        _debug_assert
        Halts the program (assumes expression failed)
        @param The expression that was tested
        @param The file where the exception ocurred
        @param The line number where the exception ocurred
    ==============================*/

    void _debug_assert(const char* expression, const char* file, int line)
    {
        // Set the assert data
        assert_expr = expression;
        assert_line = line;
        assert_file = file;

        // Intentionally cause a null pointer exception
        *((char*)(NULL)) = 0;
    }

    /*==============================
        debug_addcommand
        Adds a command for the USB to listen for
        @param The command name
        @param The command description
        @param The function pointer to execute
    ==============================*/

    void debug_addcommand(char* command, char* description, char* (*execute)())
    {
        int entry = command[0]%HASHTABLE_SIZE;
        debugCommand* slot = debug_commands_hashtable[entry];

        // Ensure debug mode is initialized
        if (!debug_initialized) {
            return;
        }

        // Ensure we haven't hit the command limit
        if (debug_commands_count == MAX_COMMANDS) {
            debug_printf("Max commands exceeded!\n");
            return;
        }

        // Look for an empty spot in the hash table
        if (slot != NULL) {
            while (slot->next != NULL) {
                slot = slot->next;
            }
            slot->next = &debug_commands_elements[debug_commands_count];
        } else {
            debug_commands_hashtable[entry] = &debug_commands_elements[debug_commands_count];
        }

        // Fill this spot with info about this command
        debug_commands_elements[debug_commands_count].command     = command;
        debug_commands_elements[debug_commands_count].description = description;
        debug_commands_elements[debug_commands_count].execute     = execute;
        debug_commands_count++;
    }

    /*==============================
        debug_pollcommands
        Check the USB for incoming commands
    ==============================*/

    void debug_pollcommands()
    {
        usbMesg msg;

        // Ensure debug mode is initialized
        if (!debug_initialized) {
            return;
        }

        // Send a read message to the USB thread
        msg.msgtype = MSG_READ;
        osSendMesg(&usbMessageQ, (OSMesg)&msg, OS_MESG_BLOCK);
    }

    /*==============================
        debug_sizecommand
        Returns the size of the data from this part of the command
        @return The size of the data in bytes, or 0
    ==============================*/

    int debug_sizecommand()
    {
        // If we're out of commands to read, return 0
        if (debug_command_current == debug_command_totaltokens) {
            return 0;
        }

        // Otherwise, return the amount of data to read
        return debug_command_incoming_size[debug_command_current];
    }

    /*==============================
        debug_parsecommand
        Stores the next part of the incoming command into the provided buffer.
        Make sure the buffer can fit the amount of data from debug_sizecommand!
        If you pass NULL, it skips this command.
        @param The buffer to store the data in
    ==============================*/

    void debug_parsecommand(void* buffer)
    {
        u8 curr = debug_command_current;

        // Skip this command if no buffer exists
        if (buffer == NULL) {
            debug_command_current++;
            return;
        }

        // If we're out of commands to read, do nothing
        if (curr == debug_command_totaltokens) {
            return;
        }

        // Read from the correct offset
        usb_skip(debug_command_incoming_start[curr]);
        usb_read(buffer, debug_command_incoming_size[curr]);
        usb_rewind(debug_command_incoming_size[curr]+debug_command_incoming_start[curr]);
        debug_command_current++;
    }

    /*==============================
        debug_commands_setup
        Reads the entire incoming string and breaks it into parts for
        debug_parsecommand and debug_sizecommand
    ==============================*/

    static void debug_commands_setup()
    {
        int i;
        int datasize = USBHEADER_GETSIZE(usb_poll());
        int dataleft = datasize;
        int filesize = 0;
        char filestep = 0;

        // Initialize the starting offsets at -1
        memset(debug_command_incoming_start, -1, COMMAND_TOKENS*sizeof(int));

        // Read data from USB in blocks
        while (dataleft > 0) {
            int readsize = BUFFER_SIZE;
            if (readsize > dataleft) {
                readsize = dataleft;
            }

            // Read a block from USB
            memset(debug_buffer, 0, BUFFER_SIZE);
            usb_read(debug_buffer, readsize);

            // Parse the block
            for (i = 0; i < readsize && dataleft > 0; i++) {
                // If we're not reading a file
                int offset = datasize-dataleft;
                u8 tok = debug_command_totaltokens;

                // Decide what to do based on the current character
                switch (debug_buffer[i]) {
                    case ' ':
                    case '\0':
                        if (filestep < 2) {
                            if (debug_command_incoming_start[tok] != -1) {
                                debug_command_incoming_size[tok] = offset-debug_command_incoming_start[tok];
                                debug_command_totaltokens++;
                            }

                            if (debug_buffer[i] == '\0') {
                                dataleft = 0;
                            }

                            break;
                        }

                    case '@':
                        filestep++;
                        if (filestep < 3) {
                            break;
                        }

                    default:
                        // Decide what to do based on the file handle
                        if (filestep == 0 && debug_command_incoming_start[tok] == -1) {
                            // Store the data offsets and sizes in the global command buffers
                            debug_command_incoming_start[tok] = offset;
                        } else if (filestep == 1) {
                            // Get the filesize
                            filesize = filesize*10 + debug_buffer[i]-'0';
                        } else if (filestep > 1) {
                            // Store the file offsets and sizes in the global command buffers
                            debug_command_incoming_start[tok] = offset;
                            debug_command_incoming_size[tok] = filesize;
                            debug_command_totaltokens++;

                            // Skip a bunch of bytes
                            if ((readsize-i)-filesize < 0) {
                                usb_skip(filesize-(readsize-i));
                            }
                            dataleft -= filesize;
                            i += filesize;
                            filesize = 0;
                            filestep = 0;
                        }
                        break;
                }
                dataleft--;
            }
        }

        // Rewind the USB fully
        usb_rewind(datasize);
    }

    /*==============================
        debug_thread_usb
        Handles the USB thread
        @param Arbitrary data that the thread can receive
    ==============================*/

    static void debug_thread_usb(void *arg)
    {
        char errortype = USBERROR_NONE;
        usbMesg* threadMsg;

        // Create the message queue for the USB message
        osCreateMesgQueue(&usbMessageQ, &usbMessageBuf, 1);

        // Thread loop
        while (1) {
            // Wait for a USB message to arrive
            osRecvMesg(&usbMessageQ, (OSMesg *)&threadMsg, OS_MESG_BLOCK);

            // Ensure there's no data in the USB (which handles MSG_READ)
            while (usb_poll() != 0) {
                int header = usb_poll();
                debugCommand* entry;

                // Ensure we're receiving a text command
                if (USBHEADER_GETTYPE(header) != DATATYPE_TEXT) {
                    errortype = USBERROR_NOTTEXT;
                    usb_purge();
                    break;
                }

                // Initialize the command trackers
                debug_command_totaltokens = 0;
                debug_command_current = 0;

                // Break the USB command into parts
                debug_commands_setup();

                // Ensure we don't read past our buffer
                if (debug_sizecommand() > BUFFER_SIZE) {
                    errortype = USBERROR_TOOMUCH;
                    usb_purge();
                    break;
                }

                // Read from the USB to retrieve the command name
                debug_parsecommand(debug_buffer);

                // Iterate through the hashtable to see if we find the command
                entry = debug_commands_hashtable[debug_buffer[0]%HASHTABLE_SIZE];
                while (entry != NULL) {
                    // If we found the command
                    if (!strncmp(debug_buffer, entry->command, debug_command_incoming_size[0])) {
                        // Execute the command function and exit the while loop
                        debug_command_error = entry->execute();
                        if (debug_command_error != NULL) {
                            errortype = USBERROR_CUSTOM;
                        }
                        usb_purge();
                        break;
                    }
                    entry = entry->next;
                }

                // If no command was found
                if (entry == NULL) {
                    // Purge the USB contents and print unknown command
                    usb_purge();
                    errortype = USBERROR_UNKNOWN;
                }
            }

            // Spit out an error if there was one during the command parsing
            if (errortype != USBERROR_NONE) {
                switch (errortype){
                    case USBERROR_NOTTEXT:
                        usb_write(DATATYPE_TEXT, "Error: USB data was not text\n", 29+1);
                        break;
                    case USBERROR_UNKNOWN:
                        usb_write(DATATYPE_TEXT, "Error: Unknown command\n", 23+1);
                        break;
                    case USBERROR_TOOMUCH:
                        usb_write(DATATYPE_TEXT, "Error: Command too large\n", 25+1);
                        break;
                    case USBERROR_CUSTOM:
                        usb_write(DATATYPE_TEXT, debug_command_error, strlen(debug_command_error)+1);
                        usb_write(DATATYPE_TEXT, "\n", 1+1);
                        break;
                }
                errortype = USBERROR_NONE;
            }

            // Handle the other USB messages
            switch (threadMsg->msgtype) {
                case MSG_WRITE:
                    usb_write(threadMsg->datatype, threadMsg->buff, threadMsg->size);
                    break;
            }
        }
    }

    /*==============================
        debug_osSyncPrintf_implementation
        Overwrites osSyncPrintf calls with this one
        @param Unused
        @param The buffer with the string
        @param The amount of characters to write
        @returns The end of the buffer that was written to
    ==============================*/

    static void* debug_osSyncPrintf_implementation(void *unused, const char *str, size_t len)
    {
        void* ret;
        usbMesg msg;

        // Clear the debug buffer and copy the formatted string to it
        memset(debug_buffer, 0, len+1);
        ret = ((char *) memcpy(debug_buffer, str, len) + len);

        // Send the printf to the usb thread
        msg.msgtype = MSG_WRITE;
        msg.datatype = DATATYPE_TEXT;
        msg.buff = debug_buffer;
        msg.size = len+1;
        osSendMesg(&usbMessageQ, (OSMesg)&msg, OS_MESG_BLOCK);

        // Return the end of the buffer
        return ret;
    }

#endif
