#include <SDL.h>

#include "input.h"

#define INPUT_MAX_HANDLES 32

// Number of registered input handles
static int num_handles = 0;

// Mapping from supported input types to registered input handles
static int mappings[__INPUT__COUNT];

// Active state of registered input handles
static bool active[INPUT_MAX_HANDLES];

// Previous active states of registered input handles
static bool last_active[INPUT_MAX_HANDLES];

void input_handle_key(int key, bool down)
{
    int handle = mappings[key];

    if (handle != INPUT_INVALID_HANDLE) {
        last_active[handle] = active[handle];
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

void input_init()
{
    input_reset();
}

void input_reset()
{
    num_handles = 0;

    memset(active, 0, sizeof(bool) * INPUT_MAX_HANDLES);

    for (int i = 0; i < __INPUT__COUNT; i++) {
        mappings[i] = INPUT_INVALID_HANDLE;
    }
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
    return true;
}

void input_update()
{
    SDL_Event event;
    int i;

    for (i = 0; i < num_handles; i++) {
        if (!last_active[i] && active[i]) {
            last_active[i] = true;
        }
    }

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_KEYDOWN:
            input_handle_event(event.key.keysym.sym, true);
            break;
        case SDL_KEYUP:
            input_handle_event(event.key.keysym.sym, false);
            break;
        case SDL_QUIT:
            exit(EXIT_SUCCESS);
        }
    }
}

bool input_active(int handle)
{
    return active[handle];
}

bool input_triggered(int handle)
{
    return active[handle] && !last_active[handle];
}

void input_read_joystick(int8_t* x, int8_t* y)
{
    // TODO: not implemented
}
