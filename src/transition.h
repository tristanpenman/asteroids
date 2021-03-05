#ifndef __TRANSITION_H
#define __TRANSITION_H

#include "types.h"

void transition_init(unsigned int next_level, unsigned int lives, unsigned int score);

void transition_loop(bool draw);

#endif
