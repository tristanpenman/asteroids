#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "loop.h"
#include "mixer.h"
#include "types.h"

static main_loop_fn_t main_loop;
static loop_exit_handler_fn_t exit_handler;
static bool running;
static int exit_code;

#ifdef __EMSCRIPTEN__
static void finish_browser_loop(void)
{
    if (exit_handler != NULL) {
        exit_handler();
    }
    emscripten_force_exit(exit_code);
}

static void emscripten_main_loop(void)
{
    if (running) {
        mixer_update();
        main_loop(true);
    }

    if (!running) {
        finish_browser_loop();
    }
}
#endif

/******************************************************************************
 *
 * Public interface
 *
 *****************************************************************************/

void loop_set_main(main_loop_fn_t new_main_loop)
{
    assert(NULL != new_main_loop);
    main_loop = new_main_loop;
}

int loop_run(void)
{
    assert(NULL != main_loop);
    running = true;
    exit_code = EXIT_SUCCESS;
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(emscripten_main_loop, 0, true);
#else
    while (running) {
        mixer_update();
        main_loop(true);
    }
#endif
    return exit_code;
}

void loop_set_exit_handler(loop_exit_handler_fn_t handler)
{
    exit_handler = handler;
}

void loop_cancel(int new_exit_code)
{
    running = false;
    exit_code = new_exit_code;
#ifdef __EMSCRIPTEN__
    emscripten_cancel_main_loop();
#endif
}
