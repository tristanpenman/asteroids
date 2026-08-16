#include <stddef.h>
#include <nusys.h>

#include "loop.h"

static main_loop_fn_t main_loop;

static void nusys_loop(int pending_gfx)
{
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
