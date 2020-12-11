#include <assert.h>
#include <math.h>
#include <nusys.h>

#include "gfx.h"

#define SHIP_ACCELERATION 100.45f

extern NUContData	contdata[1];
extern u8 contPattern;

static const float ratio = (float)SCREEN_HT / (float)SCREEN_WD;
static const float factor = 1.0f / 60.0f;
static const float degrees_to_radians = M_PI / 180.0f;

static float theta;

static float pos_x;
static float pos_y;

static float vel_x;
static float vel_y;

void shadetri(Dynamic* dynamicp);

/** The initialization of stage 00 */
void initStage00(void)
{
  theta = 0.0;

  pos_x = 0.0;
  pos_y = 0.0;

  vel_x = 0.0;
  vel_y = 0.0;
}

/**
 * Make the display list and activate the task
 */
void makeDL00(void)
{
  /* Specify the display list buffer */
  glistp = gfx_glist;

  /* The initialization of RCP */
  gfxRCPInit();

  /* TODO: does this need to be done on every display list? */
  gSPLoadUcodeL(glistp++, gspL3DEX2_fifo);

  /* Clear the frame buffer and the Z-buffer */
  gfxClearCfb();

  guOrtho(&gfx_dynamic.projection,
      0.0f,       // left
      SCREEN_WD,  // right
      SCREEN_HT,  // bottom
      0.0f,       // top
      1.0f,       // near
      10.0f,      // far
      1.0f);      // scale

  guTranslate(&gfx_dynamic.modeling, (float)SCREEN_WD / 2.0f + pos_x, (float)SCREEN_HT / 2.0f + pos_y, 0.0f);

  guRotate(&gfx_dynamic.rotation, theta, 0.0f, 0.0f, 1.0f);

  /* Global scaling factor
   *
   * TODO: Could this be incorporated into projection matrix?
   */
  guScale(&gfx_dynamic.scale, 1.0f, 1.2f, 1.f);

  /* Draw a square */
  shadetri(&gfx_dynamic);

  /* End the construction of the display list */
  gDPFullSync(glistp++);
  gSPEndDisplayList(glistp++);

  /* Check if all are put in the array */
  assert(glistp - gfx_glist < GFX_GLIST_LEN);

  /* Activate the RSP task. Switch display buffers at the end of the task. */
  nuGfxTaskStart(gfx_glist, (s32)(glistp - gfx_glist) * sizeof (Gfx),
      NU_GFX_UCODE_F3DEX, NU_SC_SWAPBUFFER);
}

/* The game progressing process for stage 0 */
void updateGame00(void)
{
  static float vel = 5.0;
  static float rot;

  /* Data reading of controller 1 */
  nuContDataGetEx(contdata, 0);

  if (contdata[0].stick_x < -0.1) {
    theta -= vel;
  } else if (contdata[0].stick_x > 0.1) {
    theta += vel;
  }

  if (theta > 360.0) {
    theta -= 360.0;
  } else if (theta < 0.0) {
    theta += 360.0;
  }

  rot = theta * degrees_to_radians;

  if (contdata[0].button & B_BUTTON) {
    vel_x += sinf(rot) * SHIP_ACCELERATION * factor;
    vel_y -= cosf(rot) * SHIP_ACCELERATION * factor;
  }

  pos_x += vel_x * factor;
  pos_y += vel_y * factor;
}

/**
 * Draw a square
 */
void shadetri(Dynamic* dynamicp)
{
  static Vtx shade_vtx[] =  {
    {  0.00, -6.40, -5.0, 0, 0, 0, 0xff, 0xff, 0xff, 0xff },
    { -5.12,  5.76, -5.0, 0, 0, 0, 0xff, 0xff, 0xff, 0xff },
    {  5.12,  5.76, -5.0, 0, 0, 0, 0xff, 0xff, 0xff, 0xff },
    { -4.48,  4.00, -5.0, 0, 0, 0, 0xff, 0xff, 0xff, 0xff },
    {  4.48,  4.00, -5.0, 0, 0, 0, 0xff, 0xff, 0xff, 0xff },
    {  0.00,  6.40, -5.0, 0, 0, 0, 0xff, 0xff, 0xff, 0xff }
  };

  /* load projection matrix */
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->projection)),
      G_MTX_LOAD | G_MTX_NOPUSH | G_MTX_PROJECTION);

  /* load model-view matrix */
  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->modeling)),
      G_MTX_LOAD | G_MTX_NOPUSH | G_MTX_MODELVIEW);

  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->rotation)),
      G_MTX_MUL | G_MTX_NOPUSH | G_MTX_MODELVIEW);

  gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(dynamicp->scale)),
      G_MTX_MUL | G_MTX_NOPUSH | G_MTX_MODELVIEW);

  /* load vertices (I think this is limited to loading 32 vertices at once) */
  gSPVertex(glistp++, &(shade_vtx[0]), 6, 0);

  /* Synchronizes RDP attribute updates by waiting for pixels to be processed during the rendering of primitives */
  gDPPipeSync(glistp++);
  /* number of pixels per cycle */
  gDPSetCycleType(glistp++, G_CYC_1CYCLE);
  /* support anti-aliased, translucent lines */
  gDPSetRenderMode(glistp++, G_RM_AA_XLU_LINE, G_RM_NOOP2);
  /* set geometry modes */
  gSPClearGeometryMode(glistp++, 0xFFFFFFFF);

  gSPLineW3D(glistp++, 0, 1, 1.5, 0);
  gSPLineW3D(glistp++, 0, 2, 1.5, 0);
  gSPLineW3D(glistp++, 3, 4, 1.5, 0);
}
