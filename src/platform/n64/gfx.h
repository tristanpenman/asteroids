#ifndef ASTEROIDS_PLATFORM_N64_GFX_H
#define ASTEROIDS_PLATFORM_N64_GFX_H

#include <nusys.h>

#include "canvas.h"
#include "graphics.h"

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
#define GFX_FRAME_COUNT 3
#define GFX_LINE_GLIST_LEN 16384
#define GFX_MAX_PROJECTIONS 16
#define GFX_MAX_VIEW3D 4
#define GFX_MAX_DYNAMIC_VERTICES 2048

struct gfx_transform {
    Mtx modeling;
    Mtx rotation;
    Mtx scale;
};

struct gfx_frame {
    Gfx triangles[GFX_GLIST_LEN];
    Gfx lines[GFX_LINE_GLIST_LEN];
    struct gfx_transform line_transforms[CANVAS_MAX_TRANSFORMS];
    Mtx line_projections[GFX_MAX_PROJECTIONS];
    Vtx dynamic_vertices[GFX_MAX_DYNAMIC_VERTICES];
    Mtx view_projections[GFX_MAX_VIEW3D];
    Mtx view_modelviews[GFX_MAX_VIEW3D];
    Vp viewports[GFX_MAX_VIEW3D + 1];
    canvas_shape_id shape_refs[CANVAS_MAX_SHAPES];
    Gfx *triangle_ptr;
    Gfx *line_ptr;
    int num_line_transforms;
    int num_line_projections;
    int num_dynamic_vertices;
    int num_views;
    int num_shape_refs;
    bool triangles_used;
    bool lines_used;
    bool clear_emitted;
    bool busy;
};

extern Gfx gfx_glist[GFX_GLIST_COUNT][GFX_GLIST_LEN];
extern Gfx *glistp;

void gfx_init(void);
void gfx_rcp_init(void);
void gfx_clear_cfb(void);
void gfx_clear_cfb_color(struct graphics_color color);
void gfx_set_cfb(void);

struct gfx_frame *gfx_active_frame(void);
bool gfx_line_commands_available(size_t count);
struct gfx_transform *gfx_alloc_line_transform(void);
Mtx *gfx_alloc_line_projection(void);
Vtx *gfx_alloc_dynamic_vertices(size_t count);
int gfx_reference_shape(canvas_shape_id shape);
void gfx_apply_viewport(const struct canvas_rect *viewport);

void canvas_n64_release_shapes(const canvas_shape_id *shapes, int count);

#endif
