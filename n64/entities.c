#include <stdlib.h>

#include "data.h"
#include "defines.h"
#include "entities.h"
#include "gfx.h"
#include "mathdefs.h"
#include "shape.h"
#include "util.h"

float calculate_asteroid_radius(unsigned int shape)
{
    float r;
    float r_max = 0.0f;

    const float *vertices = asteroid_shape_data[shape].vertices;

    unsigned int i;
    unsigned int n = asteroid_shape_data[shape].num_vertices;;

    for (i = 0; i < n * 2; i += 2) {
        r = sqrt(vertices[i] * vertices[i] + vertices[i + 1] * vertices[i + 1]);
        if (r_max < r) {
            r_max = r;
        }
    }

    return r_max;
}

void randomise_asteroid_position(struct asteroid *a)
{
    float x;
    float y;
    float dist = 0.0f;

    do {
        x = random_float(-SCREEN_WD / 2.0f, SCREEN_WD / 2.0f);
        y = random_float(-SCREEN_HT / 2.0f, SCREEN_HT / 2.0f);
        dist = sqrt(x * x + y * y);
    } while (dist < NO_ASTEROID_RADIUS);

    a->pos.x = x;
    a->pos.y = y;
}

void randomise_asteroid_rotation(struct asteroid *a)
{
    a->rot = random_float(0, 2.0f * (float) M_PI);
}

void randomise_asteroid_velocity(struct asteroid *a, float vel_scale)
{
    const float speed = random_float(ASTEROID_SPEED_MIN, ASTEROID_SPEED_MAX);
    const float rot = random_float(0, 2.0f * (float) M_PI);

    a->vel.x = sin(rot) * speed * vel_scale;
    a->vel.y = 0 - cos(rot) * speed * vel_scale;
}

void asteroid_init(struct asteroid *a)
{
    randomise_asteroid_position(a);
    randomise_asteroid_velocity(a, 100.0f);
    randomise_asteroid_rotation(a);

    a->pos_prev.x = a->pos.x;
    a->pos_prev.y = a->pos.y;
    a->visible = true;
    a->scale = 1.0f;
    a->shape = rand() % NUM_ASTEROID_SHAPES;
    a->radius = calculate_asteroid_radius(a->shape) * a->scale;
}

void asteroid_update(struct asteroid *a, float f)
{
    a->pos_prev.x = a->pos.x;
    a->pos_prev.y = a->pos.y;
    a->pos.x += a->vel.x * f;
    a->pos.y += a->vel.y * f;

    if (wrap_position(&a->pos, a->radius)) {
        a->pos_prev.x = a->pos.x;
        a->pos_prev.y = a->pos.y;
    }
}
