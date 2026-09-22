#include <SDL.h>

#include "timing.h"

static uint32_t simulation_time;
static uint32_t ticks;

#define MAX_FRAME_MILLIS 16u

void timing_reset(void)
{
    ticks = SDL_GetTicks();
    simulation_time = 0;
}

bool timing_advance(uint32_t millis)
{
    const uint32_t now = SDL_GetTicks();

    if (now < ticks) {
        simulation_time += UINT32_MAX - ticks;
        ticks = 0;
    }

    uint32_t elapsed = now - ticks;
    if (elapsed > MAX_FRAME_MILLIS) {
        elapsed = MAX_FRAME_MILLIS;
    }
    simulation_time += elapsed;
    ticks = now;
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
