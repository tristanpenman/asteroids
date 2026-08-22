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

bool graphics_begin_frame(struct graphics_color clear_color);
bool graphics_end_frame(void);
void graphics_notify_tasks_completed(void);

bool graphics_canvas_begin_phase(void);
void graphics_canvas_end_phase(void);
bool graphics_canvas_phase_active(void);

#endif
