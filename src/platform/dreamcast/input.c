#include <string.h>

#include "input.h"

static int mappings[__INPUT__COUNT];
static bool active[INPUT_MAX_HANDLES];
static bool previous[INPUT_MAX_HANDLES];
static int num_handles;

void input_init(void)
{
    input_reset();
}

void input_reset(void)
{
    int i;

    num_handles = 0;
    memset(active, 0, sizeof(active));
    memset(previous, 0, sizeof(previous));
    for (i = 0; i < __INPUT__COUNT; ++i) {
        mappings[i] = INPUT_INVALID_HANDLE;
    }
}

void input_update(void)
{
    memcpy(previous, active, sizeof(active));
    memset(active, 0, sizeof(active));
    /* Maple discovery and controller mapping are part of the playable slice. */
}

int input_register(void)
{
    return num_handles == INPUT_MAX_HANDLES ? INPUT_INVALID_HANDLE : num_handles++;
}

bool input_map(int handle, enum input inp)
{
    if (handle < 0 || handle >= num_handles || inp < 0 || inp >= __INPUT__COUNT ||
            mappings[inp] != INPUT_INVALID_HANDLE) {
        return false;
    }

    mappings[inp] = handle;
    return true;
}

bool input_active(int handle)
{
    return handle >= 0 && handle < num_handles && active[handle];
}

bool input_triggered(int handle)
{
    return handle >= 0 && handle < num_handles && active[handle] && !previous[handle];
}

void input_read_joystick(int8_t *x, int8_t *y)
{
    if (x != NULL) {
        *x = 0;
    }
    if (y != NULL) {
        *y = 0;
    }
}
