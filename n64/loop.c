#include <assert.h>
#include <nusys.h>

#include "debug.h"
#include "loop.h"

static main_loop_fn_t main_loop = NULL;

static int exit_code = 0;

static void exit_loop(bool draw)
{
    // TODO: crash and possibly report using ED64 debug API
}

static void nusys_loop(int pendingGfx)
{
    debug_pollcommands();

    main_loop(pendingGfx < 1);
}

/******************************************************************************
 *
 * Public interface
 *
 *****************************************************************************/

void set_main_loop(main_loop_fn_t new_main_loop)
{
    assert(exit_loop != main_loop);
    assert(NULL != new_main_loop);
    main_loop = new_main_loop;
}

void run_main_loop()
{
    assert(NULL != main_loop);
    assert(exit_loop != main_loop);

    nuGfxFuncSet((NUGfxFunc) nusys_loop);
    nuGfxDisplayOn();

    while (1);
}

void cancel_main_loop(int new_exit_code)
{
    assert(exit_loop != main_loop);
    main_loop = exit_loop;
    exit_code = new_exit_code;
}
