#ifndef __SHAPE_H
#define __SHAPE_H

#include "types.h"

#define NUM_LINE_SEGMENTS(line_segments) (sizeof(line_segments) / sizeof(uint16_t) / 2)

#define NUM_VERTICES(vertices) (sizeof(vertices) / sizeof(float) / 2)

struct shape {
    const float *vertices;
    uint8_t num_vertices;

    //
    // Line segments are optional. If omitted, vertices will be used to draw a
    // line loop.
    //
    // uint16_t is used (even though it's bigger than we need) because of a bug
    // in Emscripten's legacy GL implemenetation
    //
    const uint16_t *line_segments;
    uint8_t num_line_segments;
};

#endif
