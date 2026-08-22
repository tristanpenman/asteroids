#include <limits.h>
#include <stddef.h>
#include <malloc.h>
#include <string.h>

#include "canvas.h"
#include "debug.h"
#include "gfx.h"
#include "mathdefs.h"

#define L3DEX2_VERTEX_CACHE_SIZE 32
#define NORMALIZED_VERTEX_SCALE 8192.0f
#define PIXEL_VERTEX_SCALE 32.0f

struct compiled_shape
{
    Vtx *vertices;
    size_t vertex_count;
};

struct n64_shape
{
    struct canvas_point *points;
    uint16_t *segments;
    size_t point_count;
    size_t segment_count;
    bool closed;
    bool active;
    bool pending_destroy;
    unsigned int frame_references;
    struct compiled_shape compiled[2];
};

static struct n64_shape shapes[CANVAS_MAX_SHAPES];
static size_t retained_points;
static struct canvas_rect active_viewport;
static enum canvas_space active_space;
static struct canvas_color current_color;
static struct canvas_color emitted_color;
static bool color_emitted;
static float current_width;

static bool valid_space(enum canvas_space space)
{
    return space == CANVAS_SPACE_NORMALIZED || space == CANVAS_SPACE_PIXELS;
}

static float vertex_scale(void)
{
    return active_space == CANVAS_SPACE_NORMALIZED ?
        NORMALIZED_VERTEX_SCALE : PIXEL_VERTEX_SCALE;
}

static void free_shape(struct n64_shape *shape)
{
    free(shape->points);
    free(shape->segments);
    free(shape->compiled[CANVAS_SPACE_NORMALIZED].vertices);
    free(shape->compiled[CANVAS_SPACE_PIXELS].vertices);
    retained_points -= shape->point_count;
    memset(shape, 0, sizeof(*shape));
}

static bool validate_shape_data(const struct canvas_shape_data *data)
{
    size_t i;
    if (data == NULL || data->points == NULL || data->point_count < 2 ||
        data->point_count > CANVAS_MAX_RETAINED_POINTS ||
        retained_points + data->point_count > CANVAS_MAX_RETAINED_POINTS ||
        data->segment_count > CANVAS_MAX_SEGMENTS ||
        (data->segments == NULL && data->segment_count != 0)) {
        return false;
    }
    if (data->segments != NULL) {
        for (i = 0; i < data->segment_count * 2; ++i) {
            if (data->segments[i] >= data->point_count) {
                return false;
            }
        }
    }
    return true;
}

canvas_shape_id canvas_shape_create(const struct canvas_shape_data *data)
{
    struct n64_shape *shape;
    int id;
    if (!validate_shape_data(data)) {
        return CANVAS_INVALID_SHAPE_ID;
    }
    for (id = 0; id < CANVAS_MAX_SHAPES; ++id) {
        if (!shapes[id].active && !shapes[id].pending_destroy &&
            shapes[id].points == NULL) {
            break;
        }
    }
    if (id == CANVAS_MAX_SHAPES) {
        return CANVAS_INVALID_SHAPE_ID;
    }
    shape = &shapes[id];
    shape->points = malloc(data->point_count * sizeof(*shape->points));
    if (shape->points == NULL) {
        return CANVAS_INVALID_SHAPE_ID;
    }
    memcpy(shape->points, data->points,
        data->point_count * sizeof(*shape->points));
    if (data->segments != NULL && data->segment_count != 0) {
        shape->segments = malloc(data->segment_count * 2 *
            sizeof(*shape->segments));
        if (shape->segments == NULL) {
            free(shape->points);
            memset(shape, 0, sizeof(*shape));
            return CANVAS_INVALID_SHAPE_ID;
        }
        memcpy(shape->segments, data->segments,
            data->segment_count * 2 * sizeof(*shape->segments));
    }
    shape->point_count = data->point_count;
    shape->segment_count = data->segment_count;
    shape->closed = data->closed;
    shape->active = true;
    retained_points += data->point_count;
    return id;
}

void canvas_shape_destroy(canvas_shape_id id)
{
    if (id < 0 || id >= CANVAS_MAX_SHAPES || !shapes[id].active) {
        return;
    }
    shapes[id].active = false;
    shapes[id].pending_destroy = true;
    if (shapes[id].frame_references == 0) {
        free_shape(&shapes[id]);
    }
}

