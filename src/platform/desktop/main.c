#include <stdio.h>
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
#include "mixer.h"
#include "options.h"
#include "storage.h"
#include "video.h"

#define FULLSCREEN_FLAG "--fullscreen"
#define HELP_FLAG "--help"
#define SANDBOX_FLAG "--sandbox"
#define SILENT_FLAG "--silent"

void cleanup(void)
{
    debug_printf("cleanup...\n");
    game_cleanup();
    mixer_cleanup();
    video_cleanup();
    SDL_Quit();
}

void help(void)
{
    printf("\nAvailable options:\n\n");
    printf("  " FULLSCREEN_FLAG "\n");
    printf("  " HELP_FLAG "\n");
    printf("  " SANDBOX_FLAG "\n");
    printf("  " SILENT_FLAG "\n");
    exit(0);
}

int main(int argc, char **argv)
{
    bool fullscreen = false;
    bool sandbox = false;
    bool silent = false;

    debug_init();

    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            if (strcmp(argv[i], FULLSCREEN_FLAG) == 0) {
                fullscreen = true;
            } else if (strcmp(argv[i], HELP_FLAG) == 0) {
                help();
            } else if (strcmp(argv[i], SANDBOX_FLAG) == 0) {
                sandbox = true;
            } else if (strcmp(argv[i], SILENT_FLAG) == 0) {
                silent = true;
            } else {
                fprintf(stderr, "unrecognised option: %s\n", argv[i]);
            }
        }
    }

    SDL_Init(0);

    srand(SDL_GetTicks());

#ifdef __EMSCRIPTEN__
    SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
#endif

    debug_printf("video_init...\n");
    if (false == video_init(LOGICAL_WIDTH_PX, LOGICAL_HEIGHT_PX, "Asteroids", fullscreen)) {
        exit(1);
    }

    if (!silent) {
        debug_printf("mixer_init...\n");
        if (!mixer_init(MIXER_DEFAULT)) {
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
    if (false == game_init(silent)) {
        exit(1);
    }

    debug_printf("game_play...\n");
    game_play(sandbox);

    cleanup();

    return 0;
}
