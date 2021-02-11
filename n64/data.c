#include <stdint.h>

#include "shape.h"

// ----------------------------------------------------------------------------
//
// Asteroid 1
//

static const float asteroid1_vertices[] = {
    -.02f, -.04f,
     .00f, -.03f,
     .02f, -.04f,
     .04f, -.02f,
     .03f,  .00f,
     .04f,  .02f,
     .00f,  .04f,
    -.04f,  .02f,
    -.04f, -.02f
};

static const uint8_t asteroid1_triangles[] = {
    0, 1, 8,
    1, 2, 3,
    1, 3, 4,
    1, 4, 6,
    1, 6, 7,
    1, 7, 8,
    4, 5, 6
};

// ----------------------------------------------------------------------------
//
// Asteroid 2
//

const float asteroid2_vertices[] = {
    -.02f, -.04f,
     .00f, -.03f,
     .02f, -.04f,
     .04f, -.02f,
     .02f, -.01f,
     .04f,  .01f,
     .02f,  .04f,
    -.01f,  .03f,
    -.02f,  .04f,
    -.04f,  .02f,
    -.03f,  .00f,
    -.04f, -.02f
};

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

// ----------------------------------------------------------------------------
//
// Asteroid 3
//

const float asteroid3_vertices[] = {
    -.01f, -.04f,
     .02f, -.04f,
     .04f, -.01f,
     .04f,  .01f,
     .02f,  .04f,
     .00f,  .04f,
     .00f,  .01f,
    -.02f,  .04f,
    -.04f,  .01f,
    -.02f,  .00f,
    -.04f, -.01f
};

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

// ----------------------------------------------------------------------------
//
// Asteroid 4
//

const float asteroid4_vertices[] = {
  -.02f, -.04f,
   .01f, -.04f,
   .04f, -.02f,
   .04f, -.01f,
   .01f,  .00f,
   .04f,  .02f,
   .02f,  .04f,
   .01f,  .03f,
  -.02f,  .04f,
  -.04f,  .01f,
  -.04f, -.01f,
  -.01f, -.02f
};

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

// ----------------------------------------------------------------------------
//
// Collected asteroid shapes
//

const struct shape asteroid_shape_data[] = {
    {
        asteroid1_vertices,
        sizeof(asteroid1_vertices) / sizeof(float) / 2,
        asteroid1_triangles,
        sizeof(asteroid1_triangles) / sizeof(uint8_t) / 3
    },
    {
        asteroid2_vertices,
        sizeof(asteroid2_vertices) / sizeof(float) / 2,
        asteroid2_triangles,
        sizeof(asteroid2_triangles) / sizeof(uint8_t) / 3
    },
    {
        asteroid3_vertices,
        sizeof(asteroid3_vertices) / sizeof(float) / 2,
        asteroid3_triangles,
        sizeof(asteroid3_triangles) / sizeof(uint8_t) / 3
    },
    {
        asteroid4_vertices,
        sizeof(asteroid4_vertices) / sizeof(float) / 2,
        asteroid4_triangles,
        sizeof(asteroid4_triangles) / sizeof(uint8_t) / 3
    }
};
