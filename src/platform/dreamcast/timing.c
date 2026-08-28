#include <kos.h>

#include "timing.h"

#define MAX_FRAME_MILLIS 100

static uint32_t simulation_time;
static uint64_t previous_millis;

void reset_simulation_time(void)
{
    simulation_time = 0;
    previous_millis = timer_ms_gettime64();
}

void produce_simulation_time(void)
{
    uint64_t now = timer_ms_gettime64();
    uint64_t elapsed = now - previous_millis;

    previous_millis = now;
    if (elapsed > MAX_FRAME_MILLIS) {
        elapsed = MAX_FRAME_MILLIS;
    }
    simulation_time += (uint32_t)elapsed;
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
