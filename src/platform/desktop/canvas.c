#ifdef _MSC_VER
#include <SDL.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <SDL_opengl.h>

#include "canvas.h"
#include "debug.h"
#include "mathdefs.h"
#include "shape.h"
#include "text.h"
#include "vec.h"
#include "video.h"

#define MAX_SHAPES 64

extern struct vec_2d origin;

static const struct shape *shapes[MAX_SHAPES];

static int num_shapes = 0;

void canvas_reset(void)
{
    num_shapes = 0;
    text_reset();
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

void canvas_continue_drawing(void)
{
    // nothing to do
}

void canvas_set_colour(float r, float g, float b)
{
    glColor3f(r, g, b);
}

bool canvas_draw_shape(int shape, struct vec_2d position, float rotation, struct vec_2d scale)
{
    if (shape + 1 > num_shapes || shape < 0) {
        debug_printf("shape out of range: %d\n", shape);
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
        glDrawElements(GL_LINES, shapes[shape]->num_line_segments * 2, GL_UNSIGNED_SHORT, shapes[shape]->line_segments);
    } else {
        glDrawArrays(GL_LINE_STRIP, 0, shapes[shape]->num_vertices);
    }

    glPopMatrix();

    glDisableClientState(GL_VERTEX_ARRAY);

    return true;
}

void canvas_finish_drawing(bool swap)
{
    if (swap) {
        video_swap();
    }
}
