#include <nusys.h>

#include "debug.h"
#include "rumble.h"

static bool available = false;

void rumble_init()
{
    s32 ret = nuContRmbCheck(0);
    if (ret) {
        debug_printf(" - nuContRmbCheck says no rumble pak inserted\n");
        return;
    }

    debug_printf(" - nuContRmbCheck says rumble pak inserted\n");

    nuContRmbModeSet(0, NU_CONT_RMB_MODE_ENABLE);
    nuContRmbStart(0, 128, 10);

    available = true;
}

bool rumble_available()
{
    return available;
}

void rumble_start(float time, float intensity)
{
    const u16 frequency = 256.0f * intensity;
    const u16 frame = 60.0f * time;

    nuContRmbModeSet(0, NU_CONT_RMB_MODE_ENABLE);
    nuContRmbStart(0, frequency, frame);
}
