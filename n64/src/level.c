#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nusys.h>
#include <nualstl_n.h>

#include "canvas.h"
#include "collision.h"
#include "data.h"
#include "debug.h"
#include "defines.h"
#include "entities.h"
#include "gameover.h"
#include "gfx.h"
#include "input.h"
#include "loop.h"
#include "mathdefs.h"
#include "rumble.h"
#include "shape.h"
#include "timing.h"
#include "titlescreen.h"
#include "transition.h"
#include "util.h"
#include "vec.h"

// how close objects need to be before performing collision detection
#define COLLISION_THRESHOLD 0.1f

// number of milliseconds that should be simulated in each timestep
#define TIME_STEP_MILLIS 10

// ship wrapping constants
static const float max_x =  1.0 / 2.0f + SHIP_RADIUS;
static const float min_x = -1.0 / 2.0f - SHIP_RADIUS;
static const float max_y =  SCREEN_RATIO / 2.0f + SHIP_RADIUS;
static const float min_y = -SCREEN_RATIO / 2.0f - SHIP_RADIUS;

// level state
static int asteroids_hit;
static bool gameover;
static int level;
static int next_beat;
static int starting_asteroids;

// entities
static struct asteroid asteroids[MAX_ASTEROIDS];
static struct bullet bullets[MAX_BULLETS];
static struct explosion explosions[MAX_EXPLOSIONS];
static struct player player;

// shapes
static uint8_t asteroid_shapes[NUM_ASTEROID_SHAPES];
static uint8_t bullet_shape;
static uint8_t player_frame_1_shape;
static uint8_t player_frame_2_shape;

// inputs
static int input_escape;
static int input_fire;
static int input_left;
static int input_pause;
static int input_right;
static int input_thruster;

// misc state
static float next_level_countdown;

// debug
static int collision_tests_per_frame;

static musHandle snd_handle;

/******************************************************************************
 *
 * Helper functions
 *
 *****************************************************************************/

static void draw_score(int score)
{
    static char buffer[SCORE_BUFFER_SIZE];

    sprintf(buffer, "%u", score);

    canvas_draw_text(buffer, -0.45f, -0.33f, 0.65f);
}

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

static void update_player(float factor)
{
    int i;

    if (player.state == PS_EXPLODING) {
        for (i = 0; i < SHIP_EXPLOSION_SHARDS; i++) {
            player.shards[i].rot = wrap_angle(
                player.shards[i].rot +
                SHIP_EXPLOSION_SHARD_ROT_SPEED * factor * (float) player.shards[i].dir);
        }

        player.death_delay += factor;
        if (player.death_delay >= SHIP_DEATH_DELAY) {
            if (player.lives >= 0) {
                player_reset(&player);
            } else {
                gameover_init();
                set_main_loop(gameover_loop);
            }
        }
    }
}

static void check_fire_button(float factor)
{
    unsigned int i;
    unsigned int num_player_bullets = 0;

    if (player.reloading == true) {
        player.reload_delay += factor;
        if (player.reload_delay >= BULLET_DELAY) {
            player.reload_delay = 0.0f;
            player.reloading = false;
        } else {
            return;
        }
    }

    if (player.state != PS_NORMAL) {
        return;
    }

    for (i = 0; i < MAX_BULLETS; i++) {
        if (true == bullets[i].visible) {
            num_player_bullets++;
        }
    }

    if (num_player_bullets >= MAX_BULLETS) {
        return;
    }

    if (KS_DOWN == player.keys.fire) {
        for (i = 0; i < MAX_BULLETS; i++) {
            if (false == bullets[i].visible) {
                bullet_init(&bullets[i], &player.pos, player.rot);

                if (snd_handle != 0) {
                    // stop currently playing sounds
                    nuAuStlSndPlayerSndStop(snd_handle, 0);
                    snd_handle = 0;
                }

                snd_handle = nuAuStlSndPlayerPlay(3);
                nuAuStlSndPlayerSetSndPitch(snd_handle, 6);

                break;
            }
        }

        player.reload_delay = 0.0f;
        player.reloading = true;

    } else {
        player.reload_delay = 0.0f;
        player.reloading = false;
    }
}

