#include <stddef.h>

#include "canvas.h"
#include "debug.h"
#include "options.h"
#include "vec.h"
#include "video.h"

#define MAX_SHAPES 64

const struct vec_2d canvas_origin = {
    0.5f,
    ((float)LOGICAL_HEIGHT_PX / (float)LOGICAL_WIDTH_PX) / 2.0f
};

static const struct shape *shapes[MAX_SHAPES];
static int num_shapes;

void canvas_reset(void)
{
    num_shapes = 0;
}

int canvas_load_shape(const struct shape *shape)
{
    if (shape == NULL || num_shapes == MAX_SHAPES) {
        return CANVAS_INVALID_SHAPE;
    }

    shapes[num_shapes] = shape;
    return num_shapes++;
}

void canvas_start_drawing(bool clear)
{
    if (clear) {
        video_clear();
    }
}

void canvas_set_colour(float r, float g, float b)
{
    (void)r;
    (void)g;
    (void)b;
}

bool canvas_draw_shape(int shape, struct vec_2d position, float rotation, struct vec_2d scale)
{
    (void)position;
    (void)rotation;
    (void)scale;

    if (shape < 0 || shape >= num_shapes || shapes[shape] == NULL) {
        return false;
    }

    /* PVR geometry submission is intentionally deferred to the playable slice. */
    return true;
}

void canvas_finish_drawing(bool swap)
{
    if (swap) {
        video_swap();
    }
}
