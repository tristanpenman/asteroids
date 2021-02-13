#ifndef __SHAPE_H
#define __SHAPE_H

#include "types.h"

struct shape {
    const float *vertices;
    uint8_t num_vertices;

    const uint8_t *triangles;
    uint8_t num_triangles;
};

#endif
