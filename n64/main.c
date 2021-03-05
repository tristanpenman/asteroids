#include <stdlib.h>

#include <nusys.h>

#include "level.h"
#include "loop.h"
#include "vec.h"

#define INITIAL_LEVEL 1
#define INITIAL_LIVES 3
#define INITIAL_SCORE 0

// Read data from one controller
NUContData contdata[1];

// Pattern connected to the controller
u8 contPattern;

void mainproc(void)
{
  nuGfxInit();

  contPattern = nuContInit();

  srand(contdata[0].stick_x);

  srand(500);

  level_init(INITIAL_LEVEL, INITIAL_LIVES, INITIAL_SCORE);
  set_main_loop(level_loop);
  run_main_loop();
}
