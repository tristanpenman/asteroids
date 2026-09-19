#include <kos.h>

#include "debug.h"
#include "game.h"
#include "input.h"
#include "mixer.h"
#include "options.h"
#include "timing.h"
#include "video.h"

KOS_INIT_FLAGS(INIT_DEFAULT);

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    debug_init();
    debug_printf("Asteroids Dreamcast scaffold starting\n");

    if (!video_init(LOGICAL_WIDTH_PX, LOGICAL_HEIGHT_PX, "Asteroids", true)) {
        return 1;
    }

    input_init();
    timing_reset();
    mixer_init(MIXER_DEFAULT);

    /* Audio and storage remain disabled until their platform backends exist. */
    if (!game_init(true)) {
        video_cleanup();
        return 1;
    }

    game_play(false);
    game_cleanup();
    mixer_cleanup();
    video_cleanup();
    return 0;
}