void canvas_n64_release_shapes(const canvas_shape_id *ids, int count)
{
    int i;
    for (i = 0; i < count; ++i) {
        struct n64_shape *shape;
        if (ids[i] < 0 || ids[i] >= CANVAS_MAX_SHAPES) {
            continue;
        }
        shape = &shapes[ids[i]];
        if (shape->frame_references != 0) {
            --shape->frame_references;
        }
        if (shape->pending_destroy && shape->frame_references == 0) {
            free_shape(shape);
        }
    }
}

static bool convert_vertex(Vtx *vertex, struct canvas_point point, float scale)
{
    const float x = point.x * scale;
    const float y = point.y * scale;
    if (x < SHRT_MIN || x > SHRT_MAX || y < SHRT_MIN || y > SHRT_MAX) {
        return false;
    }
    memset(vertex, 0, sizeof(*vertex));
    vertex->v.ob[0] = (s16)x;
    vertex->v.ob[1] = (s16)y;
    vertex->v.ob[2] = -5;
    vertex->v.cn[0] = 255;
    vertex->v.cn[1] = 255;
    vertex->v.cn[2] = 255;
    vertex->v.cn[3] = 255;
    return true;
}

static bool compile_shape(struct n64_shape *shape, enum canvas_space space)
{
    struct compiled_shape *compiled = &shape->compiled[space];
    size_t line_count;
    size_t i;
    float scale = space == CANVAS_SPACE_NORMALIZED ?
        NORMALIZED_VERTEX_SCALE : PIXEL_VERTEX_SCALE;

    if (compiled->vertices != NULL) {
        return true;
    }
    line_count = shape->segments != NULL ? shape->segment_count :
        shape->point_count - 1 + (shape->closed ? 1 : 0);
    compiled->vertex_count = line_count * 2;
    compiled->vertices = memalign(16, compiled->vertex_count * sizeof(Vtx));
    if (compiled->vertices == NULL) {
        compiled->vertex_count = 0;
        return false;
    }
    for (i = 0; i < line_count; ++i) {
        size_t a;
        size_t b;
        if (shape->segments != NULL) {
            a = shape->segments[i * 2];
            b = shape->segments[i * 2 + 1];
        } else {
            a = i;
            b = i + 1;
            if (b == shape->point_count) {
                b = 0;
            }
        }
        if (!convert_vertex(&compiled->vertices[i * 2], shape->points[a], scale) ||
            !convert_vertex(&compiled->vertices[i * 2 + 1], shape->points[b], scale)) {
            free(compiled->vertices);
            compiled->vertices = NULL;
            compiled->vertex_count = 0;
            return false;
        }
    }
    return true;
}

static bool emit_projection(enum canvas_space space)
{
    Mtx *projection;
    const float scale = space == CANVAS_SPACE_NORMALIZED ?
        NORMALIZED_VERTEX_SCALE : PIXEL_VERTEX_SCALE;
    if (!gfx_line_commands_available(1)) {
        return false;
    }
    projection = gfx_alloc_line_projection();
    if (projection == NULL) {
        return false;
    }
    if (space == CANVAS_SPACE_NORMALIZED) {
        const float aspect = (float)active_viewport.width /
            (float)active_viewport.height;
        guOrtho(projection, -aspect * scale, aspect * scale, -scale, scale,
            1.0f, 10.0f, 1.0f);
    } else {
        guOrtho(projection, 0.0f, active_viewport.width * scale,
            active_viewport.height * scale, 0.0f, 1.0f, 10.0f, 1.0f);
    }
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(projection),
        G_MTX_LOAD | G_MTX_NOPUSH | G_MTX_PROJECTION);
    return true;
}

