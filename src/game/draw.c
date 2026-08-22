#include <stdio.h>

#include "draw.h"
#include "canvas.h"
#include "defines.h"
#include "entities.h"
#include "mathdefs.h"
#include "shape.h"
#include "text.h"
#include "vec.h"

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

static void set_gray(float value)
{
    struct canvas_color color;
    uint8_t component;
    if (value < 0.0f) {
        value = 0.0f;
    } else if (value > 1.0f) {
        value = 1.0f;
    }
    component = (uint8_t)(value * 255.0f);
    color.red = component;
    color.green = component;
    color.blue = component;
    color.alpha = 255;
    canvas_set_color(color);
}

static void draw_ship_at(struct vec_2d position, float rotation,
    struct vec_2d scale, float color)
{
    static const struct vec_2d points[] = {
        {-0.012f, 0.038f - SHIP_PIVOT},
        {0.0f, -SHIP_PIVOT},
        {0.012f, 0.038f - SHIP_PIVOT},
        {-0.0105f, 0.0325f - SHIP_PIVOT},
        {0.0105f, 0.0325f - SHIP_PIVOT}
    };
    static const uint16_t segments[] = {0, 1, 1, 2, 3, 4};
    set_gray(color);
    shape_canvas_draw_lines(points, 5, segments, 3, position, rotation, scale);
}

void draw_explosions(const struct explosion *ee, unsigned int n)
{
    unsigned int i;
    set_gray(1.0f);
    for (i = 0; i < n; ++i, ++ee) {
        struct vec_2d points[EXPLOSION_PARTICLES * 2];
        uint16_t segments[EXPLOSION_PARTICLES * 2];
        unsigned int p;
        if (!ee->visible) {
            continue;
        }
        for (p = 0; p < EXPLOSION_PARTICLES; ++p) {
            const float angle = (float)(2.0 * M_PI) /
                (float)EXPLOSION_PARTICLES * (float)p;
            const float inner = ee->time;
            const float outer = ee->time + 0.006f;
            points[p * 2].x = sinf(angle) * inner;
            points[p * 2].y = -cosf(angle) * inner;
            points[p * 2 + 1].x = sinf(angle) * outer;
            points[p * 2 + 1].y = -cosf(angle) * outer;
            segments[p * 2] = (uint16_t)(p * 2);
            segments[p * 2 + 1] = (uint16_t)(p * 2 + 1);
        }
        shape_canvas_draw_lines(points, EXPLOSION_PARTICLES * 2, segments,
            EXPLOSION_PARTICLES, ee->pos, 0.0f,
            (struct vec_2d){EXPLOSION_SPEED, EXPLOSION_SPEED});
    }
}

void draw_lives(int lives)
{
    int i;
    const struct vec_2d scale = {0.5f, 0.5f};
    for (i = 0; i < lives; ++i) {
        const struct vec_2d position = {
            (0.05f + 0.04f * (float)(i + 1)) * 0.5f,
            0.11f * 0.5f
        };
        draw_ship_at(position, 0.0f, scale, 1.0f);
    }
    set_gray(1.0f);
}

void draw_player_exploding(const struct player *p)
{
    static const struct vec_2d shard[] = {{-0.008f, 0.0f}, {0.008f, 0.0f}};
    const struct vec_2d unit = {1.0f, 1.0f};
    const float color = 1.0f - (p->death_delay / SHIP_EXPLOSION_LENGTH);
    unsigned int i;

    draw_ship_at(p->pos, p->rot, unit, color);
    set_gray(1.0f - (p->death_delay / SHIP_DEATH_DELAY));
    for (i = 0; i < SHIP_EXPLOSION_SHARDS; ++i) {
        const float angle = p->shards[i].angle;
        const float dx = sinf(angle) * p->death_delay * SHIP_EXPLOSION_SPEED;
        const float dy = -cosf(angle) * p->death_delay * SHIP_EXPLOSION_SPEED;
        struct vec_2d position;
        position.x = p->pos.x + cosf(p->rot) * dx - sinf(p->rot) * dy;
        position.y = p->pos.y + sinf(p->rot) * dx + cosf(p->rot) * dy;
        shape_canvas_draw_line_strip(shard, 2, false, position,
            p->rot + p->shards[i].rot, unit);
    }
    set_gray(1.0f);
}
