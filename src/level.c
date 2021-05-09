#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "canvas.h"
#include "collision.h"
#include "data.h"
#include "defines.h"
#include "draw.h"
#include "entities.h"
#include "loop.h"
#include "mathdefs.h"
#include "mixer.h"
#include "options.h"
#include "shape.h"
#include "timing.h"
#include "titlescreen.h"
#include "transition.h"
#include "util.h"
#include "vec.h"
#include "video.h"

#ifndef __EMSCRIPTEN__
#include "highscores.h"
#include "initials.h"
#endif

#define TIME_STEP_MILLIS 5

extern int explosion_channel;
extern int phaser_channel;
extern int thruster_channel;

// level state
static unsigned int asteroids_hit;
static bool gameover;
static unsigned int level;
static unsigned int next_beat;
static unsigned int starting_asteroids;

// entities
static struct asteroid asteroids[MAX_ASTEROIDS];
static struct bullet bullets[MAX_BULLETS];
static struct explosion explosions[MAX_EXPLOSIONS];
static struct player player;

static float beat_delay;
static float beat_delay_limit;
static float gameover_countdown;
static float next_level_countdown;

// shapes
static int asteroid_shapes[NUM_ASTEROID_SHAPES];

/******************************************************************************
 *
 * Helper functions
 *
 *****************************************************************************/

static unsigned int num_asteroids_for_level(int level) {
    switch (level) {
        case 1:
            return 4;
        case 2:
            return 6;
        case 3:
            return 8;
        case 4:
            return 10;
        default:
            break;
    }

    return 11;
}

void update_player(struct player *p, float factor)
{
    unsigned int i;
    float s, d, rot = p->rot;
    struct vec_2d *pos = &p->pos;
    struct vec_2d *vel = &p->vel;
    int spin = 0;

    if (p->state == PS_NORMAL) {
        if (KS_ACTIVE == p->keys.left) {
            spin = -1;
        } else if (KS_ACTIVE == p->keys.right) {
            spin = 1;
        }

        p->rot = wrap_angle(p->rot + SHIP_ROTATION_SPEED * factor * spin);

        if (KS_ACTIVE == p->keys.up) {
            vel->x += sinf(rot) * SHIP_ACCELERATION * factor;
            vel->y -= cosf(rot) * SHIP_ACCELERATION * factor;
            if (thruster_channel == -1) {
                thruster_channel = mixer_play_sample(SOUND_THRUSTER);
            }
        }
    }

    s = vec_2d_magnitude(vel);                    /* Calculate speed */

    if (s > MAX_SPEED) {                          /* Limit speed */
        s = MAX_SPEED;
        vec_2d_normalise(vel);
        vec_2d_scale(vel, MAX_SPEED);
    }

    p->pos_prev.x = pos->x;
    p->pos_prev.y = pos->y;

    pos->x += vel->x * factor;                    /* Calculate position */
    pos->y += vel->y * factor;

    if (p->state == PS_EXPLODING) {
        d = s * (1.0f - 1.5f * factor);
    } else {
        d = s * (1.0f - 0.5f * factor);           /* Calculate dampening */
    }

    if (d > 0.0f) {
        vec_2d_normalise(vel);                    /* Apply dampening */
        vec_2d_scale(vel, d);
    }

    if (p->state == PS_NORMAL) {
        if (wrap_position(&p->pos, SHIP_RADIUS)) {
            p->pos_prev.x = pos->x;
            p->pos_prev.y = pos->y;
        }

        if (p->keys.up == KS_UP) {
            p->phase = 0.0f;
        } else {
            p->phase += factor;
            if (p->phase > SHIP_THRUSTER_BLINK * 2.0f) {
                p->phase = 0.0f;
            }
        }

        if (ENABLE_AUTO_FIRE) {
            if (p->reloading == true) {
                p->reload_delay += factor;
            }

            if (p->reload_delay >= BULLET_DELAY) {
                p->reload_delay = 0.0f;
                p->reloading = false;
            }
        }

    } else if (p->state == PS_EXPLODING) {
        for (i = 0; i < SHIP_EXPLOSION_SHARDS; i++) {
            p->shards[i].rot = wrap_angle(
                p->shards[i].rot +
                SHIP_EXPLOSION_SHARD_ROT_SPEED * factor * (float) p->shards[i].dir);
        }

        p->death_delay += factor;
        if (p->death_delay >= SHIP_DEATH_DELAY) {
            if (p->lives >= 0) {
                /* TODO: Make sure that ship won't reappear near asteroids */
                player_reset(p);
#ifndef __EMSCRIPTEN__
            } else if (is_high_score(p->score)) {
                initials_init(p->score);
                set_main_loop(initials_loop);
#endif
            } else {
                titlescreen_init();
                set_main_loop(titlescreen_loop);
            }
        }
    }
}

