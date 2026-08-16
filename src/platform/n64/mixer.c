#include <nusys.h>
#include <nualstl_n.h>

#include "mixer.h"
#include "segment.h"

static bool initialized;

bool mixer_init(int num_mixer_channels)
{
    if (initialized) {
        return true;
    }

    if (num_mixer_channels != MIXER_DEFAULT &&
        num_mixer_channels != NU_AU_CHANNELS) {
        return false;
    }

    if (nuAuStlInit() <= 0) {
        return false;
    }

    nuAuStlPtrBankInit(AUDIO_POINTER_BANK_SIZE);
    nuAuStlPtrBankSet(
        AUDIO_POINTER_BANK_START,
        AUDIO_POINTER_BANK_SIZE,
        AUDIO_WAVE_BANK_START);
    nuAuStlSndPlayerDataSet(
        AUDIO_EFFECTS_BANK_START,
        AUDIO_EFFECTS_BANK_SIZE);

    initialized = true;
    return true;
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
    if (initialized) {
        nuAuStlSndPlayerStop(0);
    }
}
