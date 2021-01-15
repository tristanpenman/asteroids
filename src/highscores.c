#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "canvas.h"
#include "draw.h"
#include "loop.h"
#include "options.h"
#include "titlescreen.h"
#include "types.h"
#include "video.h"

#define HIGHSCORES_BUFFER_SIZE 100

static struct highscores scores;

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

void highscore_screen_loop()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_KEYUP:
            if (event.key.keysym.sym == SDLK_KP_ENTER ||
                event.key.keysym.sym == SDLK_RETURN) {
                reset_titlescreen_state();
                set_main_loop(titlescreen_loop);
                return;
            }
            break;
        case SDL_QUIT:
            exit(EXIT_SUCCESS);
        }
    }

    canvas_start_drawing(true);
    draw_highscores(&scores);
    canvas_finish_drawing(true);
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

