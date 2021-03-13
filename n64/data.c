#include <stdlib.h>

#include "shape.h"
#include "types.h"

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

// ----------------------------------------------------------------------------
//
// Asteroid 2
//

static const float asteroid2_vertices[] = {
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

// ----------------------------------------------------------------------------
//
// Asteroid 3
//

static const float asteroid3_vertices[] = {
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

// ----------------------------------------------------------------------------
//
// Asteroid 4
//

static const float asteroid4_vertices[] = {
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

// ----------------------------------------------------------------------------
//
// Collected asteroid shapes
//

const struct shape asteroid_shape_data[] = {
    {
        asteroid1_vertices,
        sizeof(asteroid1_vertices) / sizeof(float) / 2,
        NULL,
        0
    },
    {
        asteroid2_vertices,
        sizeof(asteroid2_vertices) / sizeof(float) / 2,
        NULL,
        0
    },
    {
        asteroid3_vertices,
        sizeof(asteroid3_vertices) / sizeof(float) / 2,
        NULL,
        0
    },
    {
        asteroid4_vertices,
        sizeof(asteroid4_vertices) / sizeof(float) / 2,
        NULL,
        0
    }
};

// ----------------------------------------------------------------------------
//
// Player
//

static const float player_vertices[] = {
    0.000f, -0.020f,
   -0.016f,  0.018f,
    0.016f,  0.018f,
    0.014f,  0.0125f,
   -0.014f,  0.0125f,
    0.000f,  0.02f
};

static const uint8_t player_frame_1_line_segments[] = {
    0, 1,
    0, 2,
    3, 4
};

static const uint8_t player_frame_2_line_segments[] = {
    0, 1,
    0, 2,
    3, 4,
    3, 5,
    4, 5
};

const struct shape player_frame_1_shape_data = {
    player_vertices,
    sizeof(player_vertices) / sizeof(float) / 2,
    player_frame_1_line_segments,
    sizeof(player_frame_1_line_segments) / sizeof(uint8_t) / 2
};

const struct shape player_frame_2_shape_data = {
    player_vertices,
    sizeof(player_vertices) / sizeof(float) / 2,
    player_frame_2_line_segments,
    sizeof(player_frame_2_line_segments) / sizeof(uint8_t) / 2
};
