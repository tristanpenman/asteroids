#include "canvas.h"
#include "collision.h"
#include "data.h"
#include "sandbox.h"
#include "timing.h"
#include "vec.h"

#define NUM_ASTEROID_SHAPES 4
#define TIME_STEP_MILLIS 10

static int asteroid_shape_ids[NUM_ASTEROID_SHAPES];
static int box_shape_id;

static struct vec_2d pos1;
static struct vec_2d pos2;

static bool move_down;
static bool move_left;
static bool move_right;
static bool move_up;

static bool collision;

bool sandbox_init()
{
    unsigned int i;

    canvas_reset();

    for (i = 0; i < NUM_ASTEROID_SHAPES; ++i) {
        asteroid_shape_ids[i] = canvas_load_shape(&asteroid_shape_data[i]);
        if (asteroid_shape_ids[i] == CANVAS_INVALID_SHAPE) {
            return false;
        }
    }

    box_shape_id = canvas_load_shape(&box_shape_data);
    if (box_shape_id == CANVAS_INVALID_SHAPE) {
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
    // TODO: reimplement input handling

    // Update and consume unused simulation time
    produce_simulation_time();
    while (maybe_consume_simulation_time(TIME_STEP_MILLIS)) {
        // TODO

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

        canvas_draw_line_segments(
            asteroid_shape_ids[0],
            pos1,
            0,
            vec_2d_unit
        );

        if (collision) {
            canvas_set_colour(1.0f, 0.0f, 0.0f);
        }

        canvas_draw_line_segments(
            box_shape_id,
            pos2,
            0,
            vec_2d_unit
        );

        canvas_finish_drawing(true);
    }
}
