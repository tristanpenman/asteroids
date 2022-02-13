#include <nusys.h>

beginseg
  name    "code"
  flags   BOOT OBJECT
  entry   nuBoot
  address NU_SPEC_BOOT_ADDR
  stack   NU_SPEC_BOOT_STACK
  include "codesegment.o"
  include "$(ROOT)/usr/lib/PR/rspboot.o"

  include "$(ROOT)/usr/lib/PR/gspF3DEX2.fifo.o"
  include "$(ROOT)/usr/lib/PR/gspL3DEX2.fifo.o"
  include "$(ROOT)/usr/lib/PR/gspF3DEX2.Rej.fifo.o"
  include "$(ROOT)/usr/lib/PR/gspF3DEX2.NoN.fifo.o"
  include "$(ROOT)/usr/lib/PR/gspF3DLX2.Rej.fifo.o"
  include "$(ROOT)/usr/lib/PR/gspS2DEX2.fifo.o"

  include "$(ROOT)/usr/lib/PR/n_aspMain.o"
endseg

beginseg
  name "audiopbank"
  flags RAW
  include "assets/audio.ptr"
endseg

beginseg
  name "audiowbank"
  flags RAW
  include "assets/audio.wbk"
endseg

beginseg
  name "audiosfx"
  flags RAW
  include "assets/audio.bfx"
endseg

beginwave
  name "asteroids"
  include "code"
  include "audiopbank"
  include "audiowbank"
  include "audiosfx"
endwave
