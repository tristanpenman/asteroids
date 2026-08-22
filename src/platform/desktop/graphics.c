#ifdef _MSC_VER
#include <SDL.h>
#endif

#include <stddef.h>
#include <math.h>
#include <SDL_opengl.h>

#include "graphics.h"
#include "video.h"

enum graphics_phase
{
    GRAPHICS_PHASE_IDLE,
    GRAPHICS_PHASE_FRAME,
    GRAPHICS_PHASE_VIEW3D,
    GRAPHICS_PHASE_CANVAS
};

static enum graphics_phase phase;
static bool canvas_recorded;

static void load_perspective(float fov, float aspect, float near_plane,
    float far_plane)
{
    const double top = (double)near_plane * tan((double)fov * M_PI / 360.0);
    const double right = top * (double)aspect;

    glFrustum(-right, right, -top, top, near_plane, far_plane);
}

static void load_camera(const struct graphics_camera *camera)
{
    float fx = camera->target.x - camera->position.x;
    float fy = camera->target.y - camera->position.y;
    float fz = camera->target.z - camera->position.z;
    float sx;
    float sy;
    float sz;
    float ux;
    float uy;
    float uz;
    float length = sqrtf(fx * fx + fy * fy + fz * fz);
    GLfloat matrix[16];

    if (length == 0.0f) {
        return;
    }
    fx /= length;
    fy /= length;
    fz /= length;

    sx = fy * camera->up.z - fz * camera->up.y;
    sy = fz * camera->up.x - fx * camera->up.z;
    sz = fx * camera->up.y - fy * camera->up.x;
    length = sqrtf(sx * sx + sy * sy + sz * sz);
    if (length == 0.0f) {
        return;
    }
    sx /= length;
    sy /= length;
    sz /= length;

    ux = sy * fz - sz * fy;
    uy = sz * fx - sx * fz;
    uz = sx * fy - sy * fx;

    matrix[0] = sx;  matrix[4] = sy;  matrix[8] = sz;   matrix[12] = 0.0f;
    matrix[1] = ux;  matrix[5] = uy;  matrix[9] = uz;   matrix[13] = 0.0f;
    matrix[2] = -fx; matrix[6] = -fy; matrix[10] = -fz; matrix[14] = 0.0f;
    matrix[3] = 0.0f; matrix[7] = 0.0f; matrix[11] = 0.0f; matrix[15] = 1.0f;

    glMultMatrixf(matrix);
    glTranslatef(-camera->position.x, -camera->position.y,
        -camera->position.z);
}

bool graphics_begin_frame(struct graphics_color clear_color)
{
    if (phase != GRAPHICS_PHASE_IDLE) {
        return false;
    }

    video_clear_color(clear_color.red / 255.0f, clear_color.green / 255.0f,
        clear_color.blue / 255.0f, clear_color.alpha / 255.0f);
    canvas_recorded = false;
    phase = GRAPHICS_PHASE_FRAME;
    return true;
}

bool graphics_begin_view3d(const struct graphics_rect *viewport,
    const struct graphics_camera *camera)
{
    if (phase != GRAPHICS_PHASE_FRAME || canvas_recorded || viewport == NULL ||
        camera == NULL ||
        viewport->width <= 0 || viewport->height <= 0 ||
        camera->near_plane <= 0.0f || camera->far_plane <= camera->near_plane ||
        camera->vertical_fov <= 0.0f || camera->vertical_fov >= 180.0f) {
        return false;
    }

    glViewport(viewport->x, viewport->y, viewport->width, viewport->height);
    glScissor(viewport->x, viewport->y, viewport->width, viewport->height);
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    load_perspective(camera->vertical_fov,
        (float)viewport->width / (float)viewport->height,
        camera->near_plane, camera->far_plane);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    load_camera(camera);
    phase = GRAPHICS_PHASE_VIEW3D;
    return true;
}

void graphics_end_view3d(void)
{
    if (phase == GRAPHICS_PHASE_VIEW3D) {
        phase = GRAPHICS_PHASE_FRAME;
    }
}

bool graphics_canvas_begin_phase(void)
{
    if (phase != GRAPHICS_PHASE_FRAME || canvas_recorded) {
        return false;
    }
    canvas_recorded = true;
    phase = GRAPHICS_PHASE_CANVAS;
    return true;
}

void graphics_canvas_end_phase(void)
{
    if (phase == GRAPHICS_PHASE_CANVAS) {
        phase = GRAPHICS_PHASE_FRAME;
    }
}

bool graphics_canvas_phase_active(void)
{
    return phase == GRAPHICS_PHASE_CANVAS;
}

bool graphics_end_frame(void)
{
    if (phase != GRAPHICS_PHASE_FRAME) {
        return false;
    }
    video_swap();
    phase = GRAPHICS_PHASE_IDLE;
    return true;
}

void graphics_notify_tasks_completed(void)
{
}
