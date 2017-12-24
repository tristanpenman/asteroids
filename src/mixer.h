#ifndef __MIXER_H
#define __MIXER_H

#include <stdbool.h>

#define MIXER_DEFAULT -1
#define MIXER_INVALID_CHANNEL -1
#define MIXER_INVALID_SAMPLE -1

/**
 * Initialise SDL's audio subsystem and acquire an audio device
 *
 * @param  num_mixer_channels  number of (virtual) mixer channels to create,
 *                             or \c MIXER_DEFAULT (-1) for the default number
 *                             of channels allocated by the mixer library
 *
 * @return \c true if mixer is initialised with desired number of channels; \c false otherwise
 */
bool mixer_init(int num_mixer_channels);

/**
 * Set a function to call when a channel finishes playing a sample
 *
 * This function will be called, with the ID of the channel that has completed,
 * before subsequent calls to \c mixer_play_sample will re-use that same
 * channel ID.
 *
 * @param  callback  pointer to callback function
 */
void mixer_set_channel_completion_handler(void (*callback)(int channel));

/**
 * Load a sample from an external file
 *
 * @param  path  path to the file to load
 *
 * @return a non-negative handle on success; \c MIXER_INVALID_SAMPLE otherwise
 */
int mixer_load_sample_from_file(const char *path);

/**
 * Start playing a sample on the first available channel
 *
 * @param  sample  handle of the sample to start playing, as returned by
 *                 \c mixer_load_sample_from_file
 *
 * @return ID of the channel selected; \c MIXER_INVALID_CHANNEL otherwise
 */
int mixer_play_sample(int sample);

/**
 * Stop the sample that is playing on a given channel
 *
 * If a channel completion callback function has been registered, it will be
 * called before this function returns.
 *
 * @param  sample  handle of the channel on which to stop playing, as returned
 *                 by \c mixer_play_sample
 *
 * @return \c true if the channel has been stopped; \c false otherwise
 */
bool mixer_stop_playing_on_channel(int channel);

/**
 * Unload all samples and release the audio device acquired by \c mixer_init
  */
void mixer_cleanup();

#endif
