#include "timing.h"

static uint32_t simulation_time;

void timing_reset(void)
{
    simulation_time = 0;
}

bool timing_advance(uint32_t millis)
{
    simulation_time += 1000u / 60u;
    return millis > 0 && millis <= simulation_time;
}

bool timing_consume(uint32_t millis)
{
    if (millis == 0 || millis > simulation_time) {
        return false;
    }

    simulation_time -= millis;
    return true;
}

uint32_t timing_residual(void)
{
    return simulation_time;
}
