#include <stddef.h>
#include <string.h>

#include "canvas.h"
#include "defines.h"
#include "gfx.h"
#include "mathdefs.h"
#include "text.h"
#include "vec.h"

#define MAX_SHAPES 64
#define MAX_TRANSFORMS 128
#define MAX_VERTICES 256

struct geometry
{
    const Vtx *vertices;
    uint8_t num_vertices;
    const uint16_t *indices;
    uint8_t num_indices;
    enum canvas_primitive primitive;
};

struct colour
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

const struct vec_2d canvas_origin = {
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

static uint8_t colour_component(float component)
{
    if (component <= 0.0f) {
        return 0;
    }
    if (component >= 1.0f) {
        return 255;
    }
    return (uint8_t)(component * 255.0f);
}

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
    if (num_shapes >= MAX_SHAPES ||
        num_vertices + shape->num_vertices > MAX_VERTICES) {
        return CANVAS_INVALID_SHAPE;
    }

    for (int i = 0; i < shape->num_vertices; ++i) {
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
    shapes[num_shapes].indices = shape->indices;
    shapes[num_shapes].num_indices = shape->num_indices;
    shapes[num_shapes].primitive = shape->primitive;
    num_vertices += shape->num_vertices;

    return num_shapes++;
}

static void canvas_prepare_drawing(bool clear)
{
    glistp = gfx_glist[current_gfx_list];
    gfx_rcp_init();

    if (clear) {
        gfx_clear_cfb();
    }

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

    canvas_prepare_drawing(clear);
}

void canvas_set_colour(float r, float g, float b)
{
    primitive_colour.r = colour_component(r);
    primitive_colour.g = colour_component(g);
    primitive_colour.b = colour_component(b);
}

bool canvas_draw_shape(int shape, struct vec_2d position, float rotation, struct vec_2d scale)
{
    if (shape < 0 || shape >= num_shapes || num_transforms >= MAX_TRANSFORMS) {
        return false;
    }

    struct gfx_transform *transform = &transforms[num_transforms++];
    guTranslate(&transform->modeling, position.x * SCREEN_WD, position.y * SCREEN_WD, 0.0f);
    guRotate(&transform->rotation, rotation * RAD_TO_DEG, 0.0f, 0.0f, 1.0f);
    guScale(&transform->scale, scale.x, scale.y, 1.0f);

    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&transform->modeling), G_MTX_LOAD | G_MTX_NOPUSH | G_MTX_MODELVIEW);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&transform->rotation), G_MTX_MUL | G_MTX_NOPUSH | G_MTX_MODELVIEW);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&transform->scale), G_MTX_MUL | G_MTX_NOPUSH | G_MTX_MODELVIEW);
    gSPVertex(glistp++, shapes[shape].vertices, shapes[shape].num_vertices, 0);
    gDPSetRenderMode(glistp++, G_RM_AA_XLU_LINE, G_RM_AA_XLU_LINE2);
    gDPSetCombineMode(glistp++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
    gDPSetPrimColor(glistp++, 0, 0, primitive_colour.r, primitive_colour.g, primitive_colour.b, 255);
    gSPClearGeometryMode(glistp++, 0xffffffff);
    gSPSetGeometryMode(glistp++, G_SHADE | G_SHADING_SMOOTH);

    switch (shapes[shape].primitive) {
    case CANVAS_LINES:
        if (shapes[shape].indices != NULL) {
            for (int i = 0; i < shapes[shape].num_indices; i += 2) {
                gSPLineW3D(glistp++, shapes[shape].indices[i],
                    shapes[shape].indices[i + 1], 0.5, 0);
            }
        } else {
            for (int i = 0; i < shapes[shape].num_vertices - 1; i += 2) {
                gSPLineW3D(glistp++, i, i + 1, 0.5, 0);
            }
        }
        break;
    case CANVAS_POINTS:
        for (int i = 0; i < shapes[shape].num_vertices; ++i) {
            gSPLineW3D(glistp++, i, i, 0.5, 0);
        }
        break;
    case CANVAS_LINE_STRIP:
        for (int i = 0; i < shapes[shape].num_vertices - 1; ++i) {
            gSPLineW3D(glistp++, i, i + 1, 0.5, 0);
        }
        break;
    }

    return true;
}

void canvas_finish_drawing(bool swap)
{
    gDPFullSync(glistp++);
    gSPEndDisplayList(glistp++);
    size_t glist_size = (size_t)(glistp - gfx_glist[current_gfx_list]);
    if (glist_size >= GFX_GLIST_LEN) {
        return;
    }

    nuGfxTaskStart(gfx_glist[current_gfx_list],
        (s32)(glist_size * sizeof(Gfx)), NU_GFX_UCODE_L3DEX2,
        swap ? NU_SC_SWAPBUFFER : NU_SC_NOSWAPBUFFER);
    current_gfx_list = (current_gfx_list + 1) % GFX_GLIST_COUNT;

    if (!swap) {
        canvas_prepare_drawing(false);
    }
}
