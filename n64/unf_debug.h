#ifndef UNFL_DEBUG_H
#define UNFL_DEBUG_H

#if DEBUG_MODE

    /*==============================
        debug_initialize
        Initializes the debug and USB library.
    ==============================*/

    void debug_initialize();

    /*==============================
        debug_printf
        Prints a formatted message to the developer's command prompt.
        Supports up to 256 characters.
        @param A string to print
        @param variadic arguments to print as well
    ==============================*/

    void debug_printf(const char* message, ...);

    /*==============================
        debug_dumpbinary
        Dumps a binary file through USB
        @param The file to dump
        @param The size of the file
    ==============================*/

    void debug_dumpbinary(void* file, int size);

    /*==============================
        debug_assert
        Halts the program if the expression fails.
        @param The expression to test
    ==============================*/

    #define debug_assert(expr) (expr) ? ((void)0) : _debug_assert(#expr, __FILE__, __LINE__)

    /*==============================
        debug_pollcommands
        Check the USB for incoming commands.
    ==============================*/

    void debug_pollcommands();

    /*==============================
        debug_addcommand
        Adds a command for the USB to read.
        @param The command name
        @param The command description
        @param The function pointer to execute
    ==============================*/

    void debug_addcommand(char* command, char* description, char*(*execute)());

    /*==============================
        debug_parsecommand
        Stores the next part of the incoming command into the provided buffer.
        Make sure the buffer can fit the amount of data from debug_sizecommand!
        If you pass NULL, it skips this command.
        @param The buffer to store the data in
    ==============================*/

    extern void debug_parsecommand(void* buffer);

    /*==============================
        debug_sizecommand
        Returns the size of the data from this part of the command.
        @return The size of the data in bytes, or 0
    ==============================*/

    int debug_sizecommand();

    /*==============================
        debug_printcommands
        Prints a list of commands to the developer's command prompt.
    ==============================*/

    void debug_printcommands();

    // Ignore this, use the macro instead
    void _debug_assert(const char* expression, const char* file, int line);

#else

    // Overwrite library functions with useless macros if debug mode is disabled
    #define debug_initialize()
    #define debug_assert(a)
    #define debug_pollcommands()
    #define debug_addcommand(a, b, c)
    #define debug_parsecommand() NULL
    #define debug_sizecommand() 0

    inline void debug_printf(const char* message, ...) {
      // variadic macros not available before C99
    }

#endif

#endif
