#ifndef __COLLISION_H
#define __COLLISION_H

#include "types.h"

struct shape;
struct vec_2d;

bool collision_test_shape_line_segment_full(
    const struct shape *a, const struct vec_2d *a_pos, float a_rot, float a_scale,
    const struct vec_2d *line_start, const struct vec_2d *line_end,
    struct vec_2d *intersection, float *t);

bool collision_test_shape_line_segment(
    const struct shape *a, const struct vec_2d *a_pos, float a_rot, float a_scale,
    const struct vec_2d *line_start, const struct vec_2d *line_end);

bool collision_test_shapes_full(
    const struct shape *a, const struct vec_2d *a_pos, float a_rot, float a_scale,
    const struct shape *b, const struct vec_2d *b_pos, float b_rot, float b_scale,
    struct vec_2d *intersection, float *t);

bool collision_test_shapes(
    const struct shape *a, const struct vec_2d *a_pos, float a_rot, float a_scale,
    const struct shape *b, const struct vec_2d *b_pos, float b_rot, float b_scale);

#endif
