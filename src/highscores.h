#ifndef __HIGHSCORES_H
#define __HIGHSCORES_H

#include <stdbool.h>

struct highscores;

void load_highscores();
void dump_highscores();

bool is_high_score(unsigned int score);

void highscore_screen_loop();

void insert_new_high_score(unsigned int score, char initials[4]);

#endif
