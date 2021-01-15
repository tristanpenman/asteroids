#ifndef _GFX_H
#define _GFX_H

#include <nusys.h>

#define SCREEN_HT 240
#define SCREEN_WD 320

// The maximum length of the display list of one task
#define GFX_GLIST_LEN 2048

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
extern Gfx gfx_glist[GFX_GLIST_LEN];

extern void gfx_rcp_init(void);
extern void gfx_clear_cfb(void);

#endif
