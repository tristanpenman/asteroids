#ifndef __DEBUG_H
#define __DEBUG_H

#include <assert.h>

#if DEBUG

    void debug_init();

    void debug_printf(const char* message, ...);

    #define debug_assert(expr) assert(expr)

#else

    // Overwrite library functions with useless macros if debug mode is disabled
    #define debug_init()

    inline void debug_printf(const char* message, ...) {
      // variadic macros not available before C99
    }

    #define debug_assert(a)

#endif

#endif
