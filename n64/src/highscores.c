#include <string.h>

#include "canvas.h"
#include "debug.h"
#include "defines.h"
#include "highscores.h"
#include "input.h"
#include "loop.h"
#include "storage.h"
#include "titlescreen.h"

#define HIGHSCORES_BUFFER_SIZE 128

static struct highscores scores;

static int input_return;

bool highscores_load()
{
    char buffer[HIGHSCORES_BUFFER_SIZE];
    int current_entry;
    int result;

    // clear highscores data structure
    memset(&scores, 0, sizeof(struct highscores));
    for (current_entry = 0; current_entry < 10; ++current_entry) {
        memcpy(&scores.entries[current_entry].initials, "---", sizeof(char) * 4);
    }

    // attempt to load high scores
    result = storage_read(HIGHSCORES_FILE, buffer, 0, HIGHSCORES_BUFFER_SIZE);
    if (result != STORAGE_OK) {
        if (result == STORAGE_ERR_OPEN_FILE) {
            debug_printf(" - high scores not available\n");
        } else {
            debug_printf(" - failed to read high scores: %d\n", result);
        }
        return false;
    }

    memcpy(&scores, buffer, sizeof(struct highscores));
    return true;
}

void highscores_save()
{
    char buffer[HIGHSCORES_BUFFER_SIZE];
    int result;

    memset(buffer, 0, HIGHSCORES_BUFFER_SIZE);
    memcpy(buffer, &scores, sizeof(struct highscores));

    result = storage_write(HIGHSCORES_FILE, buffer, 0, HIGHSCORES_BUFFER_SIZE);
    if (result < STORAGE_OK) {
        debug_printf(" - failed to write high scores: %d\n", result);
    }
}

bool is_high_score(uint32_t score)
{
    int i;

    for (i = 0; i < 10; ++i) {
        if (!scores.entries[i].initials[0] != '-' || score > scores.entries[i].score) {
            return true;
        }
    }

    return false;
}

void insert_new_high_score(uint32_t score, const char initials[4])
{
    int i, j;

    for (i = 0; i < 10; ++i) {
        if (!scores.entries[i].initials[0] != '-' || scores.entries[i].score <= score) {
            for (j = 9; j > i; --j) {
                memcpy(&scores.entries[j], &scores.entries[j - 1], sizeof(struct score));
            }
            memcpy(&scores.entries[i].initials, initials, sizeof(char) * 4);
            scores.entries[i].score = score;
            break;
        }
    }
}

void highscores_init()
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

void highscores_loop(bool draw)
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
    // draw_highscores(&scores);
    canvas_finish_drawing(true);
}
