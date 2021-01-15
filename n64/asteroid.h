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

#endif
