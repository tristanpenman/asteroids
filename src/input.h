#ifndef __INPUT_H
#define __INPUT_H

#include <stdbool.h>

#define INPUT_INVALID_HANDLE -1

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

  // controllers
  INPUT_BUTTON_START,
  INPUT_BUTTON_A,
  INPUT_BUTTON_B,
  INPUT_BUTTON_Z,

  // sentinal
  __INPUT__COUNT
};

void input_init();

void input_reset();

void input_update();

int input_register();

bool input_map(int handle, enum input);

bool input_active(int handle);

#endif