static void update_player_keystate(struct player* p, const SDL_Event* event)
{
    if (SDL_KEYDOWN == event->type) {
        switch (event->key.keysym.sym) {
        case SDLK_LEFT:
            p->keys.left = KS_ACTIVE;
            p->keys.right = (KS_ACTIVE == p->keys.right) ? KS_DOWN : p->keys.right;
            break;

        case SDLK_RIGHT:
            p->keys.right = KS_ACTIVE;
            p->keys.left = (KS_ACTIVE == p->keys.left) ? KS_DOWN : p->keys.left;
            break;

        case SDLK_UP:
            p->keys.up = KS_ACTIVE;
            break;

        case SDLK_SPACE:
            p->keys.fire = KS_ACTIVE;
            break;


        default:
            break;
        }

    } else if (SDL_KEYUP == event->type) {
        switch (event->key.keysym.sym) {
        case SDLK_LEFT:
            p->keys.left = KS_UP;
            p->keys.right = (KS_DOWN == p->keys.right) ? KS_ACTIVE : p->keys.right;
            break;

        case SDLK_RIGHT:
            p->keys.right = KS_UP;
            p->keys.left = (KS_DOWN == p->keys.left) ? KS_ACTIVE : p->keys.left;
            break;

        case SDLK_UP:
            p->keys.up = KS_UP;
            if (thruster_channel > -1) {
                mixer_stop_playing_on_channel(thruster_channel);
            }
            break;

        case SDLK_SPACE:
            p->keys.fire = KS_UP;
            p->reload_delay = 0.0f;
            p->reloading = false;
            break;

        default:
            break;
        }
    }
}

void check_fire_button(struct player *p, struct bullet *bb, unsigned int n, float f)
{
    unsigned int i;
    unsigned int num_player_bullets = 0;

    if (p->state != PS_NORMAL || p->reloading == true) {
        return;
    }

    for (i = 0; i < n; i++) {
        if (true == bb[i].visible) {
            num_player_bullets++;
        }
    }

    if (num_player_bullets >= MAX_BULLETS) {
        return;
    }

    if (ENABLE_AUTO_FIRE) {
        if (p->keys.fire == KS_DOWN) {
            p->keys.fire = KS_ACTIVE;
        }
    }

    if (KS_ACTIVE == p->keys.fire) {
        p->keys.fire = KS_DOWN;
        for (i = 0; i < n; i++, bb++) {
            if (false == bb->visible) {
                bb->visible = true;
                bb->travelled = 0.0f;
                bb->pos.x = p->pos.x - sinf(p->rot) * (0 - SHIP_PIVOT);
                bb->pos.y = p->pos.y + cosf(p->rot) * (0 - SHIP_PIVOT);
                bb->pos_prev.x = bb->pos.x;
                bb->pos_prev.y = bb->pos.y;

                bb->vel.x = sinf(p->rot);
                bb->vel.y = 0 - cosf(p->rot);

                vec_2d_normalise(&bb->vel);
                vec_2d_scale(&bb->vel, BULLET_SPEED);

                if (phaser_channel > -1) {
                    mixer_stop_playing_on_channel(phaser_channel);
                }

                phaser_channel = mixer_play_sample(SOUND_PHASER);
                break;
            }
        }

        p->reload_delay = 0.0f;
        p->reloading = true;
    }
}

void update_bullets(struct bullet *bb, unsigned int n, float f)
{
    static struct vec_2d t;
    unsigned int i;

    for (i = 0; i < n; i++, bb++) {
        if (true == bb->visible) {
            t.x = bb->vel.x * f;
            t.y = bb->vel.y * f;
            if (bb->pos.x + t.x != bb->pos.x ||
                bb->pos.y + t.y != bb->pos.y) {
                bb->pos_prev.x = bb->pos.x;
                bb->pos_prev.y = bb->pos.y;
            }
            bb->pos.x += t.x;
            bb->pos.y += t.y;
            bb->travelled += vec_2d_magnitude(&t);
            if (bb->travelled > MAX_BULLET_DISTANCE) {
                bb->visible = false;
            } else if (wrap_position(&bb->pos, 0) == true) {
                bb->pos_prev.x = bb->pos.x;
                bb->pos_prev.y = bb->pos.y;
            }
        }
    }
}

void update_score(struct player *p, int amount)
{
    const int lives_given = p->score / 10000;
    p->score += amount;
    p->lives += p->score / 10000 - lives_given;
}

/******************************************************************************
 *
 * Explosion helpers
 *
 *****************************************************************************/

void init_explosion(struct explosion *e, const struct vec_2d *pos)
{
    e->pos.x = pos->x;
    e->pos.y = pos->y;
    e->time = 0.0f;
    e->visible = true;
}

