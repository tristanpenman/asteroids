#ifndef ASTEROIDS_GAME_TYPES_H
#define ASTEROIDS_GAME_TYPES_H

#ifdef N64

#define true 1
#define false 0
typedef int bool;
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

#else

#include <stdbool.h>
#include <stdint.h>

#endif

#endif
