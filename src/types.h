#ifndef __TYPES_H
#define __TYPES_H

#include <stdbool.h>

#include "options.h"
#include "vec.h"

enum asize
{
    ASIZE_LARGE  = 1,
    ASIZE_MEDIUM = 2,
    ASIZE_SMALL  = 4
};

struct asteroid
{
    bool visible;
    struct vec_2d pos;
    struct vec_2d pos_prev;
    struct vec_2d vel;
    float radius;
    float rot;
    enum asize size;
    int shape;
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

struct bullet
{
    bool visible;
    struct vec_2d pos_prev;
    struct vec_2d pos;
    struct vec_2d vel;
    float travelled;
};

struct explosion
{
    bool visible;
    struct vec_2d pos;
    float time;
};

struct score
{
    bool used;
    char initials[4];
    unsigned int score;
};

struct highscores
{
    struct score entries[10];
};

#endif