void explode_player(struct player *p)
{
    unsigned int i;

    p->state = PS_EXPLODING;

    for (i = 0; i < SHIP_EXPLOSION_SHARDS; i++) {
        p->shards[i].angle = ((2 * M_PI) / (float) SHIP_EXPLOSION_SHARDS) * (float) i;
        p->shards[i].rot = random_float(0 - (float) M_PI, (float) M_PI);
        if (p->shards[i].rot < 0.0f) {
            p->shards[i].dir = -1;
        } else {
            p->shards[i].dir = 1;
        }
    }
}

void explode_asteroid(struct asteroid *a,
    struct asteroid *aa, unsigned int na,
    struct explosion *ea, unsigned int ne)
{
    float vel_scale = 1.0f;

    unsigned int i;

    for (i = 0; i < ne; i++, ea++) {
        if (ea->visible == false) {
            init_explosion(ea, &a->pos);
            break;
        }
    }

    if (a->scale < 0.49f) {
        a->visible = false;
        return;
    } else if (a->scale < 0.99f) {
        a->scale = 0.25f;
        vel_scale = 1.5f;
    } else {
        a->scale = 0.5f;
        vel_scale = 1.25f;
    }

    a->shape = rand() % NUM_ASTEROID_SHAPES;
    a->radius = calculate_asteroid_radius(a->shape) * a->scale;

    randomise_asteroid_velocity(a, vel_scale);
    randomise_asteroid_rotation(a);

    for (i = 0; i < na; i++) {
        if (aa[i].visible == false) {
            aa[i].visible = true;
            aa[i].pos.x = a->pos.x;
            aa[i].pos.y = a->pos.y;
            aa[i].pos_prev.x = a->pos_prev.x;
            aa[i].pos_prev.y = a->pos_prev.y;
            aa[i].scale = a->scale;
            aa[i].shape = rand() % NUM_ASTEROID_SHAPES;
            aa[i].radius = calculate_asteroid_radius(aa[i].shape) * aa[i].scale;
            randomise_asteroid_velocity(&aa[i], vel_scale);
            randomise_asteroid_rotation(&aa[i]);
            break;
        }
    }
}

void update_explosions(struct explosion *ee, unsigned int n, float f)
{
    unsigned int i;

    for (i = 0; i < n; i++, ee++) {
        if (ee->visible == true) {
            ee->time += f;
            if (ee->time > EXPLOSION_LENGTH) {
                ee->visible = false;
            }
        }
    }
}

/******************************************************************************
 *
 * Collision helpers
 *
 *****************************************************************************/

void check_collisions(struct player *p, struct asteroid *aa, unsigned int na,
    struct bullet *ba, unsigned int nb, struct explosion *ea, unsigned ne,
    unsigned int *asteroids_hit)
{
    float dx, dy;
    unsigned int i, j;
    bool asteroid_hit = false;
    float ascale;

    // Check for asteroid collisions
    for (j = 0; j < na; j++) {
        if (aa[j].visible == false) {
            continue;
        }

        ascale = aa[j].scale;

        if (asteroid_hit == false) {
            /* Check for bullet collisions */
            for (i = 0; i < nb; i++) {
                if (ba[i].visible == false) {
                    continue;
                }

                /* Player bullet, test against asteroids */

                dx = ba[i].pos.x - aa[j].pos.x;
                dy = ba[i].pos.y - aa[j].pos.y;
                if (sqrt (dx * dx + dy * dy) <= aa[j].radius) {
                    if (test_asteroid_bullet_collision(&ba[i], &aa[j]) == true) {
                        ba[i].visible = false;
                        asteroid_hit = true;
                        // TODO: Should this be 'continue'?
                        break;
                    }
                }
            }
        }

        if (p->state == PS_NORMAL && asteroid_hit == false) {
            dx = p->pos.x - aa[j].pos.x;
            dy = p->pos.y - aa[j].pos.y;
            if (sqrt (dx * dx + dy * dy) <= SHIP_RADIUS + aa[j].radius) {
                if (test_asteroid_ship_collision(p, &aa[j]) == true) {
                    explode_player(p);
                    asteroid_hit = true;
                    p->lives--;
                }
            }
        }

        if (asteroid_hit == true) {
            asteroid_hit = false;
            (*asteroids_hit)++;
            explode_asteroid(&aa[j], aa, na, ea, ne);

            if (explosion_channel > -1) {
                mixer_stop_playing_on_channel(explosion_channel);
            }
            explosion_channel = mixer_play_sample(SOUND_EXPLOSION);

            if (ascale < 0.49f) {
                update_score(p, 100);
            } else if (ascale < 0.99f) {
                update_score(p, 50);
            } else {
                update_score(p, 20);
            }

            p->hit++;
        }
    }
}

/******************************************************************************
 *
 * Update logic
 *
 *****************************************************************************/

