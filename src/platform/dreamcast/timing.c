#include <kos.h>

#include "timing.h"

#define MAX_FRAME_MILLIS 16u

static uint32_t simulation_time;
static uint64_t previous_millis;

void timing_reset(void)
{
    simulation_time = 0;
    previous_millis = timer_ms_gettime64();
}

bool timing_advance(uint32_t millis)
{
    uint64_t now = timer_ms_gettime64();
    uint64_t elapsed = now - previous_millis;

    previous_millis = now;
    if (elapsed > MAX_FRAME_MILLIS) {
        elapsed = MAX_FRAME_MILLIS;
    }
    simulation_time += (uint32_t)elapsed;
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
