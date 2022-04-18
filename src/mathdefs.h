#ifndef __MATHDEFS_H
#define __MATHDEFS_H

#include <math.h>

#ifdef N64

#define sinf sin
#define cosf cos
#define sqrtf sqrt

#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

#define RAD_TO_DEG (180.0 / M_PI)

#endif
