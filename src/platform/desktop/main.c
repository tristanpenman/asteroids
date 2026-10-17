#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <SDL.h>

#include "debug.h"
#include "game.h"
#include "highscores.h"
#include "input.h"
#include "loop.h"
#include "mixer.h"
#include "options.h"
#include "storage.h"
#include "video.h"

#define FULLSCREEN_FLAG "--fullscreen"
#define HELP_FLAG "--help"
#define SANDBOX_FLAG "--sandbox"
#define SILENT_FLAG "--silent"

static bool sdl_initialized;
static bool video_initialized;
static bool mixer_initialized;
static bool game_initialized;

static void cleanup(void)
{
    debug_printf("cleanup...\n");
    if (game_initialized) {
        game_cleanup();
        game_initialized = false;
    }
    if (mixer_initialized) {
        mixer_cleanup();
        mixer_initialized = false;
    }
    if (video_initialized) {
        video_cleanup();
        video_initialized = false;
    }
    if (sdl_initialized) {
        SDL_Quit();
        sdl_initialized = false;
    }
}

void help(void)
{
    printf("\nAvailable options:\n\n");
    printf("  " FULLSCREEN_FLAG "\n");
    printf("  " HELP_FLAG "\n");
    printf("  " SANDBOX_FLAG "\n");
    printf("  " SILENT_FLAG "\n");
}

int main(int argc, char **argv)
{
    bool fullscreen = false;
    bool sandbox = false;
    bool silent = false;
    bool show_help = false;
    int exit_code = EXIT_SUCCESS;

    debug_init();

    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            if (strcmp(argv[i], FULLSCREEN_FLAG) == 0) {
                fullscreen = true;
            } else if (strcmp(argv[i], HELP_FLAG) == 0) {
                show_help = true;
            } else if (strcmp(argv[i], SANDBOX_FLAG) == 0) {
                sandbox = true;
            } else if (strcmp(argv[i], SILENT_FLAG) == 0) {
                silent = true;
            } else {
                fprintf(stderr, "unrecognised option: %s\n", argv[i]);
            }
        }
    }

    if (show_help) {
        help();
        return EXIT_SUCCESS;
    }

    sdl_initialized = true;
    if (SDL_Init(0) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        exit_code = EXIT_FAILURE;
        goto shutdown;
    }

    srand(SDL_GetTicks());

#ifdef __EMSCRIPTEN__
    SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
#endif

    debug_printf("video_init...\n");
    video_initialized = true;
    if (false == video_init(LOGICAL_WIDTH_PX, LOGICAL_HEIGHT_PX, "Asteroids", fullscreen)) {
        exit_code = EXIT_FAILURE;
        goto shutdown;
    }

    if (!silent) {
        debug_printf("mixer_init...\n");
        mixer_initialized = mixer_init(MIXER_DEFAULT);
        if (!mixer_initialized) {
            debug_printf(" - mixer not available");
        }
    }

    debug_printf("input_init...\n");
    input_init();

    if (storage_available()) {
        debug_printf("highscores_load...\n");
        if (!highscores_load()) {
            debug_printf("highscores_save...\n");
            highscores_save();
        }
    } else {
        debug_printf(" - storage not available\n");
    }

    debug_printf("game_init...\n");
    game_initialized = true;
    if (false == game_init(silent)) {
        exit_code = EXIT_FAILURE;
        goto shutdown;
    }

    debug_printf("game_play...\n");
    loop_set_exit_handler(cleanup);
    exit_code = game_play(sandbox);

shutdown:
    cleanup();

    return exit_code;
}
