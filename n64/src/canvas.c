#include <assert.h>
#include <math.h>
#include <string.h>

#include <nusys.h>

#include "canvas.h"
#include "data.h"
#include "defines.h"
#include "gfx.h"
#include "shape.h"
#include "vec.h"

#define MAX_TRANSFORMS 30
#define MAX_SHAPES 64
#define MAX_VERTICES 128
#define MAX_GLYPHS 128

#define RAD_TO_DEG (180.0 / M_PI)

struct geometry {
    const Vtx *vertices;
    uint8_t num_vertices;

    const uint8_t *line_segments;
    uint8_t num_line_segments;
};

struct color {
    float r;
    float g;
    float b;
};

static Vtx vertices[MAX_VERTICES];
static struct geometry shapes[MAX_SHAPES];

static int num_shapes = 0;
static int num_vertices = 0;

static Dynamic transforms[MAX_TRANSFORMS];

static int num_transforms = 0;

static struct color primitive_color;

static int font_shape_ids[MAX_GLYPHS];

void canvas_reset()
{
    int i = 0;

    for (i = 0; i < MAX_GLYPHS; i++) {
        font_shape_ids[i] = CANVAS_INVALID_SHAPE;
    }

    num_shapes = 0;
    num_vertices = 0;

    primitive_color.r = 255.0f;
    primitive_color.g = 255.0f;
    primitive_color.b = 255.0f;
}

int canvas_load_shape(const struct shape *shape)
{
    int i;

    if (num_shapes == MAX_SHAPES) {
        return CANVAS_INVALID_SHAPE;
    }

    if (num_vertices + shape->num_vertices >= MAX_VERTICES) {
        return CANVAS_INVALID_SHAPE;
    }

    for (i = 0; i < shape->num_vertices; i++) {
        Vtx *vertex = &vertices[num_vertices + i];

        memset(vertex, 0, sizeof(Vtx));

        vertex->v.ob[0] = shape->vertices[i * 2] * (float)SCREEN_WD;
        vertex->v.ob[1] = shape->vertices[i * 2 + 1] * (float)SCREEN_WD;
        vertex->v.ob[2] = -5;

        vertex->v.cn[0] = 0xFF;
        vertex->v.cn[1] = 0xFF;
        vertex->v.cn[2] = 0XFF;
        vertex->v.cn[3] = 0xFF;
    }

    shapes[num_shapes].vertices = &vertices[num_vertices];
    shapes[num_shapes].num_vertices = shape->num_vertices;
    shapes[num_shapes].line_segments = shape->line_segments;
    shapes[num_shapes].num_line_segments = shape->num_line_segments;

    num_vertices += shape->num_vertices;

    return num_shapes++;
}

void canvas_start_drawing(bool clear)
{
    // discard transforms used for previous frame
    num_transforms = 0;

    // Specify the display list buffer
    glistp = gfx_glist;

    // The initialization of RCP
    gfx_rcp_init();

    if (clear) {
        // Clear the frame buffer and the Z-buffer
        gfx_clear_cfb();
    }

    // Synchronizes RDP attribute updates by waiting for pixels to be processed
    // during the rendering of primitives
    gDPPipeSync(glistp++);

    // number of pixels per cycle
    gDPSetCycleType(glistp++, G_CYC_1CYCLE);
}

void canvas_set_colour(float r, float g, float b)
{
    primitive_color.r = r;
    primitive_color.g = g;
    primitive_color.b = b;
}

bool canvas_draw_shape(int shape, struct vec_2d position, float rotation, struct vec_2d scale)
{
    Dynamic *transform = &transforms[num_transforms++];

    guOrtho(&transform->projection,
        -SCREEN_WD / 2.f,  // left
         SCREEN_WD / 2.f,  // right
         SCREEN_HT / 2.f,  // bottom
        -SCREEN_HT / 2.f,  // top
        1.0f,       // near
        10.0f,      // far
        1.0f);      // scale

    guTranslate(&transform->modeling, position.x * SCREEN_WD, position.y * SCREEN_WD, 0.0f);
    guRotate(&transform->rotation, rotation, 0.0f, 0.0f, 1.0f);
    guScale(&transform->scale, scale.x, scale.y, 1.f);

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

    // support anti-aliased, translucent lines
    gDPSetRenderMode(glistp++, G_RM_AA_XLU_LINE, G_RM_AA_XLU_LINE2);

    // set line color
    gDPSetCombineMode(glistp++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
    gDPSetPrimColor(glistp++, 0, 0, primitive_color.r, primitive_color.g, primitive_color.b, 255);

    gSPClearGeometryMode(glistp++, 0xFFFFFFFF);
    gSPSetGeometryMode(glistp++, G_SHADE | G_SHADING_SMOOTH);

    {
        const uint8_t *line_segments = shapes[shape].line_segments;
        int i;
        if (line_segments) {
            for (i = 0; i < shapes[shape].num_line_segments * 2; i += 2) {
                gSPLineW3D(glistp++, line_segments[i], line_segments[i + 1], 0.5, 0);
            }
        } else {
            const size_t num_vertices = shapes[shape].num_vertices;
            for (i = 0; i < shapes[shape].num_vertices - 1; i++) {
                gSPLineW3D(glistp++, i, (i + 1), 0.5, 0);
            }
        }
    }

    return true;
}

void canvas_draw_text(const char *text, float x, float y, float spacing, float scale_factor)
{
    struct vec_2d position = {
        x,
        y
    };

    struct vec_2d scale = {
        scale_factor,
        scale_factor
    };

    const char *s = text;

    while (*s) {
        int shape_index;
        int c = *s;
        if (c >= MAX_GLYPHS) {
            goto next;
        }
            
        if (font_shape_ids[c] == CANVAS_INVALID_SHAPE) {
            shape_index = ascii_to_font_mapping[c];
            if (shape_index < 0) {
                goto next;
            }

            font_shape_ids[c] = canvas_load_shape(&font_shape_data[shape_index]);
            if (font_shape_ids[c] == CANVAS_INVALID_SHAPE) {
                goto next;
            }
        }

        canvas_draw_shape(font_shape_ids[c], position, 0, scale);
next:
        position.x += FONT_WIDTH * scale_factor;
        position.x += spacing * scale_factor;
        s++;
    }
}

void canvas_draw_text_centered(const char *text, float size, float y, float spacing)
{
    const float width = ((float)strlen(text) * (FONT_WIDTH + spacing)) - spacing;

    canvas_draw_text(text, 0 - (width * size / 2.0f), y, spacing, size);
}

void canvas_finish_drawing(bool swap)
{
    gDPFullSync(glistp++);
    gSPEndDisplayList(glistp++);

    assert(glistp - gfx_glist < GFX_GLIST_LEN);

    // Activate the RSP task
    nuGfxTaskStart(gfx_glist, (s32)(glistp - gfx_glist) * sizeof (Gfx),
        NU_GFX_UCODE_L3DEX2, swap ? NU_SC_SWAPBUFFER : NU_SC_NOSWAPBUFFER);
}
