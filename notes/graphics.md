# Graphics

This proposal introduces dynamic graphics memory first, makes its lifetime safe across submitted frames second, and only then separates frame submission from the 2D canvas so that a 3D phase can be added. Each section assumes that the previous section has already been implemented.

## 1. Dynamically Allocate Shape Data and Display Lists

Replace the fixed-size shape storage and display-list arrays with backend-owned dynamic allocations.

Creating a shape should allocate and copy all data needed by the backend. The desktop backend may retain copied points and indices directly. The N64 backend should allocate aligned `Vtx` data, convert the supplied points, and copy the indices. The caller may release its source arrays as soon as shape creation returns successfully. Shape creation should return an invalid handle if validation or allocation fails, without retaining a partially created shape.

Each frame should also allocate its display-list storage dynamically. A backend may retain completed frame allocations for reuse, but capacity should grow in response to measured demand rather than being imposed by fixed global arrays. Growth must occur while the frame is being recorded and before it is submitted. Allocation failure should reject the current draw or frame without writing beyond the available storage.

Platform-specific alignment and address conversion remain backend responsibilities. In particular, N64 display lists and vertex data must be suitable for RSP access before submission.

### Code and API Changes

Replace `canvas_load_shape()` and the global `canvas_reset()` lifecycle with explicit shape creation and destruction:

```c
typedef int canvas_shape_id;

#define CANVAS_INVALID_SHAPE_ID (-1)

struct canvas_shape_data
{
    const float *vertices;
    size_t vertex_count;
    const uint16_t *indices;
    size_t index_count;
    enum canvas_primitive primitive;
};

canvas_shape_id canvas_shape_create(const struct canvas_shape_data *data);
void canvas_shape_destroy(canvas_shape_id shape);
```

Callers should retain shape handles in the module that owns them. For example, `draw_load_shapes()` should create its ship, shard, and particle shapes, while a corresponding `draw_unload_shapes()` should destroy them. Other screens should no longer need to reload every shape after a global reset.

The N64 backend should replace its fixed `vertices`, `shapes`, and display-list arrays with allocated records. A small internal display-list builder should track a command pointer, used count, and capacity. It should reserve space before emitting each command and grow the buffer only while recording. The desktop backend should follow the same ownership contract even if its native renderer does not require a command buffer.

## 2. Keep Memory Valid for the Duration of a Frame

Introduce an explicit frame object that owns every allocation used while recording and executing that frame. This includes display lists, transforms, immediate geometry, and references to retained shapes.

Drawing a retained shape should add a reference from the active frame to that shape. Destroying a shape invalidates its handle immediately, but its backing allocation is freed only after every recorded or submitted frame that references it has completed. Immediate geometry and transforms should be copied into frame-owned storage before the drawing call returns, so stack data and caller-owned arrays never need to outlive the call.

Frames should move through three states: available, recording, and submitted. A submitted frame and all of its allocations remain untouched until the platform reports that its graphics work has completed. Only then may the frame release deferred shape references and either free or reuse its storage. A small frame ring can provide another available frame while earlier frames are still executing.

No display list or referenced allocation may be resized, freed, or reused after submission. This rule provides the lifetime guarantee required by asynchronous N64 graphics tasks and is also safe for desktop backends.

### Code and API Changes

Add a backend-internal frame ring and move all transient drawing storage into each frame entry. A representative N64 layout is:

```c
enum graphics_frame_state
{
    GRAPHICS_FRAME_AVAILABLE,
    GRAPHICS_FRAME_RECORDING,
    GRAPHICS_FRAME_SUBMITTED
};

struct n64_graphics_frame
{
    enum graphics_frame_state state;
    struct n64_display_list display_list;
    struct gfx_transform *transforms;
    size_t transform_count;
    canvas_shape_id *shape_references;
    size_t shape_reference_count;
};
```

`canvas_start_drawing()` should acquire an available frame and make it the recording frame. `canvas_draw_shape()` should append the shape handle to that frame's reference list before emitting commands. `canvas_finish_drawing()` should submit the frame and mark it unavailable until the platform completion callback releases its references.

Each retained shape record should track whether its public handle is alive and how many frames still reference its backing allocation. `canvas_shape_destroy()` should invalidate the handle immediately and free the allocation only when that reference count reaches zero. Any API accepting immediate vertices should copy them into the recording frame's storage before returning.

These changes are internal to the canvas lifecycle at this stage. Keeping the existing start and finish calls until frame ownership is proven avoids combining the lifetime work with the later 3D API migration.

## 3. Add a 3D Phase Before the 2D Canvas

Once frames own their memory and lifetime, move clearing, task submission, and buffer presentation out of the canvas and into a graphics-frame interface. The intended order is:

```text
begin frame -> optional 3D phase -> optional 2D canvas phase -> end frame
```

`graphics_begin_frame()` should acquire frame storage and clear the selected framebuffer. A later 3D interface can record one or more views into the frame. `canvas_begin()` should then establish an orthographic native-line phase without clearing, and `canvas_end()` should finish that phase without presenting. `graphics_end_frame()` should submit all recorded phases and arrange a single presentation after the final phase.

On N64, a frame containing both phases maps naturally to an F3DEX2 task followed by an L3DEX2 task. The 3D task renders without swapping, and the canvas task renders to the same framebuffer and performs the swap. Each task owns a separate dynamically allocated display list within the same frame and establishes all state required by its microcode. A 3D-only frame, canvas-only frame, and empty optional phase must also remain valid.

Restricting the canvas to the final phase avoids alternating microcodes and keeps task ordering, memory ownership, and presentation predictable. The frame object retains all allocations and shape references required by both tasks until both have completed.

### Code and API Changes

Add a platform graphics interface and make the canvas an ordered phase within it:

```c
bool graphics_begin_frame(struct graphics_color clear_color);

bool graphics_begin_view3d(
    const struct graphics_rect *viewport,
    const struct graphics_camera *camera);
void graphics_end_view3d(void);

bool canvas_begin(const struct canvas_rect *viewport, enum canvas_space space);
void canvas_end(void);

bool graphics_end_frame(void);
```

Replace `canvas_start_drawing(clear)` with `graphics_begin_frame(clear_color)` followed by `canvas_begin()`. Replace `canvas_finish_drawing(swap)` with `canvas_end()` followed by `graphics_end_frame()`. Clearing and swapping should then disappear from both canvas backends.

Add `graphics.h` plus desktop and N64 `graphics.c` implementations. The frame record should contain separate 3D and canvas display-list builders, while its shared reference list keeps resources for both phases alive. The N64 backend should submit the F3DEX2 list with no swap, then submit the L3DEX2 list with the swap flag. If only one phase exists, that phase becomes the final task and performs the swap.

Game drawing should be split into phase-specific functions, such as `draw_world_3d()` and `draw_canvas_2d()`, called between the matching begin and end functions. Existing canvas-only screens can use the same graphics frame lifecycle without opening a 3D phase.
