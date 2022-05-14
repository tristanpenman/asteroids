#include <stdlib.h>

#include <nusys.h>
#include <nualstl_n.h>

#include "debug.h"
#include "highscores.h"
#include "input.h"
#include "level.h"
#include "loop.h"
#include "rumble.h"
#include "sandbox.h"
#include "segment.h"
#include "storage.h"
#include "titlescreen.h"
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

char* titlescreen()
{
    titlescreen_init();
    set_main_loop(titlescreen_loop);

    return "ok";
}

void mainproc()
{
    nuGfxInit();

    debug_init();
    debug_addcommand("sandbox", "Switch to sandbox mode", sandbox);
    debug_addcommand("level", "Switch to level mode", level);
    debug_addcommand("titlescreen", "Switch to titlescreen", titlescreen);
    debug_printf("Starting...\n");

    debug_printf("input_init...\n");
    input_init();
   
    debug_printf("storage_init...\n");
    storage_init();

    debug_printf("highscores_load...\n");
    if (storage_available()) {
        if (!highscores_load()) {
            debug_printf("highscores_save...\n");
            highscores_save();
        }
    } else {
        debug_printf(" - storage not available\n");
    }

    debug_printf("rumble_init...\n");
    rumble_init();

    debug_printf("audio_init...\n");
    audio_init();

    titlescreen_init();
    set_main_loop(titlescreen_loop);

    run_main_loop();
}
