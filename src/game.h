#ifndef __GAME_H
#define __GAME_H

#include <stdbool.h>

bool game_init(bool audio_enabled);
void game_play();
void game_cleanup();

#endif
