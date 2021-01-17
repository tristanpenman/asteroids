#ifndef __CANVAS_H
#define __CANVAS_H

#include <stdbool.h>

#define CANVAS_INVALID_SHAPE -1

struct shape;
struct vec_2d;

/**
 * Unload all previously loaded shapes
 */
void canvas_reset();

/**
 * Load a shape so that it can be drawn using \c video_draw_shape
 */
int canvas_load_shape(const struct shape *);

/**
 * Start drawing shapes onto the canvas
 */
void canvas_start_drawing(bool clear);

/**
 * Draw the lines defined by a shape
 */
bool canvas_draw_lines(int shape, struct vec_2d position, float rotation, struct vec_2d scale);

/**
 * Draw the triangles defined by a shape
 */
bool canvas_draw_triangles(int shape, struct vec_2d position, float rotation, struct vec_2d scale);

/**
 * Finish drawing shapes to the canvas
 */
void canvas_finish_drawing(bool swap);

#endif
