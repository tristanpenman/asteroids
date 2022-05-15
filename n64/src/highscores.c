#include <string.h>

#include "debug.h"
#include "defines.h"
#include "highscores.h"
#include "storage.h"

/******************************************************************************
 *
 * Definitions
 *
 *****************************************************************************/

#define ROUND_UP(N, S) ((((N) + (S) - 1) / (S)) * (S))

#define HIGHSCORES_BUFFER_SIZE ROUND_UP(sizeof(struct highscores), 32)

struct score
{
    char initials[4];
    uint32_t score;
};

struct highscores
{
    struct score entries[NUM_SCORES];
};

static struct highscores scores;

/******************************************************************************
 *
 * Public interface
 *
 *****************************************************************************/

bool highscores_load()
{
    char buffer[HIGHSCORES_BUFFER_SIZE];
    int current_entry;
    int result;

    // clear highscores data structure
    memset(&scores, 0, sizeof(struct highscores));
    for (current_entry = 0; current_entry < NUM_SCORES; ++current_entry) {
        memcpy(&scores.entries[current_entry].initials, "---", sizeof(char) * 4);
    }

    // attempt to load high scores
    result = storage_read(HIGHSCORES_FILE, buffer, sizeof(struct highscores));
    if (result < STORAGE_OK) {
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

bool highscores_save()
{
    char buffer[HIGHSCORES_BUFFER_SIZE];
    int result;

    memset(buffer, 0, HIGHSCORES_BUFFER_SIZE);
    memcpy(buffer, &scores, sizeof(struct highscores));

    result = storage_write(HIGHSCORES_FILE, buffer, HIGHSCORES_BUFFER_SIZE);
    if (result < STORAGE_OK) {
        debug_printf(" - failed to write high scores: %d\n", result);
        return false;
    }

    return true;
}

bool highscores_check(uint32_t score)
{
    int i;

    for (i = 0; i < NUM_SCORES; ++i) {
        if (scores.entries[i].initials[0] == '-' || score > scores.entries[i].score) {
            return true;
        }
    }

    return false;
}

bool highscores_insert(uint32_t score, const char initials[4])
{
    int i, j;

    for (i = 0; i < NUM_SCORES; ++i) {
        if (scores.entries[i].initials[0] == '-' || scores.entries[i].score <= score) {
            for (j = NUM_SCORES - 1; j > i; --j) {
                memcpy(&scores.entries[j], &scores.entries[j - 1], sizeof(struct score));
            }
            memcpy(&scores.entries[i].initials, initials, sizeof(char) * 4);
            scores.entries[i].score = score;
            return true;
        }
    }

    return false;
}

bool highscores_read(uint32_t index, uint32_t *score, char initials[4])
{
    if (index >= NUM_SCORES) {
        return false;
    }

    *score = scores.entries[index].score;
    memcpy(initials, scores.entries[index].initials, 4);
    return true;
}
