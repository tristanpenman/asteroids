#include <stdlib.h>

#include <nusys.h>

#include "loop.h"
#include "stage00.h"
#include "vec.h"

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

  stage00_init();
  set_main_loop(stage00_loop);
  run_main_loop();
}
