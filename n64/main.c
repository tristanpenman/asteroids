#include <stdlib.h>

#include <nusys.h>
#include <nualstl_n.h>

#include "level.h"
#include "loop.h"
#include "segment.h"
#include "vec.h"

#define INITIAL_LEVEL 1
#define INITIAL_LIVES 3
#define INITIAL_SCORE 0

// Read data from one controller
NUContData contdata[1];

// Pattern connected to the controller
u8 contPattern;

musHandle sndHandle = 0;

void initAudio(void)
{
    nuAuStlInit();
    nuAuStlPtrBankInit(PBANK_SIZE);
    nuAuStlPtrBankSet((u8*)PBANK_START, PBANK_SIZE, (u8*)WBANK_START);
    nuAuStlSndPlayerDataSet((u8*)SFX_START, SFX_SIZE);
}

void mainproc(void)
{
    contPattern = nuContInit();

    initAudio();

    nuGfxInit();

    srand(contdata[0].stick_x);

    srand(500);

    level_init(INITIAL_LEVEL, INITIAL_LIVES, INITIAL_SCORE);
    set_main_loop(level_loop);
    run_main_loop();
}
