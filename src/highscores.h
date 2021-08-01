#ifndef __HIGHSCORES_H
#define __HIGHSCORES_H

#include "types.h"

struct score
{
    bool used;
    char initials[4];
    unsigned int score;
};

struct highscores
{
    struct score entries[10];
};

void load_highscores();
void dump_highscores();

bool is_high_score(unsigned int score);

void insert_new_high_score(unsigned int score, const char initials[4]);

void highscores_init();

void highscores_loop(bool draw);

#endif
