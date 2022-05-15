#include <stdio.h>

#include "canvas.h"
#include "defines.h"
#include "highscores.h"
#include "input.h"
#include "leaderboard.h"
#include "loop.h"
#include "timing.h"
#include "titlescreen.h"

static int input_return;

void leaderboard_draw()
{
    int i;
    uint32_t score;
    char initials[4];
    char str[100];

    canvas_draw_text_centered("LEADERBOARD", -0.21f, 0.39f);

    for (i = 0; i < NUM_SCORES; i++) {
        if (highscores_read(i, &score, initials) && initials[0] != '-') {
            sprintf(str, "%2d   %.3s %10d ", i + 1, initials, score);
        } else {
            sprintf(str, "%2d   ---          - ", i + 1);
        }

        canvas_draw_text_centered(str, -0.14f + 0.03f * (float) i, 0.24f);
    }

    canvas_draw_text_centered("PRESS ENTER FOR MAIN MENU",  0.26f, 0.24f);
}

/******************************************************************************
 *
 * Public interface
 *
 *****************************************************************************/

void leaderboard_init()
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

    canvas_start_drawing(true);
    leaderboard_draw();
    canvas_finish_drawing(true);
}
