#include <SDL.h>

#include "input.h"

#define INPUT_MAX_HANDLES 32

// Number of registered input handles
static int num_handles = 0;

// Mapping from supported input types to registered input handles
static int mappings[__INPUT__COUNT];

// Active state of registered input handles
static bool active[INPUT_MAX_HANDLES];

void input_handle_key(int key, bool down)
{
    if (mappings[key] != INPUT_INVALID_HANDLE) {
        active[mappings[key]] = down;
    }
}

void input_handle_event(int sym, bool down)
{
    switch (sym) {
    case SDLK_ESCAPE:
        input_handle_key(INPUT_KEY_ESCAPE, down);
        break;
    case SDLK_KP_ENTER:
        input_handle_key(INPUT_KEY_ENTER, down);
        break;
    case SDLK_RETURN:
        input_handle_key(INPUT_KEY_RETURN, down);
        break;
    case SDLK_h:
        input_handle_key(INPUT_KEY_H, down);
        break;
    default:
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

    for (int i = 0; i < __INPUT__COUNT; i++) {
        active[i] = false;
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
