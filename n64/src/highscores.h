#ifndef __HIGHSCORES_H
#define __HIGHSCORES_H

#include "types.h"

#define NUM_SCORES 10

struct score
{
    char initials[4];
    uint32_t score;
};

struct highscores
{
    struct score entries[NUM_SCORES];
};

bool highscores_load();
void highscores_save();

bool is_high_score(uint32_t score);

void insert_new_high_score(uint32_t score, const char initials[4]);

void highscores_init();

void highscores_loop(bool draw);

#endif
