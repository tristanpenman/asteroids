#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "debug.h"

#if DEBUG

    void debug_init()
    {
        debug_printf("debug_init...\n");
    }

    void debug_printf(const char* message, ...)
    {
        va_list args;
        va_start(args, message);
        vprintf(message, args);
        va_end(args);
    }

#endif
