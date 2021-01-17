#ifndef __ASTEROID_H
#define __ASTEROID_H

#include <stdbool.h>

#include "vec.h"

struct asteroid
{
    bool visible;
    struct vec_2d pos;
    struct vec_2d pos_prev;
    struct vec_2d vel;
    float radius;
    float rot;
    float scale;
    int shape;
};

float calculate_asteroid_radius(unsigned int shape);
void randomise_asteroid_rotation(struct asteroid *a);
void randomise_asteroid_velocity(struct asteroid *a, float vel_scale);

void asteroid_init(struct asteroid *a);
void asteroid_update(struct asteroid *a, float f);

#endif
