#ifndef __GAME_H
#define __GAME_H

#include "types.h"

bool game_init(bool silent);
void game_play(bool sandbox);
void game_cleanup();

#endif
