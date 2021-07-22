#ifndef __CANVAS_H
#define __CANVAS_H

#include "types.h"
#include "vec.h"

#define CANVAS_INVALID_SHAPE -1

struct shape;

/**
 * Unload all previously loaded shapes
 */
void canvas_reset();

/**
 * Load a shape so that it can be drawn using \c canvas_draw_lines or \c canvas_draw_triangles
 */
int canvas_load_shape(const struct shape *);

/**
 * Start drawing shapes onto the canvas
 */
void canvas_start_drawing(bool clear);

/**
 * Change the colour that will be used when drawing lines
 */
void canvas_set_colour(float r, float g, float b);

/**
 * Draw the lines segments defined by a shape
 */
bool canvas_draw_line_segments(int shape, struct vec_2d position, float rotation, struct vec_2d scale);

/**
 * Finish drawing shapes to the canvas
 */
void canvas_finish_drawing(bool swap);

#endif
