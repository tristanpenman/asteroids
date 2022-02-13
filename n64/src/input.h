#ifndef __INPUT_H
#define __INPUT_H

#include "types.h"

#define INPUT_INVALID_HANDLE -1

#define INPUT_MAX_HANDLES 32

enum input
{
    // alphabet
    INPUT_KEY_A,
    INPUT_KEY_B,
    INPUT_KEY_C,
    INPUT_KEY_D,
    INPUT_KEY_E,
    INPUT_KEY_F,
    INPUT_KEY_G,
    INPUT_KEY_H,
    INPUT_KEY_I,
    INPUT_KEY_J,
    INPUT_KEY_K,
    INPUT_KEY_L,
    INPUT_KEY_M,
    INPUT_KEY_N,
    INPUT_KEY_O,
    INPUT_KEY_P,
    INPUT_KEY_Q,
    INPUT_KEY_R,
    INPUT_KEY_S,
    INPUT_KEY_T,
    INPUT_KEY_U,
    INPUT_KEY_V,
    INPUT_KEY_W,
    INPUT_KEY_X,
    INPUT_KEY_Y,
    INPUT_KEY_Z,

    // meta
    INPUT_KEY_ESCAPE,
    INPUT_KEY_ENTER,
    INPUT_KEY_RETURN,
    INPUT_KEY_SPACE,

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
