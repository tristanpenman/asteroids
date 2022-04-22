# Plan

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
  + ~~Port physics sandbox~~
  + ~~Figure out how to get UNFLoader fault handler working~~
  + UNFLoader command interface (in progress)
  + Basic menu system (in progress)
  + Scripts for pre-processing audio files
  + ~~Asteroids font for main screen~~
  + Mixer interface
  + High score screens
    + Store high scores on controller pak
- Dreamcast version
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
