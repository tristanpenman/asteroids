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
    -.04f, -.02f,
    -.02f, -.04f
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
    -.04f, -.02f,
    -.02f, -.04f
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
    -.04f, -.01f,
    -.01f, -.04f
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
    -.01f, -.02f,
    -.02f, -.04f
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
// Box (debug shape)
//

static const float box_vertices[] = {
    -.04f, -.04f,
     .04f, -.04f,
     .04f,  .04f,
    -.04f,  .04f
};

const struct shape box_shape_data = {
    box_vertices,
    sizeof(box_vertices) / sizeof(float) / 2,
    NULL,
    0
};

// ----------------------------------------------------------------------------
//
// Bullet
//

static const float bullet_vertices[] = {
    0.0f, 0.0f,
    0.0f, 0.02f
};

static const uint8_t bullet_line_segments[] = {
    0, 1
};

const struct shape bullet_shape_data = {
    bullet_vertices,
    sizeof(bullet_vertices) / sizeof(float) / 2,
    bullet_line_segments,
    sizeof(bullet_line_segments) / sizeof(uint8_t) / 2
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

// ----------------------------------------------------------------------------
//
// Font
//
// TODO: Use a common set of vertices, with different line segments
//

static const float font_a_vertices[] = {
    .00f,  .06f,
    .00f,  .02f,
    .015f, .00f,
    .03f,  .02f,
    .03f,  .06f,
    .03f,  .03f,
    .00f,  .03f
};

static const float font_b_vertices[] = {
    .00f, .06f,
    .00f, .00f,
    .02f, .00f,
    .03f, .01f,
    .03f, .02f,
    .02f, .03f,
    .00f, .03f,
    .02f, .03f,
    .03f, .04f,
    .03f, .05f,
    .02f, .06f,
    .00f, .06f
};

static const float font_c_vertices[] = {
    .03f, .00f,
    .01f, .00f,
    .00f, .01f,
    .00f, .05f,
    .01f, .06f,
    .03f, .06f
};

static const float font_d_vertices[] = {
    .00f, .00f,
    .02f, .00f,
    .03f, .01f,
    .03f, .05f,
    .02f, .06f,
    .00f, .06f,
    .00f, .00f
};

static const float font_e_vertices[] = {
    .03f, .00f,
    .00f, .00f,
    .00f, .03f,
    .02f, .03f,
    .00f, .03f,
    .00f, .06f,
    .03f, .06f
};

static const float font_f_vertices[] = {
    .03f, .00f,
    .00f, .00f,
    .00f, .03f,
    .02f, .03f,
    .00f, .03f,
    .00f, .06f
};

static const float font_g_vertices[] = {
    .03f, .01f,
    .02f, .00f,
    .01f, .00f,
    .00f, .01f,
    .00f, .05f,
    .01f, .06f,
    .02f, .06f,
    .03f, .05f,
    .03f, .04f,
    .02f, .04f
};

static const float font_h_vertices[] = {
    .00f, .00f,
    .00f, .06f,
    .00f, .03f,
    .03f, .03f,
    .03f, .00f,
    .03f, .06f
};

static const float font_i_vertices[] = {
    .00f,  .00f,
    .03f,  .00f,
    .015f, .00f,
    .015f, .06f,
    .00f,  .06f,
    .03f,  .06f
};

static const float font_j_vertices[] = {
    .01f, .00f,
    .03f, .00f,
    .03f, .06f,
    .01f, .06f,
    .00f, .05f
};

static const float font_k_vertices[] = {
    .00f, .00f,
    .00f, .06f,
    .00f, .03f,
    .03f, .00f,
    .00f, .03f,
    .03f, .06f
};

static const float font_l_vertices[] = {
    .00f, .00f,
    .00f, .06f,
    .03f, .06f
};

static const float font_m_vertices[] = {
    .00f,  .06f,
    .00f,  .00f,
    .015f, .02f,
    .03f,  .00f,
    .03f,  .06f
};

static const float font_n_vertices[] = {
    .00f, .06f,
    .00f, .00f,
    .03f, .06f,
    .03f, .00f
};

static const float font_o_vertices[] = {
    .00f, .00f,
    .03f, .00f,
    .03f, .06f,
    .00f, .06f,
    .00f, .00f
};

static const float font_p_vertices[] = {
    .00f, .06f,
    .00f, .00f,
    .03f, .00f,
    .03f, .03f,
    .00f, .03f
};

static const float font_q_vertices[] = {
    .02f,  .05f,
    .025f, .055f,
    .02f,  .06f,
    .00f,  .06f,
    .00f,  .00f,
    .03f,  .00f,
    .03f,  .05f,
    .025f, .055f,
    .03f,  .06f
};

static const float font_r_vertices[] = {
    .00f, .06f,
    .00f, .00f,
    .03f, .00f,
    .03f, .03f,
    .00f, .03f,
    .01f, .03f,
    .03f, .06f
};

static const float font_s_vertices[] = {
    .03f, .00f,
    .00f, .00f,
    .00f, .03f,
    .03f, .03f,
    .03f, .06f,
    .00f, .06f
};

static const float font_t_vertices[] = {
    .00f,  .00f,
    .03f,  .00f,
    .015f, .00f,
    .015f, .06f
};

static const float font_u_vertices[] = {
    .00f, .00f,
    .00f, .06f,
    .03f, .06f,
    .03f, .00f
};

static const float font_v_vertices[] = {
    .00f,  .00f,
    .015f, .06f,
    .03f,  .00f
};

static const float font_w_vertices[] = {
    .00f,  .00f,
    .00f,  .06f,
    .015f, .04f,
    .03f,  .06f,
    .03f,  .00f
};

static const float font_x_vertices[] = {
    .00f,  .00f,
    .03f,  .06f,
    .015f, .03f,
    .03f,  .00f,
    .00f,  .06f
};

static const float font_y_vertices[] = {
    .00f,  .00f,
    .015f, .03f,
    .03f,  .00f,
    .015f, .03f,
    .015f, .06f
};

static const float font_z_vertices[] = {
    .00f, .00f,
    .03f, .00f,
    .00f, .06f,
    .03f, .06f
};

static const float font_0_vertices[] = {
    .00f, .00f,
    .03f, .00f,
    .03f, .06f,
    .00f, .06f,
    .00f, .00f,
    .03f, .06f
};

static const float font_1_vertices[] = {
    .015f, .00f,
    .015f, .06f
};

static const float font_2_vertices[] = {
    .00f, .00f,
    .03f, .00f,
    .03f, .03f,
    .00f, .03f,
    .00f, .06f,
    .03f, .06f
};

static const float font_3_vertices[] = {
    .00f, .00f,
    .03f, .00f,
    .03f, .03f,
    .00f, .03f,
    .03f, .03f,
    .03f, .06f,
    .00f, .06f
};

static const float font_4_vertices[] = {
    .00f, .00f,
    .00f, .03f,
    .03f, .03f,
    .03f, .00f,
    .03f, .06f
};

static const float font_5_vertices[] = {
    .03f, .00f,
    .00f, .00f,
    .00f, .03f,
    .03f, .03f,
    .03f, .06f,
    .00f, .06f
};

static const float font_6_vertices[] = {
    .03f, .00f,
    .01f, .00f,
    .00f, .01f,
    .00f, .06f,
    .03f, .06f,
    .03f, .03f,
    .00f, .03f
};

static const float font_7_vertices[] = {
    .00f, .00f,
    .03f, .00f,
    .03f, .06f
};

static const float font_8_vertices[] = {
    .00f, .03f,
    .00f, .00f,
    .03f, .00f,
    .03f, .03f,
    .00f, .03f,
    .00f, .06f,
    .03f, .06f,
    .03f, .03f
};

static const float font_9_vertices[] = {
    .03f, .06f,
    .03f, .03f,
    .00f, .03f,
    .00f, .00f,
    .03f, .00f,
    .03f, .03f
};

static const float font_underscore_vertices[] = {
    .00f, .06f,
    .03f, .06f
};

static const float font_minus_vertices[] = {
    .00f, .03f,
    .03f, .03f
};

const struct shape font_shape_data[] = {
    {
        font_a_vertices,
        sizeof(font_a_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_b_vertices,
        sizeof(font_b_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_c_vertices,
        sizeof(font_c_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_d_vertices,
        sizeof(font_d_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_e_vertices,
        sizeof(font_e_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_f_vertices,
        sizeof(font_f_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_g_vertices,
        sizeof(font_g_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_h_vertices,
        sizeof(font_h_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_i_vertices,
        sizeof(font_i_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_j_vertices,
        sizeof(font_j_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_k_vertices,
        sizeof(font_k_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_l_vertices,
        sizeof(font_l_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_m_vertices,
        sizeof(font_m_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_n_vertices,
        sizeof(font_n_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_o_vertices,
        sizeof(font_o_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_p_vertices,
        sizeof(font_p_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_q_vertices,
        sizeof(font_q_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_r_vertices,
        sizeof(font_r_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_s_vertices,
        sizeof(font_s_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_t_vertices,
        sizeof(font_t_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_u_vertices,
        sizeof(font_u_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_v_vertices,
        sizeof(font_v_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_w_vertices,
        sizeof(font_w_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_x_vertices,
        sizeof(font_x_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_y_vertices,
        sizeof(font_y_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_z_vertices,
        sizeof(font_z_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_0_vertices,
        sizeof(font_0_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_1_vertices,
        sizeof(font_1_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_2_vertices,
        sizeof(font_2_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_3_vertices,
        sizeof(font_3_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_4_vertices,
        sizeof(font_4_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_5_vertices,
        sizeof(font_5_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_6_vertices,
        sizeof(font_6_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_7_vertices,
        sizeof(font_7_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_8_vertices,
        sizeof(font_8_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_9_vertices,
        sizeof(font_9_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_underscore_vertices,
        sizeof(font_underscore_vertices) / sizeof(float) / 2,
        NULL,
        0
    }, {
        font_minus_vertices,
        sizeof(font_minus_vertices) / sizeof(float) / 2,
        NULL,
        0
    }
};

const uint8_t font_glyph_count = sizeof(font_shape_data) / sizeof(struct shape);

const int8_t ascii_to_font_mapping[128] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,   // 00 - 2C
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,   // 10 - 1F
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,               // 20 - 2C

    37,     // 2D (minus)

    -1,     // 2E
    -1,     // 2F

    26,     // 30 (0)
    27,     // 31 (1)
    28,     // 32 (2)
    29,     // 33 (3)
    30,     // 34 (4)
    31,     // 35 (5)
    32,     // 36 (6)
    33,     // 37 (7)
    34,     // 38 (8)
    35,     // 39 (9)

    -1,     // 3A
    -1,     // 3B
    -1,     // 3C
    -1,     // 3D
    -1,     // 3E
    -1,     // 3F
    -1,     // 40

    0,      // 41 (A)
    1,      // 42 (B)
    2,      // 43 (C)
    3,      // 44 (D)
    4,      // 45 (E)
    5,      // 46 (F)
    6,      // 47 (G)
    7,      // 48 (H)
    8,      // 49 (I)
    9,      // 4A (J)
    10,     // 4B (K)
    11,     // 4C (L)
    12,     // 4D (M)
    13,     // 4E (N)
    14,     // 4F (O)
    15,     // 50 (P)
    16,     // 51 (Q)
    17,     // 52 (R)
    18,     // 53 (S)
    19,     // 54 (T)
    20,     // 55 (U)
    21,     // 56 (V)
    22,     // 57 (W)
    23,     // 58 (X)
    24,     // 59 (Y)
    25,     // 5A (Z)

    -1,     // 5B
    -1,     // 5C
    -1,     // 5D
    -1,     // 5E

    36,     // 5F (underscore)

    -1,     // 60

    0,      // 61 (a)
    1,      // 62 (b)
    2,      // 63 (c)
    3,      // 64 (d)
    4,      // 65 (e)
    5,      // 66 (f)
    6,      // 67 (g)
    7,      // 68 (h)
    8,      // 69 (i)
    9,      // 6A (j)
    10,     // 6B (k)
    11,     // 6C (l)
    12,     // 6D (m)
    13,     // 6E (n)
    14,     // 6F (o)
    15,     // 70 (p)
    16,     // 71 (q)
    17,     // 72 (r)
    18,     // 73 (s)
    19,     // 74 (t)
    20,     // 75 (u)
    21,     // 76 (v)
    22,     // 77 (w)
    23,     // 78 (x)
    24,     // 79 (y)
    25      // 7A (z)

    -1,     // 7B
    -1,     // 7C
    -1,     // 7D
    -1,     // 7E
    -1,     // 7F
};
