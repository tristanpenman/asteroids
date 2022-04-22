#ifndef __TRANSITION_H
#define __TRANSITION_H

#include "types.h"

void transition_init(int next_level, int lives, int score);

void transition_loop(bool draw);

#endif
