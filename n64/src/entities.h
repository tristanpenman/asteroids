#ifndef __ENTITIES_H
#define __ENTITIES_H

#include "defines.h"
#include "types.h"
#include "vec.h"

struct explosion
{
    bool visible;
    struct vec_2d pos;
    float time;
};

struct asteroid
{
    bool visible;
    struct vec_2d pos;
    struct vec_2d pos_prev;
    struct vec_2d vel;
    float radius;
    float rot;
    float scale;
    int shape;
};

struct bullet
{
    bool visible;
    struct vec_2d pos_prev;
    struct vec_2d pos;
    struct vec_2d vel;
    float rot;
    float travelled;
};

enum keystate
{
    KS_UP = 0,
    KS_DOWN = 1,
    KS_ACTIVE = 2
};

struct player_keys
{
    enum keystate left;
    enum keystate right;
    enum keystate up;
    enum keystate fire;
};

struct shard
{
    float angle;
    float rot;
    int dir;
};

enum player_state
{
    PS_NORMAL,
    PS_EXPLODING
};

struct player
{
    bool reloading;
    enum player_state state;
    unsigned int score;
    unsigned int hit;
    int lives;
    float phase;
    float reload_delay;
    float death_delay;
    float rot;
    struct vec_2d pos;
    struct vec_2d pos_prev;
    struct vec_2d vel;
    struct player_keys keys;
    struct shard shards[SHIP_EXPLOSION_SHARDS];
};

float calculate_asteroid_radius(unsigned int shape);
void randomise_asteroid_rotation(struct asteroid *a);
void randomise_asteroid_velocity(struct asteroid *a, float vel_scale);

void asteroid_init(struct asteroid *a);
void asteroid_update(struct asteroid *a, float f, const struct vec_2d *lim);

void bullet_init(struct bullet *b, const struct vec_2d *pos, float r);
void bullet_update(struct bullet *b, float f);

void player_init(struct player *p);
void player_reset(struct player *p);

#endif
