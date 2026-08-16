#include <stddef.h>
#include <string.h>

#include "canvas.h"
#include "defines.h"
#include "gfx.h"
#include "mathdefs.h"
#include "shape.h"
#include "text.h"
#include "vec.h"

#define MAX_SHAPES 64
#define MAX_TRANSFORMS 128
#define MAX_VERTICES 256

struct geometry
{
    const Vtx *vertices;
    uint8_t num_vertices;
    const uint16_t *line_segments;
    uint8_t num_line_segments;
};

struct colour
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

const struct vec_2d origin = {
    0.5f,
    SCREEN_RATIO / 2.0f
};

static Vtx vertices[MAX_VERTICES];
static struct geometry shapes[MAX_SHAPES];
static struct gfx_transform transforms[MAX_TRANSFORMS];
static struct colour primitive_colour;
static int num_shapes;
static int num_vertices;
static int num_transforms;
static int current_gfx_list;
static Mtx frame_projection;

void canvas_reset(void)
{
    num_shapes = 0;
    num_vertices = 0;
    text_reset();
    primitive_colour.r = 255;
    primitive_colour.g = 255;
    primitive_colour.b = 255;
    current_gfx_list = 0;
}

int canvas_load_shape(const struct shape *shape)
{
    int i;

    if (num_shapes >= MAX_SHAPES ||
        num_vertices + shape->num_vertices > MAX_VERTICES) {
        return CANVAS_INVALID_SHAPE;
    }

    for (i = 0; i < shape->num_vertices; ++i) {
        Vtx *vertex = &vertices[num_vertices + i];

        memset(vertex, 0, sizeof(*vertex));
        vertex->v.ob[0] = (s16)(shape->vertices[i * 2] * SCREEN_WD);
        vertex->v.ob[1] = (s16)(shape->vertices[i * 2 + 1] * SCREEN_WD);
        vertex->v.ob[2] = -5;
        vertex->v.cn[0] = 0xff;
        vertex->v.cn[1] = 0xff;
        vertex->v.cn[2] = 0xff;
        vertex->v.cn[3] = 0xff;
    }

    shapes[num_shapes].vertices = &vertices[num_vertices];
    shapes[num_shapes].num_vertices = shape->num_vertices;
    shapes[num_shapes].line_segments = shape->line_segments;
    shapes[num_shapes].num_line_segments = shape->num_line_segments;
    num_vertices += shape->num_vertices;

    return num_shapes++;
}

static void canvas_prepare_drawing(void)
{
    gDPPipeSync(glistp++);
    gDPSetCycleType(glistp++, G_CYC_1CYCLE);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&frame_projection),
        G_MTX_LOAD | G_MTX_NOPUSH | G_MTX_PROJECTION);
}

void canvas_start_drawing(bool clear)
{
    num_transforms = 0;

    guOrtho(&frame_projection,
        -SCREEN_WD / 2.0f, SCREEN_WD / 2.0f,
        SCREEN_HT / 2.0f, -SCREEN_HT / 2.0f,
        1.0f, 10.0f, 1.0f);

    glistp = gfx_glist[current_gfx_list];
    gfx_rcp_init();
    if (clear) {
        gfx_clear_cfb();
    }
    canvas_prepare_drawing();
}

void canvas_continue_drawing(void)
{
    glistp = gfx_glist[current_gfx_list];
    gfx_rcp_init();
    canvas_prepare_drawing();
}

void canvas_set_colour(float r, float g, float b)
{
    primitive_colour.r = (uint8_t)r;
    primitive_colour.g = (uint8_t)g;
    primitive_colour.b = (uint8_t)b;
}

bool canvas_draw_shape(int shape, struct vec_2d position, float rotation,
    struct vec_2d scale)
{
    struct gfx_transform *transform;
    int i;

    if (shape < 0 || shape >= num_shapes || num_transforms >= MAX_TRANSFORMS) {
        return false;
    }

    transform = &transforms[num_transforms++];
    guTranslate(&transform->modeling,
        position.x * SCREEN_WD, position.y * SCREEN_WD, 0.0f);
    guRotate(&transform->rotation, rotation * RAD_TO_DEG, 0.0f, 0.0f, 1.0f);
    guScale(&transform->scale, scale.x, scale.y, 1.0f);

    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&transform->modeling),
        G_MTX_LOAD | G_MTX_NOPUSH | G_MTX_MODELVIEW);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&transform->rotation),
        G_MTX_MUL | G_MTX_NOPUSH | G_MTX_MODELVIEW);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&transform->scale),
        G_MTX_MUL | G_MTX_NOPUSH | G_MTX_MODELVIEW);
    gSPVertex(glistp++, shapes[shape].vertices, shapes[shape].num_vertices, 0);
    gDPSetRenderMode(glistp++, G_RM_AA_XLU_LINE, G_RM_AA_XLU_LINE2);
    gDPSetCombineMode(glistp++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
    gDPSetPrimColor(glistp++, 0, 0, primitive_colour.r,
        primitive_colour.g, primitive_colour.b, 255);
    gSPClearGeometryMode(glistp++, 0xffffffff);
    gSPSetGeometryMode(glistp++, G_SHADE | G_SHADING_SMOOTH);

    if (shapes[shape].line_segments != NULL) {
        for (i = 0; i < shapes[shape].num_line_segments * 2; i += 2) {
            gSPLineW3D(glistp++, shapes[shape].line_segments[i],
                shapes[shape].line_segments[i + 1], 0.5, 0);
        }
    } else {
        for (i = 0; i < shapes[shape].num_vertices - 1; ++i) {
            gSPLineW3D(glistp++, i, i + 1, 0.5, 0);
        }
    }

    return true;
}

void canvas_finish_drawing(bool swap)
{
    size_t glist_size;

    gDPFullSync(glistp++);
    gSPEndDisplayList(glistp++);
    glist_size = (size_t)(glistp - gfx_glist[current_gfx_list]);
    if (glist_size >= GFX_GLIST_LEN) {
        return;
    }
    nuGfxTaskStart(gfx_glist[current_gfx_list],
        (s32)(glist_size * sizeof(Gfx)), NU_GFX_UCODE_L3DEX2,
        swap ? NU_SC_SWAPBUFFER : NU_SC_NOSWAPBUFFER);

    current_gfx_list = (current_gfx_list + 1) % GFX_GLIST_COUNT;
}
