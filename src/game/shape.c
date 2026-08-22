#include "shape.h"

#include "graphics.h"
#include "options.h"
#include "vec.h"

#define LEGACY_TO_NORMALIZED \
    (2.0f * (float)LOGICAL_WIDTH_PX / (float)LOGICAL_HEIGHT_PX)

static canvas_shape_id owned_shapes[CANVAS_MAX_SHAPES];
static size_t owned_shape_count;

const struct vec_2d origin = {
    0.5f,
    (float)LOGICAL_HEIGHT_PX / (float)LOGICAL_WIDTH_PX / 2.0f
};

canvas_shape_id shape_canvas_create(const struct shape *shape)
{
    struct canvas_point points[256];
    struct canvas_shape_data data;
    canvas_shape_id id;
    size_t i;

    if (shape == NULL || shape->num_vertices < 2 ||
        owned_shape_count >= CANVAS_MAX_SHAPES) {
        return CANVAS_INVALID_SHAPE_ID;
    }

    for (i = 0; i < shape->num_vertices; ++i) {
        points[i].x = shape->vertices[i * 2] * LEGACY_TO_NORMALIZED;
        points[i].y = -shape->vertices[i * 2 + 1] * LEGACY_TO_NORMALIZED;
    }

    data.points = points;
    data.point_count = shape->num_vertices;
    data.segments = shape->line_segments;
    data.segment_count = shape->num_line_segments;
    data.closed = false;
    id = canvas_shape_create(&data);
    if (id != CANVAS_INVALID_SHAPE_ID) {
        owned_shapes[owned_shape_count++] = id;
    }
    return id;
}

void shape_canvas_destroy_all(void)
{
    size_t i;
    for (i = 0; i < owned_shape_count; ++i) {
        canvas_shape_destroy(owned_shapes[i]);
    }
    owned_shape_count = 0;
}

bool shape_canvas_draw(canvas_shape_id shape, struct vec_2d position,
    float rotation, struct vec_2d scale)
{
    struct canvas_transform transform;
    transform.position.x = position.x * LEGACY_TO_NORMALIZED;
    transform.position.y = -position.y * LEGACY_TO_NORMALIZED;
    transform.rotation = -rotation;
    transform.scale.x = scale.x;
    transform.scale.y = scale.y;
    return canvas_draw_shape(shape, &transform);
}

static void convert_transform(struct canvas_transform *output,
    struct vec_2d position, float rotation, struct vec_2d scale)
{
    output->position.x = position.x * LEGACY_TO_NORMALIZED;
    output->position.y = -position.y * LEGACY_TO_NORMALIZED;
    output->rotation = -rotation;
    output->scale.x = scale.x;
    output->scale.y = scale.y;
}

static bool convert_points(struct canvas_point *output,
    const struct vec_2d *points, size_t point_count)
{
    size_t i;
    if (output == NULL || points == NULL ||
        point_count > CANVAS_MAX_DYNAMIC_POINTS) {
        return false;
    }
    for (i = 0; i < point_count; ++i) {
        output[i].x = points[i].x * LEGACY_TO_NORMALIZED;
        output[i].y = -points[i].y * LEGACY_TO_NORMALIZED;
    }
    return true;
}

bool shape_canvas_draw_lines(const struct vec_2d *points, size_t point_count,
    const uint16_t *segments, size_t segment_count, struct vec_2d position,
    float rotation, struct vec_2d scale)
{
    struct canvas_point converted[CANVAS_MAX_DYNAMIC_POINTS];
    struct canvas_transform transform;
    if (!convert_points(converted, points, point_count)) {
        return false;
    }
    convert_transform(&transform, position, rotation, scale);
    return canvas_draw_lines(converted, point_count, segments, segment_count,
        &transform);
}

bool shape_canvas_draw_line_strip(const struct vec_2d *points,
    size_t point_count, bool closed, struct vec_2d position, float rotation,
    struct vec_2d scale)
{
    struct canvas_point converted[CANVAS_MAX_DYNAMIC_POINTS];
    struct canvas_transform transform;
    if (!convert_points(converted, points, point_count)) {
        return false;
    }
    convert_transform(&transform, position, rotation, scale);
    return canvas_draw_line_strip(converted, point_count, closed, &transform);
}

bool shape_canvas_begin_frame(void)
{
    const struct graphics_color black = {0, 0, 0, 255};
    if (!graphics_begin_frame(black)) {
        return false;
    }
    if (!canvas_begin(NULL, CANVAS_SPACE_NORMALIZED)) {
        graphics_end_frame();
        return false;
    }
    return true;
}

bool shape_canvas_end_frame(void)
{
    canvas_end();
    return graphics_end_frame();
}
