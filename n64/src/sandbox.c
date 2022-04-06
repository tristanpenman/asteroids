#include "canvas.h"
#include "collision.h"
#include "data.h"
#include "input.h"
#include "sandbox.h"
#include "timing.h"
#include "vec.h"

#define NUM_ASTEROID_SHAPES 4
#define TIME_STEP_MILLIS 10

static int asteroid_shape_ids[NUM_ASTEROID_SHAPES];
static int box_shape_id;
static int box_shape_active_id;

static struct vec_2d pos1;
static struct vec_2d pos2;

static bool move_down;
static bool move_left;
static bool move_right;
static bool move_up;

static bool collision;

static int input_down;
static int input_left;
static int input_right;
static int input_up;

bool sandbox_init()
{
    unsigned int i;

    input_reset();

    input_down = input_register();
    input_map(input_down, INPUT_DPAD_DOWN);
    input_map(input_down, INPUT_KEY_DOWN);
    input_map(input_down, INPUT_JOYSTICK_DOWN);

    input_left = input_register();
    input_map(input_left, INPUT_DPAD_LEFT);
    input_map(input_left, INPUT_KEY_LEFT);
    input_map(input_left, INPUT_JOYSTICK_LEFT);

    input_right = input_register();
    input_map(input_right, INPUT_DPAD_RIGHT);
    input_map(input_right, INPUT_KEY_RIGHT);
    input_map(input_right, INPUT_JOYSTICK_RIGHT);

    input_up = input_register();
    input_map(input_up, INPUT_DPAD_UP);
    input_map(input_up, INPUT_KEY_UP);
    input_map(input_up, INPUT_JOYSTICK_UP);

    canvas_reset();

    for (i = 0; i < NUM_ASTEROID_SHAPES; ++i) {
        asteroid_shape_ids[i] = canvas_load_shape(&asteroid_shape_data[i]);
        if (asteroid_shape_ids[i] == CANVAS_INVALID_SHAPE) {
            return false;
        }
    }

    // white box when there is no collision detected
    box_shape_id = canvas_load_shape(&box_shape_data);
    if (box_shape_id == CANVAS_INVALID_SHAPE) {
        return false;
    }

    // red box when collision is detected
    box_shape_active_id = canvas_load_shape_with_color(&box_shape_data, 0xff, 0, 0);
    if (box_shape_active_id == CANVAS_INVALID_SHAPE) {
        return false;
    }

    pos1.x = -0.2;
    pos1.y = 0;

    pos2.x = 0;
    pos2.y = 0;

    move_down = false;
    move_left = false;
    move_right = false;
    move_up = false;

    collision = false;

    return true;
}

void sandbox_loop(bool draw)
{
    int8_t x;
    int8_t y;

    input_update();

    input_read_joystick(&x, &y);

    move_left = false;
    move_right = false;

    if (x < -20 || input_active(input_left)) {
        move_left = true;
    } else if (x > 20 || input_active(input_right)) {
        move_right = true;
    }

    move_up = false;
    move_down = false;

    if (y < -20 || input_active(input_down)) {
        move_down = true;
    } else if (y > 20 || input_active(input_up)) {
        move_up = true;
    }

    // Update and consume unused simulation time
    produce_simulation_time();
    while (maybe_consume_simulation_time(TIME_STEP_MILLIS)) {
        if (move_left) {
            pos1.x -= 0.001f;
        }

        if (move_right) {
            pos1.x += 0.001f;
        }

        if (move_up) {
            pos1.y -= 0.001f;
        }

        if (move_down) {
            pos1.y += 0.001f;
        }

        collision = collision_test_shapes(
            &asteroid_shape_data[0], &pos1, 0, 1.0f,
            &box_shape_data, &pos2, 0, 1.0f
        );
    }

    if (draw) {
        canvas_start_drawing(true);

        canvas_draw_shape(
            asteroid_shape_ids[0],
            pos1,
            0,
            vec_2d_unit
        );

        if (collision) {
            canvas_set_colour(255.f, 0.f, 0.f);
        } else {
            canvas_set_colour(255.f, 255.f, 255.f);
        }

        canvas_draw_shape(
            collision ? box_shape_active_id : box_shape_id,
            pos2,
            0,
            vec_2d_unit
        );

        canvas_finish_drawing(true);
    }
}
