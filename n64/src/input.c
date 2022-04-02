#include <string.h>

#include <nusys.h>
#include <nualstl_n.h>

#include "input.h"

#define INPUT_MAX_HANDLES 32

struct check {
    enum input inp;
    int bitmask;
};

// Number of registered input handles
static int num_handles = 0;

// Number of mappings that have been added
static int num_mappings = 0;

// Mapping from supported input types to registered input handles
static int mappings[__INPUT__COUNT];

// Active state of registered input handles
static bool active[INPUT_MAX_HANDLES];

// Triggered state of registered input handles
static bool triggered[INPUT_MAX_HANDLES];

// Compacted list of mappings, that will be checked on each update
static struct check checklist[__INPUT__COUNT];

// Mapping from input enum to N64 bitmasks, where available
static int bitmasks[__INPUT__COUNT];

// Controller data
static NUContData cont_data[1];
static u8 cont_pattern;
static u16 prev_button;

void input_check_button(enum input inp, int bitmask)
{
    int handle = mappings[inp];

    if (handle == INPUT_INVALID_HANDLE) {
        return;
    }

    // button activation
    if (cont_data[0].trigger & bitmask) {
        active[handle] = true;
        triggered[handle] = true;
        return;
    }

    // button deactivation
    if (prev_button & bitmask && !(cont_data[0].button & bitmask)) {
        active[handle] = false;
    }
}

/******************************************************************************
 *
 * Public interface
 *
 *****************************************************************************/

void input_init()
{
    input_reset();

    cont_pattern = nuContInit();

    memset(bitmasks, 0, sizeof(int) * __INPUT__COUNT);

    bitmasks[INPUT_BUTTON_A] = A_BUTTON;
    bitmasks[INPUT_BUTTON_B] = B_BUTTON;
    bitmasks[INPUT_BUTTON_Z] = Z_TRIG;
    bitmasks[INPUT_BUTTON_START] = START_BUTTON;
}

void input_reset()
{
    int i;

    num_handles = 0;
    num_mappings = 0;

    memset(active, 0, sizeof(bool) * INPUT_MAX_HANDLES);
    memset(triggered, 0, sizeof(bool) * INPUT_MAX_HANDLES);

    for (i = 0; i < __INPUT__COUNT; i++) {
        mappings[i] = INPUT_INVALID_HANDLE;
    }
}

void input_update()
{
    int i = 0;

    for (i = 0; i < num_handles; i++) {
        triggered[i] = false;
    }

    nuContDataGetEx(cont_data, 0);

    for (i = 0; i < num_mappings; i++) {
        input_check_button(checklist[i].inp, checklist[i].bitmask);
    }

    prev_button = cont_data[0].button;
}

int input_register()
{
    if (num_handles == INPUT_MAX_HANDLES) {
        return INPUT_INVALID_HANDLE;
    }

    return num_handles++;
}

bool input_map(int handle, enum input inp)
{
    if (mappings[inp] != INPUT_INVALID_HANDLE) {
        return false;
    }

    mappings[inp] = handle;

    checklist[num_mappings].inp = inp;
    checklist[num_mappings].bitmask = bitmasks[inp];
    num_mappings++;

    return true;
}

bool input_active(int handle)
{
    return active[handle];
}

bool input_triggered(int handle)
{
    return triggered[handle];
}

void input_read_joystick(int8_t* x, int8_t* y)
{
    if (x) {
        *x = cont_data[0].stick_x;
    }

    if (y) {
        *y = cont_data[0].stick_y;
    }
}
