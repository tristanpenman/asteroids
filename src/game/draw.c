#include <stdio.h>

#include "canvas.h"
#include "debug.h"
#include "defines.h"
#include "draw.h"
#include "entities.h"
#include "mathdefs.h"
#include "options.h"
#include "text.h"
#include "vec.h"

static const float ship_vertices[] = {
    -0.012f,  0.038f - SHIP_PIVOT,
     0.000f,  0.000f - SHIP_PIVOT,
     0.012f,  0.038f - SHIP_PIVOT,
    -0.0105f, 0.0325f - SHIP_PIVOT,
     0.0105f, 0.0325f - SHIP_PIVOT
};

static const uint16_t ship_indices[] = {
    0, 1,
    1, 2,
    3, 4
};

static const struct shape ship_shape = {
    ship_vertices,
    NUM_VERTICES(ship_vertices),
    ship_indices,
    NUM_INDICES(ship_indices),
    CANVAS_LINES
};

static const float shard_vertices[] = {
    -0.008f, 0.0f,
     0.008f, 0.0f
};

static const struct shape shard_shape = {
    shard_vertices,
    NUM_VERTICES(shard_vertices),
    NULL,
    0,
    CANVAS_LINES
};

static float particle_vertices[EXPLOSION_PARTICLES * 2];

static const struct shape particle_shape = {
    particle_vertices,
    NUM_VERTICES(particle_vertices),
    NULL,
    0,
    CANVAS_POINTS
};

static int particle_shape_id = CANVAS_INVALID_SHAPE;
static int shard_shape_id = CANVAS_INVALID_SHAPE;
static int ship_shape_id = CANVAS_INVALID_SHAPE;

static float clamp_colour(float colour)
{
    if (colour < 0.0f) {
        return 0.0f;
    }
    if (colour > 1.0f) {
        return 1.0f;
    }
    return colour;
}

void draw_load_shapes(void)
{
    for (unsigned int i = 0; i < EXPLOSION_PARTICLES; ++i) {
        const float angle = 2.0f * PI / (float)EXPLOSION_PARTICLES * (float)i;
        particle_vertices[i * 2] = sinf(angle);
        particle_vertices[i * 2 + 1] = -cosf(angle);
    }

    particle_shape_id = canvas_load_shape(&particle_shape);
    shard_shape_id = canvas_load_shape(&shard_shape);
    ship_shape_id = canvas_load_shape(&ship_shape);

    debug_assert(particle_shape_id != CANVAS_INVALID_SHAPE);
    debug_assert(shard_shape_id != CANVAS_INVALID_SHAPE);
    debug_assert(ship_shape_id != CANVAS_INVALID_SHAPE);
}

void draw_level_title(int level)
{
    char titlecard[100];

    sprintf(titlecard, "LEVEL %u", level);
    text_draw_centered(titlecard, -0.05f, 0.35f);
}

void draw_score(int score)
{
    static char buffer[SCORE_BUFFER_SIZE];

    sprintf(buffer, "%u", score);
    text_draw(buffer, -0.475f, -0.365f, 0.35f);
}

void draw_explosions(const struct explosion *ee, unsigned int n)
{
    for (unsigned int i = 0; i < n; ++i, ++ee) {
        if (ee->visible) {
            const struct vec_2d scale = {
                EXPLOSION_SPEED * ee->time,
                EXPLOSION_SPEED * ee->time
            };
            canvas_set_colour(1.0f, 1.0f, 1.0f);
            canvas_draw_shape(particle_shape_id, ee->pos, 0.0f, scale);
        }
    }
}

void draw_lives(int lives)
{
    const float canvas_height = (float)LOGICAL_HEIGHT_PX / (float)LOGICAL_WIDTH_PX;
    const struct vec_2d scale = {0.5f, 0.5f};
    struct vec_2d position = {-0.455f, 0.055f - canvas_height / 2.0f};

    canvas_set_colour(1.0f, 1.0f, 1.0f);
    for (int i = 0; i < lives; ++i) {
        canvas_draw_shape(ship_shape_id, position, 0.0f, scale);
        position.x += 0.02f;
    }
}

void draw_player_exploding(const struct player *p)
{
    const float ship_colour = 1.0f - (p->death_delay / SHIP_EXPLOSION_LENGTH);
    const float shard_colour = 1.0f - (p->death_delay / SHIP_DEATH_DELAY);
    const float clamped_ship_colour = clamp_colour(ship_colour);
    const float clamped_shard_colour = clamp_colour(shard_colour);
    const float sinr = sinf(p->rot);
    const float cosr = cosf(p->rot);

    canvas_set_colour(clamped_ship_colour, clamped_ship_colour, clamped_ship_colour);
    canvas_draw_shape(ship_shape_id, p->pos, p->rot, vec_2d_unit);

    canvas_set_colour(clamped_shard_colour, clamped_shard_colour, clamped_shard_colour);
    for (unsigned int i = 0; i < SHIP_EXPLOSION_SHARDS; ++i) {
        const float angle = p->shards[i].angle;
        const float distance = p->death_delay * SHIP_EXPLOSION_SPEED;
        const float x = sinf(angle) * distance;
        const float y = -cosf(angle) * distance;
        const struct vec_2d position = {
            p->pos.x + cosr * x - sinr * y,
            p->pos.y + sinr * x + cosr * y
        };

        canvas_draw_shape(shard_shape_id, position, p->rot + p->shards[i].rot, vec_2d_unit);
    }

    canvas_set_colour(1.0f, 1.0f, 1.0f);
}
