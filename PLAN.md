# Plan

Desktop is the gameplay baseline. Bring the console ports to parity through the shared game code and platform interfaces.

## Completed recently

- Portable math/vector code and N64 Makefile dependency tracking.
- Web build serving and SDL caching, with updated Web and Docker instructions.
- Desktop build-directory recovery, safer mixer handling, and normal cleanup on shutdown.
- Dreamcast KallistiOS Docker build and ELF/BIN/CDI packaging scaffold.

## Desktop

- Finish the debug console and sandbox controls.
- Add USB-controller support and verify fullscreen behavior.

## N64

- Map logical sound samples to the sound bank; verify playback and cleanup.
- Harden Controller Pak save/load and test card errors and high-score persistence.
- Complete controller mappings and verify canvas/text coverage and frame limits.
- Replace assumed-60-Hz timing with measured elapsed time; test frame pacing.
- Reconcile remaining gameplay differences and validate full playthrough, audio, saves, and exit behavior.

## Dreamcast

- Implement Maple controller input, PVR canvas/text rendering, AICA audio, and VMU-backed storage.
- Complete backend initialization/cleanup and validate timing and video modes.
- Test all game states, persistence, and shutdown against the desktop baseline.
- Verify clean Docker builds and test ELF, dcload, and CDI workflows on emulator or hardware.

## General

- Maintain a concise cross-platform smoke-test checklist and automated tests.
- Continue gameplay improvements such as the alien ship and attract mode.
