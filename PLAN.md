# Plan

This project has two main goals at the moment. First is to implement a very light-weight set of interfaces for writing simple games in C, that can also be built and deployed using Emscripten. Second is to show that this can be applied to a more esoteric platform, such as the Nintendo 64.

Some interesting platform-related issues include supporting both fixed and variable timesteps, as well as radically different graphics and audio architectures. Handling different kinds of input is also a challenge.

Much of the inspiration for this project comes from the (massively) cross-platform Retro City Rampage.

## TODO

- Improve original code
  + ~~Factor out basic canvas and shape API~~
  + ~~Backport logging API from N64 version~~
  + Implement simplistic debug console, based on UNFLoader command interface
    + Make sandbox runtime accessible
    + Reset level state
  + Support for USB controllers
  + Full-screen mode
- N64 version
  + ~~Legacy and modern SDK makefiles~~
  + ~~Engine boilerplate~~
  + ~~Basic graphics and audio~~
  + ~~Integrate UNFLoader debug interface~~
  + Port physics sandbox (in progress)
  + Implement UNFLoader command interface (in progress)
  + Packer template for dev environment (in progress)
  + Basic menu system
  + Scripts for pre-processing audio files
  + Figure out how to get UNFLoader fault handler working
    + GDB stub (?)
  + Asteroids font for main screen
  + Mixer interface
  + High score screens
    + Store high scores on controller pak
- General improvements
  + Alien ship
  + Improve emscripten wrapper
  + Framerate counter
  + Draw-call counter and other metrics
