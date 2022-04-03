#include <stdlib.h>

#include <nusys.h>
#include <nualstl_n.h>

#include "debug.h"
#include "input.h"
#include "level.h"
#include "loop.h"
#include "rumble.h"
#include "sandbox.h"
#include "segment.h"
#include "vec.h"

#define NUM_PI_MSGS 8

#define INITIAL_LEVEL 1
#define INITIAL_LIVES 3
#define INITIAL_SCORE 0

void audio_init()
{
    nuAuStlInit();
    nuAuStlPtrBankInit(PBANK_SIZE);
    nuAuStlPtrBankSet((u8*)PBANK_START, PBANK_SIZE, (u8*)WBANK_START);
    nuAuStlSndPlayerDataSet((u8*)SFX_START, SFX_SIZE);
}

char* sandbox()
{
    sandbox_init();
    set_main_loop(sandbox_loop);

    return "ok";
}

char* level()
{
    level_init(INITIAL_LEVEL, INITIAL_LIVES, INITIAL_SCORE);
    set_main_loop(level_loop);

    return "ok";
}

void mainproc()
{
    nuGfxInit();

    debug_init();
    debug_addcommand("sandbox", "Switch to sandbox mode", sandbox);
    debug_addcommand("level", "Switch to level mode", level);
    debug_printf("Starting...\n");

    input_init();
    rumble_init();
    audio_init();

    level_init(INITIAL_LEVEL, INITIAL_LIVES, INITIAL_SCORE);
    set_main_loop(level_loop);
    run_main_loop();
}
