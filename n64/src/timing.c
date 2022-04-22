#include "types.h"

static uint32_t simulation_time = 0;
static uint32_t ticks;

/******************************************************************************
 *
 * Public interface
 *
 *****************************************************************************/

void reset_simulation_time()
{
    simulation_time = 0;
}

void produce_simulation_time()
{
    simulation_time += 1000.0f / 60.0f;
}

bool maybe_consume_simulation_time(uint32_t millis)
{
    if (millis == 0) {
        return false;
    }

    if (millis > simulation_time) {
        return false;
    }

    simulation_time -= millis;
    return true;
}

uint32_t residual_simulation_time()
{
    return simulation_time;
}
