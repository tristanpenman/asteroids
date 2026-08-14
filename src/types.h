#ifndef ASTEROIDS_TYPES_H
#define ASTEROIDS_TYPES_H

#ifdef N64

#define true 1
#define false 0
typedef int bool;
typedef unsigned char uint8_t;

#else

#include <stdbool.h>
#include <stdint.h>

#endif

#endif
