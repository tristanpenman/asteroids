#include <nusys.h>
#include <string.h>

#include "input.h"

struct input_check {
    enum input input;
    int bitmask;
};

static int num_handles;
static int num_mappings;
static int mappings[__INPUT__COUNT];
static bool active[INPUT_MAX_HANDLES];
static bool triggered[INPUT_MAX_HANDLES];
static struct input_check checklist[__INPUT__COUNT];
static int bitmasks[__INPUT__COUNT];
static NUContData controller_data[1];
static u16 previous_buttons;

static void input_check_button(enum input input, int bitmask)
{
    int handle = mappings[input];

    if (handle == INPUT_INVALID_HANDLE) {
        return;
    }

    if (controller_data[0].trigger & bitmask) {
        active[handle] = true;
        triggered[handle] = true;
        return;
    }

    if ((previous_buttons & bitmask) && !(controller_data[0].button & bitmask)) {
        active[handle] = false;
    }
}

void input_init(void)
{
    input_reset();
    nuContInit();

    memset(bitmasks, 0, sizeof(bitmasks));
    bitmasks[INPUT_BUTTON_A] = A_BUTTON;
    bitmasks[INPUT_BUTTON_B] = B_BUTTON;
    bitmasks[INPUT_BUTTON_Z] = Z_TRIG;
    bitmasks[INPUT_BUTTON_START] = START_BUTTON;
    bitmasks[INPUT_DPAD_DOWN] = D_JPAD;
    bitmasks[INPUT_DPAD_LEFT] = L_JPAD;
    bitmasks[INPUT_DPAD_RIGHT] = R_JPAD;
    bitmasks[INPUT_DPAD_UP] = U_JPAD;
}

void input_reset(void)
{
    int i;

    num_handles = 0;
    num_mappings = 0;
    previous_buttons = 0;

    memset(active, 0, sizeof(active));
    memset(triggered, 0, sizeof(triggered));

    for (i = 0; i < __INPUT__COUNT; ++i) {
        mappings[i] = INPUT_INVALID_HANDLE;
    }
}

void input_update(void)
{
    int i;

    for (i = 0; i < num_handles; ++i) {
        triggered[i] = false;
    }

    nuContDataGetEx(controller_data, 0);

    for (i = 0; i < num_mappings; ++i) {
        input_check_button(checklist[i].input, checklist[i].bitmask);
    }

    previous_buttons = controller_data[0].button;
}

int input_register(void)
{
    if (num_handles == INPUT_MAX_HANDLES) {
        return INPUT_INVALID_HANDLE;
    }

    return num_handles++;
}

bool input_map(int handle, enum input input)
{
    if (handle < 0 || handle >= num_handles) {
        return false;
    }

    if (input < 0 || input >= __INPUT__COUNT) {
        return false;
    }

    if (mappings[input] != INPUT_INVALID_HANDLE || bitmasks[input] == 0) {
        return false;
    }

    mappings[input] = handle;
    checklist[num_mappings].input = input;
    checklist[num_mappings].bitmask = bitmasks[input];
    ++num_mappings;

    return true;
}

bool input_active(int handle)
{
    return handle >= 0 && handle < num_handles && active[handle];
}

bool input_triggered(int handle)
{
    return handle >= 0 && handle < num_handles && triggered[handle];
}

void input_read_joystick(int8_t *x, int8_t *y)
{
    if (x) {
        *x = controller_data[0].stick_x;
    }

    if (y) {
        *y = controller_data[0].stick_y;
    }
}
