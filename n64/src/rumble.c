#include <nusys.h>

#include "debug.h"
#include "rumble.h"

void rumble_init()
{
    s32 ret = nuContRmbCheck(0);
    if (ret) {
        debug_printf("Rumble Pak detected: no (%d)\n", ret);
        return;
    }

    debug_printf("Rumble Pak detected: yes\n");

    nuContRmbModeSet(0, NU_CONT_RMB_MODE_ENABLE);
    nuContRmbStart(0, 128, 10);
}

void rumble_start(float time, float intensity)
{
    const u16 frequency = 256.0f * intensity;
    const u16 frame = 60.0f * time;

    nuContRmbModeSet(0, NU_CONT_RMB_MODE_ENABLE);
    nuContRmbStart(0, frequency, frame);
}
