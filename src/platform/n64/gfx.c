#include "gfx.h"

#include <string.h>
#include <malloc.h>

#ifndef GFX_FRAMEBUFFER_COUNT
#define GFX_FRAMEBUFFER_COUNT 3
#endif

Gfx gfx_glist[GFX_GLIST_COUNT][GFX_GLIST_LEN];
Gfx *glistp;
static struct gfx_frame frames[GFX_FRAME_COUNT] __attribute__((aligned(16)));
static struct gfx_frame *current_frame;
static int current_frame_index;

enum graphics_phase
{
    GRAPHICS_PHASE_IDLE,
    GRAPHICS_PHASE_FRAME,
    GRAPHICS_PHASE_VIEW3D,
    GRAPHICS_PHASE_CANVAS
};

static enum graphics_phase phase;
static struct graphics_color frame_clear_color;
static unsigned char graphics_heap[512 * 1024] __attribute__((aligned(16)));

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

    InitHeap(graphics_heap, sizeof(graphics_heap));
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

void gfx_set_cfb(void)
{
    gDPSetColorImage(glistp++, G_IM_FMT_RGBA, G_IM_SIZ_16b, SCREEN_WD,
        osVirtualToPhysical(nuGfxCfb_ptr));
}

void gfx_clear_cfb(void)
{
    const struct graphics_color black = {0, 0, 0, 255};
    gfx_clear_cfb_color(black);
}

void gfx_clear_cfb_color(struct graphics_color color)
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
    {
        const u16 packed = GPACK_RGBA5551(color.red, color.green, color.blue,
            color.alpha != 0);
        gDPSetFillColor(glistp++, (packed << 16) | packed);
    }
    gDPFillRectangle(glistp++, 0, 0, SCREEN_WD - 1, SCREEN_HT - 1);
    gDPPipeSync(glistp++);
    gDPSetCycleType(glistp++, G_CYC_1CYCLE);
}

static void release_frame(struct gfx_frame *frame)
{
    if (frame->num_shape_refs != 0) {
        canvas_n64_release_shapes(frame->shape_refs, frame->num_shape_refs);
        frame->num_shape_refs = 0;
    }
    frame->busy = false;
}

void graphics_notify_tasks_completed(void)
{
    int i;
    for (i = 0; i < GFX_FRAME_COUNT; ++i) {
        release_frame(&frames[i]);
    }
}

bool graphics_begin_frame(struct graphics_color clear_color)
{
    if (phase != GRAPHICS_PHASE_IDLE) {
        return false;
    }
    current_frame = &frames[current_frame_index];
    if (current_frame->busy) {
        current_frame = NULL;
        return false;
    }
    release_frame(current_frame);
    current_frame->triangle_ptr = current_frame->triangles;
    current_frame->line_ptr = current_frame->lines;
    current_frame->num_line_transforms = 0;
    current_frame->num_line_projections = 0;
    current_frame->num_dynamic_vertices = 0;
    current_frame->num_views = 0;
    current_frame->triangles_used = false;
    current_frame->lines_used = false;
    current_frame->clear_emitted = false;
    frame_clear_color = clear_color;
    phase = GRAPHICS_PHASE_FRAME;
    return true;
}

static void begin_triangle_task(void)
{
    if (current_frame->triangles_used) {
        glistp = current_frame->triangle_ptr;
        return;
    }
    glistp = current_frame->triangles;
    gfx_rcp_init();
    gfx_set_cfb();
    gfx_clear_cfb_color(frame_clear_color);
    current_frame->clear_emitted = true;
    current_frame->triangles_used = true;
}

bool graphics_begin_view3d(const struct graphics_rect *rect,
    const struct graphics_camera *camera)
{
    Mtx *projection;
    Mtx *modelview;
    Vp *viewport;
    u16 normalization;
    int index;

    if (phase != GRAPHICS_PHASE_FRAME || current_frame->lines_used ||
        rect == NULL || camera == NULL ||
        rect->width <= 0 || rect->height <= 0 || camera->near_plane <= 0.0f ||
        camera->far_plane <= camera->near_plane ||
        current_frame->num_views >= GFX_MAX_VIEW3D) {
        return false;
    }
    begin_triangle_task();
    index = current_frame->num_views++;
    projection = &current_frame->view_projections[index];
    modelview = &current_frame->view_modelviews[index];
    viewport = &current_frame->viewports[index];
    viewport->vp.vscale[0] = rect->width * 2;
    viewport->vp.vscale[1] = rect->height * 2;
    viewport->vp.vscale[2] = G_MAXZ / 2;
    viewport->vp.vscale[3] = 0;
    viewport->vp.vtrans[0] = (rect->x * 4) + rect->width * 2;
    viewport->vp.vtrans[1] = (rect->y * 4) + rect->height * 2;
    viewport->vp.vtrans[2] = G_MAXZ / 2;
    viewport->vp.vtrans[3] = 0;
    gSPViewport(glistp++, OS_K0_TO_PHYSICAL(viewport));
    gDPSetScissor(glistp++, G_SC_NON_INTERLACE, rect->x, rect->y,
        rect->x + rect->width, rect->y + rect->height);
    guPerspective(projection, &normalization, camera->vertical_fov,
        (float)rect->width / (float)rect->height, camera->near_plane,
        camera->far_plane, 1.0f);
    guLookAt(modelview, camera->position.x, camera->position.y,
        camera->position.z, camera->target.x, camera->target.y,
        camera->target.z, camera->up.x, camera->up.y, camera->up.z);
    gSPPerspNormalize(glistp++, normalization);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(projection),
        G_MTX_PROJECTION | G_MTX_LOAD | G_MTX_NOPUSH);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(modelview),
        G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    current_frame->triangle_ptr = glistp;
    phase = GRAPHICS_PHASE_VIEW3D;
    return true;
}

