#include "timing.h"

static uint32_t simulation_time;

void reset_simulation_time(void)
{
    simulation_time = 0;
}

void produce_simulation_time(void)
{
    simulation_time += 1000 / 60;
}

bool maybe_consume_simulation_time(uint32_t millis)
{
    if (millis == 0 || millis > simulation_time) {
        return false;
    }

    simulation_time -= millis;
    return true;
}

uint32_t residual_simulation_time(void)
{
    return simulation_time;
}
