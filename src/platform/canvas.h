#ifndef ASTEROIDS_PLATFORM_CANVAS_H
#define ASTEROIDS_PLATFORM_CANVAS_H

#include "types.h"

#define CANVAS_INVALID_SHAPE -1

#define NUM_INDICES(indices) (sizeof(indices) / sizeof(uint16_t))
#define NUM_VERTICES(vertices) (sizeof(vertices) / sizeof(float) / 2)

struct vec_2d;

/** Half-extents of the drawable area in centered game coordinates. */
extern const struct vec_2d canvas_origin;

enum canvas_primitive
{
    CANVAS_LINE_STRIP,
    CANVAS_LINES,
    CANVAS_POINTS
};

struct shape
{
    const float *vertices;
    uint8_t num_vertices;

    // Indices are optional. uint16_t is used because of a bug in Emscripten's
    // legacy GL implementation.
    const uint16_t *indices;
    uint8_t num_indices;

    enum canvas_primitive primitive;
};

/**
 * Unload all previously loaded shapes
 */
void canvas_reset(void);

/**
 * Load a shape so that it can be drawn using \c canvas_draw_shape
 */
int canvas_load_shape(const struct shape *shape);

/**
 * Start drawing shapes onto the canvas
 */
void canvas_start_drawing(bool clear);

/**
 * Change the drawing colour. Components are normalized to the range 0 to 1.
 */
void canvas_set_colour(float r, float g, float b);

/**
 * Draw a shape in centered game coordinates
 */
bool canvas_draw_shape(int shape, struct vec_2d position, float rotation, struct vec_2d scale);

/**
 * Finish drawing shapes to the canvas. When swap is false, drawing can
 * continue in a new batch without swapping the display buffer.
 */
void canvas_finish_drawing(bool swap);

#endif
