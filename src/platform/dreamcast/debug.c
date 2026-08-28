#include <stdarg.h>
#include <stdio.h>

#include "debug.h"

#if DEBUG

void debug_init(void)
{
    debug_printf("Dreamcast debug console ready\n");
}

void debug_printf(const char *message, ...)
{
    va_list args;

    va_start(args, message);
    vprintf(message, args);
    va_end(args);
}

#endif
