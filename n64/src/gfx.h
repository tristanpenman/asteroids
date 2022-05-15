#ifndef __GFX_H
#define __GFX_H

#include <nusys.h>

#define SCREEN_HT 240
#define SCREEN_WD 320

#define SCREEN_RATIO ((float)SCREEN_HT / (float)SCREEN_WD)

// The maximum length of the display list of one task
#define GFX_GLIST_LEN 2048

// Number of gfx lists
#define GLX_GLIST_COUNT 15

// The projection-matrix structure
typedef struct {
    Mtx projection;

    // Modeling matrices
    Mtx modeling;
    Mtx rotation;
    Mtx scale;
} Dynamic;

extern Dynamic gfx_dynamic;
extern Gfx* glistp;
extern Gfx gfx_glist[GLX_GLIST_COUNT][GFX_GLIST_LEN];

void gfx_rcp_init(void);
void gfx_clear_cfb(void);

#endif
