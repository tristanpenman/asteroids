#ifndef ASTEROIDS_GAME_GAME_H
#define ASTEROIDS_GAME_GAME_H

#include "types.h"

bool game_init(bool silent);
void game_play(bool sandbox);
void game_cleanup(void);

#endif
