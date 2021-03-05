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
struct player player;

// shapes
static uint8_t asteroid_shapes[NUM_ASTEROID_SHAPES];

static uint8_t player_frame_1_shape;
static uint8_t player_frame_2_shape;

/******************************************************************************
 *
 * Collision helpers
 *
 *****************************************************************************/

void check_collisions(struct player *p, struct asteroid *aa, unsigned int na,
    unsigned int *asteroids_hit)
{
    // TODO
}

/******************************************************************************
 *
 * Update logic
 *
 *****************************************************************************/

void level_draw()
{
    int i;
    char conbuf[20];

    canvas_start_drawing(true);
    canvas_draw_line_segments(player_frame_1_shape, player.pos, player.rot, vec_2d_unit);

    for (i = 0; i < NUM_ASTEROIDS; i++) {
        canvas_draw_line_segments(asteroid_shapes[asteroids[i].shape], asteroids[i].pos, 0, vec_2d_unit);
    }

    canvas_finish_drawing(false);

    // Change character representation positions */
    nuDebConTextPos(0,12,23);
    sprintf(conbuf, "pos_x=%7.3f", player.pos.x);
    nuDebConCPuts(0, conbuf);

    nuDebConTextPos(0,12,24);
    sprintf(conbuf, "pos_y=%7.3f", player.pos.y);
    nuDebConCPuts(0, conbuf);

    // Display characters on the frame buffer
    nuDebConDisp(NU_SC_SWAPBUFFER);
}

void level_update()
{
    int i;
    struct vec_2d lim;
    static float rot;

    // Data reading of controller 1
    nuContDataGetEx(contdata, 0);

    if (contdata[0].stick_x < -20) {
        player.rot -= SHIP_ROTATION_SPEED;
    } else if (contdata[0].stick_x > 20) {
        player.rot += SHIP_ROTATION_SPEED;
    }

    if (player.rot > 360.0) {
        player.rot -= 360.0;
    } else if (player.rot < 0.0) {
        player.rot += 360.0;
    }

    rot = player.rot * degrees_to_radians;

    if (contdata[0].button & B_BUTTON) {
        player.vel.x += sinf(rot) * SHIP_ACCELERATION * factor;
        player.vel.y -= cosf(rot) * SHIP_ACCELERATION * factor;
    }

    player.pos.x += player.vel.x * factor;
    player.pos.y += player.vel.y * factor;

    if (player.pos.y > max_y) {
        player.pos.y = min_y;
    } else if (player.pos.y < min_y) {
        player.pos.y = max_y;
    }

    if (player.pos.x > max_x) {
        player.pos.x = min_x;
    } else if (player.pos.x < min_x) {
        player.pos.x = max_x;
    }

    for (i = 0; i < NUM_ASTEROIDS; i++) {
        lim.x = 1.f / 2.f + asteroids[i].radius;
        lim.y = 0.75f / 2.f + asteroids[i].radius;

        asteroid_update(&asteroids[i], 1.f / 60.f, &lim);
    }

    {
        unsigned int asteroids_hit = 0;
        check_collisions(&player, asteroids, NUM_ASTEROIDS, &asteroids_hit);
    }
}

/******************************************************************************
 *
 * Public interface
 *
 *****************************************************************************/

void level_init(unsigned int level, unsigned int lives, unsigned int score)
{
    int i;

    canvas_reset();

    player_frame_1_shape = canvas_load_shape(&player_frame_1_shape_data);
    player_frame_2_shape = canvas_load_shape(&player_frame_2_shape_data);

    for (i = 0; i < NUM_ASTEROID_SHAPES; i++) {
        asteroid_shapes[i] = canvas_load_shape(&asteroid_shape_data[i]);
    }

    for (i = 0; i < NUM_ASTEROIDS; i++) {
        asteroid_init(&asteroids[i]);
    }

    player.rot = 0;
    player.pos.x = 0;
    player.pos.y = 0;
    player.vel.x = 0;
    player.vel.y = 0;
}

void level_loop(bool draw)
{
    level_update();

    if (draw) {
        level_draw();
    }
}
