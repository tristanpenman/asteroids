#ifndef __TYPES_H
#define __TYPES_H

#ifdef N64

#define true 1
#define false 0
typedef int bool;
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;

#else

#include <stdbool.h>
#include <stdint.h>

#endif

#endif