static void update_score(int amount)
{
    const int lives_given = player.score / 10000;

    player.score += amount;
    player.lives += player.score / 10000 - lives_given;
}

/******************************************************************************
 *
 * Explosion helpers
 *
 *****************************************************************************/

static void explode_player()
{
    int i;

    player.state = PS_EXPLODING;

    for (i = 0; i < SHIP_EXPLOSION_SHARDS; i++) {
        player.shards[i].angle = ((2 * M_PI) / (float) SHIP_EXPLOSION_SHARDS) * (float) i;
        player.shards[i].rot = random_float(0 - (float) M_PI, (float) M_PI);
        if (player.shards[i].rot < 0.0f) {
            player.shards[i].dir = -1;
        } else {
            player.shards[i].dir = 1;
        }
    }
}

static void explode_asteroid(int i)
{
    struct asteroid *a = &asteroids[i];
    float vel_scale = 1.0f;

    debug_printf("explode_asteroid\n");

    // destroy small asteroids
    if (a->scale < 0.49f) {
        a->visible = false;
        return;
    }

    // other asteroids just become smaller
    if (a->scale < 0.99f) {
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

    for (i = 0; i < MAX_ASTEROIDS; i++) {
        if (asteroids[i].visible == false) {
            asteroids[i].visible = true;
            asteroids[i].pos.x = a->pos.x;
            asteroids[i].pos.y = a->pos.y;
            asteroids[i].pos_prev.x = a->pos_prev.x;
            asteroids[i].pos_prev.y = a->pos_prev.y;
            asteroids[i].scale = a->scale;
            asteroids[i].shape = rand() % NUM_ASTEROID_SHAPES;
            asteroids[i].radius = calculate_asteroid_radius(asteroids[i].shape) * asteroids[i].scale;
            randomise_asteroid_velocity(&asteroids[i], vel_scale);
            randomise_asteroid_rotation(&asteroids[i]);
            break;
        }
    }
}

/******************************************************************************
 *
 * Collision helpers
 *
 *****************************************************************************/

static bool should_test_collisions(const struct vec_2d *a, const struct vec_2d *b)
{
    const float dx = a->x - b->x;
    const float dy = a->y - b->y;
    const float dist_sq = dx * dx + dy * dy;
    const float threshold = COLLISION_THRESHOLD;
    const float threshold_sq = threshold * threshold;

    return dist_sq < threshold_sq;
}

static void check_collisions()
{
    int i;
    int j;
    bool asteroid_hit = false;
    bool collision = false;
    float asteroid_scale;

    // Check for asteroid collisions
    for (j = 0; j < MAX_ASTEROIDS; j++) {
        struct asteroid *asteroid = &asteroids[j];
        if (asteroid->visible == false) {
            continue;
        }

        // Check for asteroid-bullet collisions
        for (i = 0; i < MAX_BULLETS; i++) {
            if (bullets[i].visible == false) {
                continue;
            }

            // broad phase
            if (!should_test_collisions(&bullets[i].pos, &asteroid->pos)) {
                continue;
            }

            collision_tests_per_frame++;

            // narrow phase
            collision = collision_test_shapes(
                &bullet_shape_data, &bullets[i].pos, bullets[i].rot, 1.0f,
                &asteroid_shape_data[asteroid->shape], &asteroid->pos, 0, asteroid->scale);

            if (collision) {
                // Bullets can only hit one asteroid
                bullets[i].visible = false;
                asteroid_hit = true;
                break;
            }
        }

        // deal with player-asteroid collisions
        if (player.state == PS_NORMAL && asteroid_hit == false) {
            // broad phase
            if (should_test_collisions(&player.pos, &asteroid->pos)) {
                collision_tests_per_frame++;

                // narrow phase
                collision = collision_test_shapes(
                    &player_frame_1_shape_data, &player.pos, player.rot, 1.0f,
                    &asteroid_shape_data[asteroid->shape], &asteroid->pos, 0, asteroid->scale);

                if (collision) {
                    debug_printf("collision between ship and asteroid %d\n", j);
                    explode_player();
                    asteroid_hit = true;
                    player.lives--;
                }
            }
        }

        if (asteroid_hit) {
            asteroid_hit = false;
            asteroids_hit++;
            explode_asteroid(j);

            // if (explosion_channel > -1) {
            //     mixer_stop_playing_on_channel(explosion_channel);
            // }
            // explosion_channel = mixer_play_sample(SOUND_EXPLOSION);

            asteroid_scale = asteroid->scale;
            if (asteroid_scale < 0.49f) {
                update_score(100);
            } else if (asteroid_scale < 0.99f) {
                update_score(50);
            } else {
                update_score(20);
            }

            // feedback
            rumble_start(0.3, 0.3);
        }
    }
}

/******************************************************************************
 *
 * Update logic
 *
 *****************************************************************************/

static void level_draw()
{
    int i;
    struct vec_2d scale;

#ifdef DEBUG
    char debug[100];
    sprintf(debug, "collision tests: %d\n", collision_tests_per_frame);
#endif

    canvas_start_drawing(true);

    if (player.state == PS_EXPLODING) {
        float c = 255.0f - player.death_delay / SHIP_DEATH_DELAY * 255.0f;
        if (c < 0.f) {
            c = 0.f;
        }
        canvas_set_colour(c, c, c);
    }

    // draw player ship
    canvas_draw_shape(player_frame_1_shape, player.pos, player.rot, vec_2d_unit);

    canvas_set_colour(255.0f, 255.0f, 255.0f);

    // draw asteroids
    for (i = 0; i < MAX_ASTEROIDS; i++) {
        if (asteroids[i].visible == false) {
            continue;
        }

        scale.x = asteroids[i].scale;
        scale.y = asteroids[i].scale;

        canvas_draw_shape(
            asteroid_shapes[asteroids[i].shape],
            asteroids[i].pos,
            asteroids[i].rot,
            scale);
    }

    // draw bullets
    for (i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].visible) {
            canvas_draw_shape(bullet_shape, bullets[i].pos, bullets[i].rot, vec_2d_unit);
        }
    }

    draw_score(player.score);

