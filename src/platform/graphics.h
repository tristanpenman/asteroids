#ifndef ASTEROIDS_PLATFORM_GRAPHICS_H
#define ASTEROIDS_PLATFORM_GRAPHICS_H

#include "types.h"

struct graphics_color
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;
};

struct graphics_rect
{
    int x;
    int y;
    int width;
    int height;
};

struct graphics_vec3
{
    float x;
    float y;
    float z;
};

struct graphics_camera
{
    struct graphics_vec3 position;
    struct graphics_vec3 target;
    struct graphics_vec3 up;
    float vertical_fov;
    float near_plane;
    float far_plane;
};

bool graphics_begin_frame(struct graphics_color clear_color);
bool graphics_begin_view3d(const struct graphics_rect *viewport,
    const struct graphics_camera *camera);
void graphics_end_view3d(void);
bool graphics_end_frame(void);
void graphics_notify_tasks_completed(void);

bool graphics_canvas_begin_phase(void);
void graphics_canvas_end_phase(void);
bool graphics_canvas_phase_active(void);

#endif
