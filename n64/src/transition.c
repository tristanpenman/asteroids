#include <stdio.h>
#include <stdlib.h>

#include "canvas.h"
#include "defines.h"
#include "input.h"
#include "level.h"
#include "loop.h"
#include "timing.h"

static int level = 1;
static int lives = 3;
static int score = 0;

static float elapsed = 0.f;

static char titlecard[100];

/******************************************************************************
 *
 * Public interface
 *
 *****************************************************************************/

void transition_init(int next_level, int next_lives, int next_score)
{
    canvas_reset();
    input_reset();

    level = next_level;
    lives = next_lives;
    score = next_score;
    elapsed = 0.f;

    sprintf(titlecard, "LEVEL %u", next_level);
}

void transition_loop(bool draw)
{
    uint32_t residual;

    input_update();

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
    canvas_draw_text_centered(titlecard, -0.05f, 0.65f);
    canvas_finish_drawing(true);
}
