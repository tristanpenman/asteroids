#include <kos.h>

#include "debug.h"
#include "video.h"

static bool initialized;

bool video_init(int width, int height, const char *title, bool fullscreen)
{
    (void)title;
    (void)fullscreen;

    if (initialized) {
        return true;
    }

    if (width != 640 || height != 480) {
        debug_printf("Dreamcast video scaffold only supports 640x480\n");
        return false;
    }

    pvr_init_defaults();
    initialized = true;
    return true;
}

void video_clear(void)
{
    if (!initialized) {
        return;
    }

    pvr_wait_ready();
    pvr_scene_begin();
}

void video_swap(void)
{
    if (initialized) {
        pvr_scene_finish();
    }
}

void video_cleanup(void)
{
    if (initialized) {
        pvr_shutdown();
        initialized = false;
    }
}

void video_set_window_title(const char *title)
{
    (void)title;
}
