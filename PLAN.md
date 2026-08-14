# Plan

- Improve original code
  + ~~Factor out basic canvas and shape API~~
  + ~~Backport logging API from N64 version~~
  + Implement simplistic debug console, based on UNFLoader command interface
    + Make sandbox runtime accessible
    + Reset level state
  + Support for USB controllers
  + Full-screen mode
- N64 port (**in progress**)
  + ~~Develop N64 version in separate repo~~
  + Refactor game logic around platform abstraction
  + Re-import N64 version
- Dreamcast port
  * Start Dreamcast port using KallistiOS
  + Allow highscores to be saved to VMU
- General improvements
  + Alien ship
  + Attract mode
  + Improve emscripten wrapper
  + Framerate counter
  + Draw-call counter and other metrics
  + Automated testing
