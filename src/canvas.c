#ifdef _MSC_VER
#include <SDL.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL_opengl.h>

#include "canvas.h"
#include "data.h"
#include "defines.h"
#include "mathdefs.h"
#include "shape.h"
#include "vec.h"
#include "video.h"

#define MAX_SHAPES 64
#define MAX_GLYPHS 128

extern struct vec_2d origin;

static const struct shape* shapes[MAX_SHAPES];

static int num_shapes = 0;

static int font_shape_ids[MAX_GLYPHS];

void canvas_reset()
{
    int i = 0;

    for (i = 0; i < MAX_GLYPHS; i++) {
        font_shape_ids[i] = CANVAS_INVALID_SHAPE;
    }

    num_shapes = 0;
}

int canvas_load_shape(const struct shape *shape)
{
    if (num_shapes == MAX_SHAPES) {
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

    glColor3f(1.0f, 1.0f, 1.0f);
}

void canvas_set_colour(float r, float g, float b)
{
    glColor3f(r, g, b);
}

bool canvas_draw_shape(int shape, struct vec_2d position, float rotation, struct vec_2d scale)
{
    if (shape + 1 > num_shapes || shape < 0) {
        return false;
    }

    glEnableClientState(GL_VERTEX_ARRAY);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glTranslatef(position.x + origin.x, position.y + origin.y, 0.0f);
    glRotatef(rotation * RAD_TO_DEG, 0.0f, 0.0f, 1.0f);
    glScalef((GLfloat) scale.x, (GLfloat) scale.y, 1.0f);

    glVertexPointer(2, GL_FLOAT, 0, shapes[shape]->vertices);
    if (shapes[shape]->line_segments) {
        glDrawElements(GL_LINES, shapes[shape]->num_line_segments * 2, GL_UNSIGNED_BYTE, shapes[shape]->line_segments);
    } else {
        glDrawArrays(GL_LINE_STRIP, 0, shapes[shape]->num_vertices);
    }

    glPopMatrix();

    glDisableClientState(GL_VERTEX_ARRAY);

    return true;
}

void canvas_draw_text(const char *text, float x, float y, float spacing, float scale_factor)
{
    struct vec_2d position = {
        x,
        y
    };

    struct vec_2d scale = {
        scale_factor,
        scale_factor
    };

    const char *s = text;

    while (*s) {
        int shape_index;
        int c = *s;
        if (c >= MAX_GLYPHS) {
            goto next;
        }
            
        if (font_shape_ids[c] == CANVAS_INVALID_SHAPE) {
            shape_index = ascii_to_font_mapping[c];
            if (shape_index < 0) {
                goto next;
            }

            font_shape_ids[c] = canvas_load_shape(&font_shape_data[shape_index]);
            if (font_shape_ids[c] == CANVAS_INVALID_SHAPE) {
                goto next;
            }
        }

        canvas_draw_shape(font_shape_ids[c], position, 0, scale);
next:
        position.x += FONT_WIDTH * scale_factor;
        position.x += spacing * scale_factor;
        s++;
    }
}

void canvas_draw_text_centered(const char *text, float size, float y, float spacing)
{
    const float width = ((float)strlen(text) * (FONT_WIDTH + spacing)) - spacing;

    canvas_draw_text(text, 0 - (width * size / 2.0f), y, spacing, size);
}

void canvas_finish_drawing(bool swap)
{
    if (swap) {
        video_swap();
    }
}
