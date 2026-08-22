#include <stdio.h>

#include "canvas.h"
#include "defines.h"
#include "highscores.h"
#include "input.h"
#include "leaderboard.h"
#include "loop.h"
#include "shape.h"
#include "text.h"
#include "timing.h"
#include "titlescreen.h"

static int input_return;

/******************************************************************************
 *
 * Helper functions
 *
 *****************************************************************************/

static void leaderboard_draw(void)
{
    uint32_t score;
    char initials[4];
    char str[100];

    if (!shape_canvas_begin_frame()) {
        return;
    }

    for (int i = 0; i < NUM_SCORES; i++) {
        if (highscores_read(i, &score, initials) && initials[0] >= 'A' && initials[0] <= 'Z') {
            sprintf(str, "%2d   %.3s %10d ", i + 1, initials, score);
        } else {
            sprintf(str, "%2d   ---          - ", i + 1);
        }

        text_draw_centered(str, -0.3f + 0.054f * (float) i, 0.45f);

    }

#ifdef N64
    text_draw_centered("PRESS START FOR MAIN MENU", 0.27f, 0.45f);
#else
    text_draw_centered("PRESS ENTER FOR MAIN MENU", 0.27f, 0.35f);
#endif

    shape_canvas_end_frame();
}

/******************************************************************************
 *
 * Public interface
 *
 *****************************************************************************/

void leaderboard_init(void)
{
    input_reset();

    input_return = input_register();
    input_map(input_return, INPUT_BUTTON_A);
    input_map(input_return, INPUT_BUTTON_B);
    input_map(input_return, INPUT_BUTTON_START);
    input_map(input_return, INPUT_KEY_ENTER);
    input_map(input_return, INPUT_KEY_ESCAPE);
    input_map(input_return, INPUT_KEY_RETURN);
}

void leaderboard_loop(bool draw)
{
    input_update();

    if (input_active(input_return)) {
        titlescreen_init();
        set_main_loop(titlescreen_loop);
        return;
    }

    if (!draw) {
        return;
    }

    leaderboard_draw();
}
