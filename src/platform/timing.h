#ifndef ASTEROIDS_PLATFORM_TIMING_H
#define ASTEROIDS_PLATFORM_TIMING_H

#include "types.h"

#define TIMING_STEP_MILLIS 16u

void timing_reset(void);
bool timing_advance(uint32_t millis);
bool timing_consume(uint32_t millis);
uint32_t timing_residual(void);

#endif
