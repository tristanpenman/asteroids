#ifdef _MSC_VER
#include <SDL.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <SDL_opengl.h>

#include "canvas.h"
#include "graphics.h"
#include "mathdefs.h"
#include "video.h"

struct desktop_shape
{
    struct canvas_point *points;
    uint16_t *segments;
    size_t point_count;
    size_t segment_count;
    bool closed;
    bool active;
    bool pending_destroy;
    bool referenced;
};

static struct desktop_shape shapes[CANVAS_MAX_SHAPES];
static size_t retained_points;
static struct canvas_rect active_viewport;
static enum canvas_space active_space;

static bool valid_space(enum canvas_space space)
{
    return space == CANVAS_SPACE_NORMALIZED || space == CANVAS_SPACE_PIXELS;
}

static void free_shape(struct desktop_shape *shape)
{
    free(shape->points);
    free(shape->segments);
    retained_points -= shape->point_count;
    memset(shape, 0, sizeof(*shape));
}

static void reclaim_shapes(void)
{
    int i;
    for (i = 0; i < CANVAS_MAX_SHAPES; ++i) {
        shapes[i].referenced = false;
        if (shapes[i].pending_destroy) {
            free_shape(&shapes[i]);
        }
    }
}

static bool validate_shape_data(const struct canvas_shape_data *data)
{
    size_t i;
    if (data == NULL || data->points == NULL || data->point_count < 2 ||
        data->point_count > CANVAS_MAX_RETAINED_POINTS ||
        retained_points + data->point_count > CANVAS_MAX_RETAINED_POINTS ||
        data->segment_count > CANVAS_MAX_SEGMENTS ||
        (data->segments == NULL && data->segment_count != 0)) {
        return false;
    }
    if (data->segments != NULL) {
        for (i = 0; i < data->segment_count * 2; ++i) {
            if (data->segments[i] >= data->point_count) {
                return false;
            }
        }
    }
    return true;
}

canvas_shape_id canvas_shape_create(const struct canvas_shape_data *data)
{
    struct desktop_shape *shape;
    int id;

    if (!validate_shape_data(data)) {
        return CANVAS_INVALID_SHAPE_ID;
    }
    for (id = 0; id < CANVAS_MAX_SHAPES; ++id) {
        if (!shapes[id].active && !shapes[id].pending_destroy &&
            shapes[id].points == NULL) {
            break;
        }
    }
    if (id == CANVAS_MAX_SHAPES) {
        return CANVAS_INVALID_SHAPE_ID;
    }

    shape = &shapes[id];
    shape->points = malloc(data->point_count * sizeof(*shape->points));
    if (shape->points == NULL) {
        return CANVAS_INVALID_SHAPE_ID;
    }
    memcpy(shape->points, data->points,
        data->point_count * sizeof(*shape->points));

    if (data->segments != NULL && data->segment_count != 0) {
        shape->segments = malloc(data->segment_count * 2 *
            sizeof(*shape->segments));
        if (shape->segments == NULL) {
            free(shape->points);
            memset(shape, 0, sizeof(*shape));
            return CANVAS_INVALID_SHAPE_ID;
        }
        memcpy(shape->segments, data->segments,
            data->segment_count * 2 * sizeof(*shape->segments));
    }

    shape->point_count = data->point_count;
    shape->segment_count = data->segment_count;
    shape->closed = data->closed;
    shape->active = true;
    retained_points += data->point_count;
    return id;
}

void canvas_shape_destroy(canvas_shape_id id)
{
    if (id < 0 || id >= CANVAS_MAX_SHAPES || !shapes[id].active) {
        return;
    }
    shapes[id].active = false;
    shapes[id].pending_destroy = true;
    if (!shapes[id].referenced) {
        free_shape(&shapes[id]);
    }
}

