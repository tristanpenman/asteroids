#include <string.h>

#include "canvas.h"
#include "draw.h"
#include "highscores.h"
#include "input.h"
#include "loop.h"
#include "timing.h"
#include "titlescreen.h"

static int current_initial;
static char initials[4];
static unsigned int score;

static int input_backspace;
static int input_done;
static int input_escape;
static int input_letters[26];

static void handle_input()
{
    int i;

    if (input_triggered(input_escape)) {
        titlescreen_init();
        set_main_loop(titlescreen_loop);
        return;
    }

    if (current_initial < 3) {
        for (i = 0; i < 26; i++) {
            if (!input_triggered(input_letters[i])) {
                continue;
            }

            initials[current_initial] = (char)(i + 'A');
            reset_simulation_time();
            current_initial++;
            if (current_initial < 3) {
                initials[current_initial] = '_';
            }

            return;
        }
    }

    if (current_initial > 0 && input_triggered(input_backspace)) {
        initials[--current_initial] = '_';
        reset_simulation_time();
        for (i = current_initial + 1; i < 3; ++i) {
            initials[i] = ' ';
        }

        return;
    }

    if (current_initial == 3 && input_triggered(input_done)) {
        insert_new_high_score(score, initials);
        dump_highscores();
        titlescreen_init();
        set_main_loop(titlescreen_loop);
        return;
    }
}

/******************************************************************************
 *
 * Public interface
 *
 *****************************************************************************/

void initials_init(unsigned int new_score)
{
    int i;

    canvas_reset();

    input_reset();

    input_backspace = input_register();
    input_map(input_backspace, INPUT_KEY_BACKSPACE);
    input_map(input_backspace, INPUT_KEY_DELETE);

    input_done = input_register();
    input_map(input_done, INPUT_KEY_ENTER);
    input_map(input_done, INPUT_KEY_RETURN);

    input_escape = input_register();
    input_map(input_escape, INPUT_KEY_ESCAPE);

    for (i = 0; i < 26; i++) {
        input_letters[i] = input_register();
        input_map(input_letters[i], INPUT_KEY_A + i);
    }

    current_initial = 0;
    memcpy(initials, "_  ", 4);
    score = new_score;
}

void initials_loop(bool draw)
{
    input_update();

    handle_input();

    produce_simulation_time();
    if (maybe_consume_simulation_time(500)) {
        reset_simulation_time();
        if (initials[current_initial] == ' ') {
            initials[current_initial] = '_';
        } else if (initials[current_initial] == '_') {
            initials[current_initial] = ' ';
        }
    }

    if (!draw) {
        return;
    }

    canvas_start_drawing(true);
    draw_new_high_score_message();
    draw_new_high_score_input(initials);
    if (current_initial == 3) {
        draw_new_high_score_enter_to_continue();
    }
    draw_score(score);
    canvas_finish_drawing(true);
}
