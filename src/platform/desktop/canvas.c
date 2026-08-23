#include <stdio.h>
#include <stdlib.h>

#ifdef _MSC_VER
#include <SDL.h>
#endif
#include <SDL_opengl.h>

#include "canvas.h"
#include "debug.h"
#include "mathdefs.h"
#include "options.h"
#include "text.h"
#include "vec.h"
#include "video.h"

#define MAX_SHAPES 64

const struct vec_2d canvas_origin = {
    0.5f,
    ((float)LOGICAL_HEIGHT_PX / (float)LOGICAL_WIDTH_PX) / 2.0f
};

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

    glTranslatef(position.x + canvas_origin.x, position.y + canvas_origin.y, 0.0f);
    glRotatef(rotation * RAD_TO_DEG, 0.0f, 0.0f, 1.0f);
    glScalef((GLfloat) scale.x, (GLfloat) scale.y, 1.0f);

    glVertexPointer(2, GL_FLOAT, 0, shapes[shape]->vertices);
    switch (shapes[shape]->primitive) {
    case CANVAS_LINES:
        if (shapes[shape]->indices != NULL) {
            glDrawElements(GL_LINES, shapes[shape]->num_indices,
                GL_UNSIGNED_SHORT, shapes[shape]->indices);
        } else {
            glDrawArrays(GL_LINES, 0, shapes[shape]->num_vertices);
        }
        break;
    case CANVAS_POINTS:
        glDrawArrays(GL_POINTS, 0, shapes[shape]->num_vertices);
        break;
    case CANVAS_LINE_STRIP:
        glDrawArrays(GL_LINE_STRIP, 0, shapes[shape]->num_vertices);
        break;
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
