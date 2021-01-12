#include <assert.h>
#include <math.h>
#include <string.h>

#include <nusys.h>

#include "canvas.h"
#include "gfx.h"
#include "shape.h"
#include "vec.h"

#define MAX_TRANSFORMS 30
#define MAX_SHAPES 64
#define MAX_VERTICES 100

#define RAD_TO_DEG (180.0 / M_PI)

extern struct vec_2d origin;

struct geometry {
    const Vtx *vertices;
    uint8_t num_vertices;

    const uint8_t *triangles;
    uint8_t num_triangles;
};

static Vtx vertices[MAX_VERTICES];
static struct geometry shapes[MAX_SHAPES];

static int num_shapes = 0;
static int num_vertices = 0;

static Dynamic transforms[MAX_TRANSFORMS];

static int num_transforms = 0;

void canvas_reset()
{
    num_shapes = 0;
    num_vertices = 0;
}

int canvas_load_shape(const struct shape *shape)
{
    if (num_shapes == MAX_SHAPES) {
        return CANVAS_INVALID_SHAPE;
    }

    if (num_vertices + shape->num_vertices >= MAX_VERTICES) {
        return CANVAS_INVALID_SHAPE;
    }

    for (int i = 0; i < shape->num_vertices; i++) {
        Vtx *vertex = &vertices[num_vertices + i];

        memset(vertex, 0, sizeof(Vtx));

        vertex->v.ob[0] = shape->vertices[i * 2];
        vertex->v.ob[1] = shape->vertices[i * 2 + 1];
        vertex->v.ob[2] = -5;

        vertex->v.cn[0] = 0xFF;
        vertex->v.cn[1] = 0xFF;
        vertex->v.cn[2] = 0xFF;
        vertex->v.cn[3] = 0xFF;
    }

    shapes[num_shapes].vertices = &vertices[num_vertices];
    shapes[num_shapes].num_vertices = shape->num_vertices;
    shapes[num_shapes].triangles = shape->triangles;
    shapes[num_shapes].num_triangles = shape->num_triangles;

    num_vertices += shape->num_vertices;

    return num_shapes++;
}

void canvas_start_drawing()
{
    // Specify the display list buffer
    glistp = gfx_glist;

    // The initialization of RCP
    gfxRCPInit();

    // Clear the frame buffer and the Z-buffer
    gfxClearCfb();

    num_transforms = 0;
}

bool canvas_draw_lines(int shape, struct vec_2d position, float rotation, struct vec_2d scale)
{
    // TODO

    return false;
}

bool canvas_draw_triangles(int shape, struct vec_2d position, float rotation, struct vec_2d scale)
{
    Dynamic *transform = &transforms[num_transforms++];

    guOrtho(&transform->projection,
        0.0f,       // left
        SCREEN_WD,  // right
        SCREEN_HT,  // bottom
        0.0f,       // top
        1.0f,       // near
        10.0f,      // far
        1.0f);      // scale

    guTranslate(&transform->modeling,
        (float)SCREEN_WD / 2.0f + position.x,
        (float)SCREEN_HT / 2.0f + position.y,
        0.0f);
    guRotate(&transform->rotation, rotation, 0.0f, 0.0f, 1.0f);
    // Global scaling factor
    // TODO: Could this be incorporated into projection matrix?
    guScale(&transform->scale, 1.0f, 1.2f, 1.f);

    // load projection matrix
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(transform->projection)),
        G_MTX_LOAD | G_MTX_NOPUSH | G_MTX_PROJECTION);

    // load and transform model-view matrix
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(transform->modeling)),
        G_MTX_LOAD | G_MTX_NOPUSH | G_MTX_MODELVIEW);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(transform->rotation)),
        G_MTX_MUL | G_MTX_NOPUSH | G_MTX_MODELVIEW);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&(transform->scale)),
        G_MTX_MUL | G_MTX_NOPUSH | G_MTX_MODELVIEW);

    // load vertices (I think this is limited to loading 32 vertices at once)
    gSPVertex(glistp++, shapes[shape].vertices, shapes[shape].num_vertices, 0);

    // Synchronizes RDP attribute updates by waiting for pixels to be processed
    // during the rendering of primitives
    gDPPipeSync(glistp++);

    // number of pixels per cycle
    gDPSetCycleType(glistp++, G_CYC_1CYCLE);

    gDPSetRenderMode(glistp++, G_RM_AA_OPA_SURF, G_RM_AA_OPA_SURF2);

    gSPClearGeometryMode(glistp++, 0xFFFFFFFF);
    gSPSetGeometryMode(glistp++, G_SHADE | G_SHADING_SMOOTH);

    const uint8_t *triangles = shapes[shape].triangles;
    for (int i = 0; i < shapes[shape].num_triangles * 3 - 2; i += 3) {
        gSP1Triangle(glistp++, triangles[i], triangles[i + 1], triangles[i + 2], 0);
    }

    return true;
}

void canvas_finish_drawing(bool swap)
{
    gDPFullSync(glistp++);
    gSPEndDisplayList(glistp++);

    assert(glistp - gfx_glist < GFX_GLIST_LEN);

    // Activate the RSP task
    nuGfxTaskStart(gfx_glist, (s32)(glistp - gfx_glist) * sizeof (Gfx),
        NU_GFX_UCODE_F3DEX2, swap ? NU_SC_SWAPBUFFER : NU_SC_NOSWAPBUFFER);
}
