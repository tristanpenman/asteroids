#include <limits.h>

#include "mathdefs.h"

long lroundf(float x)
{
    if (x != x) {
        return 0L;
    }

    if (x >= (float)LONG_MAX) {
        return LONG_MAX;
    }

    if (x <= (float)LONG_MIN) {
        return LONG_MIN;
    }

    if (x >= 0.0f) {
        return (long)(x + 0.5f);
    } else {
        return (long)(x - 0.5f);
    }
}

float sqrtf(float x)
{
    return (float)sqrt((double)x);
}
