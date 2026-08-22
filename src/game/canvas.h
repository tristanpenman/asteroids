#ifndef ASTEROIDS_GAME_CANVAS_H
#define ASTEROIDS_GAME_CANVAS_H

#include <stddef.h>

#include "types.h"

typedef int canvas_shape_id;

#define CANVAS_INVALID_SHAPE_ID (-1)

#define CANVAS_MAX_SHAPES 64
#define CANVAS_MAX_RETAINED_POINTS 256
#define CANVAS_MAX_TRANSFORMS 512
#define CANVAS_MAX_DYNAMIC_POINTS 256
#define CANVAS_MAX_SEGMENTS 512
#define CANVAS_MIN_LINE_WIDTH 1.0f
#define CANVAS_MAX_LINE_WIDTH 8.0f

struct canvas_color
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;
};

struct canvas_point
{
    float x;
    float y;
};

struct canvas_transform
{
    struct canvas_point position;
    float rotation;
    struct canvas_point scale;
};

struct canvas_rect
{
    int x;
    int y;
    int width;
    int height;
};

enum canvas_space
{
    CANVAS_SPACE_NORMALIZED,
    CANVAS_SPACE_PIXELS
};

struct canvas_shape_data
{
    const struct canvas_point *points;
    size_t point_count;
    const uint16_t *segments;
    size_t segment_count;
    bool closed;
};

canvas_shape_id canvas_shape_create(const struct canvas_shape_data *data);
void canvas_shape_destroy(canvas_shape_id shape);

bool canvas_begin(const struct canvas_rect *viewport, enum canvas_space space);
bool canvas_set_space(enum canvas_space space);
void canvas_end(void);

void canvas_set_color(struct canvas_color color);
bool canvas_set_line_width(float width);

bool canvas_draw_shape(canvas_shape_id shape,
    const struct canvas_transform *transform);
bool canvas_draw_line(struct canvas_point a, struct canvas_point b,
    const struct canvas_transform *transform);
bool canvas_draw_line_strip(const struct canvas_point *points,
    size_t point_count, bool closed,
    const struct canvas_transform *transform);
bool canvas_draw_lines(const struct canvas_point *points, size_t point_count,
    const uint16_t *segments, size_t segment_count,
    const struct canvas_transform *transform);

#endif