bool canvas_begin(const struct canvas_rect *viewport, enum canvas_space space)
{
    if (!valid_space(space) || !graphics_canvas_begin_phase()) {
        return false;
    }
    if (viewport == NULL) {
        active_viewport.x = 0;
        active_viewport.y = 0;
        active_viewport.width = SCREEN_WD;
        active_viewport.height = SCREEN_HT;
    } else {
        active_viewport = *viewport;
    }
    if (active_viewport.width <= 0 || active_viewport.height <= 0 ||
        !gfx_line_commands_available(10)) {
        graphics_canvas_end_phase();
        return false;
    }
    gfx_apply_viewport(&active_viewport);
    gDPPipeSync(glistp++);
    gDPSetCycleType(glistp++, G_CYC_1CYCLE);
    gDPSetRenderMode(glistp++, G_RM_AA_XLU_LINE, G_RM_AA_XLU_LINE2);
    gDPSetCombineMode(glistp++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);
    gSPClearGeometryMode(glistp++, 0xffffffff);
    gSPSetGeometryMode(glistp++, G_SHADE | G_SHADING_SMOOTH);
    gSPTexture(glistp++, 0, 0, 0, 0, G_OFF);
    active_space = space;
    current_color.red = 255;
    current_color.green = 255;
    current_color.blue = 255;
    current_color.alpha = 255;
    current_width = 1.0f;
    color_emitted = false;
    if (!emit_projection(space)) {
        graphics_canvas_end_phase();
        return false;
    }
    return true;
}

bool canvas_set_space(enum canvas_space space)
{
    if (!graphics_canvas_phase_active() || !valid_space(space)) {
        return false;
    }
    if (space != active_space) {
        if (!emit_projection(space)) {
            return false;
        }
        active_space = space;
    }
    return true;
}

void canvas_end(void)
{
    if (graphics_canvas_phase_active()) {
        graphics_canvas_end_phase();
    }
}

void canvas_set_color(struct canvas_color color)
{
    if (graphics_canvas_phase_active()) {
        current_color = color;
    }
}

bool canvas_set_line_width(float width)
{
    if (!graphics_canvas_phase_active() || width < CANVAS_MIN_LINE_WIDTH ||
        width > CANVAS_MAX_LINE_WIDTH) {
        return false;
    }
    current_width = width;
    return true;
}

static bool colors_equal(struct canvas_color a, struct canvas_color b)
{
    return a.red == b.red && a.green == b.green && a.blue == b.blue &&
        a.alpha == b.alpha;
}

static void emit_color(void)
{
    if (!color_emitted || !colors_equal(current_color, emitted_color)) {
        gDPSetPrimColor(glistp++, 0, 0, current_color.red, current_color.green,
            current_color.blue, current_color.alpha);
        emitted_color = current_color;
        color_emitted = true;
    }
}

static struct gfx_transform *emit_transform(
    const struct canvas_transform *transform)
{
    struct gfx_transform *matrices = gfx_alloc_line_transform();
    const float scale = vertex_scale();
    struct canvas_transform identity;
    if (matrices == NULL) {
        return NULL;
    }
    if (transform == NULL) {
        identity.position.x = 0.0f;
        identity.position.y = 0.0f;
        identity.rotation = 0.0f;
        identity.scale.x = 1.0f;
        identity.scale.y = 1.0f;
        transform = &identity;
    }
    guTranslate(&matrices->modeling, transform->position.x * scale,
        transform->position.y * scale, 0.0f);
    guRotate(&matrices->rotation, transform->rotation * RAD_TO_DEG,
        0.0f, 0.0f, 1.0f);
    guScale(&matrices->scale, transform->scale.x, transform->scale.y, 1.0f);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&matrices->modeling),
        G_MTX_LOAD | G_MTX_NOPUSH | G_MTX_MODELVIEW);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&matrices->rotation),
        G_MTX_MUL | G_MTX_NOPUSH | G_MTX_MODELVIEW);
    gSPMatrix(glistp++, OS_K0_TO_PHYSICAL(&matrices->scale),
        G_MTX_MUL | G_MTX_NOPUSH | G_MTX_MODELVIEW);
    return matrices;
}

static void emit_compiled_lines(const Vtx *vertices, size_t vertex_count)
{
    size_t offset = 0;
    while (offset < vertex_count) {
        size_t count = vertex_count - offset;
        size_t i;
        if (count > L3DEX2_VERTEX_CACHE_SIZE) {
            count = L3DEX2_VERTEX_CACHE_SIZE;
        }
        if (count & 1) {
            --count;
        }
        gSPVertex(glistp++, &vertices[offset], count, 0);
        for (i = 0; i < count; i += 2) {
            gSPLineW3D(glistp++, i, i + 1, current_width * 0.5f, 0);
        }
        offset += count;
    }
}

