#include <stdbool.h>
#include <stdio.h>

#include <SDL.h>

#define GL_GLEXT_PROTOTYPES 1
#include <SDL_opengl.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#define EM_TARGET "canvas"
#endif

#include "video.h"

// Pointer to window created by a call to SDL_CreateWindow
static SDL_Window *sdl_window = NULL;

// Handle for OpenGL context returned by a call to SDL_GL_CreateContext
static SDL_GLContext sdl_glcontext = NULL;

// Width and height used when window was created
static int logical_width = 0;
static int logical_height = 0;

// Current width and height of the canvas, in pixels
static int canvas_width = 0;
static int canvas_height = 0;

// Pixel density of the monitor on which the window is being displayed
static float pixel_density = 1.f;

/******************************************************************************
 *
 * Helper functions
 *
 *****************************************************************************/

#ifdef __EMSCRIPTEN__

static int prev_inner_width = 0;
static int prev_inner_height = 0;

static EM_BOOL resize_callback(int eventType, const EmscriptenUiEvent *uiEvent, void *userdata)
{
    const int inner_width = uiEvent->windowInnerWidth;
    const int inner_height = uiEvent->windowInnerHeight;

    // Calculate actual canvas size as power of logical canvas size
    int new_width = logical_width;
    int new_height = logical_height;
    while (new_width * 2 < inner_width && new_height * 2 < inner_height) {
        new_width *= 2;
        new_height *= 2;
    }

    // Update canvas size so that further CSS calculations reflect new size
    emscripten_set_element_css_size(EM_TARGET, new_width, new_height);

    // Ensure that resizing won't cause page contents to overflow, but only if shrinking the
    // canvas wouldn't make it smaller than it's logical size
    if (new_width > logical_width && new_height > logical_height) {
        const bool window_shrunk = inner_width < prev_inner_width || inner_height < prev_inner_height;
        const bool canvas_expanded = new_width > canvas_width || new_height > canvas_height;
        if (window_shrunk || canvas_expanded) {
            const int client_width = EM_ASM_INT({
                return document.body.clientWidth;
            }, NULL);
            const int client_height = EM_ASM_INT({
                return document.body.clientHeight;
            }, NULL);

            if (client_width > inner_width || client_height > inner_height) {
                new_width /= 2;
                new_height /= 2;
                emscripten_set_element_css_size(EM_TARGET, new_width, new_height);
            }
        }
    }

    if (new_width != canvas_width || new_height != canvas_height) {
#ifdef DEBUG
        printf("resizing canvas from (%d, %d) to (%d, %d)\n", canvas_width, canvas_height,
            new_width, new_height);
#endif
        SDL_SetWindowSize(sdl_window, new_width, new_height);
        canvas_width = new_width;
        canvas_height = new_height;
    }

    prev_inner_width = inner_width;
    prev_inner_height = inner_height;

    return true;
}

#else

static int window_width;
static int window_height;

static bool reset_window_metrics()
{
    const int old_pixel_density = pixel_density;
    SDL_GL_GetDrawableSize(sdl_window, &canvas_width, &canvas_height);
    pixel_density = (float)canvas_width / (float)window_width;
    return old_pixel_density != pixel_density;
}

static int event_filter(void *userdata, SDL_Event *event)
{
    if (event->type == SDL_WINDOWEVENT) {
        if (event->window.event == SDL_WINDOWEVENT_RESIZED) {
            window_width = event->window.data1;
            window_height = event->window.data2;
            video_clear();
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            video_swap();
        }
    }

    return 1;
}

#endif

/******************************************************************************
 *
 * Public interface
 *
 *****************************************************************************/

bool video_init(int width, int height, const char *title, bool fullscreen)
{
    if (0 > SDL_InitSubSystem(SDL_INIT_VIDEO)) {
        fprintf(stderr, "SDL_InitSubSystem failed: %s\n", SDL_GetError());
        return false;
    }

    logical_width = width;
    logical_height = height;

    int new_width = width;
    int new_height = height;

#ifdef __EMSCRIPTEN__
    const int inner_width = EM_ASM_INT({
        return window.innerWidth;
    });
    const int inner_height = EM_ASM_INT({
        return window.innerHeight;
    });
#else
    SDL_Rect rect;
    if (0 != SDL_GetDisplayUsableBounds(0, &rect)) {
        fprintf(stderr, "SDL_GetDisplayUsableBounds failed: %s\n", SDL_GetError());
        return false;
    }
    const int inner_width = rect.w;
    const int inner_height = rect.h;
#endif

#ifdef DEBUG
    printf("client area = (%d, %d)\n", inner_width, inner_height);
#endif

    while (new_width * 2 < inner_width && new_height * 2 < inner_height) {
        new_width *= 2;
        new_height *= 2;
    }

    int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    if (fullscreen == true) {
        flags |= SDL_WINDOW_FULLSCREEN;
    }

#ifndef __EMSCRIPTEN__
    flags |= SDL_WINDOW_ALLOW_HIGHDPI;
#endif

    if (NULL == sdl_window) {
        sdl_window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED,
                SDL_WINDOWPOS_UNDEFINED, new_width, new_height, flags);
    }

    if (NULL == sdl_window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        return false;
    }

