#include <stdlib.h>

#include <nusys.h>

#include "defines.h"
#include "mathdefs.h"
#include "util.h"
#include "vec.h"

/******************************************************************************
 *
 * Public interface
 *
 *****************************************************************************/

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
