#include "gfx.h"
#include "input.h"
#include "storage.h"
#include "timing.h"

void mainproc(void)
{
    gfx_init();
    input_init();
    reset_simulation_time();
    storage_available();

    for (;;) {
        // TODO: Implement a proper game loop with timing and input handling.
    }
}
