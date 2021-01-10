#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <SDL.h>

#include "asteroid.h"
#include "canvas.h"
#include "draw.h"
#include "highscores.h"
#include "level.h"
#include "loop.h"
#include "options.h"
#include "shape.h"
#include "timing.h"
#include "transition.h"
#include "types.h"
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

extern struct shape asteroid_shapes[];

extern struct vec_2d unit;
extern struct vec_2d zero;

/******************************************************************************
 *
 * Public interface
 *
 *****************************************************************************/

bool reset_titlescreen_state()
{
    unsigned int i;

    canvas_reset();

    for (i = 0; i < NUM_ASTEROID_SHAPES; ++i) {
        asteroid_shape_ids[i] = canvas_load_shape(&asteroid_shapes[i]);
        if (asteroid_shape_ids[i] == CANVAS_INVALID_SHAPE) {
            return false;
        }
    }

    enter_down = false;
    h_down = false;
    for (i = 0; i < NUM_ASTEROIDS; ++i) {
        init_asteroid(&asteroids[i]);
    }

    return true;
}

void titlescreen_loop()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_KP_ENTER ||
                event.key.keysym.sym == SDLK_RETURN) {
                enter_down = true;
            } else if (event.key.keysym.sym == SDLK_h) {
                h_down = true;
            }
            break;
        case SDL_KEYUP:
            if (enter_down == true && (
                event.key.keysym.sym == SDLK_KP_ENTER ||
                event.key.keysym.sym == SDLK_RETURN)) {
                enter_down = false;
                reset_transition_state(1, 3, 0);
                set_main_loop(transition_loop);
                return;
#ifndef __EMSCRIPTEN__
            } else if (h_down == true && event.key.keysym.sym == SDLK_h) {
                set_main_loop(highscore_screen_loop);
                return;
            } else if (event.key.keysym.sym == SDLK_ESCAPE) {
                cancel_main_loop(EXIT_SUCCESS);
                return;
#endif
            }
            break;
        case SDL_QUIT:
            exit(EXIT_SUCCESS);
        }
    }

    // Update and consume unused simulation time
    produce_simulation_time();
    while (maybe_consume_simulation_time(TIME_STEP_MILLIS)) {
        update_asteroids(asteroids, NUM_ASTEROIDS, TIME_STEP_MILLIS / 1000.f);
    }

    // Unused simulation time, used to smooth animation
    const float residual = residual_simulation_time() / 1000.f;
    if (residual < 0.f) {
        printf("%f\n", residual);
    }

    video_clear();

    for (int i = 0; i < NUM_ASTEROIDS; i++) {
        const struct vec_2d position = {
          asteroids[i].pos.x + asteroids[i].vel.x * residual,
          asteroids[i].pos.y + asteroids[i].vel.y * residual
        };

        assert(canvas_draw_lines(
            asteroid_shape_ids[asteroids[i].shape],
            position,
            asteroids[i].rot,
            unit
        ));
    }

    // draw_asteroids(asteroids, NUM_ASTEROIDS, true, residual);

    draw_title();
    draw_instructions();
    video_swap();
}
