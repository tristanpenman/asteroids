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

struct edge
{
    struct vec_2d a;
    struct vec_2d b;
};

static float signed_2d_triangle_area(const struct vec_2d *a,
            const struct vec_2d *b, const struct vec_2d *c)
{
    return (a->x - c->x) * (b->y - c->y) - (a->y - c->y) * (b->x - c->x);
}

static bool test_line_intersect(
    const struct vec_2d *a, const struct vec_2d *b,
    const struct vec_2d *c, const struct vec_2d *d)
{
    const float a1 = signed_2d_triangle_area(a, b, d);
    const float a2 = signed_2d_triangle_area(a, b, c);
    if (a1 * a2 < 0.0f) {
        const float a3 = signed_2d_triangle_area(c, d, a);
        const float a4 = a3 + a2 - a1;
        if (a3 * a4 < 0.0f) {
            return true;
        }
    }

    return false;
}

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
            if (test_line_intersect(&a3, &a4, e1, e2) == true ||
                test_line_intersect(&a3, &a5, e1, e2) == true ||
                test_line_intersect(&a4, &a6, e1, e2) == true ||
                test_line_intersect(&a5, &a6, e1, e2) == true) {
                return true;
            }
        }

        if (true == wrap) {
            e1 = &vertices[num_vertices - 1];
            e2 = &vertices[0];
            if (test_line_intersect(&a3, &a4, e1, e2) == true ||
                test_line_intersect(&a3, &a5, e1, e2) == true ||
                test_line_intersect(&a4, &a6, e1, e2) == true ||
                test_line_intersect(&a5, &a6, e1, e2) == true) {
                return true;
            }
        }
    }

    return false;
}

/******************************************************************************
 *
 * Public interface
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
