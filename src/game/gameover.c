#include "canvas.h"
#include "defines.h"
#include "draw.h"
#include "gameover.h"
#include "input.h"
#include "level.h"
#include "loop.h"
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
    canvas_reset();
    draw_load_shapes();
    input_reset();

    elapsed = 0.f;
}

void gameover_loop(bool draw)
{
    uint32_t residual;

    input_update();

    timing_advance(1u);
    residual = timing_residual();
    if (timing_consume(residual)) {
        elapsed += (float) residual;
    }

    if (elapsed >= GAME_OVER_DELAY_MS) {
        titlescreen_init();
        loop_set_main(titlescreen_loop);
        return;
    }

    if (!draw) {
        return;
    }

    canvas_start_drawing(true);
    text_draw_centered("GAME OVER", -0.05f, 0.65f);
    canvas_finish_drawing(true);
}
