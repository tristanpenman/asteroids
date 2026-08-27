# Dreamcast

In addition to Nintendo 64 and desktop/web targets, I would like to support a Sega Dreamcast target that compiles the existing shared game code with a new `src/platform/dreamcast` implementation. The first release should preserve desktop gameplay and presentation rather than add Dreamcast-only features.

The initial target is a self-bootable image that:

- boots on an emulator and retail Dreamcast hardware;
- displays the full game at a stable 60 Hz on NTSC/VGA systems and behaves correctly at 50 Hz on PAL systems;
- supports one standard controller, including hot-plugging;
- plays all five sound effects with the channel semantics expected by the game;
- loads and saves high scores on a VMU; and
- leaves `src/game` free of Dreamcast/KallistiOS headers and conditionals.

VMU LCD artwork, rumble, multiple controllers, network features, and a custom boot animation are useful follow-ups, but are not required for the first playable port.

## SDK and toolchain choice

### Recommended: KallistiOS

Use [KallistiOS (KOS)](https://github.com/KallistiOS/KallistiOS) and its `sh-elf` GCC/Newlib toolchain. KOS is the actively maintained, open-source, permissively licensed Dreamcast SDK. It already provides the services this port needs: PowerVR rendering, video modes, Maple controller and VMU access, AICA sound-effect playback, timers, an ISO9660 filesystem, `dcload`, and GDB support. It also supports the C99 used by this repository.

Pin a known-good KOS release or commit in the build environment. Do not build against an unpinned `master`, because toolchain defaults and APIs can change. Record the KOS version, GCC version, build flags, and required attribution in the release notes. The AICA ARM toolchain is not needed unless the project later introduces a custom sound driver; KOS supplies a standard driver.

Host setup options:

- Linux, macOS, and WSL: use KOS's `utils/kos-chain` toolchain builder and KOS environment directly.
- Native Windows: [DreamSDK](https://github.com/dreamsdk/dreamsdk) is a ready-to-use KOS distribution and is suitable for contributors who do not want to build the cross-toolchain.
- Reproducible project/CI builds: add a pinned container image or `Dockerfile.dreamcast`, following the existing N64 workflow. Building GCC and KOS on every game build would be too slow, so publish or cache the completed toolchain image separately.

### Other options considered

- Sega's Katana SDK is historical proprietary software. It is not appropriate for a public, reproducible open-source build and should not be a supported path.
- libronin is an alternative low-level homebrew library, but adopting it would mean replacing KOS's integrated peripheral, filesystem, audio, debug, and packaging ecosystem. It offers no clear advantage for this small port.
- SDL/KallistiGL/GLdc are libraries on top of KOS, not complete SDK choices. SDL would reduce some initial porting but retain desktop-oriented abstractions and bring unnecessary dependencies. KallistiGL or GLdc could reproduce the legacy OpenGL renderer quickly, but a small native PVR canvas should be more predictable and easier to tune. Keep GLdc as a time-boxed fallback if native rendering delays the first frame.

## Relevant hardware and software constraints

- The SH-4 is a 32-bit little-endian CPU with good single-precision floating point support. Keep the current `float`-based game math and compile with the KOS-supported single-precision ABI; avoid accidental `double` promotion in hot rendering paths.
- Memory is split into 16 MiB main RAM, 8 MiB video RAM, and 2 MiB sound RAM. This game is small, but PVR vertex buffers, framebuffers, loaded WAV samples, and duplicated disc assets should still have explicit budgets.
- The PowerVR2 is tile based. Rendering is submitted as polygon lists, not an OpenGL-style stream of native line primitives. Lines and points therefore need to become thin camera-facing quads/triangles, or use a compatibility library.
- The game uses centered, aspect-ratio-dependent coordinates. Preserve that contract and present a 4:3 image. Start at 640x480 RGB565 with double buffering. Detect cable/region and select a supported NTSC, PAL, or VGA mode; never assume VGA is attached.
- PAL may refresh at 50 Hz. Simulation is already fixed at 10 ms, so timing must use elapsed time instead of adding a hard-coded 16 ms per displayed frame. Rendering can occur at the display refresh rate without changing game speed.
- Maple devices can be inserted and removed while running. A VMU operation can also cause its controller to re-enumerate briefly. Input must reacquire the controller and avoid turning a disconnect into stuck buttons.
- A standard controller has D-pad, analogue stick, A/B/X/Y, Start, and analogue triggers, but no N64-style Z button. Map logical actions to Dreamcast buttons without exposing Maple button constants to game code.
- VMU capacity is small and writes are block-oriented. A BIOS-visible save needs Dreamcast package metadata and an icon. Storage writes must be bounded, checked for free space, and resilient to removal or a full VMU. Do not write every frame.
- Disc data is read-only and seek latency is high. Load the five small effects once at startup and keep gameplay assets compiled in or loaded in batches.
- There is no normal process exit on console. `cancel_main_loop()` should stop scheduling game work and enter a safe idle/return-to-loader path; cleanup must not rely on a desktop OS reclaiming resources.

## Architecture

Create this layout:

```text
src/platform/dreamcast/
    main.c
    canvas.c
    debug.c
    input.c
    loop.c
    mixer.c
    storage.c
    timing.c
    video.c
dreamcast/
    Makefile
    romdisk/
    ip.txt                 # optional dcload network settings; not committed
scripts/
    build-dreamcast.sh
Dockerfile.dreamcast
```

The Dreamcast Makefile should explicitly list the same `GAME_SRCS` as CMake and exactly one implementation of every platform contract. Define only `ASTEROIDS_PLATFORM_DREAMCAST=1`. Follow KOS's `Makefile.rules` conventions to produce an unscrambled ELF first, then optional `.bin` and self-boot `.cdi` artifacts. Keep Dreamcast build logic out of desktop CMake, matching the N64 precedent.

Before implementing the target, make two small cross-platform corrections:

1. Rewrite the SDL-specific comments in `video.h` as a platform-neutral lifecycle contract. Decide whether `video_set_window_title()` is an optional no-op or should be removed from the shared contract.
2. Replace `mixer_load_sample_from_file(path)` in game code with logical sample IDs, as already proposed in `notes/platforms.md`. Desktop may map IDs to WAV paths, N64 to bank entries, and Dreamcast to files in `/rd` or `/cd`. This avoids putting Dreamcast disc paths in shared code and also unblocks the incomplete N64 mixer.

### Lifecycle and main loop

`main.c` should initialize debug output, video/PVR, audio, input, timing, and storage, then call the same game initialization used by desktop. KOS owns low level startup, so no custom CRT or linker script should be needed.

Use a simple blocking console loop initially:

1. sample elapsed monotonic time;
2. update input and mixer bookkeeping;
3. call the selected `main_loop_fn_t`, allowing drawing when the PVR is ready;
4. submit the scene and wait/yield as required; and
5. apply a maximum elapsed-time clamp after pauses, VMU access, or debugging so the fixed-step simulation cannot spend seconds catching up.

`set_main_loop()` can safely switch callbacks between frames. `run_main_loop()` must not recursively start a new loop when game states change. Make `cancel_main_loop()` set a termination flag and preserve the requested exit code for `dcload`, even if a disc build ultimately idles or reboots.

### Video and canvas

Use KOS's native PVR API for the production renderer:

- initialize only the polygon lists actually used and size vertex buffers from measurements rather than accepting large defaults indefinitely;
- retain loaded shapes in main RAM as compact platform geometry;
- apply position, rotation, scale, and `canvas_origin` on the CPU;
- expand each line segment into a fixed-pixel-width quad and each point into a small quad, then submit opaque or translucent PVR vertices;
- close `CANVAS_LINE_STRIP` segments correctly and honor optional `uint16_t` indices;
- batch all geometry of one colour/list where practical, and submit once per frame; and
- preserve the meaning of `canvas_finish_drawing(false)`: finish a batch but do not display until a later call with `swap == true`.

Start with opaque white geometry on a black background. Add antialiasing or translucent edge quads only after parity and frame-time targets are met. Test line width on composite video as well as VGA; one physical pixel may be too thin on an interlaced television.

The title screen currently has no platform-neutral requirement for the N64 3D logo, so the Dreamcast port should go directly to the shared title screen.

### Input

Poll the first attached `MAPLE_FUNC_CONTROLLER` device each frame. Convert its state into the existing `active` and rising-edge `triggered` arrays. On detach, clear all state; on attach, establish the current state without generating false trigger events. Apply a dead zone and scale the analogue stick into the existing signed 8-bit range.

Initial physical mapping:

```text
Dreamcast D-pad / analogue stick -> movement and menu navigation
A                                -> thrust / confirm
B or right trigger               -> fire / back, according to game state
Start                            -> start / pause
```

The current input layer permits only one physical input mapping per logical enum value, while game actions may map several enum values to one handle. Use that existing model for the first port. Add trigger-axis enum values later only if treating a trigger as a button inside `dreamcast/input.c` is insufficient.

### Timing

Implement timing from a KOS monotonic timer with millisecond or better resolution. Accumulate the real elapsed duration, retain fractional precision between frames, and cap a single production interval (for example at 100 ms). Continue consuming the shared 10 ms fixed step. Verify game speed at 50 and 60 Hz and after controller/VMU hot-plug events.

### Audio

Use KOS's AICA sound-effect manager (`snd_sfx_*`) for the five short effects. Convert assets during the host build to the simplest supported mono PCM or ADPCM format, with sample rate and bit depth chosen after listening on hardware. Load them into sound RAM once and keep logical sample handles.

The platform mixer must emulate the shared contract rather than merely start sounds:

- allocate/reuse virtual channels deterministically;
- return a stable channel ID from play;
- report explicit stops and natural completion exactly once to the deferred callback from `mixer_update()`;
- support the pitch operation required by `level.c` before declaring parity; and
- free all AICA handles during cleanup.

If KOS cannot report natural completion with the required granularity, record sample duration and use the monotonic clock for channel completion. Confirm that retriggering thrust, phaser, and beat sounds matches the desktop baseline.

### Storage and VMU

Select the first writable VMU attached to the active controller, then fall back to other ports. Wrap the high-score payload in a `vmu_pkg_t` package with short and long descriptions, application ID, and a simple monochrome/icon asset so it is valid in the Dreamcast BIOS manager. Use the `/vmu/<port><slot>` filesystem or the corresponding KOS VMU APIs, but keep those paths internal to `storage.c`.

Because the shared storage API expects exact byte reads, validate package metadata and payload length, then copy only the requested payload. For writes, write a temporary package when the filesystem permits it and replace the old save only after success. Return the existing portable error codes for no VMU, open, read, and write failure; log the more specific KOS error in debug builds. Missing or corrupt saves should fall back to default scores without blocking the game.

The current storage API cannot distinguish no space, removal, or corrupt data. Keep the first implementation compatible, then consider adding portable error codes if tests show the UI needs to communicate those cases.

### Debugging and packaging

Route `debug_printf()` to KOS console output. Support three feedback loops:

- emulator: launch the ELF directly in a current Dreamcast emulator for the fastest normal iteration;
- hardware loader: use `dcload-ip` with a Broadband/LAN Adapter, or `dcload-serial` with a coder's cable, for rapid upload and console output;
- disc image: generate a self-boot `.cdi` for release testing on emulator, optical media, or an optical-drive emulator.

Add symbol-rich debug and optimized release configurations. Preserve the ELF and map file beside packaged outputs. Enable stack traces/assertions in debug builds and document GDB attachment separately; do not make physical debug hardware a prerequisite for ordinary development.

## Implementation sequence

### Phase 0: freeze the baseline and interfaces

- Run the desktop smoke test from `notes/platforms.md` and capture reference screenshots/audio behavior.
- Finish the logical-sample mixer change and neutralize `video.h` comments.
- Add compile-time checks for assumptions that matter on SH-4 (`sizeof(float)`, integer widths, shape limits, and save payload size).
- Confirm desktop and N64 builds remain unchanged.

Exit criterion: shared game code has no asset paths or API wording that forces an SDL implementation.

### Phase 1: reproducible toolchain and hello frame

- Pin KOS/toolchain versions and create the cached container build.
- Add the Dreamcast Makefile, explicit source lists, and build script.
- Build a minimal ELF, initialize a safe video mode, clear to black, and print diagnostics.
- Run that ELF in an emulator and on hardware before integrating the game.

Exit criterion: one documented command produces an ELF and optional CDI, and a black frame appears on both emulator and hardware.

### Phase 2: playable silent slice

- Implement timing, loop scheduling, controller input, and the basic native PVR canvas.
- Stub mixer and storage behind visible debug warnings.
- Enter the shared title screen, start a game, move, thrust, fire, pause, die, and return through all game states.
- Fix platform contract bugs in the contract or Dreamcast layer, not with Dreamcast `#ifdef`s in `src/game`.

Exit criterion: the entire game is playable without audio or persistent scores at correct speed under NTSC, PAL, and VGA configurations.

### Phase 3: audio and persistence

- Add host-side sound conversion and AICA playback/channel tracking.
- Implement VMU discovery, packaged high-score reads/writes, free-space errors, and removal handling.
- Test a new VMU, existing save, corrupt save, full VMU, no VMU, and removal during access.

Exit criterion: audio behavior matches desktop and scores survive a reboot in a BIOS-visible save.

### Phase 4: hardware hardening and release

- Profile CPU frame time, PVR list/buffer use, main RAM, VRAM, and sound RAM.
- Soak-test every game state and repeated state transitions for at least 30 minutes.
- Test composite/RGB and VGA output where equipment is available, controller and VMU hot-plugging, 50/60 Hz, and at least one emulator plus retail hardware.
- Verify clean boot from the final CDI, asset paths and filename case, KOS attribution, and release artifact checksums.
- Add optional rumble and VMU LCD output only after the release criteria pass.

Exit criterion: no gameplay regressions, stable frame pacing, no PVR overflows or unbounded allocations, and a reproducible release image.

## Risks and decisions to validate early

- **Line appearance:** native expanded quads may differ from legacy OpenGL and N64 lines. Decide an acceptable pixel width from real composite and VGA output before optimizing the renderer.
- **Mixer completion semantics:** verify what KOS exposes for natural AICA completion before building the channel abstraction around it.
- **PAL behavior:** establish whether the first release supports PAL50, PAL60, or both, and make that choice visible rather than silently forcing an unsupported mode.
- **Save atomicity:** validate the chosen VMU replacement sequence on removal and full-card failures. The payload is small, but corruption recovery matters more than write speed.
- **Input language:** game screens should use neutral prompts or platform labels; keyboard-only text would make the port technically playable but unfinished.
- **Build reproducibility:** pinning only KOS while downloading a floating GCC or packaging tool is insufficient. Capture the complete image inputs and licenses.

## First concrete work items

1. Introduce logical audio sample IDs on desktop and complete the same contract for N64.
2. Add `Dockerfile.dreamcast`, `dreamcast/Makefile`, and `scripts/build-dreamcast.sh` with pinned KOS inputs.
3. Add skeletal Dreamcast implementations that build and log unsupported calls.
4. Bring up video plus a hard-coded PVR line, then implement `canvas` geometry expansion with host tests.
5. Connect the title screen and controller, then complete timing and all game states before starting VMU or audio work.

## Primary references

- [KallistiOS repository, capabilities, examples, and license](https://github.com/KallistiOS/KallistiOS)
- [KOS toolchain builder](https://github.com/KallistiOS/KallistiOS/blob/master/utils/kos-chain/README.md)
- [KOS PowerVR initialization](https://kos-docs.dreamcast.wiki/group__pvr__init.html)
- [KOS video API and cable/video modes](https://kos-docs.dreamcast.wiki/video_8h.html)
- [KOS Maple API](https://kos-docs.dreamcast.wiki/maple_8h.html)
- [KOS sound-effect API](https://kos-docs.dreamcast.wiki/group__audio__sfx.html)
- [KOS filesystem and VMU FAQ](https://kos-docs.dreamcast.wiki/md_doc_2FAQ.html)
- [DreamSDK](https://github.com/dreamsdk/dreamsdk)
