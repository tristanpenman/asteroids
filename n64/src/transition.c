#include <stdlib.h>

#include "canvas.h"
#include "defines.h"
#include "input.h"
#include "level.h"
#include "loop.h"
#include "timing.h"

static unsigned int level = 1;
static unsigned int lives = 3;
static unsigned int score = 0;

static float elapsed = 0.f;

/******************************************************************************
 *
 * Public interface
 *
 *****************************************************************************/

void transition_init(unsigned int next_level, unsigned int next_lives, unsigned int next_score)
{
    level = next_level;
    lives = next_lives;
    score = next_score;
    elapsed = 0.f;
}

void transition_loop(bool draw)
{
    uint32_t residual;

    // input_update();

    produce_simulation_time();
    residual = residual_simulation_time();
    while (maybe_consume_simulation_time(residual)) {
        elapsed += (float) residual;
    }

    if (elapsed >= START_LEVEL_DELAY_MS) {
        level_init(level, lives, score);
        set_main_loop(level_loop);
        return;
    }

    if (!draw) {
        return;
    }

    canvas_start_drawing(true);
    // draw_score(score);
    // draw_lives(lives);
    // draw_level_title(level);
    canvas_finish_drawing(true);
}
