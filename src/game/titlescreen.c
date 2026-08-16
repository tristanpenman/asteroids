#include <stdlib.h>

#include "canvas.h"
#include "data.h"
#include "debug.h"
#include "draw.h"
#include "entities.h"
#include "input.h"
#include "leaderboard.h"
#include "loop.h"
#include "timing.h"
#include "titlescreen.h"
#include "transition.h"
#include "vec.h"

#define NUM_ASTEROID_SHAPES 4
#define NUM_ASTEROIDS 5
#define TIME_STEP_MILLIS 10

static bool enter_down;
static bool h_down;
static struct asteroid asteroids[NUM_ASTEROIDS];

static int asteroid_shape_ids[NUM_ASTEROID_SHAPES];

static int input_leaderboard;
static int input_start;
static int input_quit;

/******************************************************************************
 *
 * Helper functions
 *
 *****************************************************************************/

static void draw_instructions(void)
{
#ifdef ASTEROIDS_PLATFORM_N64
    canvas_draw_text_centered("PRESS START TO PLAY", -0.07f, 0.3f);
#else
    canvas_draw_text_centered("PRESS ENTER TO PLAY", -0.07f, 0.3f);
    canvas_draw_text_centered("SPACE - FIRE", 0.025f, 0.20f);
    canvas_draw_text_centered("ARROWS - DIRECTION", 0.055f, 0.20f);
    canvas_draw_text_centered("UP - THRUSTER", 0.085f, 0.20f);
#ifndef __EMSCRIPTEN__
    canvas_draw_text_centered("ESC - EXIT", 0.115f, 0.20f);
    canvas_draw_text_centered("PRESS H FOR HIGH SCORES", 0.20f, 0.3f);
#endif
#endif
}

/******************************************************************************
 *
 * Public interface
 *
 *****************************************************************************/

bool titlescreen_init(void)
{
    input_reset();

    // high scores action
    input_leaderboard = input_register();
    debug_assert(input_leaderboard != INPUT_INVALID_HANDLE);
#ifndef __EMSCRIPTEN__
    input_map(input_leaderboard, INPUT_KEY_H);
#endif

    // start action
    input_start = input_register();
    debug_assert(input_start != INPUT_INVALID_HANDLE);
    input_map(input_start, INPUT_KEY_ENTER);
    input_map(input_start, INPUT_KEY_RETURN);
    input_map(input_start, INPUT_BUTTON_START);

    // quit aciton
    input_quit = input_register();
    debug_assert(input_quit != INPUT_INVALID_HANDLE);
#ifndef __EMSCRIPTEN__
    input_map(input_quit, INPUT_KEY_ESCAPE);
#endif

    // reset button states
    enter_down = false;
    h_down = false;

    canvas_reset();

    for (unsigned int i = 0; i < NUM_ASTEROID_SHAPES; ++i) {
        asteroid_shape_ids[i] = canvas_load_shape(&asteroid_shape_data[i]);
        if (asteroid_shape_ids[i] == CANVAS_INVALID_SHAPE) {
            return false;
        }
    }

    for (unsigned int i = 0; i < NUM_ASTEROIDS; ++i) {
        asteroid_init(&asteroids[i]);
    }

    return true;
}

void titlescreen_loop(bool draw)
{
    float residual;

    input_update();

#ifndef ASTEROIDS_PLATFORM_N64
    if (input_active(input_leaderboard)) {
        h_down = true;
    } else if (h_down) {
        leaderboard_init();
        set_main_loop(leaderboard_loop);
        return;
    } else {
        h_down = false;
    }
#endif

    if (input_active(input_start)) {
        enter_down = true;
    } else if (enter_down) {
        transition_init(1, 3, 0);
        set_main_loop(transition_loop);
        return;
    } else {
        enter_down = false;
    }

#ifndef ASTEROIDS_PLATFORM_N64
    if (input_active(input_quit)) {
        exit(EXIT_SUCCESS);
    }
#endif

    // Update and consume unused simulation time
    produce_simulation_time();
    while (maybe_consume_simulation_time(TIME_STEP_MILLIS)) {
        for (int i = 0; i < NUM_ASTEROIDS; i++) {
            asteroid_update(&asteroids[i], TIME_STEP_MILLIS / 1000.f);
        }
    }

    if (!draw) {
        return;
    }

    // Unused simulation time, used to smooth animation
    residual = (float) residual_simulation_time() / 1000.f;
    canvas_start_drawing(true);

    for (int i = 0; i < NUM_ASTEROIDS; i++) {
        const struct vec_2d position = {
            asteroids[i].pos.x + asteroids[i].vel.x * residual,
            asteroids[i].pos.y + asteroids[i].vel.y * residual
        };

        canvas_draw_shape(
            asteroid_shape_ids[asteroids[i].shape],
            position,
            asteroids[i].rot,
            vec_2d_unit);
    }

    canvas_draw_text_centered("ASTEROIDS", -0.20f, 0.8f);

    draw_instructions();

    canvas_finish_drawing(true);
}
