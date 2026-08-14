#include "input.h"
#include "storage.h"
#include "timing.h"

void mainproc(void)
{
    input_init();
    reset_simulation_time();
    storage_available();

    for (;;) {
    }
}
