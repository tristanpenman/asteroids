#include <nusys.h>

void stage00(int);
void makeDL00(void);

// Read data from one controller
NUContData contdata[1];

// Pattern connected to the controller
u8 contPattern;

void initStage00();
void updateGame00();

void mainproc(void)
{
  nuGfxInit();

  contPattern = nuContInit();

  initStage00();

  nuGfxFuncSet((NUGfxFunc)stage00);

  nuGfxDisplayOn();

  while (1);
}

void stage00(int pendingGfx)
{
  // Provide a display list task if there are no RCP tasks in progress
  if (pendingGfx < 1) {
    makeDL00();
  }

  updateGame00();
}
