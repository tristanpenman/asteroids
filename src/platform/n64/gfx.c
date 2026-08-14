#include "gfx.h"

#ifndef GFX_FRAMEBUFFER_COUNT
#define GFX_FRAMEBUFFER_COUNT 3
#endif

Gfx gfx_glist[GFX_GLIST_COUNT][GFX_GLIST_LEN];
Gfx *glistp;

static u16 framebuffers[GFX_FRAMEBUFFER_COUNT][SCREEN_HT * SCREEN_WD]
    __attribute__((aligned(64)));
static u16 *framebuffer_ptrs[GFX_FRAMEBUFFER_COUNT];
static u16 depth_buffer[SCREEN_HT * SCREEN_WD] __attribute__((aligned(64)));
static Vp viewport;

static Gfx setup_rdp_state[] = {
    gsDPSetRenderMode(G_RM_OPA_SURF, G_RM_OPA_SURF2),
    gsDPSetCombineMode(G_CC_SHADE, G_CC_SHADE),
#ifdef LOW_RESOLUTION
    gsDPSetColorDither(G_CD_BAYER),
#else
    gsDPSetColorDither(G_CD_DISABLE),
    gsDPSetAlphaDither(G_AD_DISABLE),
#endif
    gsSPEndDisplayList()
};

static Gfx setup_rsp_state[] = {
    gsSPViewport(&viewport),
    gsSPClearGeometryMode(0xFFFFFFFF),
    gsSPSetGeometryMode(G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH | G_CULL_BACK),
    gsSPTexture(0, 0, 0, 0, G_OFF),
    gsSPEndDisplayList()
};

static void gfx_update_viewport(void)
{
    viewport.vp.vscale[0] = SCREEN_WD * 2;
    viewport.vp.vscale[1] = SCREEN_HT * 2;
    viewport.vp.vscale[2] = G_MAXZ / 2;
    viewport.vp.vscale[3] = 0;

    viewport.vp.vtrans[0] = SCREEN_WD * 2;
    viewport.vp.vtrans[1] = SCREEN_HT * 2;
    viewport.vp.vtrans[2] = G_MAXZ / 2;
    viewport.vp.vtrans[3] = 0;
}

void gfx_init(void)
{
    int i;

    nuGfxInit();

    for (i = 0; i < GFX_FRAMEBUFFER_COUNT; ++i) {
        framebuffer_ptrs[i] = framebuffers[i];
    }

    nuGfxSetCfb(framebuffer_ptrs, GFX_FRAMEBUFFER_COUNT);
    nuGfxSetZBuffer(depth_buffer);

#ifndef LOW_RESOLUTION
    osViSetMode(&osViModeNtscHpf1);
    osViSetSpecialFeatures(
        OS_VI_GAMMA_OFF | OS_VI_GAMMA_DITHER_OFF | OS_VI_DITHER_FILTER_ON);
#endif

    gfx_update_viewport();
}

void gfx_rcp_init(void)
{
    gfx_update_viewport();

    gSPSegment(glistp++, 0, 0x0);
    gSPDisplayList(glistp++, OS_K0_TO_PHYSICAL(setup_rsp_state));
    gSPDisplayList(glistp++, OS_K0_TO_PHYSICAL(setup_rdp_state));
    gDPSetScissor(glistp++, G_SC_NON_INTERLACE, 0, 0, SCREEN_WD, SCREEN_HT);
}

void gfx_clear_cfb(void)
{
    gDPSetDepthImage(glistp++, OS_K0_TO_PHYSICAL(nuGfxZBuffer));
    gDPSetCycleType(glistp++, G_CYC_FILL);
    gDPSetColorImage(glistp++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WD,
        OS_K0_TO_PHYSICAL(nuGfxZBuffer));
    gDPSetFillColor(glistp++,
        (GPACK_ZDZ(G_MAXFBZ, 0) << 16) | GPACK_ZDZ(G_MAXFBZ, 0));
    gDPFillRectangle(glistp++, 0, 0, SCREEN_WD - 1, SCREEN_HT - 1);
    gDPPipeSync(glistp++);

    gDPSetColorImage(glistp++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WD,
        osVirtualToPhysical(nuGfxCfb_ptr));
    gDPSetFillColor(glistp++,
        (GPACK_RGBA5551(0, 0, 0, 1) << 16) | GPACK_RGBA5551(0, 0, 0, 1));
    gDPFillRectangle(glistp++, 0, 0, SCREEN_WD - 1, SCREEN_HT - 1);
    gDPPipeSync(glistp++);
}
