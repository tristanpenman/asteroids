#include <math.h>
#include <stdlib.h>

#include <nusys.h>

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
    // while (angle > 2 * M_PI) {
    //     angle = angle - 2 * M_PI;
    // }
    // while (angle < 0.0f) {
    //     angle = 2 * M_PI - angle;
    // }

    return angle;
}

bool wrap_position(struct vec_2d *pos, float radius)
{
    bool wrapped = false;

    // float *x = &pos->x;
    // float *y = &pos->y;

    // if (*x < 0 - origin.x - radius) {      /* Wrap horizontally */
    //     *x = origin.x + radius;
    //     wrapped = true;
    // } else if (*x > origin.x + radius) {
    //     *x = 0 - origin.x - radius;
    //     wrapped = true;
    // }

    // if (*y < 0 - origin.y - radius) {      /* Wrap vertically */
    //     *y = origin.y + radius;
    //     wrapped = true;
    // } else if (*y > origin.y + radius) {
    //     *y = 0 - origin.y - radius;
    //     wrapped = true;
    // }

    return wrapped;
}