#ifdef DEBUG
    canvas_draw_text_centered(debug, -0.35f, 0.3f);
#endif

    canvas_finish_drawing(true);
}

static void level_update()
{
    // useful constants
    static const float degrees_to_radians = M_PI / 180.0f;

    int i;
    float rot;
    int8_t joystick_x;
    unsigned int num_asteroids = 0;

    collision_tests_per_frame = 0;

    input_update();
    input_read_joystick(&joystick_x, NULL);

    if (input_active(input_escape)) {
        titlescreen_init();
        set_main_loop(titlescreen_loop);
        return;
    }

    produce_simulation_time();
    while (maybe_consume_simulation_time(TIME_STEP_MILLIS)) {
        const float factor = (float)(TIME_STEP_MILLIS) / 1000.f;

        update_player(factor);

        if (joystick_x < -20 || input_active(input_left)) {
            player.rot -= SHIP_ROTATION_SPEED;
            if (player.rot < 0.0) {
                player.rot += 360.0;
            }
        } else if (joystick_x > 20 || input_active(input_right)) {
            player.rot += SHIP_ROTATION_SPEED;
            if (player.rot > 360.0) {
                player.rot -= 360.0;
            }
        }

        rot = player.rot * degrees_to_radians;

        player.keys.fire = input_active(input_fire) ? KS_DOWN : KS_UP;

        if (player.state == PS_NORMAL) {
            check_fire_button(factor);
        }

        if (input_active(input_thruster)) {
            player.vel.x += sinf(rot) * SHIP_ACCELERATION * factor;
            player.vel.y -= cosf(rot) * SHIP_ACCELERATION * factor;
        }

        player.pos.x += player.vel.x * factor;
        player.pos.y += player.vel.y * factor;

        if (player.pos.y > max_y) {
            player.pos.y = min_y;
        } else if (player.pos.y < min_y) {
            player.pos.y = max_y;
        }

        if (player.pos.x > max_x) {
            player.pos.x = min_x;
        } else if (player.pos.x < min_x) {
            player.pos.x = max_x;
        }

        for (i = 0; i < MAX_ASTEROIDS; i++) {
            if (asteroids[i].visible) {
                struct vec_2d lim;
                lim.x = 1.f / 2.f + asteroids[i].radius;
                lim.y = 0.75f / 2.f + asteroids[i].radius;
                asteroid_update(&asteroids[i], factor, &lim);
            }
        }

        for (i = 0; i < MAX_BULLETS; i++) {
            if (bullets[i].visible) {
                bullet_update(&bullets[i], factor);
            }
        }

        check_collisions();

        for (i = 0; i < MAX_ASTEROIDS; ++i) {
            if (asteroids[i].visible) {
                num_asteroids++;
            }
        }

        if (0 == num_asteroids) {
            next_level_countdown -= factor;
        } else {
        //     beat_delay += factor;
        //     beat_delay_limit = 1.2f - 0.6f * ((float)asteroids_hit / (float)(starting_asteroids * 5.f));

        //     if (beat_delay >= beat_delay_limit) {
        //         if (next_beat == 0) {
        //             mixer_play_sample(SOUND_BEAT1);
        //             next_beat = 1;
        //         } else {
        //             mixer_play_sample(SOUND_BEAT2);
        //             next_beat = 0;
        //         }
        //         beat_delay = 0.0f;
        //     }
        }

        if (next_level_countdown <= 0.f) {
            transition_init(level + 1, player.lives, player.score);
            set_main_loop(transition_loop);
        }

        // update_explosions(explosions, MAX_EXPLOSIONS, factor);
    }
}

