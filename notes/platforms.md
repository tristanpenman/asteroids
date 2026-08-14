# Platforms

This note discusses making the project platform-agnostic.

The aim is to treat the current `asteroids` repository as the canonical project and current gameplay implementation and to import the code from [asteroids64](https://github.com/tristanpenman/asteroids64) as a platform port, not as a second copy of the game.

The main objective is:

```text
Shared game code
    ├── desktop platform (SDL2 + OpenGL + SDL_mixer)
    └── N64 platform (NuSystem + RSP/RDP + Controller Pak)
```

“Desktop” should first reproduce the current native build with no intentional gameplay changes. Emscripten can initially remain a desktop-platform variant, then be separated later if its differences become substantial.

Later I would like to add Sega Dreamcast as a supported platform.

## Target source layout

```text
src/
├── game/
│   ├── canvas.h
│   ├── collision.c/.h
│   ├── data.c/.h
│   ├── draw.c/.h
│   ├── entities.c/.h
│   ├── game.c/.h
│   ├── gameover.c/.h
│   ├── highscores.c/.h
│   ├── initials.c/.h
│   ├── leaderboard.c/.h
│   ├── level.c/.h
│   ├── sandbox.c/.h
│   ├── titlescreen.c/.h
│   ├── transition.c/.h
│   ├── vec.c/.h
│   └── shared types and constants
│
├── platform/
│   ├── debug.h
│   ├── input.h
│   ├── loop.h
│   ├── mixer.h
│   ├── storage.h
│   ├── timing.h
│   ├── video.h
│   ├── rumble.h
│   │
│   ├── desktop/
│   │   ├── main.c
│   │   ├── canvas.c
│   │   ├── debug.c
│   │   ├── input.c
│   │   ├── loop.c
│   │   ├── mixer.c
│   │   ├── storage.c
│   │   ├── timing.c
│   │   └── video.c
│   │
│   └── n64/
│       ├── main.c
│       ├── canvas.c
│       ├── debug.c
│       ├── gfx.c/.h
│       ├── input.c
│       ├── loop.c
│       ├── mixer.c
│       ├── rumble.c
│       ├── storage.c
│       ├── timing.c
│       ├── segment.h
│       └── unfloader/
│
assets/
├── desktop/
└── n64/
```

The public headers in `src/platform` define the contract used by game code. Platform implementations may include SDL, OpenGL, NuSystem, or N64 SDK headers; game code may not.

## Phase 1: Establish the desktop baseline

Before restructuring, document and verify the current behavior:

- Native CMake build succeeds.
- Title screen, leaderboard, gameplay, pause, game-over, initials entry, and sandbox work.
- Keyboard mappings and exit behavior are recorded.
- Audio samples and channel behavior are recorded.
- High-score loading and saving are checked.
- Windowed and fullscreen behavior are checked.
- Emscripten still builds, if it is part of the required baseline.

Add a lightweight smoke-test checklist and preserve representative screenshots or recordings. This becomes the acceptance baseline for every refactoring phase.

Keep CMake as the build system for the desktop platform only. Establish the current CMake target as the explicit desktop baseline, including its native and Emscripten variants, and name its targets and source groups accordingly. Do not attempt to make CMake configure or build the N64 platform.

```cmake
add_executable(asteroids_desktop ${GAME_SRCS} ${DESKTOP_PLATFORM_SRCS})
```

### N64 Builds

The N64 platform will use a separate custom Makefile derived from `asteroids64`. That Makefile will compile the same shared game sources together with the N64 platform sources, assembly, linker script, SDK libraries, and ROM-packaging steps. Source lists should be kept explicit in both build systems so each platform selects exactly one implementation of every platform contract.

Define `ASTEROIDS_PLATFORM_DESKTOP` for the CMake target and `ASTEROIDS_PLATFORM_N64` for the Makefile target only where compile-time platform identification is unavoidable. Prefer selecting different implementation files over adding platform conditionals to shared game code.

Success criterion: the project still behaves identically, CMake unambiguously builds the desktop platform, and the intended boundary between the desktop CMake build and future N64 Makefile build is documented.

## Phase 2: Create boundaries without changing behavior

Move one subsystem at a time. After each move, rebuild and run the desktop smoke tests.

The initial platform contracts should cover:

- Application lifecycle: startup configuration, shutdown, and platform exit.
- Rendering/canvas: frame begin, continuation, color, shape, text, and frame submission.
- Input: registration, logical mappings, active/triggered state, and analogue stick.
- Timing: produce, consume, reset, and residual simulation time.
- Audio: initialise, load logical samples, play, stop, pitch, and completion state.
- Storage: availability, read, and write.
- Main-loop scheduling: select a game state and schedule frames.
- Logging and assertions.
- Optional feedback such as rumble.

Do not expose native handles or SDK types through these headers.

At the end of this phase, `src/game` should contain no SDL, OpenGL, Emscripten, NuSystem, or N64 SDK includes or symbols.

## Phase 3: Remove platform leakage from shared game code

Several current modules need more than a move.

### Rendering

The current `canvas` interface is the best existing abstraction, but `draw.c` still makes direct OpenGL calls for:

- The player ship
- Ship explosion shards
- Particle explosions
- Transform setup
- Lives display

Extend `canvas.h` with portable line, point, transform, or geometry operations, then express all drawing through it. Alternatively, represent these elements as additional `struct shape` data.

Use the current desktop `canvas.c` as the first implementation and the N64 `canvas.c`/`gfx.c` as the second.

Reconcile the shape index type deliberately. Desktop currently uses `uint16_t` line indices due to an Emscripten legacy-GL issue; N64 uses `uint8_t`. The shared representation should support both, or platform canvas implementations should convert shared geometry during loading.

### Audio

The N64 fork calls `nuAuStlSndPlayer*` directly from `level.c`. Add the required operations to `mixer.h`, especially pitch and reliable stop/status operations. Move all NuSystem audio code into `platform/n64/mixer.c`.

Use logical sample identifiers rather than platform file paths in game code:

```c
enum game_sample {
    GAME_SAMPLE_BEAT_1,
    GAME_SAMPLE_BEAT_2,
    GAME_SAMPLE_EXPLOSION,
    GAME_SAMPLE_PHASER,
    GAME_SAMPLE_THRUSTER
};
```

Desktop maps these to WAV files; N64 maps them to sound-bank indices.

### Rumble

Keep rumble as an optional platform service. The desktop implementation should be a no-op unless controller rumble is later added. This lets shared game code request feedback without `#ifdef N64`.

### Input and UI text

Keep logical actions in game code and physical controls in platform mappings. Avoid conditionals such as “PRESS START” versus “PRESS ENTER” in `leaderboard.c`. Obtain the appropriate label from the input/platform layer or use neutral text such as “PRESS TO CONTINUE.”

It should be possible to query the input layer to find what is available, either statically through `#ifdef`s or at runtime when the cost will be low.

### Timing and loop scheduling

Desktop currently derives elapsed time from `SDL_GetTicks`; N64 adds a fixed 60 Hz interval. Preserve the shared fixed-step consumer, but have the platform supply elapsed milliseconds.

The scheduler also differs fundamentally:

- Desktop owns a blocking loop.
- Emscripten registers a callback.
- N64 registers a NuSystem graphics callback.

Keep scheduling platform-specific while retaining the shared `main_loop_fn_t` game-state callback.

### Standard-library compatibility

Do not use the N64 fork’s broad `platform.h` to inject SDK headers, math aliases, integer definitions, and debug symbols into shared modules. Provide narrow compatibility headers and use `<stdint.h>`, `<stdbool.h>`, and `<math.h>` wherever the N64 toolchain supports them.

Success criterion: platform dependencies are visible from the build file and directory layout, not hidden behind pervasive preprocessor branches.

## Phase 4: reconcile the two gameplay branches

Do not copy all `asteroids64/src` files over the current game. Nearly every common module has diverged, including entities, collision, drawing, menus, level logic, constants, and data.

For each shared module:

1. Start with the current `asteroids` version.
2. Identify N64 changes as either platform accommodation, bug fix, or gameplay change.
3. Move platform accommodations behind an interface.
4. Port genuine bug fixes intentionally.
5. Reject or separately document gameplay changes unless parity is desired.
6. Verify desktop behavior before proceeding.

Pay particular attention to:

- `MAX_BULLETS` changing from 20 to 10.
- Player shape representation changing from an array to separate frames.
- Different position-wrapping and asteroid-update APIs.
- Player state and collision differences.
- High-score filename and storage-size constraints.
- Screen coordinates and text scales.
- Game-over return types and state-transition differences.
- The newer desktop `game.c`, mixer abstraction, and cleanup path, which do not exist in the N64 fork.

Success criterion: there is one copy of each gameplay module, with documented choices for every behavior-changing divergence.

## Phase 5: add the N64 platform

Once desktop is cleanly separated, import only the N64-specific material:

- NuSystem entry point and main-loop registration
- RSP/RDP renderer and canvas implementation
- Controller input
- Controller Pak storage
- NuSystem audio-bank implementation
- Rumble Pak support
- UNFLoader debug support
- ROM header, entry assembly, linker script, and segment definitions
- N64-specific assets and build tooling

Adapt those implementations to the shared contracts rather than changing game code to match the old fork.

The N64 `main.c` should follow the same lifecycle as desktop:

```text
platform init
→ game init
→ load highscores when storage exists
→ choose initial game state
→ run platform loop
```

Sandbox and UNFLoader commands should call shared game-state transition functions rather than duplicating startup logic.

## Phase 6: build-system integration

Keep CMake as the desktop build initially. Add explicit source lists:

- `asteroids_game`: shared game library
- `asteroids_desktop`: desktop entry point and platform implementations

Avoid globbing platform sources because it makes accidental cross-platform compilation easy.

Import the N64 Makefile and Docker setup at the repository root or under `platform/n64`, but have it compile:

- The same shared game source list
- Only `platform/n64` implementations
- N64 assembly and assets

Useful commands should be unambiguous:

```sh
./scripts/build-desktop.sh
./scripts/build-web.sh
./scripts/build-n64.sh
```

Both builds should define a single platform macro only for truly unavoidable compile-time properties. Prefer separate implementation files over `#ifdef` blocks.
