#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "canvas.h"
#include "data.h"
#include "entities.h"
#include "input.h"
#include "loop.h"
#include "timing.h"
#include "transition.h"
#include "vec.h"

#define NUM_ASTEROID_SHAPES 4
#define NUM_ASTEROIDS 5
#define TIME_STEP_MILLIS 10

static bool enter_down;
static bool h_down;
static struct asteroid asteroids[NUM_ASTEROIDS];

static int asteroid_shape_ids[NUM_ASTEROID_SHAPES];

static int input_highscores;
static int input_start;
static int input_quit;

/******************************************************************************
 *
 * Public interface
 *
 *****************************************************************************/

bool titlescreen_init()
{
    unsigned int i;

    input_reset();

    // high scores action
    input_highscores = input_register();
    assert(input_highscores != INPUT_INVALID_HANDLE);
#ifndef __EMSCRIPTEN__
    input_map(input_highscores, INPUT_KEY_H);
#endif

    // start action
    input_start = input_register();
    assert(input_start != INPUT_INVALID_HANDLE);
    input_map(input_start, INPUT_KEY_ENTER);
    input_map(input_start, INPUT_KEY_RETURN);
    input_map(input_start, INPUT_BUTTON_START);

    // quit aciton
    input_quit = input_register();
    assert(input_quit != INPUT_INVALID_HANDLE);
#ifndef __EMSCRIPTEN__
    input_map(input_quit, INPUT_KEY_ESCAPE);
#endif

    // reset button states
    enter_down = false;
    h_down = false;

    canvas_reset();

    for (i = 0; i < NUM_ASTEROID_SHAPES; ++i) {
        asteroid_shape_ids[i] = canvas_load_shape(&asteroid_shape_data[i]);
        if (asteroid_shape_ids[i] == CANVAS_INVALID_SHAPE) {
            return false;
        }
    }

    for (i = 0; i < NUM_ASTEROIDS; ++i) {
        asteroid_init(&asteroids[i]);
    }

    return true;
}

void titlescreen_loop(bool draw)
{
    int i;

    int8_t joystick_x;

    input_update();
    input_read_joystick(&joystick_x, NULL);

    if (input_active(input_highscores)) {
        h_down = true;
    } else if (h_down) {
        // highscores_init();
        // set_main_loop(highscores_loop);
        return;
    } else {
        h_down = false;
    }

    if (input_active(input_start) || joystick_x < -20) {
        enter_down = true;
    } else if (enter_down) {
        transition_init(1, SHIP_LIVES, 0);
        set_main_loop(transition_loop);
        return;
    } else {
        enter_down = false;
    }

    if (input_active(input_quit)) {
        // exit(EXIT_SUCCESS);
    }

    // Update and consume unused simulation time
    produce_simulation_time();
    while (maybe_consume_simulation_time(TIME_STEP_MILLIS)) {
        for (i = 0; i < NUM_ASTEROIDS; i++) {
            struct vec_2d lim;
            lim.x = 1.f / 2.f + asteroids[i].radius;
            lim.y = 0.75f / 2.f + asteroids[i].radius;
            asteroid_update(&asteroids[i], TIME_STEP_MILLIS / 1000.f, &lim);
        }
    }

    if (!draw) {
        return;
    }

    canvas_start_drawing(true);

    for (i = 0; i < NUM_ASTEROIDS; i++) {
        canvas_draw_shape(
            asteroid_shape_ids[asteroids[i].shape],
            asteroids[i].pos,
            asteroids[i].rot,
            vec_2d_unit);
    }

    canvas_draw_text_centered("ASTEROIDS", 1.1f, -0.20f, FONT_SPACE);

    canvas_draw_text_centered("PRESS START", 0.6f, 0, FONT_SPACE);

    canvas_finish_drawing(true);
}
