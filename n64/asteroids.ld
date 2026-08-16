OUTPUT_ARCH (mips)

#include <nusys.h>

#define BEGIN_SEG(name, addr) \
    _##name##SegmentStart = ADDR(.name); \
    _##name##SegmentRomStart = __romPos; \
    .name addr : AT(__romPos)

#define END_SEG(name) \
    _##name##SegmentEnd = ADDR(.name) + SIZEOF(.name); \
    _##name##SegmentRomEnd = __romPos + SIZEOF(.name); \
    __romPos += SIZEOF(.name);

#define BEGIN_NOLOAD(name) \
    _##name##SegmentBssStart = ADDR(.name.noload); \
    .name.noload (NOLOAD) :

#define END_NOLOAD(name) \
    _##name##SegmentBssEnd = ADDR(.name.noload) + SIZEOF(.name.noload); \
    _##name##SegmentBssSize = SIZEOF(.name.noload);

SECTIONS
{
    __romPos = 0;

    BEGIN_SEG(boot, 0x04000000)
    {
        BUILD_DIR/n64/asm/rom_header.o(.text);
        BUILD_DIR/boot.6102.o(.data);
    }
    END_SEG(boot)

    BEGIN_SEG(code, NU_SPEC_BOOT_ADDR) SUBALIGN(16)
    {
        BUILD_DIR/n64/asm/entry.o(.start);
        /usr/lib/n64/nusys/nusys_rom.o(.text);
        BUILD_DIR/src/platform/n64*.o(.text);
        BUILD_DIR/src/game*.o(.text);
        */libultra_rom.a:*.o(.text);
        */libnustd.a:*.o(.text);
        */libnusys.a:*.o(.text);
        */libnualstl_n.a:*.o(.text);
        */libn_mus.a:*.o(.text);
        */libn_audio_sc.a:*.o(.text);
        */libgcc.a:*.o(.text);
        /usr/lib/n64/PR/rspboot.o(.text);
        /usr/lib/n64/PR/n_aspMain.o(.text);
        /usr/lib/n64/PR/gspF3DEX2.fifo.o(.text);
        /usr/lib/n64/PR/gspL3DEX2.fifo.o(.text);
        /usr/lib/n64/PR/gspF3DEX2.Rej.fifo.o(.text);
        /usr/lib/n64/PR/gspF3DEX2.NoN.fifo.o(.text);
        /usr/lib/n64/PR/gspF3DLX2.Rej.fifo.o(.text);
        /usr/lib/n64/PR/gspS2DEX2.fifo.o(.text);

        /usr/lib/n64/nusys/nusys_rom.o(.data*);
        BUILD_DIR/src/platform/n64*.o(.data*);
        BUILD_DIR/src/game*.o(.data*);
        */libultra_rom.a:*.o(.data*);
        */libnustd.a:*.o(.data*);
        */libnusys.a:*.o(.data*);
        */libnualstl_n.a:*.o(.data*);
        */libn_mus.a:*.o(.data*);
        */libn_audio_sc.a:*.o(.data*);
        /usr/lib/n64/PR/rspboot.o(.data*);
        /usr/lib/n64/PR/n_aspMain.o(.data*);
        /usr/lib/n64/PR/gspF3DEX2.fifo.o(.data*);
        /usr/lib/n64/PR/gspL3DEX2.fifo.o(.data*);
        /usr/lib/n64/PR/gspF3DEX2.Rej.fifo.o(.data*);
        /usr/lib/n64/PR/gspF3DEX2.NoN.fifo.o(.data*);
        /usr/lib/n64/PR/gspF3DLX2.Rej.fifo.o(.data*);
        /usr/lib/n64/PR/gspS2DEX2.fifo.o(.data*);

        /usr/lib/n64/nusys/nusys_rom.o(.*rodata*);
        BUILD_DIR/src/platform/n64*.o(.*rodata*);
        BUILD_DIR/src/game*.o(.*rodata*);
        */libultra_rom.a:*.o(.*rodata*);
        */libnustd.a:*.o(.*rodata*);
        */libnusys.a:*.o(.*rodata*);
        */libnualstl_n.a:*.o(.*rodata*);
        */libn_mus.a:*.o(.*rodata*);
        */libn_audio_sc.a:*.o(.*rodata*);
        */libgcc.a:*.o(.*rodata*);
    }
    END_SEG(code)

    BEGIN_NOLOAD(code)
    {
        /usr/lib/n64/nusys/nusys_rom.o(COMMON);
        /usr/lib/n64/nusys/nusys_rom.o(.scommon);
        /usr/lib/n64/nusys/nusys_rom.o(.*bss*);
        BUILD_DIR/src/platform/n64*.o(COMMON);
        BUILD_DIR/src/platform/n64*.o(.scommon);
        BUILD_DIR/src/platform/n64*.o(.*bss*);
        BUILD_DIR/src/game*.o(COMMON);
        BUILD_DIR/src/game*.o(.scommon);
        BUILD_DIR/src/game*.o(.*bss*);
        */libultra_rom.a:*.o(COMMON);
        */libultra_rom.a:*.o(.scommon);
        */libultra_rom.a:*.o(.*bss*);
        */libnustd.a:*.o(COMMON);
        */libnustd.a:*.o(.scommon);
        */libnustd.a:*.o(.*bss*);
        */libnusys.a:*.o(COMMON);
        */libnusys.a:*.o(.scommon);
        */libnusys.a:*.o(.*bss*);
        */libnualstl_n.a:*.o(COMMON);
        */libnualstl_n.a:*.o(.scommon);
        */libnualstl_n.a:*.o(.*bss*);
        */libn_mus.a:*.o(COMMON);
        */libn_mus.a:*.o(.scommon);
        */libn_mus.a:*.o(.*bss*);
        */libn_audio_sc.a:*.o(COMMON);
        */libn_audio_sc.a:*.o(.scommon);
        */libn_audio_sc.a:*.o(.*bss*);
    }
    END_NOLOAD(code)

    BEGIN_SEG(rawdata, __romPos)
    {
        BUILD_DIR/n64/assets/rawdata.o(.data*);
    }
    END_SEG(rawdata)

    /DISCARD/ :
    {
        *(*);
    }
}
