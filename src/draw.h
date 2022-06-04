#ifndef __DRAW_H
#define __DRAW_H

struct explosion;
struct player;

void draw_explosions(const struct explosion *, unsigned int n);
void draw_level_title(int level);
void draw_lives(int lives);
void draw_player_exploding(const struct player *);
void draw_score(int score);

#endif
