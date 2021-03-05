#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "canvas.h"
#include "draw.h"
#include "highscores.h"
#include "loop.h"
#include "timing.h"
#include "titlescreen.h"
#include "video.h"

static int current_initial;
static char initials[4];
static unsigned int score;

void initials_init(unsigned int new_score)
{
    current_initial = 0;
    memcpy(initials, "_  ", 4);
    score = new_score;
}

void initials_loop(bool draw)
{
    int i = 0;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_KEYUP:
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                titlescreen_init();
                set_main_loop(titlescreen_loop);
                return;
            } else if (current_initial == 3 && (
                    event.key.keysym.sym == SDLK_KP_ENTER ||
                    event.key.keysym.sym == SDLK_RETURN)) {
                insert_new_high_score(score, initials);
                dump_highscores();
                titlescreen_init();
                set_main_loop(titlescreen_loop);
                return;
            } else if (current_initial < 3 &&
                    event.key.keysym.sym >= SDLK_a &&
                    event.key.keysym.sym <= SDLK_z) {
                initials[current_initial] = event.key.keysym.sym - 32;
                reset_simulation_time();
                current_initial++;
                if (current_initial < 3) {
                    initials[current_initial] = '_';
                }
            } else if (current_initial > 0 && event.key.keysym.sym == SDLK_BACKSPACE) {
                initials[--current_initial] = '_';
                reset_simulation_time();
                for (i = current_initial + 1; i < 3; ++i) {
                    initials[i] = ' ';
                }
            }
            break;
        case SDL_QUIT:
            exit(EXIT_SUCCESS);
        }
    }

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
