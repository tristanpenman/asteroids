#ifndef ASTEROIDS_GAME_GAME_H
#define ASTEROIDS_GAME_GAME_H

#include "types.h"

extern int explosion_channel;
extern int phaser_channel;
extern int thruster_channel;

bool game_init(bool silent);
void game_play(bool sandbox);
void game_cleanup(void);

#endif