bool canvas_draw_shape(canvas_shape_id id,
    const struct canvas_transform *transform)
{
    struct n64_shape *shape;
    struct compiled_shape *compiled;
    size_t batches;
    size_t commands;
    int reference_result;
    if (!graphics_canvas_phase_active() || id < 0 ||
        id >= CANVAS_MAX_SHAPES || !shapes[id].active) {
        return false;
    }
    shape = &shapes[id];
    if (!compile_shape(shape, active_space)) {
        return false;
    }
    compiled = &shape->compiled[active_space];
    batches = (compiled->vertex_count + L3DEX2_VERTEX_CACHE_SIZE - 1) /
        L3DEX2_VERTEX_CACHE_SIZE;
    commands = 3 + batches + compiled->vertex_count / 2 +
        (!color_emitted || !colors_equal(current_color, emitted_color) ? 1 : 0);
    if (!gfx_line_commands_available(commands)) {
        return false;
    }
    reference_result = gfx_reference_shape(id);
    if (reference_result < 0) {
        return false;
    }
    if (reference_result > 0) {
        ++shape->frame_references;
    }
    if (emit_transform(transform) == NULL) {
        return false;
    }
    emit_color();
    emit_compiled_lines(compiled->vertices, compiled->vertex_count);
    return true;
}

static bool draw_dynamic(const struct canvas_point *points, size_t point_count,
    const uint16_t *segments, size_t segment_count, bool strip, bool closed,
    const struct canvas_transform *transform)
{
    Vtx *vertices;
    size_t line_count = strip ? point_count - 1 + (closed ? 1 : 0) :
        segment_count;
    size_t vertex_count = line_count * 2;
    size_t batches = (vertex_count + L3DEX2_VERTEX_CACHE_SIZE - 1) /
        L3DEX2_VERTEX_CACHE_SIZE;
    size_t commands = 3 + batches + line_count +
        (!color_emitted || !colors_equal(current_color, emitted_color) ? 1 : 0);
    size_t i;
    const float scale = vertex_scale();

    if (vertex_count > GFX_MAX_DYNAMIC_VERTICES ||
        !gfx_line_commands_available(commands)) {
        return false;
    }
    vertices = gfx_alloc_dynamic_vertices(vertex_count);
    if (vertices == NULL) {
        return false;
    }
    for (i = 0; i < line_count; ++i) {
        size_t a;
        size_t b;
        if (strip) {
            a = i;
            b = i + 1;
            if (b == point_count) {
                b = 0;
            }
        } else {
            a = segments[i * 2];
            b = segments[i * 2 + 1];
        }
        if (!convert_vertex(&vertices[i * 2], points[a], scale) ||
            !convert_vertex(&vertices[i * 2 + 1], points[b], scale)) {
            return false;
        }
    }
    if (emit_transform(transform) == NULL) {
        return false;
    }
    emit_color();
    emit_compiled_lines(vertices, vertex_count);
    return true;
}

bool canvas_draw_lines(const struct canvas_point *points, size_t point_count,
    const uint16_t *segments, size_t segment_count,
    const struct canvas_transform *transform)
{
    size_t i;
    if (!graphics_canvas_phase_active() || points == NULL || point_count < 2 ||
        point_count > CANVAS_MAX_DYNAMIC_POINTS || segments == NULL ||
        segment_count > CANVAS_MAX_SEGMENTS) {
        return false;
    }
    for (i = 0; i < segment_count * 2; ++i) {
        if (segments[i] >= point_count) {
            return false;
        }
    }
    return draw_dynamic(points, point_count, segments, segment_count, false,
        false, transform);
}

bool canvas_draw_line_strip(const struct canvas_point *points,
    size_t point_count, bool closed,
    const struct canvas_transform *transform)
{
    if (!graphics_canvas_phase_active() || points == NULL || point_count < 2 ||
        point_count > CANVAS_MAX_DYNAMIC_POINTS) {
        return false;
    }
    return draw_dynamic(points, point_count, NULL, 0, true, closed, transform);
}

bool canvas_draw_line(struct canvas_point a, struct canvas_point b,
    const struct canvas_transform *transform)
{
    struct canvas_point points[2];
    points[0] = a;
    points[1] = b;
    return canvas_draw_line_strip(points, 2, false, transform);
}
