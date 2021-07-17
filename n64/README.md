# Asteroids 64

This is the code for a work-in-progress N64 port of my Asteroids clone.

The general approach that I've taken is to gradually refactor the code so that platform-specific code is well encapsulated, and back-porting those changes to the original codebase.

My hope is that this will eventually make it possible to merge the two codebases, and build both versions from the same set of source and data files.

## Compiling the ROM

This code has been written to compile using the original Nintendo 64 SDK on Windows (or Wine), and also using CrashOverride95's [Modern SDK](https://crashoveride95.github.io/n64hbrew/modernsdk/index.html) which is Linux port of the SDK.

To compile on Windows or Wine, you can run `compile.bat`. A script for running via wine has also been included in `build.sh`.

To compile on Linux using the Modern SDK, run:

    make -f Makefile.modern

## Emulator Support

The code is frequently tested using [Sixtyforce](https://sixtyforce.com/) on macOS.

## Real Hardware

The ROM can simply be copied to a flash cart such as the [EverDrive-64](https://krikzz.com/store/home/55-everdrive-64-x7.html). I've tested this using an X7 cart, however it should work on any flash cart.

### Loader64

The ROM can also be loaded via USB. Before I discovered UNFLoader, I used a small C program called [loader64](https://github.com/tristanpenman/loader64). This was based on code written by [James Friend](https://github.com/jsdf), who has made a number of great contributions to the N64 Homebrew community.

### UNFLoader

It is also possible to load the ROM using [UNFLoader](https://github.com/buu342/N64-UNFLoader/), e.g:

    UNFLoader -d -r asteroids.n64

The `-d` flag enables debug mode, and will show the output of `debug_printf` calls from the ROM in your terminal.
