# Asteroids

Be thrilled by this high fidelity reproduction of Atari's 1979 classic, Asteroids!

## Background

This is a tidied up version of an Asteroids clone that I wrote over the course of several weekends, as a way to procrastinate while studying for exams. The aim was to get the game running quickly, taking a 'less-is-more' approach. Graphics were implemented using legacy OpenGL, while window management, audio and input were all handled by SDL.

I later decided to port the game to the web using Emscripten.

## Demo

A playable demo can be found [here](https://tristanpenman.com/demos/asteroids).

## Dependencies

* SDL2
* SDL_Mixer2
* CMake
* Emscripten (for web builds)

## Build

### macOS and Linux

The project currently depends on SDL2 and SDL2_mixer, and builds are handled by CMake. Once those dependencies are installed, native macOS and Linux builds are relatively simple:

    mkdir build
    cd build
    cmake ..
    make

### Emscripten

The project can also be compiled to Javascript using Emscripten.

    mkdir embuild
    cd embuild
    emcmake cmake ..
    emmake make
    emrun asteroids.html

## License

This code has been released under the MIT License. See the [LICENSE](LICENSE) file for more information.

### Assets

Game assets are under copyright by Atari.

The graphics and audio that have been reproduced here are all used in good faith. The clone is intentionally incomplete, so as to not detract from the value of any Atari releases of the game.
