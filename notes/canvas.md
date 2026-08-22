# Canvas

This note explores ideas for an intermediate-mode 2D renderer, in which vector shapes are retained as resources, while color, transform, and draw order are submitted imperatively each frame.

The canvas contract is specifically for rasterized line primitives. On N64 it is implemented with L3DEX2 and `gSPLine3D` or `gSPLineW3D`. On desktop it is implemented with the platform's native line primitive. A conforming backend should not simply turn canvas lines into triangle strips.

## Design Position

The canvas is not a general 2D painting API. It is a small vector-line renderer with these properties:

- Vertices and segment topology define a shape.
- Shapes may be loaded once and drawn many times.
- Immediate vertices may also be supplied during a frame.
- Draw calls use the current canvas color and line width.
- Each draw has a 2D translation, rotation, and non-uniform scale.
- Lines use the backend's native rasterization rules.
- The canvas occupies an ordered phase of a graphics frame.
- Clearing, task submission, and framebuffer swapping belong to the graphics frame, not the canvas.

This deliberately does not promise joins, caps, miter limits, antialiasing equivalence, or pixel-identical line width across platforms. Those are properties of a stroke tessellator, which this design explicitly avoids.

## Why Native Lines

Native lines are a good match for Asteroids:

- Its assets are already vertices plus line-segment indices.
- L3DEX2 directly transforms and clips those vertices.
- A segment needs two indices rather than generated quad geometry.
- Shape scaling and rotation remain matrix operations.
- The vector appearance comes from the line rasterizer rather than constructed polygons.
- Static shapes do not need preprocessing into width-specific meshes.

The cost is that line rendering is less controllable. Connected segments can overlap or leave rasterization artifacts at corners, line-width behavior differs between N64 and desktop, and future backends may not expose useful native wide lines. Those differences are accepted as part of this API rather than hidden behind triangle conversion.

## Terminology

This design uses **intermediate mode** to mean that resources are retained but scene instances are not:

- `canvas_shape_create()` retains immutable vertices and topology.
- `canvas_draw_shape()` immediately records one instance into the active frame.
- The canvas does not retain a scene graph or redisplay old instances automatically.
- Setters affect subsequent draws in the current canvas phase.

This is close to the current Asteroids model while making resource and frame ownership explicit.

## Core Types

```c
#ifndef OMNIGETRON_CANVAS_H
#define OMNIGETRON_CANVAS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef int canvas_shape_id;

#define CANVAS_INVALID_SHAPE_ID (-1)

struct canvas_color
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;
};

struct canvas_point
{
    float x;
    float y;
};

struct canvas_transform
{
    struct canvas_point position;
    float rotation; /* radians, counter-clockwise */
    struct canvas_point scale;
};

struct canvas_rect
{
    int x;
    int y;
    int width;
    int height;
};

enum canvas_space
{
    CANVAS_SPACE_NORMALIZED,
    CANVAS_SPACE_PIXELS
};

#endif
```

Normalized space is centered in the viewport, has positive x to the right and positive y upward, and gives the visible area a height of 2. The x extent follows the viewport aspect ratio. Pixel space starts at the viewport's top-left and has positive y downward.

The initial implementation should use normalized space for Asteroids shapes and pixel space for HUD elements. A shape's points are expressed in the active canvas space; shape handles are therefore not implicitly tied to a particular resolution.

The active space may change within a canvas phase. Changing it updates the orthographic projection but does not end the phase or create another graphics task. This allows normalized game geometry and a pixel-space HUD to share one physical L3DEX2 task.

## Shape Data and Lifetime

```c
struct canvas_shape_data
{
    const struct canvas_point *points;
    size_t point_count;

    /* Optional index pairs: a0, b0, a1, b1, ... */
    const uint16_t *segments;
    size_t segment_count;

    /* Used only when segments is NULL. */
    bool closed;
};

canvas_shape_id canvas_shape_create(const struct canvas_shape_data *data);
void canvas_shape_destroy(canvas_shape_id shape);
```

When `segments` is non-null, each pair describes one independent line. When it is null, `segment_count` is assumed to be zero, and consecutive points form a line strip; `closed` optionally adds the final-to-first segment.

Creation validates every index and copies or compiles the data before returning. The caller may release the source arrays after a successful call.

