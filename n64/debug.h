#ifndef __DEBUG_H
#define __DEBUG_H

#if DEBUG

    void debug_init();

    void debug_printf(const char* message, ...);

    void debug_dumpbinary(void* file, int size);

    #define debug_assert(expr) (expr) ? ((void)0) : _debug_assert(#expr, __FILE__, __LINE__)

    void debug_pollcommands();

    void debug_addcommand(char* command, char* description, char*(*execute)());

    void debug_parsecommand(void* buffer);

    int debug_sizecommand();

    // Ignore this, use the macro instead
    void _debug_assert(const char* expression, const char* file, int line);

#else

    // Overwrite library functions with useless macros if debug mode is disabled
    #define debug_init()
    #define debug_assert(a)
    #define debug_pollcommands()
    #define debug_addcommand(a, b, c)
    #define debug_parsecommand() NULL
    #define debug_sizecommand() 0

    static inline void debug_printf(const char* message, ...) {
      // variadic macros not available before C99
    }

#endif

#endif
