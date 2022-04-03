#include <stdlib.h>

#include "data.h"
#include "entities.h"
#include "shape.h"
#include "mathdefs.h"
#include "vec.h"

/******************************************************************************
 *
 * Helper functions
 *
 *****************************************************************************/

static float signed_2d_triangle_area(
    const struct vec_2d *a,
    const struct vec_2d *b,
    const struct vec_2d *c)
{
    return (a->x - c->x) * (b->y - c->y) - (a->y - c->y) * (b->x - c->x);
}

static bool lines_intersect_full(
    const struct vec_2d *a, const struct vec_2d *b,
    const struct vec_2d *c, const struct vec_2d *d,
    struct vec_2d *intersection, float *t)
{
    const float a1 = signed_2d_triangle_area(a, b, d);
    const float a2 = signed_2d_triangle_area(a, b, c);

    if (a1 * a2 < 0.0f) {
        const float a3 = signed_2d_triangle_area(c, d, a);
        const float a4 = a3 + a2 - a1;

        if (a3 * a4 < 0.0f) {
            if (intersection || t) {
                const float toi = a3 / (a3 - a4);

                if (t) {
                    *t = toi;
                }

                if (intersection) {
                    intersection->x = a->x + toi * (b->x - a->x);
                    intersection->y = a->y + toi * (b->y - a->y);
                }
            }

            return true;
        }
    }

    return false;
}

static bool lines_intersect(
    const struct vec_2d *a, const struct vec_2d *b,
    const struct vec_2d *c, const struct vec_2d *d)
{
    return lines_intersect_full(a, b, c, d, NULL, NULL);
}

static struct vec_2d transform(
    const struct vec_2d *v,
    const struct vec_2d *pos, float sinr, float cosr, float scale)
{
    const struct vec_2d result = {
        (cosr * v->x - sinr * v->y) * scale + pos->x,
        (cosr * v->y + sinr * v->x) * scale + pos->y
    };

    return result;
}

/******************************************************************************
 *
 * Public interface
 *
 *****************************************************************************/

bool collision_test_shape_line_segment_full(
    const struct shape *b, const struct vec_2d *b_pos, float b_rot, float b_scale,
    const struct vec_2d *a_start, const struct vec_2d *a_end,
    struct vec_2d *intersection, float *t)
{
    const float sinr = sinf(b_rot);
    const float cosr = cosf(b_rot);

    int i;

    if (b->num_line_segments > 0) {
        for (i = 0; i < b->num_line_segments * 2; i += 2) {
            const struct vec_2d *b1 = (struct vec_2d *) &b->vertices[2 * b->line_segments[i]];
            const struct vec_2d *b2 = (struct vec_2d *) &b->vertices[2 * b->line_segments[i + 1]];
            const struct vec_2d b_start = transform(b1, b_pos, sinr, cosr, b_scale);
            const struct vec_2d b_end = transform(b2, b_pos, sinr, cosr, b_scale);
            if (lines_intersect_full(a_start, a_end, &b_start, &b_end, intersection, t)) {
                return true;
            }
        }
    } else {
        const int n = b->num_vertices * 2;
        for (i = 0; i < n; i += 2) {
            const struct vec_2d *b1 = (struct vec_2d *) &b->vertices[i];
            const struct vec_2d *b2 = (struct vec_2d *) &b->vertices[(i + 2) % n];
            const struct vec_2d b_start = transform(b1, b_pos, sinr, cosr, b_scale);
            const struct vec_2d b_end = transform(b2, b_pos, sinr, cosr, b_scale);
            if (lines_intersect_full(a_start, a_end, &b_start, &b_end, intersection, t)) {
                return true;
            }
        }
    }

    return false;
}

bool collision_test_shape_line_segment(
    const struct shape *b, const struct vec_2d *b_pos, float b_rot, float b_scale,
    const struct vec_2d *a_start, const struct vec_2d *a_end)
{
    return collision_test_shape_line_segment_full(
        b, b_pos, b_rot, b_scale, a_start, a_end, NULL, NULL);
}

bool collision_test_shapes_full(
    const struct shape *a, const struct vec_2d *a_pos, float a_rot, float a_scale,
    const struct shape *b, const struct vec_2d *b_pos, float b_rot, float b_scale,
    struct vec_2d *intersection, float *t)
{
    int i;
    float sinr = sinf(a_rot);
    float cosr = cosf(a_rot);

    if (a->num_line_segments > 0) {
        for (i = 0; i < a->num_line_segments * 2; i += 2) {
            const struct vec_2d *a1 = (struct vec_2d *) &a->vertices[2 * a->line_segments[i]];
            const struct vec_2d *a2 = (struct vec_2d *) &a->vertices[2 * a->line_segments[i + 1]];
            const struct vec_2d a_start = transform(a1, a_pos, sinr, cosr, a_scale);
            const struct vec_2d a_end = transform(a2, a_pos, sinr, cosr, a_scale);
            if (collision_test_shape_line_segment_full(
                    b, b_pos, b_rot, b_scale, &a_start, &a_end, intersection, t)) {
                return true;
            }
        }
    } else {
        const int n = a->num_vertices * 2;
        for (i = 0; i < n; i += 2) {
            const struct vec_2d *a1 = (struct vec_2d *) &a->vertices[i];
            const struct vec_2d *a2 = (struct vec_2d *) &a->vertices[(i + 2) % n];
            const struct vec_2d a_start = transform(a1, a_pos, sinr, cosr, a_scale);
            const struct vec_2d a_end = transform(a2, a_pos, sinr, cosr, a_scale);
            if (collision_test_shape_line_segment_full(
                    b, b_pos, b_rot, b_scale, &a_start, &a_end, intersection, t)) {
                return true;
            }
        }
    }

    return false;
}

bool collision_test_shapes(
    const struct shape *a, const struct vec_2d *a_pos, float a_rot, float a_scale,
    const struct shape *b, const struct vec_2d *b_pos, float b_rot, float b_scale)
{
    return collision_test_shapes_full(
        a, a_pos, a_rot, a_scale, b, b_pos, b_rot, b_scale, NULL, NULL);
}
