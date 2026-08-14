#ifndef ASTEROIDS_UTIL_H
#define ASTEROIDS_UTIL_H

#include "types.h"

struct vec_2d;

float random_float(float low, float high);

float wrap_angle(float angle);

bool wrap_position(struct vec_2d *pos, float radius);

#endif
