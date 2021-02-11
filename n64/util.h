#ifndef __UTIL_H
#define __UTIL_H

#include <stdbool.h>

struct vec_2d;

float random_float(float low, float high);

float wrap_angle(float angle);

bool wrap_position(struct vec_2d *pos, const struct vec_2d *lim);

#endif
