#ifndef __VIDEO_H
#define __VIDEO_H

#include "types.h"

/**
 * Initialise SDL's video subsystem and create a window with an OpenGL context
 *
 * This function can be called more than once, and will return \c true if a
 * window already exists. It will ensure that the window supports double-
 * buffering, but will not attempt to resize an existing window.
 *
 * @param  width       desired window width
 * @param  height      desired window height
 * @param  title       window title
 * @param  fullscreen  whether to create a fullscreen window
 *
 * @return \c true if window was created or already exists; \c false otherwise
 */
bool video_init(int width, int height, const char *title, bool fullscreen);

/**
 * Activate OpenGL context and clear the color buffer
 */
void video_clear();

/**
 * Swap the front buffer with the back buffer
 */
void video_swap();

/**
 * Delete the OpenGL context and window created by a call to \c video_init
 */
void video_cleanup();

/**
 * Update the window title
 */
void video_set_window_title(const char *title);

#endif
