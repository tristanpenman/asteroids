#include <stdlib.h>

#include <SDL.h>

#include "canvas.h"
#include "draw.h"
#include "level.h"
#include "loop.h"
#include "options.h"
#include "timing.h"
#include "video.h"

static unsigned int level = 1;
static unsigned int lives = 3;
static unsigned int score = 0;

static float elapsed = 0.f;

/******************************************************************************
 *
 * Public interface
 *
 *****************************************************************************/

void reset_transition_state(unsigned int next_level, unsigned int next_lives, unsigned int next_score)
{
    level = next_level;
    lives = next_lives;
    score = next_score;
    elapsed = 0.f;
}

void transition_loop()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            exit(EXIT_SUCCESS);
        }
    }

    produce_simulation_time();
    const uint32_t residual = residual_simulation_time();
    while (maybe_consume_simulation_time(residual)) {
        elapsed += residual;
    }

    if (elapsed < START_LEVEL_DELAY_MS) {
        canvas_start_drawing(true);
        draw_score(score);
        draw_lives(lives);
        draw_level_title(level);
        canvas_finish_drawing(true);
    } else {
        reset_level_state(level, lives, score);
        set_main_loop(level_loop);
    }
}
