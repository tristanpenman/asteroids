#include "mixer.h"

bool mixer_init(int num_mixer_channels)
{
    (void)num_mixer_channels;
    return false;
}

void mixer_update(void)
{
    // Playback handle tracking will be added with sample playback support.
}

void mixer_set_channel_completion_handler(void (*callback)(int channel))
{
    (void)callback;
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
}
