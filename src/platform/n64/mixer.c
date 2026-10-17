#include <limits.h>
#include <stddef.h>
#include <string.h>

#include <nusys.h>
#include <nualstl_n.h>

#include "mixer.h"
#include "segment.h"

static bool initialized;
static int active_channels[NU_AU_CHANNELS];
static void (*channel_completion_handler)(int channel);
static int num_samples = 0;

bool mixer_init(int num_mixer_channels)
{
    if (initialized) {
        return true;
    }

    if (num_mixer_channels != MIXER_DEFAULT &&
        num_mixer_channels != NU_AU_CHANNELS) {
        return false;
    }

    if (AUDIO_POINTER_BANK_START == NULL || AUDIO_POINTER_BANK_SIZE == 0 ||
        AUDIO_WAVE_BANK_START == NULL || AUDIO_EFFECTS_BANK_START == NULL ||
        AUDIO_EFFECTS_BANK_SIZE == 0) {
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

    for (int i = 0; i < NU_AU_CHANNELS; ++i) {
        active_channels[i] = MIXER_INVALID_CHANNEL;
    }

    initialized = true;
    return true;
}

void mixer_update(void)
{
    if (!initialized) {
        return;
    }

    for (int i = 0; i < NU_AU_CHANNELS; ++i) {
        int channel = active_channels[i];

        if (channel != MIXER_INVALID_CHANNEL &&
            nuAuStlSndPlayerGetSndState(channel) == 0) {
            active_channels[i] = MIXER_INVALID_CHANNEL;

            if (channel_completion_handler != NULL) {
                channel_completion_handler(channel);
            }
        }
    }
}

void mixer_set_channel_completion_handler(void (*callback)(int channel))
{
    channel_completion_handler = callback;
}

int mixer_load_sample_from_file(const char *path)
{
    (void)path;
    if (!initialized || num_samples == INT_MAX) {
        return MIXER_INVALID_SAMPLE;
    }
    return num_samples++;
}

int mixer_play_sample(int sample)
{
    if (!initialized || sample < 0) {
        return MIXER_INVALID_CHANNEL;
    }

    /* Retire completed handles before NuSystem has a chance to reuse one. */
    mixer_update();

    int channel = nuAuStlSndPlayerPlay(sample);
    if (channel == 0) {
        return MIXER_INVALID_CHANNEL;
    }

    for (int i = 0; i < NU_AU_CHANNELS; ++i) {
        if (active_channels[i] == MIXER_INVALID_CHANNEL) {
            active_channels[i] = channel;
            return channel;
        }
    }

    /* Every playing handle must remain trackable for completion callbacks. */
    nuAuStlSndPlayerSndStop(channel, 0);
    return MIXER_INVALID_CHANNEL;
}

bool mixer_stop_playing_on_channel(int channel)
{
    for (int i = 0; i < NU_AU_CHANNELS; ++i) {
        if (active_channels[i] == channel) {
            active_channels[i] = MIXER_INVALID_CHANNEL;
            nuAuStlSndPlayerSndStop(channel, 0);
            if (channel_completion_handler != NULL) {
                channel_completion_handler(channel);
            }
            return true;
        }
    }

    return false;
}

void mixer_cleanup(void)
{
    channel_completion_handler = NULL;
    if (initialized) {
        nuAuStlSndPlayerStop(0);
    }
    initialized = false;
    num_samples = 0;
    for (int i = 0; i < NU_AU_CHANNELS; ++i) {
        active_channels[i] = MIXER_INVALID_CHANNEL;
    }
}
