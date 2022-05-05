#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "canvas.h"
#include "debug.h"
#include "draw.h"
#include "highscores.h"
#include "input.h"
#include "loop.h"
#include "options.h"
#include "storage.h"
#include "titlescreen.h"

#define HIGHSCORES_BUFFER_SIZE 96

static struct highscores scores;

static int input_return;

void load_highscores()
{
    int current_entry;
    int result;
    char buffer[HIGHSCORES_BUFFER_SIZE];
    char *p;
    struct score *current_score;
    int n;

    // Clear highscores data structure
    memset(&scores, 0, sizeof(struct highscores));
    for (current_entry = 0; current_entry < 10; ++current_entry) {
        memcpy(&scores.entries[current_entry].initials, "---", sizeof(char) * 4);
    }

    result = storage_read(HIGHSCORES_FILE, buffer, 0, HIGHSCORES_BUFFER_SIZE);
    if (result < STORAGE_OK) {
        debug_printf("Failed to read high scores: %d\n", result);
        return;
    }

    p = buffer;
    current_entry = 0;
    while (current_entry < 10) {
        current_score = &scores.entries[current_entry];
        result = sscanf(p, "%3[A-Z0-9 ],%u%n", current_score->initials, &current_score->score, &n);
        if (result != 2) {
            debug_printf("failed to parse high score entry %d\n", current_entry);
            break;
        }

        p += n;
        current_score->used = true;
        current_entry++;
    }
}

void dump_highscores()
{
    int current_entry;
    int result;
    char buffer[HIGHSCORES_BUFFER_SIZE];
    char *p;

    memset(buffer, 0, HIGHSCORES_BUFFER_SIZE);

    p = buffer;
    for (current_entry = 0; current_entry < 10 && scores.entries[current_entry].used; ++current_entry) {
        result = sprintf(p, "%s,%u\n", scores.entries[current_entry].initials, scores.entries[current_entry].score);
        if (result < 0) {
            fprintf(stderr, "Error while preparing high score data: %s\n", strerror(errno));
            break;
        }

        p += result;
    }

    result = storage_write(HIGHSCORES_FILE, buffer, 0, HIGHSCORES_BUFFER_SIZE);
    if (result < STORAGE_OK) {
        debug_printf("Failed to write high scores: %d\n", result);
    }
}

bool is_high_score(unsigned int score)
{
    for (int i = 0; i < 10; ++i) {
        if (!scores.entries[i].used || score > scores.entries[i].score) {
            return true;
        }
    }

    return false;
}

void insert_new_high_score(unsigned int score, const char initials[4])
{
    int i, j;

    for (i = 0; i < 10; ++i) {
        if (!scores.entries[i].used || scores.entries[i].score <= score) {
            for (j = 9; j > i; --j) {
                memcpy(&scores.entries[j], &scores.entries[j - 1], sizeof(struct score));
            }
            memcpy(&scores.entries[i].initials, initials, sizeof(char) * 4);
            scores.entries[i].score = score;
            scores.entries[i].used = true;
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
    draw_highscores(&scores);
    canvas_finish_drawing(true);
}
