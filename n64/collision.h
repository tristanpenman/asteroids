#ifndef __COLLISION_H
#define __COLLISION_H

#include <stdbool.h>

struct asteroid;
struct bullet;
struct player;

bool test_asteroid_bullet_collision(struct bullet* b, struct asteroid *a);
bool test_asteroid_ship_collision(struct player *p, struct asteroid *a);

#endif
