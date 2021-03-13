#include "data.h"
#include "entities.h"
#include "mathdefs.h"
#include "options.h"
#include "shape.h"
#include "vec.h"

/******************************************************************************
 *
 * Helper functions (new)
 *
 *****************************************************************************/

static float signed_2d_triangle_area(
    const struct vec_2d *a,
    const struct vec_2d *b,
    const struct vec_2d *c)
{
    return (a->x - c->x) * (b->y - c->y) - (a->y - c->y) * (b->x - c->x);
}

static bool lines_intersect(
    const struct vec_2d *a, const struct vec_2d *b,
    const struct vec_2d *c, const struct vec_2d *d)
{
    const float a1 = signed_2d_triangle_area(a, b, d);
    const float a2 = signed_2d_triangle_area(a, b, c);
    if (a1 * a2 >= 0.0f) {
        return false;
    }

    const float a3 = signed_2d_triangle_area(c, d, a);
    const float a4 = a3 + a2 - a1;
    return a3 * a4 < 0.0f;
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
 * Helper functions (old)
 *
 *****************************************************************************/

static bool test_asteroid_against_polygon(const struct asteroid *asteroid,
    const struct vec_2d vertices[], int num_vertices, bool wrap,
    const struct vec_2d *offset)
{
    const float *geometry = asteroid_shape_data[asteroid->shape].vertices;

    const unsigned int num_asteroid_vertices = asteroid_shape_data[asteroid->shape].num_vertices;

    const float sinr = sinf(asteroid->rot);
    const float cosr = cosf(asteroid->rot);
    const float scale = asteroid->scale;

    struct vec_2d a1, a2, a3, a4, a5, a6;
    const struct vec_2d *e1, *e2;

    unsigned int i, j;

    for (i = 0; i < num_asteroid_vertices; i++) {
        if (i == 0) {
            a1.x = (cosr * geometry[i * 2 + 1] - sinr * geometry[i * 2 + 2]) * scale;
            a1.y = (cosr * geometry[i * 2 + 2] + sinr * geometry[i * 2 + 1]) * scale;
        } else {
            a1.x = a2.x;
            a1.y = a2.y;
        }

        if (i == num_asteroid_vertices - 1) {
            a2.x = (cosr * geometry[1] - sinr * geometry[2]) * scale;
            a2.y = (cosr * geometry[2] + sinr * geometry[1]) * scale;
        } else {
            a2.x = (cosr * geometry[i * 2 + 3] - sinr * geometry[i * 2 + 4]) * scale;
            a2.y = (cosr * geometry[i * 2 + 4] + sinr * geometry[i * 2 + 3]) * scale;
        }

        a3.x = a1.x + asteroid->pos.x - offset->x;
        a3.y = a1.y + asteroid->pos.y - offset->y;
        a4.x = a2.x + asteroid->pos.x - offset->x;
        a4.y = a2.y + asteroid->pos.y - offset->y;

        a5.x = a1.x + asteroid->pos_prev.x - offset->x;
        a5.y = a1.y + asteroid->pos_prev.y - offset->y;
        a6.x = a2.x + asteroid->pos_prev.x - offset->x;
        a6.y = a2.y + asteroid->pos_prev.y - offset->y;

        for (j = 0; j < num_vertices - 1; ++j) {
            e1 = &vertices[j];
            e2 = &vertices[j + 1];
            if (lines_intersect(&a3, &a4, e1, e2) == true ||
                lines_intersect(&a3, &a5, e1, e2) == true ||
                lines_intersect(&a4, &a6, e1, e2) == true ||
                lines_intersect(&a5, &a6, e1, e2) == true) {
                return true;
            }
        }

        if (true == wrap) {
            e1 = &vertices[num_vertices - 1];
            e2 = &vertices[0];
            if (lines_intersect(&a3, &a4, e1, e2) == true ||
                lines_intersect(&a3, &a5, e1, e2) == true ||
                lines_intersect(&a4, &a6, e1, e2) == true ||
                lines_intersect(&a5, &a6, e1, e2) == true) {
                return true;
            }
        }
    }

    return false;
}

/******************************************************************************
 *
 * Public interface (new)
 *
 *****************************************************************************/

bool collision_test_shape_line_segment(
    const struct shape *b, const struct vec_2d *b_pos, float b_rot, float b_scale,
    const struct vec_2d *a_start, const struct vec_2d *a_end)
{
    const float sinr = sinf(b_rot);
    const float cosr = cosf(b_rot);

    if (b->num_line_segments > 0) {
        for (int i = 0; i < b->num_line_segments * 2; i += 2) {
            const struct vec_2d *b1 = (struct vec_2d *) &b->vertices[2 * b->line_segments[i]];
            const struct vec_2d *b2 = (struct vec_2d *) &b->vertices[2 * b->line_segments[i + 1]];
            const struct vec_2d b_start = transform(b1, b_pos, sinr, cosr, b_scale);
            const struct vec_2d b_end = transform(b2, b_pos, sinr, cosr, b_scale);
            if (lines_intersect(a_start, a_end, &b_start, &b_end)) {
                return true;
            }
        }
    } else {
        const int n = b->num_vertices * 2;
        for (int i = 0; i < n; i += 2) {
            const struct vec_2d *b1 = (struct vec_2d *) &b->vertices[i];
            const struct vec_2d *b2 = (struct vec_2d *) &b->vertices[(i + 2) % n];
            const struct vec_2d b_start = transform(b1, b_pos, sinr, cosr, b_scale);
            const struct vec_2d b_end = transform(b2, b_pos, sinr, cosr, b_scale);
            if (lines_intersect(a_start, a_end, &b_start, &b_end)) {
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
    // TODO: perform low granularity test using bounding sphere

    const float sinr = sinf(a_rot);
    const float cosr = cosf(a_rot);

    if (a->num_line_segments > 0) {
        for (int i = 0; i < a->num_line_segments * 2; i += 2) {
            const struct vec_2d *a1 = (struct vec_2d *) &a->vertices[2 * a->line_segments[i]];
            const struct vec_2d *a2 = (struct vec_2d *) &a->vertices[2 * a->line_segments[i + 1]];
            const struct vec_2d a_start = transform(a1, a_pos, sinr, cosr, a_scale);
            const struct vec_2d a_end = transform(a2, a_pos, sinr, cosr, a_scale);
            if (collision_test_shape_line_segment(b, b_pos, b_rot, b_scale, &a_start, &a_end)) {
                return true;
            }
        }
    } else {
        const int n = a->num_vertices * 2;
        for (int i = 0; i < n; i += 2) {
            const struct vec_2d *a1 = (struct vec_2d *) &a->vertices[i];
            const struct vec_2d *a2 = (struct vec_2d *) &a->vertices[(i + 2) % n];
            const struct vec_2d a_start = transform(a1, a_pos, sinr, cosr, a_scale);
            const struct vec_2d a_end = transform(a2, a_pos, sinr, cosr, a_scale);
            if (collision_test_shape_line_segment(b, b_pos, b_rot, b_scale, &a_start, &a_end)) {
                return true;
            }
        }
    }

    return false;
}

/******************************************************************************
 *
 * Public interface (old)
 *
 *****************************************************************************/

bool test_asteroid_bullet_collision(struct bullet* b, struct asteroid *a)
{
    const struct vec_2d vertices[] = {
        b->pos_prev,
        b->pos
    };

    const struct vec_2d offset = { 0.f, 0.f };

    return test_asteroid_against_polygon(a, vertices, 2, false, &offset);
}

bool test_asteroid_ship_collision(struct player *p, struct asteroid *a)
{
    const float sinr = sinf(p->rot);
    const float cosr = cosf(p->rot);

    // Ship delta
    const struct vec_2d delta = {
        p->pos.x - p->pos_prev.x,
        p->pos.y - p->pos_prev.y
    };

    struct vec_2d vertices[3];

    // Ship vertices
    vertices[0].x =  -0.012f * cosr - (0.038f - SHIP_PIVOT) * sinr + p->pos.x;
    vertices[0].y =  (0.038f - SHIP_PIVOT) * cosr + -0.012f * sinr + p->pos.y;
    vertices[1].x =  0 - (0 - SHIP_PIVOT) * sinr + p->pos.x;
    vertices[1].y =  (0 - SHIP_PIVOT) * cosr + p->pos.y;
    vertices[2].x =   0.012f * cosr - (0.038f - SHIP_PIVOT) * sinr + p->pos.x;
    vertices[2].y =  (0.038f - SHIP_PIVOT) * cosr + 0.012f * sinr + p->pos.y;

    return test_asteroid_against_polygon(a, vertices, 3, true, &delta);
}
