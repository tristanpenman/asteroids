#ifndef __SHAPE_H
#define __SHAPE_H

#include "types.h"

struct shape {
    const float *vertices;
    uint8_t num_vertices;

    const uint8_t *line_segments;
    uint8_t num_line_segments;
};

#endif
