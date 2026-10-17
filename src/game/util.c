#include <stdlib.h>

#ifdef _MSC_VER
#include <SDL.h>
#endif

#include "canvas.h"
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
    while (angle > 2.0f * PI) {
        angle = angle - 2.0f * PI;
    }
    while (angle < 0.0f) {
        angle = 2.0f * PI - angle;
    }
    return angle;
}

bool wrap_position(struct vec_2d *pos, float radius)
{
    bool wrapped = false;

    float *x = &pos->x;
    float *y = &pos->y;

    if (*x < -canvas_origin.x - radius) { // Wrap horizontally
        *x = canvas_origin.x + radius;
        wrapped = true;
    } else if (*x > canvas_origin.x + radius) {
        *x = -canvas_origin.x - radius;
        wrapped = true;
    }

    if (*y < -canvas_origin.y - radius) { // Wrap vertically
        *y = canvas_origin.y + radius;
        wrapped = true;
    } else if (*y > canvas_origin.y + radius) {
        *y = -canvas_origin.y - radius;
        wrapped = true;
    }

    return wrapped;
}
