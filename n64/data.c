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
// Asteroid 2
//

const uint16_t asteroid2_vertices[] = {
     -8, -16,
      0, -12,
      8, -16,
     16,  -8,
      8,  -4,
     16,   4,
      8,  16,
     -4,  12,
     -8,  16,
    -16,   8,
    -12,   0,
    -16,  -8
};

const uint8_t asteroid2_num_vertices = 12;

const uint8_t asteroid2_triangles[] = {
    0, 1, 11,
    1, 2, 4,
    1, 4, 7,
    1, 7, 10,
    1, 10, 11,
    2, 3, 4,
    4, 5, 7,
    5, 6, 7,
    7, 8, 10,
    8, 9, 10
};

const uint8_t asteroid2_num_triangles = 10;

// ----------------------------------------------------------------------------
//
// Asteroid 3
//

const uint16_t asteroid3_vertices[] = {
    -4, -16,
     8, -16,
    16, -4,
    16,  4,
     8,  16,
     0,  16,
     0,  4,
    -8,  16,
   -16,  4,
    -8,  0,
   -16, -4
};

const uint8_t asteroid3_num_vertices = 11;

const uint8_t asteroid3_triangles[] = {
    0, 1, 9,
    0, 9, 10,
    1, 2, 6,
    1, 6, 9,
    2, 3, 6,
    3, 4, 6,
    4, 5, 6,
    6, 7, 9,
    7, 8, 9
};

const uint8_t asteroid3_num_triangles = 9;

// ----------------------------------------------------------------------------
//
// Asteroid 4
//

const uint16_t asteroid4_vertices[] = {
    -8, -16,
     4, -16,
    16,  -8,
    16,  -4,
     4,   0,
    16,   8,
     8,  16,
     4,  12,
    -8,  16,
   -16,   4,
   -16,  -4,
    -4,  -8
};

const uint8_t asteroid4_num_vertices = 12;

const uint8_t asteroid4_triangles[] = {
    0, 1, 11,
    1, 2, 11,
    2, 3, 11,
    3, 4, 11,
    4, 5, 7,
    5, 6, 7,
    4, 7, 8,
    4, 8, 11,
    8, 9, 11,
    9, 10, 11
};

const uint8_t asteroid4_num_triangles = 10;

// ----------------------------------------------------------------------------
//
// Collected asteroid shapes
//

const struct shape asteroid_shape_data[] = {
    {
        asteroid1_vertices,
        asteroid1_num_vertices,
        asteroid1_triangles,
        asteroid1_num_triangles
    },
    {
        asteroid2_vertices,
        asteroid2_num_vertices,
        asteroid2_triangles,
        asteroid2_num_triangles
    },
    {
        asteroid3_vertices,
        asteroid3_num_vertices,
        asteroid3_triangles,
        asteroid3_num_triangles
    },
    {
        asteroid4_vertices,
        asteroid4_num_vertices,
        asteroid4_triangles,
        asteroid4_num_triangles
    }
};
