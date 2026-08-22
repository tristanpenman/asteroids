#include "graphics.h"
#include "video.h"

enum graphics_phase
{
    GRAPHICS_PHASE_IDLE,
    GRAPHICS_PHASE_FRAME,
    GRAPHICS_PHASE_CANVAS
};

static enum graphics_phase phase;
static bool canvas_recorded;

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
