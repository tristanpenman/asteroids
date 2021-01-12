#include <math.h>

#include "vec.h"

const struct vec_2d vec_2d_unit = {
    1.0f,
    1.0f
};

const struct vec_2d vec_2d_zero = {
    0.0f,
    0.0f
};

float vec_2d_dot(const struct vec_2d *u, const struct vec_2d *v)
{
    return u->x * v->x + u->y * v->y;
}

float vec_2d_magnitude(const struct vec_2d *v)
{
    return sqrt(v->x * v->x + v->y * v->y);
}

float vec_2d_normalise(struct vec_2d *v)
{
    const float m = vec_2d_magnitude(v);

    v->x /= m;
    v->y /= m;

    return m;
}

struct vec_2d vec_2d_normalised(const struct vec_2d *v)
{
    const float m = vec_2d_magnitude(v);
    const struct vec_2d result = {
        v->x / m,
        v->y / m
    };

    return result;
}

void vec_2d_scale(struct vec_2d *v, float f)
{
    v->x *= f;
    v->y *= f;
}
