#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __EMSCRIPTEN__
#include <SDL/SDL_mixer.h>
#else
#include <SDL.h>
#include <SDL_mixer.h>
#endif

#include "./mixer.h"

// Array of pointers to sample chunks managed by SDL_mixer
static Mix_Chunk **samples = NULL;

// Number of non-NULL pointers in the `samples` array
static int num_samples = 0;

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

    Mix_Init(0);

    if (false == audio_device_open) {
        if (0 > Mix_OpenAudio(frequency, format, channels, chunksize)) {
            fprintf(stderr, "Failed to open audio device: %s\n", Mix_GetError());
            return false;
        }

        audio_device_open = true;
    }

    if (MIXER_DEFAULT == mixer_channels) {
        Mix_AllocateChannels(MIX_CHANNELS);
    } else {
        Mix_AllocateChannels(mixer_channels);
    }

    return true;
}

void mixer_set_channel_completion_handler(void (*callback)(int))
{
    Mix_ChannelFinished(callback);
}

int mixer_load_sample_from_file(const char *path)
{
    static const size_t PTR_SIZE = sizeof(Mix_Chunk *);

    // Do not attempt to allocate more than size_t type can allow
    if (num_samples * PTR_SIZE > SIZE_MAX - PTR_SIZE) {
        fprintf(stderr, "Maximum number of samples (%d) reached while loading audio file (%s)",
                num_samples, path);
        return MIXER_INVALID_SAMPLE;
    }

    Mix_Chunk ** const realloc_samples = realloc(samples, (num_samples + 1) * PTR_SIZE);
    if (NULL == realloc_samples) {
        fprintf(stderr, "realloc failed while loading audio file (%s): %s", path, strerror(errno));
        return MIXER_INVALID_SAMPLE;
    } else {
        samples = realloc_samples;
    }

    FILE * const f = fopen(path, "rb");
    if (NULL == f) {
        fprintf(stderr, "Failed to open file (%s): %s\n", path, strerror(errno));
        return MIXER_INVALID_SAMPLE;
    }

    // Read sample
    fseek(f, 0L, SEEK_END);
    const int sz = ftell(f);
    fseek(f, 0L, SEEK_SET);
    void *buffer = malloc(sz);
    int result = fread(buffer, sz, 1, f);
    fclose(f);
    if (result != 1) {
        fprintf(stderr, "Failed to read sample from file (%s)\n", path);
        return MIXER_INVALID_SAMPLE;
    }

    // Load sample from temporary buffer, to ensure that Web Audio API is used in Emscripten
    SDL_RWops * const rw = SDL_RWFromMem(buffer, sz);
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
    static const int ANY_CHANNEL = -1;

    // Check that sample exists
    if (sample > num_samples - 1 || NULL == samples[sample]) {
        return MIXER_INVALID_CHANNEL;
    }

    // Play on first available channel
    return Mix_PlayChannel(ANY_CHANNEL, samples[sample], 0);
}

bool mixer_stop_playing_on_channel(int channel)
{
    if (0 == Mix_Playing(channel)) {
        // Channel is not currently playing
        return false;
    }

    // Callback set by Mix_ChannelFinished will be called
    Mix_HaltChannel(channel);

    return true;
}

void mixer_cleanup()
{
    static const int ALL_CHANNELS = -1;
    Mix_HaltChannel(ALL_CHANNELS);

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

    Mix_CloseAudio();
    audio_device_open = false;

    Mix_Quit();
}
