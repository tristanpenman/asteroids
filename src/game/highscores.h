#ifndef ASTEROIDS_GAME_HIGHSCORES_H
#define ASTEROIDS_GAME_HIGHSCORES_H

#include "types.h"

bool highscores_load(void);
bool highscores_save(void);

bool highscores_check(uint32_t score);
bool highscores_insert(uint32_t score, const char initials[4]);
bool highscores_read(uint32_t index, uint32_t *score, char initials[4]);

#endif
