#ifndef ASTEROIDS_PLATFORM_N64_GFX_H
#define ASTEROIDS_PLATFORM_N64_GFX_H

#include <nusys.h>

#ifdef LOW_RESOLUTION
#define SCREEN_WD 320
#define SCREEN_HT 240
#else
#define SCREEN_WD 640
#define SCREEN_HT 480
#endif

#define SCREEN_RATIO ((float)SCREEN_HT / (float)SCREEN_WD)

#define GFX_GLIST_LEN 2048
#define GFX_GLIST_COUNT 15

struct gfx_transform {
    Mtx modeling;
    Mtx rotation;
    Mtx scale;
};

extern Gfx gfx_glist[GFX_GLIST_COUNT][GFX_GLIST_LEN];
extern Gfx *glistp;

void gfx_init(void);
void gfx_rcp_init(void);
void gfx_clear_cfb(void);

#endif