void graphics_end_view3d(void)
{
    if (phase == GRAPHICS_PHASE_VIEW3D) {
        current_frame->triangle_ptr = glistp;
        phase = GRAPHICS_PHASE_FRAME;
    }
}

bool graphics_canvas_begin_phase(void)
{
    if (phase != GRAPHICS_PHASE_FRAME || current_frame->lines_used) {
        return false;
    }
    glistp = current_frame->lines;
    gfx_rcp_init();
    gfx_set_cfb();
    if (!current_frame->clear_emitted) {
        gfx_clear_cfb_color(frame_clear_color);
        current_frame->clear_emitted = true;
    }
    current_frame->lines_used = true;
    phase = GRAPHICS_PHASE_CANVAS;
    return true;
}

void graphics_canvas_end_phase(void)
{
    if (phase == GRAPHICS_PHASE_CANVAS) {
        current_frame->line_ptr = glistp;
        phase = GRAPHICS_PHASE_FRAME;
    }
}

bool graphics_canvas_phase_active(void)
{
    return phase == GRAPHICS_PHASE_CANVAS;
}

bool graphics_end_frame(void)
{
    size_t size;
    if (phase != GRAPHICS_PHASE_FRAME) {
        return false;
    }
    if (!current_frame->triangles_used && !current_frame->lines_used) {
        begin_triangle_task();
    }
    if (current_frame->triangles_used) {
        glistp = current_frame->triangle_ptr;
        gDPFullSync(glistp++);
        gSPEndDisplayList(glistp++);
        current_frame->triangle_ptr = glistp;
        size = (size_t)(glistp - current_frame->triangles) * sizeof(Gfx);
        nuGfxTaskStart(current_frame->triangles, (s32)size,
            NU_GFX_UCODE_F3DEX,
            current_frame->lines_used ? NU_SC_NOSWAPBUFFER : NU_SC_SWAPBUFFER);
    }
    if (current_frame->lines_used) {
        glistp = current_frame->line_ptr;
        gDPFullSync(glistp++);
        gSPEndDisplayList(glistp++);
        current_frame->line_ptr = glistp;
        size = (size_t)(glistp - current_frame->lines) * sizeof(Gfx);
        nuGfxTaskStart(current_frame->lines, (s32)size,
            NU_GFX_UCODE_L3DEX2, NU_SC_SWAPBUFFER);
    }
    current_frame->busy = true;
    current_frame_index = (current_frame_index + 1) % GFX_FRAME_COUNT;
    current_frame = NULL;
    phase = GRAPHICS_PHASE_IDLE;
    return true;
}

struct gfx_frame *gfx_active_frame(void)
{
    return current_frame;
}

bool gfx_line_commands_available(size_t count)
{
    return current_frame != NULL && current_frame->lines_used &&
        glistp + count + 2 < current_frame->lines + GFX_LINE_GLIST_LEN;
}

struct gfx_transform *gfx_alloc_line_transform(void)
{
    if (current_frame == NULL ||
        current_frame->num_line_transforms >= CANVAS_MAX_TRANSFORMS) {
        return NULL;
    }
    return &current_frame->line_transforms[current_frame->num_line_transforms++];
}

Mtx *gfx_alloc_line_projection(void)
{
    if (current_frame == NULL ||
        current_frame->num_line_projections >= GFX_MAX_PROJECTIONS) {
        return NULL;
    }
    return &current_frame->line_projections[current_frame->num_line_projections++];
}

Vtx *gfx_alloc_dynamic_vertices(size_t count)
{
    Vtx *result;
    if (current_frame == NULL || count > GFX_MAX_DYNAMIC_VERTICES ||
        current_frame->num_dynamic_vertices + (int)count >
            GFX_MAX_DYNAMIC_VERTICES) {
        return NULL;
    }
    result = &current_frame->dynamic_vertices[current_frame->num_dynamic_vertices];
    current_frame->num_dynamic_vertices += (int)count;
    return result;
}

int gfx_reference_shape(canvas_shape_id shape)
{
    int i;
    if (current_frame == NULL) {
        return -1;
    }
    for (i = 0; i < current_frame->num_shape_refs; ++i) {
        if (current_frame->shape_refs[i] == shape) {
            return 0;
        }
    }
    if (current_frame->num_shape_refs >= CANVAS_MAX_SHAPES) {
        return -1;
    }
    current_frame->shape_refs[current_frame->num_shape_refs++] = shape;
    return 1;
}

void gfx_apply_viewport(const struct canvas_rect *rect)
{
    Vp *viewport = &current_frame->viewports[GFX_MAX_VIEW3D];
    viewport->vp.vscale[0] = rect->width * 2;
    viewport->vp.vscale[1] = rect->height * 2;
    viewport->vp.vscale[2] = G_MAXZ / 2;
    viewport->vp.vscale[3] = 0;
    viewport->vp.vtrans[0] = (rect->x * 4) + rect->width * 2;
    viewport->vp.vtrans[1] = (rect->y * 4) + rect->height * 2;
    viewport->vp.vtrans[2] = G_MAXZ / 2;
    viewport->vp.vtrans[3] = 0;
    gSPViewport(glistp++, OS_K0_TO_PHYSICAL(viewport));
    gDPSetScissor(glistp++, G_SC_NON_INTERLACE, rect->x, rect->y,
        rect->x + rect->width, rect->y + rect->height);
}
