#include <nusys.h>

void stage00(int);
void makeDL00(void);

/** Read data from one controller */
NUContData contdata[1];

/** Pattern connected to the controller */
u8 contPattern;

/**
 * Main entry point
 */
void mainproc(void)
{
  nuGfxInit();

  /* The initialization of the controller manager  */
  contPattern = nuContInit();

  initStage00();

  nuGfxFuncSet((NUGfxFunc)stage00);

  nuGfxDisplayOn();

  while (1);
}

/**
 * Stage 00 call-back function
 *
 * \param pendingGfx passed from Nusystem; the total number of RCP tasks that
 *                   are currently processing or waiting for the process.
 */
void stage00(int pendingGfx)
{
  /* Provide a display list task if there are no RCP tasks in progress */
  if (pendingGfx < 1) {
    makeDL00();
  }

  /* The process of game progress */
  updateGame00();
}