Calling `canvas_shape_destroy()` invalidates the handle for future draw calls immediately. The backing storage remains alive until the end of the last frame that references the shape, including a frame still being recorded, after which the backend may reclaim it. This permits a presentation module to release its resources without first waiting for queued N64 graphics tasks to finish. Implementations may use deferred frees, frame reference lists, or another equivalent mechanism.

On N64, shape creation converts points to aligned `Vtx` storage and retains the segment indices. Shapes larger than the L3DEX2 vertex cache are split into internal batches. Splitting a line list is straightforward because every segment is independent. Splitting a line strip may duplicate its boundary vertex so the visual sequence remains continuous.

There is no global `canvas_reset()`. The presentation module that creates a shape destroys it during its own cleanup. A resource-group helper can be added later if screen-wide destruction becomes repetitive.

## Frame and Canvas Lifecycle

The canvas is part of the graphics frame rather than an independent presenter:

```c
bool graphics_begin_frame(struct graphics_color clear_color);

/* Optional 3D phase. */
bool graphics_begin_view3d(
    const struct graphics_rect *viewport,
    const struct graphics_camera *camera);
void graphics_end_view3d(void);

/* Optional native-line phase. */
bool canvas_begin(
    const struct canvas_rect *viewport,
    enum canvas_space space);
bool canvas_set_space(enum canvas_space space);
void canvas_end(void);

bool graphics_end_frame(void);
```

Only `graphics_begin_frame()` clears the selected framebuffer. Only `graphics_end_frame()` submits the completed work and arranges presentation. `canvas_begin()` establishes an orthographic projection, viewport, scissor, and default line state; it does not clear. `canvas_set_space()` changes between normalized and pixel coordinates inside the active phase without flushing or starting another task; color and line width are preserved. `canvas_end()` closes the logical line phase but does not swap.

The first N64 implementation should support this ordering:

```text
zero or more 3D views -> zero or one canvas phase -> present
```

Restricting the native-line canvas to the end of a frame is useful rather than arbitrary. F3DEX2 and L3DEX2 require separate tasks, so alternating 3D and canvas work would multiply tasks and complicate capacity and lifetime management. If a later game needs a 2D background before 3D, the frame recorder can be generalized to ordered typed phases.

A canvas-only frame is valid. A frame without a canvas is also valid.

## Stateful Drawing API

The canvas exposes a deliberately small immediate state:

```c
void canvas_set_color(struct canvas_color color);
bool canvas_set_line_width(float width);

bool canvas_draw_shape(
    canvas_shape_id shape,
    const struct canvas_transform *transform);
```

`canvas_begin()` resets color to opaque white and width to the backend's default line width. Setters affect subsequent calls until changed or until `canvas_end()`.

Stateful color is retained because it closely matches both the Asteroids API and the RDP primitive-color state. It avoids repeating style objects on every glyph or asteroid draw. The state is local to the current canvas phase and never leaks into a later frame.

The transform is copied during the call. Passing `NULL` means identity. Non-uniform scaling is supported because the current Asteroids renderer uses it.

`canvas_set_line_width()` returns `false` when a requested width is outside the portable supported range. The initial portable range is `1.0f` through `8.0f`, inclusive. The API does not claim subpixel precision or exact agreement between N64 and desktop. A width of `1.0f` means the default nominal one-pixel line.

## Immediate Line Submission

Dynamic debug shapes and effects should not need retained handles:

```c
bool canvas_draw_line(
    struct canvas_point a,
    struct canvas_point b,
    const struct canvas_transform *transform);

bool canvas_draw_line_strip(
    const struct canvas_point *points,
    size_t point_count,
    bool closed,
    const struct canvas_transform *transform);

bool canvas_draw_lines(
    const struct canvas_point *points,
    size_t point_count,
    const uint16_t *segments,
    size_t segment_count,
    const struct canvas_transform *transform);
```

These functions consume or copy all input before returning. On N64 they copy converted points into a bounded, aligned per-frame vertex pool and emit native line commands. They do not generate triangles.

`canvas_draw_lines()` uses explicit segment pairs and is the most direct representation of current Asteroids shape data. `canvas_draw_line_strip()` is a convenience for consecutive segments. Both may be internally split into vertex-cache-sized batches.