#ifdef __EMSCRIPTEN__
    emscripten_set_element_css_size(EM_TARGET, new_width, new_height);

    const int client_width = EM_ASM_INT({
        return document.body.clientWidth;
    });
    const int client_height = EM_ASM_INT({
        return document.body.clientHeight;
    });

    if (client_width > inner_width || client_height > inner_height) {
        new_width /= 2;
        new_height /= 2;
        emscripten_set_element_css_size(EM_TARGET, new_width, new_height);
        SDL_SetWindowSize(sdl_window, new_width, new_height);
    }

    prev_inner_width = inner_width;
    prev_inner_height = inner_height;
#else
    window_width = new_width;
    window_height = new_height;
#endif

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    if (NULL == sdl_glcontext) {
        sdl_glcontext = SDL_GL_CreateContext(sdl_window);
    }

    if (NULL == sdl_glcontext) {
        fprintf(stderr, "SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        return false;
    }

    int sdl_gl_doublebuffer = -1;
    if (0 > SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &sdl_gl_doublebuffer)) {
        fprintf(stderr, "SDL_GL_GetAttribute failed: %s\n", SDL_GetError());
        return false;
    }

    if (1 != sdl_gl_doublebuffer) {
        if (0 > SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1)) {
            fprintf(stderr, "SDL_GL_SetAttribute failed: %s\n", SDL_GetError());
            return false;
        }
    }

    SDL_GL_MakeCurrent(sdl_window, sdl_glcontext);

#ifdef __EMSCRIPTEN__
    emscripten_set_resize_callback("canvas", NULL, true, resize_callback);
    canvas_width = new_width;
    canvas_height = new_height;
#else
    SDL_GL_SetSwapInterval(1);
    reset_window_metrics();
    SDL_SetEventFilter(event_filter, NULL);
#endif

#ifdef DEBUG
    printf("initial drawable size = (%d, %d)\n", canvas_width, canvas_height);
    printf("initial pixel density = %f\n", pixel_density);
#endif

    // Scale viewport and line width according to device pixel ratio
    glViewport(0, 0, canvas_width, canvas_height);
    glLineWidth(1.5f * pixel_density);

    // Enable line smooting
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return true;
}

void video_clear()
{
    const GLfloat ratio = (GLfloat)logical_height / (GLfloat)logical_width;

    SDL_GL_MakeCurrent(sdl_window, sdl_glcontext);

#ifdef __EMSCRIPTEN__
    // Can't call SDL_GL_SetSwapInterval until we're in Emscripten's main loop, and even then,
    // calling it repeatedly will cause Emscripten to produce an excessive number of warning
    // messages about using requestAnimationFrame for the main loop.
    const int swap_interval = SDL_GL_GetSwapInterval();
    if (swap_interval != 0) {
        SDL_GL_SetSwapInterval(0);
    }

    glViewport(0, 0, canvas_width, canvas_height);
#else
    // TODO: Emscripten's canvas supports high DPI displays, but does not
    // appear to correctly handle changes in device pixel ratio (at least,
    // this was the case when I was testing in Safari).
    if (reset_window_metrics()) {
#ifdef DEBUG
        printf("canvas width = %d\n", canvas_width);
        printf("canvas height = %d\n", canvas_height);
        printf("pixel density = %f\n", pixel_density);
#endif
        glLineWidth(1.5f * pixel_density);
    }

    // Calculate viewport size based aspect ratio
    GLfloat viewport_width;
    GLfloat viewport_height;
    if (canvas_width * ratio >= canvas_height) {
        viewport_width = canvas_height / ratio;
        viewport_height = canvas_height;
    } else if (canvas_height / ratio >= canvas_width) {
        viewport_width = canvas_width;
        viewport_height = canvas_width * ratio;
    } else {
        viewport_width = canvas_width;
        viewport_height = canvas_height;
    }

    // Grey border around playable area
    viewport_width -= 40;
    viewport_height -= 40;

    // Grey background for non-playable area
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, canvas_width, canvas_height);
    glViewport(0, 0, canvas_width, canvas_height);
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Playable area
    const GLfloat x = (canvas_width - viewport_width) / 2.f;
    const GLfloat y = (canvas_height - viewport_height) / 2.f;
    glScissor(x, y, viewport_width, viewport_height);
    glViewport(x, y, viewport_width, viewport_height);
#endif

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f, 1.0f, ratio, 0.0f, -1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void video_swap()
{
    SDL_GL_SwapWindow(sdl_window);
}

void video_cleanup()
{
    if (NULL != sdl_glcontext) {
        SDL_GL_DeleteContext(sdl_glcontext);
        sdl_glcontext = NULL;
    }

    if (NULL != sdl_window) {
        SDL_DestroyWindow(sdl_window);
        sdl_window = NULL;
    }

    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void video_set_window_title(const char *title)
{
    SDL_SetWindowTitle(sdl_window, title);
}
