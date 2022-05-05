#ifndef __RUMBLE_H
#define __RUMBLE_H

#include "types.h"

void rumble_init();

bool rumble_available();

void rumble_start(float time, float intensity);

#endif
