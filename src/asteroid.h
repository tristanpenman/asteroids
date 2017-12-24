#ifndef __ASTEROID_H
#define __ASTEROID_H

struct asteroid;

float calculate_asteroid_radius(unsigned int shape);
void init_asteroid(struct asteroid *a);
void randomise_asteroid_rotation(struct asteroid *a);
void randomise_asteroid_velocity(struct asteroid *a, float vel_scale);
void update_asteroids(struct asteroid *aa, unsigned int n, float f);

#endif
