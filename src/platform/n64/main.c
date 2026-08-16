#include "gfx.h"
#include "input.h"
#include "logo.h"
#include "loop.h"
#include "storage.h"
#include "timing.h"
#include "titlescreen.h"

static Mtx projection;
static Mtx modelview;
static Mtx rotation;
static float logo_rotation;
static int gfx_glist_index;

static void game_init(void)
{
    reset_simulation_time();
    titlescreen_init();
    set_main_loop(titlescreen_loop);
    run_main_loop();
}

static void draw_logo(void)
{
    gDPSetCycleType(glistp++, G_CYC_1CYCLE);
    gDPSetRenderMode(glistp++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
    gDPSetCombineMode(glistp++, G_CC_SHADE, G_CC_SHADE);
    gSPClearGeometryMode(glistp++, 0xFFFFFFFF);
    gSPSetGeometryMode(glistp++, G_SHADE | G_SHADING_SMOOTH | G_ZBUFFER);

    gSPDisplayList(glistp++, N64Yellow_PolyList);
    gSPDisplayList(glistp++, N64Red_PolyList);
    gSPDisplayList(glistp++, N64Blue_PolyList);
    gSPDisplayList(glistp++, N64Green_PolyList);
}

static void render_logo(int pending_gfx)
{
    NUContData controller_data;
    u16 perspective_normalization;

    nuContDataGetEx(&controller_data, 0);
    if (controller_data.button & (A_BUTTON | START_BUTTON)) {
        game_init();
        return;
    }

    if (pending_gfx >= 1) {
        return;
    }

    glistp = gfx_glist[gfx_glist_index];
    gfx_rcp_init();
    gfx_clear_cfb();

    guPerspective(&projection, &perspective_normalization, 45.0f,
        (float)SCREEN_WD / (float)SCREEN_HT, 10.0f, 1000.0f, 1.0f);
    guLookAt(&modelview,
        0.0f, 0.0f, 260.0f, // camera position
        0.0f, 0.0f, 0.0f,   // look at position
        0.0f, 1.0f, 0.0f);  // up vector
    guRotate(&rotation, logo_rotation, 0.0f, 1.0f, 0.0f);

    gSPPerspNormalize(glistp++, perspective_normalization);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&projection),
        G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&modelview),
        G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&rotation),
        G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

    draw_logo();

    gDPFullSync(glistp++);
    gSPEndDisplayList(glistp++);
    nuGfxTaskStart(gfx_glist[gfx_glist_index],
        (s32)(glistp - gfx_glist[gfx_glist_index]) * sizeof(Gfx),
        NU_GFX_UCODE_F3DEX, NU_SC_SWAPBUFFER);

    gfx_glist_index = (gfx_glist_index + 1) % GFX_GLIST_COUNT;
    logo_rotation += 1.0f;
    if (logo_rotation >= 360.0f) {
        logo_rotation -= 360.0f;
    }
}

void mainproc(void)
{
    gfx_init();
    input_init();
    reset_simulation_time();
    storage_available();

    nuGfxFuncSet((NUGfxFunc)render_logo);
    nuGfxDisplayOn();

    for (;;) {
        /* NuSystem drives rendering through render_logo. */
    }
}
