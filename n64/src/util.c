#include <stdlib.h>

#include <nusys.h>

#include "defines.h"
#include "mathdefs.h"
#include "types.h"
#include "util.h"
#include "vec.h"

/******************************************************************************
 *
 * Public interface
 *
 *****************************************************************************/

uint32_t htonl(uint32_t x)
{
#if BYTE_ORDER == LITTLE_ENDIAN
    uint8_t *s = (uint8_t *)&x;
    return (uint32_t)(s[0] << 24 | s[1] << 16 | s[2] << 8 | s[3]);
#else
    return x;
#endif
}

uint32_t ntohl(uint32_t x)
{
    return htonl(x);
}

float random_float(float low, float high)
{
    return ((float)rand() * (high - low)) / (float)RAND_MAX + low;
}

float wrap_angle(float angle)
{
    while (angle > 2 * M_PI) {
        angle = angle - 2 * M_PI;
    }
    while (angle < 0.0f) {
        angle = 2 * M_PI - angle;
    }

    return angle;
}

bool wrap_position(struct vec_2d *pos, const struct vec_2d *lim)
{
    bool wrapped = false;

    float *x = &pos->x;
    float *y = &pos->y;

    // Wrap horizontally
    if (*x < -lim->x) {
        *x = lim->x;
        wrapped = true;
    } else if (*x > lim->x) {
        *x = -lim->x;
        wrapped = true;
    }

    // Wrap vertically
    if (*y < -lim->y) {
        *y = lim->y;
        wrapped = true;
    } else if (*y > lim->y) {
        *y = -lim->y;
        wrapped = true;
    }

    return wrapped;
}