There is deliberately no point primitive. Native point support and rasterization are poor across the intended backends. Effects that would otherwise use points, including the current desktop Asteroids explosion particles, should use short line segments instead.

Every capacity-limited call returns `false` without writing past a pool or display list. Debug builds should report whether points, transforms, indices, display-list commands, or task slots were exhausted.

## Text and Fonts

Fonts and text are a layer above the canvas, not part of the canvas backend contract. Asteroids text is a sequence of retained vector glyph shapes, so a font helper can create glyph shapes, measure text, and submit each glyph with `canvas_draw_shape()`:

```c
typedef int text_font_id;

struct text_style
{
    struct canvas_color color;
    float scale;
    float tracking;
};

bool text_draw(
    text_font_id font,
    struct canvas_point position,
    const char *text,
    const struct text_style *style);

float text_measure(
    text_font_id font,
    const char *text,
    const struct text_style *style);
```

This is an illustrative higher-level API rather than a required canvas interface. Font creation, destruction, character mapping, and glyph metrics belong to that helper. Its implementation temporarily applies the text color, draws each glyph with `canvas_draw_shape()`, and restores the previous color. Alternatively, it can use an internal non-stateful draw helper to avoid observable state changes. Glyphs remain ordinary retained native-line geometry and obey the same deferred lifetime rules as other shapes.

Filled rectangles, filled triangles, bitmap glyphs, and images do not belong to the native-line canvas. They can later be supplied by a separate 2D sprite or RDP-rectangle phase. Keeping that distinction explicit prevents the line API from gradually becoming a general renderer with incompatible implementation paths.

## N64 Mapping

An N64 frame containing 3D and lines maps to two graphics tasks:

```text
Task 1: F3DEX2, render 3D, NU_SC_NOSWAPBUFFER
Task 2: L3DEX2, render native lines, NU_SC_SWAPBUFFER
```

Both tasks target the framebuffer selected at `graphics_begin_frame()`. The first task performs the color and depth clear. The line task does not clear and normally disables depth testing so it acts as an overlay.

If the frame contains only 3D, the F3DEX2 task swaps. If it contains only canvas lines, the L3DEX2 task clears and swaps. The backend, not the caller, selects these flags.

Each task must install a complete baseline appropriate to its microcode. The L3DEX2 task establishes:

- framebuffer address;
- viewport and scissor;
- orthographic projection and identity model-view state;
- one-cycle mode;
- primitive-color combine mode;
- native line render mode;
- geometry modes required by the line microcode;
- texture disabled;
- depth behavior appropriate to an overlay.

No RSP state is assumed to survive the microcode boundary. RDP state may physically persist, but relying on the previous task's state would make each task fragile and difficult to debug.

The backend needs per-frame storage for both task types:

```c
struct n64_graphics_frame
{
    struct n64_triangle_task triangles;
    struct n64_line_task lines;
};
```

Each task owns its display list and dynamic resources. The frame also records references to retained shapes so their storage cannot be reclaimed while either task may still use it. A frame-ring entry remains busy until every task using it has completed. The last submitted task is responsible for presentation.

## Direct Line Emission

A retained shape draw maps approximately to:

```c
gSPMatrix(/* translation */, G_MTX_LOAD | G_MTX_MODELVIEW | G_MTX_NOPUSH);
gSPMatrix(/* rotation */,    G_MTX_MUL  | G_MTX_MODELVIEW | G_MTX_NOPUSH);
gSPMatrix(/* scale */,       G_MTX_MUL  | G_MTX_MODELVIEW | G_MTX_NOPUSH);
gSPVertex(/* retained Vtx batch */);
gDPSetPrimColor(/* current canvas color */);

for (i = 0; i < segment_count; ++i) {
    gSPLineW3D(/* segment endpoints and current native width */);
}
```

This is illustrative backend code, not a guarantee of exact command ordering. The backend should cache color and width state so repeated calls do not emit redundant RDP commands. It should also reserve display-list termination and synchronization commands before accepting a draw.

## Desktop Mapping

The desktop backend should also use line primitives directly. For an OpenGL compatibility implementation:

```text
explicit segments -> GL_LINES
open strip        -> GL_LINE_STRIP
closed strip      -> GL_LINE_LOOP
```

Color and line width map to native state, and the transform maps to the model-view matrix or a small vertex shader. No CPU or GPU path should expand lines into triangles.

