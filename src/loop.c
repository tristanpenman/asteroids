#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "loop.h"
#include "types.h"

static main_loop_fn_t main_loop = NULL;

static int exit_code = 0;

static void exit_loop(bool draw)
{
#ifdef __EMSCRIPTEN__
    emscripten_force_exit(exit_code);
#else
    exit(exit_code);
#endif
}

#ifdef __EMSCRIPTEN__
static void enscripten_main_loop()
{
    main_loop(true);
}
#endif

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
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(enscripten_main_loop, 0, true);
#else
    while (1) {
        main_loop(true);
    }
#endif
}

void cancel_main_loop(int new_exit_code)
{
    assert(exit_loop != main_loop);
    main_loop = exit_loop;
    exit_code = new_exit_code;
}
