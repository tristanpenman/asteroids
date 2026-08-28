#include <assert.h>
#include <stddef.h>
#include <kos.h>

#include "loop.h"
#include "mixer.h"

static main_loop_fn_t main_loop;
static bool running;
static int requested_exit_code;

void set_main_loop(main_loop_fn_t new_main_loop)
{
    assert(new_main_loop != NULL);
    main_loop = new_main_loop;
}

void run_main_loop(void)
{
    assert(main_loop != NULL);
    running = true;

    while (running) {
        mixer_update();
        main_loop(true);
        thd_pass();
    }
}

void cancel_main_loop(int exit_code)
{
    requested_exit_code = exit_code;
    running = false;
    (void)requested_exit_code;
}
