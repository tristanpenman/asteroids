#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <SDL.h>

#include "game.h"
#include "mixer.h"
#include "options.h"
#include "video.h"

#define FULLSCREEN_FLAG "--fullscreen"
#define HELP_FLAG "--help"
#define SILENT_FLAG "--silent"

void cleanup(void)
{
    printf("cleanup...\n");
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
    printf("  " SILENT_FLAG "\n");
    exit(0);
}

int main(int argc, char **argv)
{
    bool fullscreen = false;
    bool silent = false;

    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            if (strcmp(argv[i], SILENT_FLAG) == 0) {
                silent = true;
            } else if (strcmp(argv[i], FULLSCREEN_FLAG) == 0) {
                fullscreen = true;
            } else if (strcmp(argv[i], HELP_FLAG) == 0) {
                help();
            } else {
                fprintf(stderr, "unrecognised option: %s\n", argv[i]);
            }
        }
    }

    atexit(cleanup);

    SDL_Init(0);

#ifdef __EMSCRIPTEN__
    SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
#endif

    fprintf(stdout, "video_init...\n");
    if (false == video_init(LOGICAL_WIDTH_PX, LOGICAL_HEIGHT_PX, "Asteroids", fullscreen)) {
        exit(1);
    }

    if (!silent) {
        fprintf(stdout, "mixer_init...\n");
        if (false == mixer_init(MIXER_DEFAULT)) {
            exit(1);
        }
    }

    fprintf(stdout, "game_init...\n");
    if (false == game_init(!silent)) {
        exit(1);
    }

    fprintf(stdout, "game_play...\n");
    game_play();

    return 0;
}
