#ifndef __AABB_H
#define __AABB_H

#include "types.h"
#include "vec.h"

struct aabb
{
    struct vec_2d top_left;
    struct vec_2d bottom_right;
};

struct shape;

void find_aabb(const struct shape *s, struct aabb *bounding_box);

bool check_aabb_collision(
    const struct aabb *a, const struct vec_2d *a_pos,
    const struct aabb *b, const struct vec_2d *b_pos);

#endif
