#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "canvas.h"
#include "draw.h"
#include "highscores.h"
#include "input.h"
#include "loop.h"
#include "options.h"
#include "titlescreen.h"
#include "video.h"

#define HIGHSCORES_BUFFER_SIZE 100

static struct highscores scores;

static int input_return;

void load_highscores()
{
    int current_entry = 0;
    int result = 0;
    char buffer[HIGHSCORES_BUFFER_SIZE];
    struct score *current_score;
    FILE *file;

    // Clear highscores data structure
    memset(&scores, 0, sizeof(struct highscores));
    for (current_entry = 0; current_entry < 10; ++current_entry) {
        memcpy(&scores.entries[current_entry].initials, "---", sizeof(char) * 4);
    }

    // Attempt to open high-scores file
    file = fopen(HIGHSCORES_FILE, "r");
    if (!file) {
        if (ENOENT != errno) {
            fprintf(stderr, "Failed to open high scores file: %s\n", strerror(errno));
        }
        return;
    }

    current_entry = 0;
    while (!feof(file) && !ferror(file) && current_entry < 10) {
        // Read up to [HIGHSCORES_BUFFER_SIZE - 1] characters into buffer
        buffer[0] = '\0';
        if (NULL == fgets(buffer, HIGHSCORES_BUFFER_SIZE, file)) {
            if (ferror(file)) {
                fprintf(stderr, "Failed to read line %d of %s: %s",
                    current_entry + 1, HIGHSCORES_FILE, strerror(errno));
                clearerr(file);
            }
            break;
        }

        // Parse initials and score
        current_score = &scores.entries[current_entry];
        result = sscanf(buffer, "%3[A-Z0-9 ],%u", current_score->initials, &current_score->score);
        if (result != 2) {
            fprintf(stderr, "Skipping line %d of %s due to invalid data.\n",
                current_entry + 1, HIGHSCORES_FILE);
            continue;
        }

        current_score->used = true;
        current_entry++;
    }

    if (ferror(file)) {
        fprintf(stderr, "Failed to read high scores file: %s\n", strerror(errno));
    }

    fclose(file);
}

void dump_highscores()
{
    FILE *file = fopen(HIGHSCORES_FILE, "w");
    if (!file) {
        fprintf(stderr, "Could not open high scores file: %s\n", strerror(errno));
        return;
    }

    for (int current_entry = 0; current_entry < 10 && scores.entries[current_entry].used; ++current_entry) {
        fprintf(file, "%s,%u\n", scores.entries[current_entry].initials, scores.entries[current_entry].score);
        if (ferror(file)) {
            fprintf(stderr, "Error while writing high scores file: %s\n", strerror(errno));
            break;
        }
    }

    fclose(file);
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
