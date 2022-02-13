#ifndef __TIMING_H
#define __TIMING_H

#include "types.h"

/**
 * Reset residual simulation time to zero
 */
void reset_simulation_time();

/**
 * Update the available / residual simulation time based on system ticks
 *
 * The amount of time produced will be based on the number of system ticks
 * that have occurred since the last call to this function, or
 * \c reset_simulation_time(), whichever was most recent
 */
void produce_simulation_time();

/**
 * Consume a given amount of simulation time, if available
 *
 * If simulation time is available, the residual time will be reduced by the
 * amount that has been consumed.
 *
 * @return \c true if time was available; \c false otherwise
 */
bool maybe_consume_simulation_time(uint32_t millis);

/**
 * Get the currently available, or residual, simulation time
 *
 * @return residual simulation time, in milliseconds
 */
uint32_t residual_simulation_time();

#endif
