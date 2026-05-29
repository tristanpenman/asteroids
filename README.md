# Asteroids

Be thrilled by this high fidelity reproduction of Atari's 1979 classic, Asteroids!

This is a tidied up version of an Asteroids clone that I wrote over the course of several weekends, as a way to procrastinate while studying for exams. The aim was to get the game running quickly, taking a 'less-is-more' approach. Graphics were implemented using legacy OpenGL, while window management, audio and input were all handled by SDL.

![Asteroids Title Screen](./doc/titlescreen.png)

## Ports

This project has also been ported to the web using Emscripten, which I wrote about in [this blog post](https://tristanpenman.com/blog/posts/2018/01/08/porting-an-asteroids-clone-to-javascript/), and later to the venerable Nintendo 64. The code for the N64 port can be found [here](https://github.com/tristanpenman/asteroids64).

## Demo

A playable demo can be found [here](https://tristanpenman.com/demos/asteroids).

If you want to try the N64 version (using either an emulator or a flash-cast such as the Everdrive 64) you can download the ROM [here](https://tristanpenman.com/demos/asteroids/asteroids.n64).

## Dependencies

* SDL2
* SDL_Mixer2
* CMake
* Emscripten (for web builds)

## Build

### macOS and Linux

The project currently depends on SDL2 and SDL2_mixer, and builds are handled by CMake. Once those dependencies are installed (e.g. using Homebrew or apt), native macOS and Linux builds are relatively simple:

```bash
mkdir build
cd build
cmake ..
make
```

### Windows

The project's CMake configuration can also be used to generate Visual Studio project files. However, you will need to extract the archives in [thirdparty](./thirdparty) before you can do this.

Once those archives have been extracted, CMake GUI should be able to generate a VS solution using the default configuration.

### Emscripten

The project can also be compiled to Javascript using Emscripten.

```bash
mkdir embuild
cd embuild
emcmake cmake ..
emmake make
emrun index.html
```

Emscripten builds are only supported on Linux and macOS systems.

### Emscripten (via Docker)

If you'd rather not install the Emscripten SDK on your host machine, you can build and play the web version using Docker. This uses the official `emscripten/emsdk` toolchain image, so the only dependency is Docker itself.

Using Docker Compose:

```bash
docker compose up --build
```

Then open [http://localhost:6931](http://localhost:6931) in your browser to play.

Press `Ctrl-C` to stop the server, and run `docker compose down` to remove the container.

Alternatively, using plain Docker:

```bash
docker build -f Dockerfile.emsdk -t asteroids-emscripten .
docker run --rm -p 6931:6931 asteroids-emscripten
```

The build artifacts (`index.html`, `asteroids.js`, `asteroids.wasm` and `asteroids.data`) are produced inside the container under `/src/embuild`. If you want to extract them onto your host, you can copy them out of a running container:

```bash
docker create --name asteroids-build asteroids-emscripten
docker cp asteroids-build:/src/embuild ./embuild
docker rm asteroids-build
```

> **Note:** the `emscripten/emsdk` image is only published for `linux/amd64`, so on Apple Silicon (arm64) it runs under emulation. This is fine for building and serving the game, though the build will be a little slower.

## License

This code has been released under the MIT License. See the [LICENSE](LICENSE) file for more information.

### Assets

Game assets are under copyright by Atari.

The graphics and audio that have been reproduced here are all used in good faith. The clone is intentionally incomplete, so as to not detract from the value of any Atari releases of the game.
