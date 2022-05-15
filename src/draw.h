#ifndef __DRAW_H
#define __DRAW_H

struct bullet;
struct explosion;
struct highscores;
struct player;

void draw_explosions(const struct explosion *, unsigned int n);
void draw_leaderboard();
void draw_instructions();
void draw_level_title(int level);
void draw_lives(int lives);
void draw_new_high_score_input(const char initials[4]);
void draw_new_high_score_message();
void draw_new_high_score_enter_to_continue();
void draw_player_exploding(const struct player *);
void draw_score(unsigned int score);

#endif
