#include <stdio.h>
#include <string.h>

#include "canvas.h"
#include "defines.h"
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
static int input_up;
static int input_down;
static int input_next;

/******************************************************************************
 *
 * Helper functions
 *
 *****************************************************************************/

static void draw_score(int score)
{
    static char buffer[SCORE_BUFFER_SIZE];

    sprintf(buffer, "%u", score);

#ifdef N64
    canvas_draw_text(buffer, -0.45f, -0.33f, 0.65f);
#else
    canvas_draw_text(buffer, -0.45f, -0.33f, 0.35f);
#endif
}

static void handle_input()
{
    int i;

    bool done = false;

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

        if (current_initial < 3) {
            if (input_triggered(input_up)) {
                if (initials[current_initial] == ' ' || initials[current_initial] == '_' || initials[current_initial] == 'A') {
                    initials[current_initial] = 'Z';
                } else {
                    initials[current_initial]--;
                }
            }

            if (input_triggered(input_down)) {
                if (initials[current_initial] == ' ' || initials[current_initial] == '_' || initials[current_initial] == 'Z') {
                    initials[current_initial] = 'A';
                } else {
                    initials[current_initial]++;
                }
            }
        }

        if (input_triggered(input_next)) {
            if (initials[current_initial] != ' ' && initials[current_initial] != '_') {
                ++current_initial;
            }
        }
    } else if (input_triggered(input_next)) {
        done = true;
    }

    if (current_initial > 0 && input_triggered(input_backspace)) {
        initials[--current_initial] = '_';
        reset_simulation_time();
        for (i = current_initial + 1; i < 3; ++i) {
            initials[i] = ' ';
        }

        return;
    }

    if (!done) {
        done = current_initial == 3 && input_triggered(input_done);
    }

    if (done) {
        highscores_insert(score, initials);
        highscores_save();
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
    input_map(input_backspace, INPUT_BUTTON_B);

    input_done = input_register();
    input_map(input_done, INPUT_KEY_ENTER);
    input_map(input_done, INPUT_KEY_RETURN);

    input_escape = input_register();
    input_map(input_escape, INPUT_KEY_ESCAPE);

    for (i = 0; i < 26; i++) {
        input_letters[i] = input_register();
        input_map(input_letters[i], INPUT_KEY_A + i);
    }

    input_up = input_register();
    input_map(input_up, INPUT_DPAD_UP);

    input_down = input_register();
    input_map(input_down, INPUT_DPAD_DOWN);

    input_next = input_register();
    input_map(input_next, INPUT_BUTTON_A);

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
    draw_score(score);

#ifdef N64
    canvas_draw_text_centered("YOUR SCORE IS ONE OF THE TEN BEST", -0.19f, 0.6f);
    canvas_draw_text_centered("PLEASE ENTER YOUR INITIALS", -0.1f, 0.6f);
    canvas_draw_text_centered(initials, 0, 0.65f);
#else
    canvas_draw_text_centered("YOUR SCORE IS ONE OF THE TEN BEST", -0.13f, 0.25f);
    canvas_draw_text_centered("PLEASE ENTER YOUR INITIALS", -0.095f, 0.25f);
    canvas_draw_text_centered(initials, 0, 0.38f);
    canvas_draw_text_centered("PRESS ENTER TO CONTINUE", 0.12f, 0.25f);
    canvas_draw_text_centered("OR BACKSPACE TO MAKE CHANGES", 0.155f, 0.25f);
#endif

    canvas_finish_drawing(true);
}