/******************************************************************************
 *
 * Public interface
 *
 *****************************************************************************/

void level_init(int new_level, int new_lives, int new_score)
{
    int i;

    next_level_countdown = NEXT_WAVE_DELAY;
    asteroids_hit = 0;

    input_reset();

    input_escape = input_register();
    input_map(input_escape, INPUT_KEY_ESCAPE);

    input_left = input_register();
    input_map(input_left, INPUT_DPAD_LEFT);
    input_map(input_left, INPUT_KEY_LEFT);
    input_map(input_left, INPUT_JOYSTICK_LEFT);

    input_right = input_register();
    input_map(input_right, INPUT_DPAD_RIGHT);
    input_map(input_right, INPUT_KEY_RIGHT);
    input_map(input_right, INPUT_JOYSTICK_RIGHT);

    input_thruster = input_register();
    input_map(input_thruster, INPUT_BUTTON_A);
    input_map(input_thruster, INPUT_KEY_UP);

    input_fire = input_register();
    input_map(input_fire, INPUT_BUTTON_B);
    input_map(input_fire, INPUT_BUTTON_Z);
    input_map(input_fire, INPUT_KEY_SPACE);

    input_pause = input_register();
    input_map(input_pause, INPUT_BUTTON_START);

    canvas_reset();

    player_frame_1_shape = canvas_load_shape(&player_frame_1_shape_data);
    player_frame_2_shape = canvas_load_shape(&player_frame_2_shape_data);

    for (i = 0; i < NUM_ASTEROID_SHAPES; i++) {
        asteroid_shapes[i] = canvas_load_shape(&asteroid_shape_data[i]);
    }

    bullet_shape = canvas_load_shape(&bullet_shape_data);

    // Create some asteroids!
    starting_asteroids = num_asteroids_for_level(level);
    memset(asteroids, 0, sizeof(struct asteroid) * MAX_ASTEROIDS);
    for (i = 0; i < starting_asteroids; i++) {
        asteroid_init(&asteroids[i]);
    }

    memset(bullets, 0, sizeof(struct bullet) * MAX_BULLETS);
    memset(explosions, 0, sizeof(struct explosion) * MAX_EXPLOSIONS);

    player_init(&player);

    player.lives = new_lives;
    player.score = new_score;
}

void level_loop(bool draw)
{
    level_update();

    if (draw) {
        level_draw();
    }
}