void level_draw()
{
    const float residual = (float)residual_simulation_time() / 1000.f;

    canvas_start_drawing(true);

    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (!asteroids[i].visible) {
            continue;
        }

        const struct vec_2d position = {
            asteroids[i].pos.x + asteroids[i].vel.x * residual,
            asteroids[i].pos.y + asteroids[i].vel.y * residual
        };

        const struct vec_2d scale = {
            asteroids[i].scale,
            asteroids[i].scale
        };

        assert(canvas_draw_line_segments(
            asteroid_shapes[asteroids[i].shape],
            position,
            asteroids[i].rot,
            scale
        ));
    }

    draw_bullets(bullets, MAX_BULLETS);
    draw_explosions(explosions, MAX_EXPLOSIONS);
    if (player.state == PS_NORMAL) {
        draw_player(&player);
    } else if (player.state == PS_EXPLODING) {
        draw_player_exploding(&player);
    }
    draw_score(player.score);
    draw_lives(player.lives);

    canvas_finish_drawing(true);
}

void level_update()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_KEYUP:
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                titlescreen_init();
                set_main_loop(titlescreen_loop);
                return;
            }
            /* Fall through */
        case SDL_KEYDOWN:
            update_player_keystate(&player, &event);
            break;
        case SDL_QUIT:
            exit(EXIT_SUCCESS);
        }
    }

    produce_simulation_time();
    while (maybe_consume_simulation_time(TIME_STEP_MILLIS)) {
        const float factor = (float)(TIME_STEP_MILLIS) / 1000.f;
        update_player(&player, factor);

        if (player.state == PS_NORMAL) {
            /* Check for object creation or destruction triggers */
            check_fire_button(&player, bullets, MAX_BULLETS, factor);
        }

        // Calculate new position for projectiles
        // TODO: Make this better by updating collision detection
        // routine to take velocity and acceleration * timestep
        // into account. So instead of asking, do these objects
        // overlap at this point in time, we're asking 'do these
        // objects pass through one another during this time
        // interval?''
        update_bullets(bullets, MAX_BULLETS, factor);

        // Calculate new positions for asteroids
        for (int i = 0; i < MAX_ASTEROIDS; i++) {
            if (asteroids[i].visible) {
                asteroid_update(&asteroids[i], factor);
            }
        }


        check_collisions(&player, asteroids, MAX_ASTEROIDS,
            bullets, MAX_BULLETS, explosions, MAX_EXPLOSIONS,
            &asteroids_hit);

        unsigned int num_asteroids = 0;
        for (int i = 0; i < MAX_ASTEROIDS; ++i) {
            if (true == asteroids[i].visible) {
                num_asteroids++;
            }
        }

        if (0 == num_asteroids) {
            next_level_countdown -= factor;
        } else {
            beat_delay += factor;
            beat_delay_limit = 1.2f - 0.6f * ((float)asteroids_hit / (float)(starting_asteroids * 5.f));

            if (beat_delay >= beat_delay_limit) {
                if (next_beat == 0) {
                    mixer_play_sample(SOUND_BEAT1);
                    next_beat = 1;
                } else {
                    mixer_play_sample(SOUND_BEAT2);
                    next_beat = 0;
                }
                beat_delay = 0.0f;
            }
        }

        if (next_level_countdown <= 0.f) {
            transition_init(level + 1, player.lives, player.score);
            set_main_loop(transition_loop);
        }

        update_explosions(explosions, MAX_EXPLOSIONS, factor);
    }
}

/******************************************************************************
 *
 * Public interface
 *
 *****************************************************************************/

bool level_init(unsigned int new_level, unsigned int new_lives, unsigned int new_score)
{
    srand(SDL_GetTicks());

    level = new_level;
    next_beat = 0;
    beat_delay = 0.0f;
    beat_delay_limit = 1.2f;
    next_level_countdown = NEXT_WAVE_DELAY;
    gameover_countdown = GAMEOVER_COUNTDOWN;
    gameover = false;
    asteroids_hit = 0;

    canvas_reset();

    for (int i = 0; i < NUM_ASTEROID_SHAPES; ++i) {
        asteroid_shapes[i] = canvas_load_shape(&asteroid_shape_data[i]);
    }

    // Create some asteroids!
    starting_asteroids = num_asteroids_for_level(level);
    memset(asteroids, 0, sizeof(asteroids));
    for (int i = 0; i < starting_asteroids; i++) {
        asteroid_init(&asteroids[i]);
    }

    memset(bullets, 0, sizeof(struct bullet) * MAX_BULLETS);
    memset(explosions, 0, sizeof(struct explosion) * MAX_EXPLOSIONS);

    player_init(&player);

    player.lives = new_lives;
    player.score = new_score;

    return true;
}

void level_loop(bool draw)
{
    level_update();

    if (draw) {
        level_draw();
    }
}
