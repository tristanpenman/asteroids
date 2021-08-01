#include <assert.h>
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
#include "gfx.h"
#include "input.h"
#include "mathdefs.h"
#include "rumble.h"
#include "shape.h"
#include "timing.h"
#include "util.h"
#include "vec.h"

#define TIME_STEP_MILLIS 10

// ship wrapping constants
static const float max_x =  1.0 / 2.0f + SHIP_RADIUS;
static const float min_x = -1.0 / 2.0f - SHIP_RADIUS;
static const float max_y =  SCREEN_RATIO / 2.0f + SHIP_RADIUS;
static const float min_y = -SCREEN_RATIO / 2.0f - SHIP_RADIUS;

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

extern musHandle sndHandle;

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

void check_fire_button(float factor)
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

                if (sndHandle != 0) {
                    // stop currently playing sounds
                    nuAuStlSndPlayerSndStop(sndHandle, 0);
                    sndHandle = 0;
                }

                sndHandle = nuAuStlSndPlayerPlay(3);
                nuAuStlSndPlayerSetSndPitch(sndHandle, 6);

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

/******************************************************************************
 *
 * Explosion helpers
 *
 *****************************************************************************/

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

void explode_asteroid(int i)
{
    struct asteroid *a = &asteroids[i];
    float vel_scale = 1.0f;

    debug_printf("explode_asteroid\n");

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

void check_collisions()
{
    float dx, dy;
    unsigned int i, j;
    bool asteroid_hit = false;

    player.hit = 0;

    // Check for asteroid collisions
    for (j = 0; j < MAX_ASTEROIDS; j++) {
        if (asteroids[j].visible == false) {
            continue;
        }

        if (player.state == PS_NORMAL) {
            const bool collision = collision_test_shapes(
                &player_frame_1_shape_data, &player.pos, player.rot, 1.0f,
                &asteroid_shape_data[asteroids[j].shape], &asteroids[j].pos, 0, asteroids[j].scale);

            if (collision) {
                player.hit++;

                // TODO: deal with player-asteroid collisions

                rumble_start(0.3, 0.7);
                explode_asteroid(j);

                asteroid_hit = true;
                break;
            }
        }

        for (i = 0; i < MAX_BULLETS; i++) {
            if (bullets[i].visible) {
                const bool collision = collision_test_shapes(
                    &bullet_shape_data, &bullets[i].pos, 0, 1.0f,
                    &asteroid_shape_data[asteroids[j].shape], &asteroids[j].pos, 0, asteroids[j].scale);

                if (collision) {
                    bullets[i].visible = false;
                    asteroids_hit++;

                    rumble_start(0.3, 0.3);
                    explode_asteroid(j);

                    asteroid_hit = true;
                    break;
                }
            }
        }

        if (asteroid_hit) {
            break;
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
    int i;
    char conbuf[20];

    canvas_start_drawing(true);
    canvas_draw_line_segments(player_frame_1_shape, player.pos, player.rot, vec_2d_unit);

    for (i = 0; i < MAX_ASTEROIDS; i++) {
        if (asteroids[i].visible) {
            struct vec_2d scale = {
                asteroids[i].scale,
                asteroids[i].scale,
            };
            canvas_draw_line_segments(asteroid_shapes[asteroids[i].shape], asteroids[i].pos, 0, scale);
        }
    }

    for (i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].visible) {
            canvas_draw_line_segments(bullet_shape, bullets[i].pos, bullets[i].rot, vec_2d_unit);
        }
    }

    canvas_finish_drawing(false);

    // Print score using debug output
    sprintf(conbuf, "%d", player.score);
    nuDebConTextPos(0, 3, 3);
    nuDebConCPuts(0, conbuf);

    // Player hit?
    sprintf(conbuf, "%d", player.hit);
    nuDebConTextPos(0, 5, 3);
    nuDebConCPuts(0, conbuf);

    {
        int num_bullets = 0;
        int i = 0;
        for (i = 0; i < MAX_BULLETS; i++) {
            if (bullets[i].visible) {
                num_bullets++;
            }
        }

        // Fire?
        sprintf(conbuf, "%d", num_bullets);
        nuDebConTextPos(0, 7, 3);
        nuDebConCPuts(0, conbuf);
    }

    sprintf(conbuf, "%d", asteroids_hit);
    nuDebConTextPos(0, 7, 3);
    nuDebConCPuts(0, conbuf);

    nuDebConDisp(NU_SC_SWAPBUFFER);
}

void level_update()
{
    // useful constants
    static const float degrees_to_radians = M_PI / 180.0f;

    int i;
    float rot;
    int8_t joystick_x;

    input_update();
    input_read_joystick(&joystick_x, NULL);

    // if (input_active(input_escape)) {
    //     titlescreen_init();
    //     set_main_loop(titlescreen_loop);
    //     return;
    // }

    produce_simulation_time();
    while (maybe_consume_simulation_time(TIME_STEP_MILLIS)) {
        const float factor = (float)(TIME_STEP_MILLIS) / 1000.f;

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

        // unsigned int num_asteroids = 0;
        // for (int i = 0; i < MAX_ASTEROIDS; ++i) {
        //     if (true == asteroids[i].visible) {
        //         num_asteroids++;
        //     }
        // }

        // if (0 == num_asteroids) {
        //     next_level_countdown -= factor;
        // } else {
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
        // }

        // if (next_level_countdown <= 0.f) {
        //     transition_init(level + 1, player.lives, player.score);
        //     set_main_loop(transition_loop);
        // }

        // update_explosions(explosions, MAX_EXPLOSIONS, factor);
    }
}

/******************************************************************************
 *
 * Public interface
 *
 *****************************************************************************/

void level_init(unsigned int new_level, unsigned int new_lives, unsigned int new_score)
{
    int i;

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
