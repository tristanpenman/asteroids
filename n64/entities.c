#include <stdlib.h>
#include <string.h>

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
        x = random_float(-1.0f / 2.0f, 1.0f / 2.0f);
        y = random_float(-0.75f / 2.0f, 0.75f / 2.0f);
        dist = sqrtf(x * x + y * y);
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
    randomise_asteroid_velocity(a, 1.0f);
    randomise_asteroid_rotation(a);

    a->pos_prev.x = a->pos.x;
    a->pos_prev.y = a->pos.y;
    a->visible = true;
    a->scale = 1.0f;
    a->shape = rand() % NUM_ASTEROID_SHAPES;
    a->radius = calculate_asteroid_radius(a->shape) * a->scale;
}

void asteroid_update(struct asteroid *a, float f, const struct vec_2d *lim)
{
    a->pos_prev.x = a->pos.x;
    a->pos_prev.y = a->pos.y;
    a->pos.x += a->vel.x * f;
    a->pos.y += a->vel.y * f;

    if (wrap_position(&a->pos, lim)) {
        a->pos_prev.x = a->pos.x;
        a->pos_prev.y = a->pos.y;
    }
}

void bullet_init(struct bullet *b, const struct vec_2d *pos, float rot)
{
    b->rot = rot;

    // convert to radians
    rot = rot / 180.f * M_PI;

    b->visible = true;
    b->travelled = 0.0f;
    b->pos.x = pos->x - sinf(rot) * (0 - SHIP_PIVOT);
    b->pos.y = pos->y + cosf(rot) * (0 - SHIP_PIVOT);
    b->pos_prev.x = b->pos.x;
    b->pos_prev.y = b->pos.y;

    b->vel.x = sinf(rot);
    b->vel.y = 0 - cosf(rot);

    // vec_2d_normalise(&b->vel);
    vec_2d_scale(&b->vel, BULLET_SPEED);
}

void bullet_update(struct bullet *b, float f)
{
    static struct vec_2d t;
    t.x = b->vel.x * f;
    t.y = b->vel.y * f;

    if (b->pos.x + t.x != b->pos.x ||
        b->pos.y + t.y != b->pos.y) {
        b->pos_prev.x = b->pos.x;
        b->pos_prev.y = b->pos.y;
    }

    b->pos.x += t.x;
    b->pos.y += t.y;
    b->travelled += vec_2d_magnitude(&t);

    if (b->travelled > MAX_BULLET_DISTANCE) {
        // destroy bullets
        b->visible = false;
    } else {
        // wrap bullets
        struct vec_2d lim;
        lim.x = 1.f / 2.f + 0.1f;
        lim.y = 0.75f / 2.f + 0.1f;
        if (wrap_position(&b->pos, &lim) == true) {
            b->pos_prev.x = b->pos.x;
            b->pos_prev.y = b->pos.y;
        }
    }
}

void player_init(struct player *p)
{
    memset(p, 0, sizeof(struct player));

    p->lives = SHIP_LIVES;
    p->score = 0;
    p->hit = 0;

    player_reset(p);
}

void player_reset(struct player *p)
{
    p->state = PS_NORMAL;
    p->death_delay = 0.0f;
    p->pos.x = 0.0f;
    p->pos.y = 0.0f;
    p->reload_delay = 0.0f;
    p->reloading = false;
    p->rot = 0.0f;
    p->vel.x = 0.0f;
    p->vel.y = 0.0f;
}
