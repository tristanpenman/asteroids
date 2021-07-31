#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <SDL.h>

#include "canvas.h"
#include "data.h"
#include "draw.h"
#include "entities.h"
#include "highscores.h"
#include "input.h"
#include "level.h"
#include "loop.h"
#include "mathdefs.h"
#include "options.h"
#include "shape.h"
#include "timing.h"
#include "transition.h"
#include "util.h"
#include "vec.h"
#include "video.h"

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
    assert(input_map(input_highscores, INPUT_KEY_H));
#endif

    // start action
    input_start = input_register();
    assert(input_start != INPUT_INVALID_HANDLE);
    assert(input_map(input_start, INPUT_KEY_ENTER));
    assert(input_map(input_start, INPUT_KEY_RETURN));
    assert(input_map(input_start, INPUT_BUTTON_START));

    // quit aciton
    input_quit = input_register();
    assert(input_quit != INPUT_INVALID_HANDLE);
#ifndef __EMSCRIPTEN__
    assert(input_map(input_quit, INPUT_KEY_ESCAPE));
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
    input_update();

    if (input_active(input_highscores)) {
        h_down = true;
    } else if (h_down) {
        set_main_loop(highscores_loop);
        return;
    } else {
        h_down = false;
    }

    if (input_active(input_start)) {
        enter_down = true;
    } else if (enter_down) {
        transition_init(1, 3, 0);
        set_main_loop(transition_loop);
        return;
    } else {
        enter_down = false;
    }

    if (input_active(input_quit)) {
        exit(EXIT_SUCCESS);
    }

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
    const float residual = residual_simulation_time() / 1000.f;
    if (residual < 0.f) {
        printf("%f\n", residual);
    }

    canvas_start_drawing(true);

    for (int i = 0; i < NUM_ASTEROIDS; i++) {
        const struct vec_2d position = {
          asteroids[i].pos.x + asteroids[i].vel.x * residual,
          asteroids[i].pos.y + asteroids[i].vel.y * residual
        };

        assert(canvas_draw_line_segments(
            asteroid_shape_ids[asteroids[i].shape],
            position,
            asteroids[i].rot,
            vec_2d_unit
        ));
    }

    // draw_asteroids(asteroids, NUM_ASTEROIDS, true, residual);

    draw_title();
    draw_instructions();

    canvas_finish_drawing(true);
}
