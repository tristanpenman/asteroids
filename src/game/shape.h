#ifndef ASTEROIDS_GAME_SHAPE_H
#define ASTEROIDS_GAME_SHAPE_H

#include "types.h"
#include "canvas.h"

#define NUM_LINE_SEGMENTS(line_segments) (sizeof(line_segments) / sizeof(uint16_t) / 2)

#define NUM_VERTICES(vertices) (sizeof(vertices) / sizeof(float) / 2)

struct shape
{
    const float *vertices;
    uint8_t num_vertices;

    //
    // Line segments are optional. If omitted, vertices will be used to draw a
    // line loop.
    //
    // uint16_t is used (even though it's bigger than we need) because of a bug
    // in Emscripten's legacy GL implemenetation
    //
    const uint16_t *line_segments;
    uint8_t num_line_segments;
};

struct vec_2d;

canvas_shape_id shape_canvas_create(const struct shape *shape);
void shape_canvas_destroy_all(void);
bool shape_canvas_draw(canvas_shape_id shape, struct vec_2d position,
    float rotation, struct vec_2d scale);
bool shape_canvas_draw_lines(const struct vec_2d *points, size_t point_count,
    const uint16_t *segments, size_t segment_count, struct vec_2d position,
    float rotation, struct vec_2d scale);
bool shape_canvas_draw_line_strip(const struct vec_2d *points,
    size_t point_count, bool closed, struct vec_2d position, float rotation,
    struct vec_2d scale);
bool shape_canvas_begin_frame(void);
bool shape_canvas_end_frame(void);

#endif
