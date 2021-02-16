#include <assert.h>
#include <nusys.h>
#include <stdlib.h>

#include "canvas.h"
#include "data.h"
#include "defines.h"
#include "entities.h"
#include "gfx.h"
#include "mathdefs.h"
#include "shape.h"
#include "vec.h"

#define NUM_ASTEROIDS 5

// nusys input
extern NUContData contdata[1];
extern u8 contPattern;

// useful constants
static const float ratio = (float)SCREEN_HT / (float)SCREEN_WD;
static const float factor = 1.0f / 60.0f;
static const float degrees_to_radians = M_PI / 180.0f;

// ship wrapping constants
static const float max_x =  1.0 / 2.0f + SHIP_RADIUS;
static const float min_x = -1.0 / 2.0f - SHIP_RADIUS;
static const float max_y =  ratio / 2.0f + SHIP_RADIUS;
static const float min_y = -ratio / 2.0f - SHIP_RADIUS;

// asteroid state
static struct asteroid asteroids[NUM_ASTEROIDS];

// player state
static float theta;
static float pos_x;
static float pos_y;
static float vel_x;
static float vel_y;

// shapes
static uint8_t asteroid_shapes[NUM_ASTEROID_SHAPES];
static uint8_t player_shape;

void makeDL00(void)
{
    int i;
    char conbuf[20];
    const struct vec_2d player_pos = {
      pos_x,
      pos_y
    };

    canvas_start_drawing(true);

    canvas_draw_triangles(player_shape, player_pos, theta, vec_2d_unit);

    for (i = 0; i < NUM_ASTEROIDS; i++) {
        canvas_draw_triangles(asteroid_shapes[asteroids[i].shape], asteroids[i].pos, 0, vec_2d_unit);
    }

    canvas_finish_drawing(false);

    // Change character representation positions */
    nuDebConTextPos(0,12,23);
    sprintf(conbuf, "pos_x=%7.3f", pos_x);
    nuDebConCPuts(0, conbuf);

    nuDebConTextPos(0,12,24);
    sprintf(conbuf, "pos_y=%7.3f", pos_y);
    nuDebConCPuts(0, conbuf);

    // Display characters on the frame buffer
    nuDebConDisp(NU_SC_SWAPBUFFER);
}

void updateGame00(void)
{
    int i;
    struct vec_2d lim;

    static float rot;

    // Data reading of controller 1
    nuContDataGetEx(contdata, 0);

    if (contdata[0].stick_x < -20) {
        theta -= SHIP_ROTATION_SPEED;
    } else if (contdata[0].stick_x > 20) {
        theta += SHIP_ROTATION_SPEED;
    }

    if (theta > 360.0) {
        theta -= 360.0;
    } else if (theta < 0.0) {
        theta += 360.0;
    }

    rot = theta * degrees_to_radians;

    if (contdata[0].button & B_BUTTON) {
        vel_x += sinf(rot) * SHIP_ACCELERATION * factor;
        vel_y -= cosf(rot) * SHIP_ACCELERATION * factor;
    }

    pos_x += vel_x * factor;
    pos_y += vel_y * factor;

    if (pos_y > max_y) {
        pos_y = min_y;
    } else if (pos_y < min_y) {
        pos_y = max_y;
    }

    if (pos_x > max_x) {
        pos_x = min_x;
    } else if (pos_x < min_x) {
        pos_x = max_x;
    }

    for (i = 0; i < NUM_ASTEROIDS; i++) {
        lim.x = 1.f / 2.f + asteroids[i].radius;
        lim.y = 0.75f / 2.f + asteroids[i].radius;

        asteroid_update(&asteroids[i], 1.f / 60.f, &lim);
    }
}

/******************************************************************************
 *
 * Public interface
 *
 *****************************************************************************/

void stage00_init()
{
    int i;

    canvas_reset();

    player_shape = canvas_load_shape(&player_shape_data);

    for (i = 0; i < NUM_ASTEROID_SHAPES; i++) {
        asteroid_shapes[i] = canvas_load_shape(&asteroid_shape_data[i]);
    }

    for (i = 0; i < NUM_ASTEROIDS; i++) {
        asteroid_init(&asteroids[i]);
    }

    theta = 0.0;
    pos_x = 0.0;
    pos_y = 0.0;
    vel_x = 0.0;
    vel_y = 0.0;
}

void stage00_loop(bool draw)
{
    if (draw) {
        makeDL00();
    }

    updateGame00();
}
