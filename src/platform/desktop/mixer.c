#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __EMSCRIPTEN__
#include <SDL/SDL_mixer.h>
#else
#include <SDL.h>
#include <SDL_mixer.h>
#endif

#include "mixer.h"

// Array of pointers to sample chunks managed by SDL_mixer
static Mix_Chunk **samples = NULL;

// Number of non-NULL pointers in the `samples` array
static int num_samples = 0;
static int num_channels = 0;

// Track whether audio device has already been created
static bool audio_device_open = false;

/***************************************************************************************************
 *
 * Public interface
 *
 **************************************************************************************************/

bool mixer_init(int mixer_channels)
{
    static const Uint16 format = AUDIO_S16;
    static const int frequency = 22050;
    static const int channels  = 1;
    static const int chunksize = 512;

    if (mixer_channels < 0 && MIXER_DEFAULT != mixer_channels) {
        return false;
    }

    Mix_Init(0);

    if (false == audio_device_open) {
        if (0 > Mix_OpenAudio(frequency, format, channels, chunksize)) {
            fprintf(stderr, "Failed to open audio device: %s\n", Mix_GetError());
            Mix_Quit();
            return false;
        }

        audio_device_open = true;
    }

    num_channels = Mix_AllocateChannels(
        MIXER_DEFAULT == mixer_channels ? MIX_CHANNELS : mixer_channels);
    if (num_channels <= 0 ||
        (MIXER_DEFAULT != mixer_channels && num_channels != mixer_channels)) {
        fprintf(stderr, "Failed to allocate %d mixer channels\n", mixer_channels);
        mixer_cleanup();
        return false;
    }

    return true;
}

void mixer_update(void)
{
    // SDL_mixer dispatches channel completion callbacks itself.
}

void mixer_set_channel_completion_handler(void (*callback)(int))
{
    Mix_ChannelFinished(callback);
}

int mixer_load_sample_from_file(const char *path)
{
    static const size_t ptr_size = sizeof(Mix_Chunk *);

    if (!audio_device_open || NULL == path || num_samples == INT_MAX) {
        return MIXER_INVALID_SAMPLE;
    }

    // Do not attempt to allocate more than size_t type can allow
    if ((size_t)num_samples > SIZE_MAX / ptr_size - 1) {
        fprintf(stderr, "Maximum number of samples (%d) reached while loading audio file (%s)",
                num_samples, path);
        return MIXER_INVALID_SAMPLE;
    }

    Mix_Chunk **realloc_samples = realloc(samples, (num_samples + 1) * ptr_size);
    if (NULL == realloc_samples) {
        fprintf(stderr, "realloc failed while loading audio file (%s): %s", path, strerror(errno));
        return MIXER_INVALID_SAMPLE;
    } else {
        samples = realloc_samples;
    }

    FILE *f = fopen(path, "rb");
    if (NULL == f) {
        fprintf(stderr, "Failed to open file (%s): %s\n", path, strerror(errno));
        return MIXER_INVALID_SAMPLE;
    }

    // Read sample
    if (fseek(f, 0L, SEEK_END) != 0) {
        fprintf(stderr, "Failed to seek audio file (%s)\n", path);
        fclose(f);
        return MIXER_INVALID_SAMPLE;
    }

    const long sz = ftell(f);
    if (sz <= 0 || sz > INT_MAX || fseek(f, 0L, SEEK_SET) != 0) {
        fprintf(stderr, "Invalid or unreadable audio file (%s)\n", path);
        fclose(f);
        return MIXER_INVALID_SAMPLE;
    }

    void *buffer = malloc((size_t)sz);
    if (buffer == NULL) {
        fprintf(stderr, "Failed to allocate audio buffer for (%s)\n", path);
        fclose(f);
        return MIXER_INVALID_SAMPLE;
    }

    const size_t result = fread(buffer, (size_t)sz, 1, f);
    fclose(f);
    if (result != 1) {
        free(buffer);
        fprintf(stderr, "Failed to read sample from file (%s)\n", path);
        return MIXER_INVALID_SAMPLE;
    }

    // Load sample from temporary buffer, to ensure that Web Audio API is used in Emscripten
    SDL_RWops *rw = SDL_RWFromMem(buffer, (int)sz);
    if (NULL == rw) {
        free(buffer);
        fprintf(stderr, "SDL_RWFromMem failed while loading audio file (%s): %s\n", path,
                SDL_GetError());
        return MIXER_INVALID_SAMPLE;
    }

    samples[num_samples] = Mix_LoadWAV_RW(rw, 0);
    if (NULL == samples[num_samples]) {
        fprintf(stderr, "Mix_LoadWAV failed while loading audio file (%s): %s\n", path,
                Mix_GetError());
    }

#ifdef __EMSCRIPTEN__
    // SDL_RWclose when compiled to wasm
    SDL_FreeRW(rw);
#else
    if (0 != SDL_RWclose(rw)) {
        fprintf(stderr, "SDL_RWclose failed while loading audio file (%s): %s\n", path,
                SDL_GetError());
    }
#endif

    free(buffer);

    return NULL == samples[num_samples] ? MIXER_INVALID_SAMPLE : num_samples++;
}

int mixer_play_sample(const int sample)
{
    // Check that sample exists
    if (!audio_device_open || sample < 0 || sample >= num_samples ||
        NULL == samples[sample]) {
        return MIXER_INVALID_CHANNEL;
    }

    // Play on first available channel
    return Mix_PlayChannel(-1, samples[sample], 0);
}

bool mixer_stop_playing_on_channel(int channel)
{
    if (!audio_device_open || channel < 0 || channel >= num_channels) {
        return false;
    }

    if (0 == Mix_Playing(channel)) {
        // Channel is not currently playing
        return false;
    }

    // Callback set by Mix_ChannelFinished will be called
    Mix_HaltChannel(channel);

    return true;
}

void mixer_cleanup(void)
{
    static const int all_channels = -1;
    Mix_ChannelFinished(NULL);
    if (audio_device_open) {
        Mix_HaltChannel(all_channels);
    }

    if (NULL != samples) {
        for (int i = 0; i < num_samples; ++i) {
            if (NULL != samples[i]) {
                Mix_FreeChunk(samples[i]);
                samples[i] = NULL;
            }
        }

        free(samples);
        samples = NULL;
    }
    num_samples = 0;
    num_channels = 0;

    if (audio_device_open) {
        Mix_CloseAudio();
        audio_device_open = false;
    }

    Mix_Quit();
}
