#include "shape.h"

// ----------------------------------------------------------------------------
//
// Asteroid 1
//

const uint16_t asteroid1_vertices[] = {
      8, -16,
      0, -12,
      8, -16,
     16,  -8,
     12,   0,
     16,   8,
      0,  16,
    -16,   8,
    -16,  -8
};

const uint8_t asteroid1_num_vertices = 9;

const uint8_t asteroid1_triangles[] = {
    0, 1, 8,
    1, 2, 3,
    1, 3, 4,
    1, 4, 6,
    1, 6, 7,
    1, 7, 8,
    4, 5, 6
};

const uint8_t asteroid1_num_triangles = 7;

// ----------------------------------------------------------------------------
//
// Collected asteroid shapes
//

const struct shape asteroid_shapes[] = {
    {
        asteroid1_vertices,
        asteroid1_num_vertices,
        asteroid1_triangles,
        asteroid1_num_triangles
    }
};
