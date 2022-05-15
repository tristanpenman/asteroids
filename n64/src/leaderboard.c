#include "canvas.h"
#include "input.h"
#include "leaderboard.h"
#include "loop.h"
#include "timing.h"
#include "titlescreen.h"

static int input_return;

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

    // TODO

    canvas_finish_drawing(true);
}
