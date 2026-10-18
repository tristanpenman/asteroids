#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "input.h"
#include "loop.h"

// Number of registered input handles
static int num_handles = 0;

// Mapping from supported input types to registered input handles
static int mappings[__INPUT__COUNT];

// Active state of registered input handles
static bool active[INPUT_MAX_HANDLES];

// Pressed this frame (not merely held)
static bool triggered[INPUT_MAX_HANDLES];

void input_handle_key(int key, bool down)
{
    if (key < 0 || key >= __INPUT__COUNT) {
        return;
    }

    int handle = mappings[key];

    if (handle >= 0 && handle < num_handles) {
        if (down && !active[handle]) {
            triggered[handle] = true;
        }
        active[handle] = down;
    }
}

void input_handle_event(int sym, bool down)
{
    switch (sym) {
    case SDLK_BACKSPACE:
        input_handle_key(INPUT_KEY_BACKSPACE, down);
        break;
    case SDLK_DOWN:
        input_handle_key(INPUT_KEY_DOWN, down);
        break;
    case SDLK_DELETE:
        input_handle_key(INPUT_KEY_DELETE, down);
        break;
    case SDLK_ESCAPE:
        input_handle_key(INPUT_KEY_ESCAPE, down);
        break;
    case SDLK_LEFT:
        input_handle_key(INPUT_KEY_LEFT, down);
        break;
    case SDLK_KP_ENTER:
        input_handle_key(INPUT_KEY_ENTER, down);
        break;
    case SDLK_RETURN:
        input_handle_key(INPUT_KEY_RETURN, down);
        break;
    case SDLK_RIGHT:
        input_handle_key(INPUT_KEY_RIGHT, down);
        break;
    case SDLK_SPACE:
        input_handle_key(INPUT_KEY_SPACE, down);
        break;
    case SDLK_UP:
        input_handle_key(INPUT_KEY_UP, down);
        break;
    default:
        if (sym >= SDLK_a && sym <= SDLK_z) {
            input_handle_key(INPUT_KEY_A + sym - SDLK_a, down);
        }
        break;
    }
}

/******************************************************************************
 *
 * Public interface
 *
 *****************************************************************************/

void input_init(void)
{
    input_reset();
}

void input_reset(void)
{
    num_handles = 0;

    memset(active, 0, sizeof(bool) * INPUT_MAX_HANDLES);
    memset(triggered, 0, sizeof(triggered));

    for (int i = 0; i < __INPUT__COUNT; i++) {
        mappings[i] = INPUT_INVALID_HANDLE;
    }
}

int input_register(void)
{
    if (num_handles >= INPUT_MAX_HANDLES) {
        return INPUT_INVALID_HANDLE;
    }

    return num_handles++;
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

void input_update(void)
{
    SDL_Event event;

    memset(triggered, 0, sizeof(triggered));

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_KEYDOWN:
            input_handle_event(event.key.keysym.sym, true);
            break;
        case SDL_KEYUP:
            input_handle_event(event.key.keysym.sym, false);
            break;
        case SDL_QUIT:
            loop_cancel(EXIT_SUCCESS);
            return;
        }
    }
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
    // Not implemented
    if (x != NULL) {
        *x = 0;
    }
    if (y != NULL) {
        *y = 0;
    }
}
