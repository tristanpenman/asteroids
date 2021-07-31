#ifndef __INPUT_H
#define __INPUT_H

#include "types.h"

#define INPUT_INVALID_HANDLE -1

#define INPUT_MAX_HANDLES 32

enum input
{
    // alphabetical
    INPUT_KEY_H,

    // meta
    INPUT_KEY_ESCAPE,
    INPUT_KEY_ENTER,
    INPUT_KEY_RETURN,

    // arrow keys
    INPUT_KEY_DOWN,
    INPUT_KEY_LEFT,
    INPUT_KEY_RIGHT,
    INPUT_KEY_UP,

    // controller buttons
    INPUT_BUTTON_START,
    INPUT_BUTTON_A,
    INPUT_BUTTON_B,
    INPUT_BUTTON_Z,

    // d-pad
    INPUT_DPAD_DOWN,
    INPUT_DPAD_LEFT,
    INPUT_DPAD_RIGHT,
    INPUT_DPAD_UP,

    // joystick
    INPUT_JOYSTICK_DOWN,
    INPUT_JOYSTICK_LEFT,
    INPUT_JOYSTICK_RIGHT,
    INPUT_JOYSTICK_UP,

    // sentinal
    __INPUT__COUNT
};

void input_init();

void input_reset();

void input_update();

int input_register();

bool input_map(int handle, enum input);

bool input_active(int handle);

void input_read_joystick(int8_t* x, int8_t* y);

#endif
