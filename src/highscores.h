#ifndef __HIGHSCORES_H
#define __HIGHSCORES_H

#include "types.h"

bool highscores_load();
bool highscores_save();

bool highscores_check(uint32_t score);
bool highscores_insert(uint32_t score, const char initials[4]);
bool highscores_read(uint32_t index, uint32_t *score, char initials[4]);

#endif
