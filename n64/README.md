# Asteroids 64

This is the code for a work-in-progress N64 port of my Asteroids clone.

The general approach that I've taken is to gradually refactor the code so that platform-specific code is well encapsulated, before back-porting those changes to the original codebase. My plan is to eventually merge the two codebases, and build both versions from the same set of source and data files.

You can get the latest build [here](misc/asteroids.z64).

## Emulator Support

The code is frequently tested using [cen64](https://github.com/n64dev/cen64). It has also been tested using [Sixtyforce](https://sixtyforce.com/) on macOS and [Project64](https://www.pj64-emu.com/) on Windows.

Here's the game running in cen64:

![Screenshot in cen64](misc/screenshot.png)

## Real Hardware

The ROM can simply be copied to a flash cart such as the [EverDrive-64](https://krikzz.com/store/home/55-everdrive-64-x7.html). I've tested this using an X7 cart, however it should work on any flash cart.

## Rumble Pak

The game is compatible with Rumble Paks and Controller Paks.

When a Rumble Pak is inserted, it will be activated by ship explosions.

## Controller Pak

When an Controller Pak is inserted, high scores will be saved into a file on the Controller Pak.

## Compiling the ROM

This code has been written to compile using the original Nintendo 64 SDK on Windows (or Wine), and also using CrashOverride95's [Modern SDK](https://crashoveride95.github.io/n64hbrew/modernsdk/index.html) which is Linux port of the SDK.

To compile on Windows or Wine, you can run `compile.bat`. A script for running via wine has also been included in `build.sh`.

To compile on Linux using the Modern SDK, simply run `make`.

### UNFLoader

The ROM can also be loaded via USB.

This repo contains a heavily modified version of the USB and Debug Library from [UNFLoader](https://github.com/buu342/N64-UNFLoader/) (see [usb.c](usb.c) and [debug.c](./debug.c)). I have removed code that was not relevant to this project, and modified some function signatures to better fit with the cross-platform needs of this project.

Even with the changes, you can still load the ROM using the original `UNFLoader` executable:

    UNFLoader -d -r asteroids.n64

The `-d` flag enables debug mode, and will show the output of `debug_printf` calls from the ROM in your terminal.

### Loader64

Before I discovered UNFLoader, I used a small C program called [loader64](https://github.com/tristanpenman/loader64). This was based on code written by [James Friend](https://github.com/jsdf), who has made a number of great contributions to the N64 Homebrew community.
