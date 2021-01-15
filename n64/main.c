#include <nusys.h>

#include "loop.h"
#include "stage00.h"

// Read data from one controller
NUContData contdata[1];

// Pattern connected to the controller
u8 contPattern;

void mainproc(void)
{
  nuGfxInit();

  contPattern = nuContInit();

  stage00_init();
  set_main_loop(stage00_loop);
  run_main_loop();
}
