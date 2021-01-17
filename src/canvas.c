#include <math.h>

#ifdef _MSC_VER
#include <SDL.h>
#endif

#include <SDL_opengl.h>

#include "canvas.h"
#include "shape.h"
#include "vec.h"
#include "video.h"

#define MAX_SHAPES 64

#define RAD_TO_DEG (180.0 / M_PI)

extern struct vec_2d origin;

static const struct shape* shapes[MAX_SHAPES];

static int num_shapes = 0;

void canvas_reset()
{
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
}

bool canvas_draw_lines(int shape, struct vec_2d position, float rotation, struct vec_2d scale)
{
    if (shape + 1 > num_shapes || shape < 0) {
        return false;
    }

    glColor3f(1.0f, 1.0f, 1.0f);

    glEnableClientState(GL_VERTEX_ARRAY);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glTranslatef(position.x + origin.x, position.y + origin.y, 0.0f);
    glRotatef(rotation * RAD_TO_DEG, 0.0f, 0.0f, 1.0f);
    glScalef((GLfloat) scale.x, (GLfloat) scale.y, 1.0f);

    glVertexPointer(2, GL_FLOAT, 0, shapes[shape]->vertices);
    glDrawArrays(GL_LINE_LOOP, 0, shapes[shape]->num_vertices);

    glPopMatrix();

    glDisableClientState(GL_VERTEX_ARRAY);

    return true;
}

bool canvas_draw_triangles(int shape, struct vec_2d position, float rotation, struct vec_2d scale)
{
    if (shape + 1 > num_shapes || shape < 0) {
        return false;
    }

    glColor3f(1.0f, 1.0f, 1.0f);

    glEnableClientState(GL_VERTEX_ARRAY);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glTranslatef(position.x + origin.x, position.y + origin.y, 0.0f);
    glRotatef(rotation * RAD_TO_DEG, 0.0f, 0.0f, 1.0f);
    glScalef((GLfloat) scale.x, (GLfloat) scale.y, 1.0f);

    glVertexPointer(2, GL_FLOAT, 0, shapes[shape]->vertices);
    glDrawElements(GL_TRIANGLES, shapes[shape]->num_triangles * 3, GL_UNSIGNED_BYTE, shapes[shape]->triangles);

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
