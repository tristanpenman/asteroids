#include <string.h>

#include "collision.h"
#include "debug.h"
#include "game.h"
#include "highscores.h"
#include "initials.h"
#include "loop.h"
#include "mixer.h"
#include "sandbox.h"
#include "titlescreen.h"

#define MAX_SAMPLES 20

static const char *const audio_files[] = {
    "assets/beat1.wav",
    "assets/beat2.wav",
    "assets/explosion.wav",
    "assets/phaser.wav",
    "assets/thruster.wav"
};

static const int num_samples = sizeof(audio_files) / sizeof(audio_files[0]);

static int samples[MAX_SAMPLES];

int explosion_channel = MIXER_INVALID_CHANNEL;
int phaser_channel = MIXER_INVALID_CHANNEL;
int thruster_channel = MIXER_INVALID_CHANNEL;

static bool load_samples(void)
{
    memset(samples, 0, sizeof(int) * MAX_SAMPLES);
    for (int i = 0; i < num_samples; ++i) {
        samples[i] = mixer_load_sample_from_file(audio_files[i]);
        if (MIXER_INVALID_SAMPLE == samples[i]) {
            return false;
        }
    }
    return true;
}

static void mixer_channel_complete(int channel)
{
    if (channel == explosion_channel) {
        explosion_channel = MIXER_INVALID_CHANNEL;
    } else if (channel == phaser_channel) {
        phaser_channel = MIXER_INVALID_CHANNEL;
    } else if (channel == thruster_channel) {
        thruster_channel = MIXER_INVALID_CHANNEL;
    }
}

/******************************************************************************
 *
 * Public interface
 *
 *****************************************************************************/

bool game_init(bool silent)
{
    if (!silent) {
        mixer_set_channel_completion_handler(mixer_channel_complete);
        debug_printf("load_samples...\n");
        if (!load_samples()) {
            return false;
        }
    }
    return true;
}

void game_cleanup(void)
{
    mixer_set_channel_completion_handler(NULL);
}

void game_play(bool sandbox)
{
    if (sandbox) {
        sandbox_init();
        loop_set_main(sandbox_loop);
    } else {
        titlescreen_init();
        loop_set_main(titlescreen_loop);
    }

    loop_run();
}
