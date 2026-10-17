#ifndef ASTEROIDS_PLATFORM_MATHDEFS_H
#define ASTEROIDS_PLATFORM_MATHDEFS_H

#include <math.h>

#define PI 3.14159265358979323846f
#define RAD_TO_DEG (180.0f / PI)

/* Some platform math headers omit these C99 declarations even when their
 * libraries provide the corresponding functions. */
float cosf(float x);
long lroundf(float x);
float sinf(float x);
float sqrtf(float x);

#endif