Desktop output will not exactly match L3DEX2. In particular, wide-line ranges, antialiasing, endpoint coverage, and joins are implementation-dependent. The desktop backend should aim for the same topology, transform, nominal width, and color rather than pixel identity.

If a future graphics API has no suitable native line support, that backend may be unsupported for this canvas contract. Silently tessellating would violate the selected semantics; an explicitly different canvas implementation should instead be introduced.

## Capacity

The first portable minimums can follow the current Asteroids scale:

```c
#define CANVAS_MAX_SHAPES             64
#define CANVAS_MAX_RETAINED_POINTS   256
#define CANVAS_MAX_TRANSFORMS        128
#define CANVAS_MAX_DYNAMIC_POINTS    256
#define CANVAS_MAX_SEGMENTS          512
```

These are portable guarantees, not necessarily the exact backend array sizes. Retained points and per-frame dynamic points should be accounted separately. Display-list command capacity must also be checked before every emission rather than asserted after writing.

If one canvas phase cannot fit, the preferred response is to return `false` and increase a measured capacity. An optional `canvas_flush()` could split the canvas into another ordered L3DEX2 task without swapping, but it should only be added if real content requires it. Automatic hidden task proliferation makes performance and frame resource use harder to reason about.

## Example

```c
static const struct canvas_color player_color = {255, 255, 255, 255};

if (!graphics_begin_frame(background)) {
    return;
}

graphics_begin_view3d(&viewport, &camera);
draw_world();
graphics_end_view3d();

if (canvas_begin(&viewport, CANVAS_SPACE_NORMALIZED)) {
    struct canvas_transform ship_transform = {
        ship.position,
        ship.rotation,
        ship.scale
    };

    canvas_set_color(player_color);
    canvas_set_line_width(1.0f);
    canvas_draw_shape(ship_shape, &ship_transform);

    canvas_set_space(CANVAS_SPACE_PIXELS);
    text_draw(font, score_position, score, &score_style);
    canvas_end();
}

graphics_end_frame();
```

## Migration from Asteroids

| Current Asteroids call            | Native-line canvas equivalent                                      |
|-----------------------------------|--------------------------------------------------------------------|
| `canvas_reset()`                  | destroy owned shape handles during presentation cleanup            |
| `canvas_load_shape()`             | `canvas_shape_create()`                                            |
| `canvas_start_drawing(clear)`     | `graphics_begin_frame()` followed by `canvas_begin()`              |
| `canvas_continue_drawing()`       | reserve a future `canvas_flush()` for measured capacity pressure   |
| `canvas_set_colour()`             | `canvas_set_color()` with byte color components                    |
| `canvas_draw_shape()`             | `canvas_draw_shape()`                                              |
| `canvas_finish_drawing(swap)`     | `canvas_end()` followed eventually by `graphics_end_frame()`       |

The color conversion must be corrected during migration. The current desktop implementation treats components as normalized floats, while the current N64 implementation casts the same values directly to bytes. The new API consistently uses byte components.

The current desktop explosion effect uses points. It should be migrated to short dynamic line segments rather than adding a point primitive to the canvas. Existing Asteroids coordinates also use positive y downward, so normalized-space positions and rotations must be converted when adopting the centered, positive-y-up convention.

## Recommended First Implementation

1. Add frame ownership capable of holding one F3DEX2 task and one L3DEX2 task.
2. Implement `canvas_begin()`, state setters, retained shapes, and `canvas_draw_shape()` on N64 using native line commands.
3. Add the equivalent direct-line desktop backend.
4. Move the Asteroids vector font helper onto retained canvas shapes, keeping it above the canvas API.
5. Exercise canvas-only frames, 3D-then-canvas frames, and normalized-to-pixel space changes within one canvas task.
6. Verify deferred shape destruction while recorded and submitted frames still reference the shape.
7. Verify that the first task never swaps, the final task always swaps, and both tasks use the same framebuffer.
8. Measure display-list commands, transform use, RSP time, and task memory before adjusting capacities.

This approach treats native rasterized lines as a deliberate cross-platform feature rather than an N64-only optimization. It preserves Asteroids' compact geometry and drawing model while moving frame submission and resource ownership into a safer shared graphics lifecycle.
