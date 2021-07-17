#include <stdlib.h>

#include <nusys.h>
#include <nualstl_n.h>

#include "unf_debug.h"

#include "level.h"
#include "loop.h"
#include "segment.h"
#include "vec.h"

#define NUM_PI_MSGS 8

#define INITIAL_LEVEL 1
#define INITIAL_LIVES 3
#define INITIAL_SCORE 0

// Controller
NUContData contdata[1];
u8 contPattern;

// Audio
musHandle sndHandle = 0;

// Parallel interface
static OSMesg PiMessages[NUM_PI_MSGS];
static OSMesgQueue PiMessageQ;

void initAudio(void)
{
    nuAuStlInit();
    nuAuStlPtrBankInit(PBANK_SIZE);
    nuAuStlPtrBankSet((u8*)PBANK_START, PBANK_SIZE, (u8*)WBANK_START);
    nuAuStlSndPlayerDataSet((u8*)SFX_START, SFX_SIZE);
}

void initController(int controller_number)
{
    s32 ret = nuContRmbCheck(controller_number);
    if (ret) {
        debug_printf("[main] Rumble Pak: Fail (%d)\n", ret);
    } else {
        debug_printf("[main] Rumble Pak: OK\n");
        nuContRmbModeSet(controller_number, NU_CONT_RMB_MODE_ENABLE);
        nuContRmbStart(controller_number, 128, 10);
    }
}

void initUsb()
{
    osCreatePiManager((OSPri)OS_PRIORITY_PIMGR, &PiMessageQ, PiMessages, NUM_PI_MSGS);

    debug_initialize();
}

void mainproc(void)
{
    initUsb();
    nuGfxInit();

    debug_printf("[main] Starting...\n\n");

    contPattern = nuContInit();
    initController(0);
    initAudio();

    srand(contdata[0].stick_x);

    level_init(INITIAL_LEVEL, INITIAL_LIVES, INITIAL_SCORE);
    set_main_loop(level_loop);
    run_main_loop();
}