static void apply_projection(void)
{
    glViewport(active_viewport.x, active_viewport.y,
        active_viewport.width, active_viewport.height);
    glScissor(active_viewport.x, active_viewport.y,
        active_viewport.width, active_viewport.height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (active_space == CANVAS_SPACE_NORMALIZED) {
        const double aspect = (double)active_viewport.width /
            (double)active_viewport.height;
        glOrtho(-aspect, aspect, -1.0, 1.0, -1.0, 1.0);
    } else {
        glOrtho(0.0, active_viewport.width, active_viewport.height, 0.0,
            -1.0, 1.0);
    }
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

bool canvas_begin(const struct canvas_rect *viewport, enum canvas_space space)
{
    if (!valid_space(space) || !graphics_canvas_begin_phase()) {
        return false;
    }
    if (viewport == NULL) {
        video_get_viewport(&active_viewport.x, &active_viewport.y,
            &active_viewport.width, &active_viewport.height);
    } else {
        active_viewport = *viewport;
    }
    if (active_viewport.width <= 0 || active_viewport.height <= 0) {
        graphics_canvas_end_phase();
        return false;
    }
    active_space = space;
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4ub(255, 255, 255, 255);
    glLineWidth(1.0f);
    apply_projection();
    return true;
}

bool canvas_set_space(enum canvas_space space)
{
    if (!graphics_canvas_phase_active() || !valid_space(space)) {
        return false;
    }
    if (space != active_space) {
        active_space = space;
        apply_projection();
    }
    return true;
}

void canvas_end(void)
{
    if (!graphics_canvas_phase_active()) {
        return;
    }
    graphics_canvas_end_phase();
    reclaim_shapes();
}

void canvas_set_color(struct canvas_color color)
{
    if (graphics_canvas_phase_active()) {
        glColor4ub(color.red, color.green, color.blue, color.alpha);
    }
}

bool canvas_set_line_width(float width)
{
    if (!graphics_canvas_phase_active() || width < CANVAS_MIN_LINE_WIDTH ||
        width > CANVAS_MAX_LINE_WIDTH) {
        return false;
    }
    glLineWidth(width);
    return true;
}

static void push_transform(const struct canvas_transform *transform)
{
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    if (transform != NULL) {
        glTranslatef(transform->position.x, transform->position.y, 0.0f);
        glRotatef(transform->rotation * RAD_TO_DEG, 0.0f, 0.0f, 1.0f);
        glScalef(transform->scale.x, transform->scale.y, 1.0f);
    }
}

bool canvas_draw_shape(canvas_shape_id id,
    const struct canvas_transform *transform)
{
    struct desktop_shape *shape;
    if (!graphics_canvas_phase_active() || id < 0 ||
        id >= CANVAS_MAX_SHAPES || !shapes[id].active) {
        return false;
    }
    shape = &shapes[id];
    shape->referenced = true;
    push_transform(transform);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, sizeof(struct canvas_point), shape->points);
    if (shape->segments != NULL) {
        glDrawElements(GL_LINES, (GLsizei)(shape->segment_count * 2),
            GL_UNSIGNED_SHORT, shape->segments);
    } else {
        glDrawArrays(shape->closed ? GL_LINE_LOOP : GL_LINE_STRIP, 0,
            (GLsizei)shape->point_count);
    }
    glDisableClientState(GL_VERTEX_ARRAY);
    glPopMatrix();
    return true;
}

bool canvas_draw_lines(const struct canvas_point *points, size_t point_count,
    const uint16_t *segments, size_t segment_count,
    const struct canvas_transform *transform)
{
    size_t i;
    if (!graphics_canvas_phase_active() || points == NULL || point_count < 2 ||
        point_count > CANVAS_MAX_DYNAMIC_POINTS || segments == NULL ||
        segment_count > CANVAS_MAX_SEGMENTS) {
        return false;
    }
    for (i = 0; i < segment_count * 2; ++i) {
        if (segments[i] >= point_count) {
            return false;
        }
    }
    push_transform(transform);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, sizeof(struct canvas_point), points);
    glDrawElements(GL_LINES, (GLsizei)(segment_count * 2), GL_UNSIGNED_SHORT,
        segments);
    glDisableClientState(GL_VERTEX_ARRAY);
    glPopMatrix();
    return true;
}

bool canvas_draw_line_strip(const struct canvas_point *points,
    size_t point_count, bool closed,
    const struct canvas_transform *transform)
{
    if (!graphics_canvas_phase_active() || points == NULL || point_count < 2 ||
        point_count > CANVAS_MAX_DYNAMIC_POINTS) {
        return false;
    }
    push_transform(transform);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, sizeof(struct canvas_point), points);
    glDrawArrays(closed ? GL_LINE_LOOP : GL_LINE_STRIP, 0,
        (GLsizei)point_count);
    glDisableClientState(GL_VERTEX_ARRAY);
    glPopMatrix();
    return true;
}

bool canvas_draw_line(struct canvas_point a, struct canvas_point b,
    const struct canvas_transform *transform)
{
    struct canvas_point points[2];
    points[0] = a;
    points[1] = b;
    return canvas_draw_line_strip(points, 2, false, transform);
}
