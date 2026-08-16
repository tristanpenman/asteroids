# Mixer

This note explores an N64 mixer implementation strategy, requiring a minimal number of changes to get sound working across all platforms.

The aim is to wrap NuSystem's sound-effect player behind the existing mixer API, using the precompiled `asteroids64` sound banks. These sound banks were initially built using Windows-based sound tools for N64.

Once the mixer is in place, sound banks can be re-constructed using command line tools.

## Asset and build pipeline

Copy the entire archived `asteroids64/assets` directory to `n64/assets`. This includes the source samples (`*.aif`), pointer bank (`audio.ptr`), wave bank (`audio.wbk`), sound-effect bank (`audio.bfx`), supporting bank files, and `rawdata.s`.

Only `audio.ptr`, `audio.wbk`, and `audio.bfx` are needed at runtime. `rawdata.s` embeds them into the ROM.

The N64 build then needs:

- `n64/assets` added as an assembly source directory.
- A raw-data segment added to `n64/asteroids.ld`.
- `libmus`, `libnaudio`, `libnualstl_n`, and the N64 audio microcode added to `n64/Makefile`.
- `libmus` and `libnaudio` installed by `Dockerfile.n64`.
- A small segment header exposing the pointer-bank, wave-bank, and SFX-bank linker symbols.

## Mixer implementation

Implement `src/platform/n64/mixer.c` using:

```c
nuAuStlInit();
nuAuStlPtrBankInit(pointer_bank_size);
nuAuStlPtrBankSet(pointer_bank, pointer_bank_size, wave_bank);
nuAuStlSndPlayerDataSet(sfx_bank, sfx_bank_size);
```

The compiled sound bank uses the same ordering as the desktop manifest:

| Mixer sample | Bank ID |
| ------------ | ------- |
| `beat1`.     | 0       |
| `beat2`      | 1       |
| `explosion`  | 2       |
| `phaser`     | 3       |
| `thruster`   | 4       |

On N64, `mixer_load_sample_from_file()` should treat its path as a logical identifier rather than opening a file. It can extract the basename, ignore `.wav` versus `.aif`, and return the corresponding bank ID.  This lets `src/game/game.c` retain the existing platform-neutral asset list.

Playback maps as follows:

- `mixer_play_sample(sample)` calls `nuAuStlSndPlayerPlay(sample)`.
- The returned NuSystem `musHandle` becomes the mixer channel ID.
- `mixer_stop_playing_on_channel(channel)` calls `nuAuStlSndPlayerSndStop(channel, 0)`.
- NuSystem handle `0` maps to `MIXER_INVALID_CHANNEL`.

## Completion callbacks

NuSystem does not provide the SDL-style completion callback used by the game. Add a platform-neutral `mixer_update()` operation:

- The desktop implementation is a no-op because SDL invokes callbacks.
- The N64 implementation tracks active `musHandle` values and polls them with `nuAuStlSndPlayerGetSndState()`.
- When a handle stops, it removes the handle and invokes the registered completion handler from the game thread.

This is required for looping behaviour such as the thruster. Without completion notification, `thruster_channel` remains occupied after the sample ends and the sound will not restart.

## Startup integration

The current N64 logo starts the game using `game_init(true)`, explicitly disabling sample loading. `src/platform/n64/main.c` should eventually:

1. Call `mixer_init(MIXER_DEFAULT)` during startup.
2. Enter the game with `game_init(false)`.
3. Call `mixer_update()` once per main-loop iteration.

## Implementation sequence

1. Embed the banks and make the ROM link.
2. Implement initialization, name-to-bank lookup, playback, and stopping.
3. Add handle polling and completion callbacks, then test all five effects on emulator and hardware.
