#include <assert.h>
#include <stddef.h>
#include <kos.h>

#include "loop.h"
#include "mixer.h"

static main_loop_fn_t main_loop;
static bool running;
static int requested_exit_code;

void loop_set_main(main_loop_fn_t new_main_loop)
{
    assert(new_main_loop != NULL);
    main_loop = new_main_loop;
}

int loop_run(void)
{
    assert(main_loop != NULL);
    running = true;
    requested_exit_code = 0;

    while (running) {
        mixer_update();
        main_loop(true);
        thd_pass();
    }
    return requested_exit_code;
}

void loop_cancel(int exit_code)
{
    requested_exit_code = exit_code;
    running = false;
    (void)requested_exit_code;
}
