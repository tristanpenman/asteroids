#ifndef __SHAPE_H
#define __SHAPE_H

#include <stdint.h>

struct shape {
    const uint16_t *vertices;
    uint8_t num_vertices;

    const uint8_t *triangles;
    uint8_t num_triangles;
};

#endif
