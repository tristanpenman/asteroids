# Issues

Reviewed 2026-08-24. Both the desktop CMake target and the N64 ROM build successfully. The N64 findings below are therefore runtime/behavioural issues, not compilation failures.

## High-priority findings

### 1. N64: the leaderboard exhausts the per-frame transform pool

`leaderboard_draw()` draws ten rows of roughly 20 glyphs plus its prompt. Each glyph consumes one of only 128 entries in `transforms`. Splitting the work with `canvas_finish_drawing(false)` and `canvas_continue_drawing()` advances to a new display list, but does not reset `num_transforms` (and cannot safely reuse those matrices while earlier tasks are pending). Once the 128th glyph is reached, `canvas_drawasteroids/cmake-build-debug_shape()` silently returns false, so the lower rows and prompt are partially or wholly absent.

Provide enough task-owned transform storage for the worst case, or associate a separate checked transform arena with each submitted display list and retain it until that task finishes.

### 2. N64: display-list overflow is detected only after memory has been overwritten

Every rendering command increments the global `glistp` without a capacity check. `canvas_finish_drawing()` compares the final size with `GFX_GLIST_LEN`, but by then commands beyond 2,048 entries have already been written outside the selected list. The function also appends the full-sync and end commands before checking. An elaborate frame can corrupt the next list or unrelated state, and returning without submitting does not undo that corruption.

Reserve space before emitting each command/shape (including the two terminators), or use a checked command builder. Treat an exactly full list consistently: the current `>= GFX_GLIST_LEN` test rejects a list whose final size is exactly the declared capacity.

### 3. N64: gameplay audio can never play

`mainproc()` initializes NuSystem audio, but starts the game with `game_init(true)`, which deliberately skips sample loading and channel callback setup. Even if that argument is changed, the N64 mixer implementation returns `MIXER_INVALID_SAMPLE` for every load and `MIXER_INVALID_CHANNEL` for every play; stop and completion handling are also stubs. Calls in `level.c` therefore produce no beats, phaser, explosion, or thruster sound despite the banks being linked.

The platform mixer should map the five logical sample IDs to the sound bank and track NuSystem handles. The game should not be initialized as silent after audio initialization succeeds.

### 4. N64: saved high scores are never loaded

The desktop startup calls `highscores_load()` and creates defaults when loading fails. N64 `mainproc()` merely calls `storage_available()` and discards the result. The in-memory table therefore starts zero-initialized on every boot. The first qualifying score can overwrite the Controller Pak note with a table that has forgotten all results from previous runs. N64 startup should perform the same load/default initialization after controller initialization, with Pak errors reported without preventing play.

## Other correctness findings

### 1. N64 simulation runs at the wrong rate and assumes NTSC

Each graphics callback adds `1000 / 60`, i.e. 16 ms, rather than 16.666... ms. At 60 callbacks per second the simulation advances only 960 ms per real second (4% slow). It is also tied to the graphics callback and becomes substantially wrong on a 50 Hz PAL schedule. Use the platform timer/retrace rate with a fractional or integer-remainder accumulator; do not truncate every frame.

### 2. Desktop OpenGL attributes are requested after creating the window

`video_init()` calls `SDL_CreateWindow()` before `SDL_GL_SetAttribute()` requests the OpenGL version, depth size, and double buffering. SDL requires context attributes to be set before creating the OpenGL window/context. The later attempt to enable double buffering after querying the created context cannot change that context. This happens to work where driver defaults match, but can create an unexpected context or fail the double-buffer check on other systems.

Move all `SDL_GL_SetAttribute()` calls ahead of `SDL_CreateWindow()` and check their return values.

### 3. Desktop audio loading has unchecked size/allocation failures

`mixer_load_sample_from_file()` does not check either `fseek()`/`ftell()` or `malloc(sz)`. A failed `ftell()` yields a negative length that is converted to a huge allocation size; a failed allocation is then passed to `fread()`. It also casts the file length to `int` for `SDL_RWFromMem()` without checking `INT_MAX`. Malformed, non-seekable, or very large sample inputs can therefore cause invalid memory access or truncation. Validate every result and free the temporary buffer on every failure path.

### 4. Desktop storage cannot detect a read error

`storage_read()` stores `fread()`'s unsigned `size_t` result in `sz` and tests `sz < 0`, which is always false. A read error is returned as an ordinary short read. Use `ferror(f)` before closing the file and return `STORAGE_ERR_READ_FILE` when set.

### 5. Desktop input's public functions permit out-of-bounds access

Unlike the N64 implementation, desktop `input_map()`, `input_active()`, and `input_triggered()` do not validate handles or input enum values. Passing a failed registration result (`-1`) or invalid enum indexes static arrays out of bounds. Current game initialization stays within the limit, so this is a latent contract bug rather than a presently observed screen failure. Make desktop validation match N64 validation.

### 6. Desktop tick-wrap handling loses one millisecond

When `SDL_GetTicks()` wraps, `produce_simulation_time()` adds `UINT32_MAX - ticks` and then `now`, omitting the inclusive wrap step. Unsigned subtraction already provides the correct modulo delta, or the manual calculation needs an additional one. This occurs only after roughly 49.7 days of uptime.

## Static versus dynamic allocation on N64

There is a real memory-management benefit, but not a rendering-speed benefit by itself.

The current default high-resolution renderer statically reserves approximately:

| Resource | Size |
| --- | ---: |
| 15 display lists (2,048 `Gfx` each) | 245,760 bytes |
| Three 640x480x16-bit colour buffers | 1,843,200 bytes |
| One 640x480x16-bit depth buffer | 614,400 bytes |
| Total in `gfx.c` | about 2.58 MiB (plus padding/state) |

The link map reports all of `gfx.o` as 0x294090 bytes and total BSS as 0x2d7518 bytes, leaving relatively little of a 4 MiB console for stacks, audio heap, and future assets. The current leaderboard can submit eleven tasks from a single callback, which explains why many list slots were provisioned, but that burst itself should be redesigned or measured rather than making fifteen large slots an unquestioned global cost.

Dynamic allocation can help if it means *one-time, checked allocation from a rendering arena*:

- choose 320x240 or 640x480 buffers after detecting available RAM;
- allocate only the number of display lists that can actually be in flight;
- size command buffers from measured high-water marks plus a safety margin;
- reuse mutually exclusive logo/game scratch memory; and
- fail cleanly or fall back to a lower-memory configuration.

It does not make RSP/RDP rendering faster, create more physical memory, or make an oversized frame safe. Framebuffers, matrices, vertices, and display lists must remain alive and suitably aligned until their graphics task completes, and their cache/physical-address requirements still apply. Per-frame `malloc()` or `realloc()` would add fragmentation, nondeterministic failure, and lifetime bugs; in particular, a display list must never move while the RSP is consuming it.

Recommendation: keep immutable shape geometry static, replace the excessive fixed display-list bank with a small checked pool, and use an aligned arena initialized once for resolution-dependent framebuffers and render scratch. This captures the useful flexibility of dynamic allocation while retaining deterministic N64 rendering. Independently of allocation strategy, fix the pre-write display-list bounds checking first.
