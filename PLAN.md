# Plan

- Improve original code
  + ~~Factor out basic canvas and shape API~~
  + ~~Backport logging API from N64 version~~
  + Implement simplistic debug console, based on UNFLoader command interface
    + Make sandbox runtime accessible
    + Reset level state
  + Support for USB controllers
  + Full-screen mode
- ~~N64 port~~
  + ~~Move to separate repo~~
- Dreamcast port
  + Basic app structure
  + Port current N64 functionality
  + Allow highscores to be saved to VMU
- General improvements
  + Alien ship
  + Attract mode
  + Improve emscripten wrapper
  + Framerate counter
  + Draw-call counter and other metrics
  + Automated testing
