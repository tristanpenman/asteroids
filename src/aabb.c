#include "aabb.h"
#include "mathdefs.h"
#include "shape.h"

void find_aabb(const struct shape *s, struct aabb *bounding_box)
{
    int i;

    bounding_box->top_left.x = s->vertices[0];
    bounding_box->top_left.y = s->vertices[1];

    bounding_box->bottom_right.x = s->vertices[0];
    bounding_box->bottom_right.y = s->vertices[1];

    for (i = 2; i < s->num_vertices * 2; i += 2) {
        if (s->vertices[i] < bounding_box->top_left.x) {
            bounding_box->top_left.x = s->vertices[i];
        }
        if (s->vertices[i + 1] < bounding_box->top_left.y) {
            bounding_box->top_left.y = s->vertices[i + 1];
        }

        if (s->vertices[i] > bounding_box->bottom_right.x) {
            bounding_box->bottom_right.x = s->vertices[i];
        }
        if (s->vertices[i + 1] > bounding_box->bottom_right.y) {
            bounding_box->bottom_right.y = s->vertices[i + 1];
        }
    }
}

bool check_aabb_collision(
    const struct aabb *a, const struct vec_2d *a_pos,
    const struct aabb *b, const struct vec_2d *b_pos)
{
    float a_width = a->bottom_right.x - a->top_left.x;
    float a_height = a->bottom_right.y - a->top_left.y;

    float b_width = b->bottom_right.x - b->top_left.x;
    float b_height = b->bottom_right.y - b->top_left.y;

    float a_x = a_pos->x + a->top_left.x;
    float a_y = a_pos->y + a->top_left.y;

    float b_x = b_pos->x + b->top_left.x;
    float b_y = b_pos->y + b->top_left.y;

    return (fabsf(a_x - b_x) * 2 < (a_width + b_width)) &&
            (fabsf(a_y - b_y) * 2 < (a_height + b_height));
}
