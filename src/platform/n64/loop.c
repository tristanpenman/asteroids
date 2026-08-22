#include <stddef.h>
#include <nusys.h>

#include "loop.h"
#include "graphics.h"
#include "mixer.h"

static main_loop_fn_t main_loop;

static void nusys_loop(int pending_gfx)
{
    mixer_update();
    if (pending_gfx < 1) {
        graphics_notify_tasks_completed();
    }
    main_loop(pending_gfx < 1);
}

void set_main_loop(main_loop_fn_t new_main_loop)
{
    main_loop = new_main_loop;
}

void run_main_loop(void)
{
    nuGfxFuncSet((NUGfxFunc)nusys_loop);
    nuGfxDisplayOn();
}

void cancel_main_loop(int exit_code)
{
    (void)exit_code;
}
