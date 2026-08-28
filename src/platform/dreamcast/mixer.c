#include <stddef.h>

#include "debug.h"
#include "mixer.h"

static void (*completion_handler)(int channel);

bool mixer_init(int num_mixer_channels)
{
    (void)num_mixer_channels;
    debug_printf("Dreamcast audio is not implemented yet; continuing silently\n");
    return false;
}

void mixer_update(void)
{
}

void mixer_set_channel_completion_handler(void (*callback)(int channel))
{
    completion_handler = callback;
    (void)completion_handler;
}

int mixer_load_sample_from_file(const char *path)
{
    (void)path;
    return MIXER_INVALID_SAMPLE;
}

int mixer_play_sample(int sample)
{
    (void)sample;
    return MIXER_INVALID_CHANNEL;
}

bool mixer_stop_playing_on_channel(int channel)
{
    (void)channel;
    return false;
}

void mixer_cleanup(void)
{
    completion_handler = NULL;
}
