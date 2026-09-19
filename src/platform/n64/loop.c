#include <stddef.h>
#include <nusys.h>

#include "loop.h"
#include "mixer.h"

static main_loop_fn_t main_loop;

static void nusys_loop(int pending_gfx)
{
    mixer_update();
    main_loop(pending_gfx < 1);
}

void loop_set_main(main_loop_fn_t new_main_loop)
{
    main_loop = new_main_loop;
}

void loop_run(void)
{
    nuGfxFuncSet((NUGfxFunc)nusys_loop);
    nuGfxDisplayOn();
}

void loop_cancel(int exit_code)
{
    (void)exit_code;
}
