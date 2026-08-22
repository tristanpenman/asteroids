#include "canvas.h"
#include "defines.h"
#include "gameover.h"
#include "input.h"
#include "level.h"
#include "loop.h"
#include "shape.h"
#include "text.h"
#include "timing.h"
#include "titlescreen.h"

static float elapsed;

/******************************************************************************
 *
 * Public interface
 *
 *****************************************************************************/

void gameover_init(void)
{
    shape_canvas_destroy_all();
    text_reset();
    input_reset();

    elapsed = 0.f;
}

void gameover_loop(bool draw)
{
    uint32_t residual;

    input_update();

    produce_simulation_time();
    residual = residual_simulation_time();
    while (maybe_consume_simulation_time(residual)) {
        elapsed += (float) residual;
    }

    if (elapsed >= GAME_OVER_DELAY_MS) {
        titlescreen_init();
        set_main_loop(titlescreen_loop);
        return;
    }

    if (!draw) {
        return;
    }

    if (!shape_canvas_begin_frame()) {
        return;
    }
    text_draw_centered("GAME OVER", -0.05f, 0.65f);
    shape_canvas_end_frame();
}
