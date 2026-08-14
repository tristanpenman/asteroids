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
        BUILD_DIR/src/platform/n64/main.o(.text);
        */libultra_rom.a:*.o(.text);
        */libnustd.a:*.o(.text);
        */libnusys.a:*.o(.text);
        */libgcc.a:*.o(.text);

        /usr/lib/n64/nusys/nusys_rom.o(.data*);
        BUILD_DIR/src/platform/n64/main.o(.data*);
        */libultra_rom.a:*.o(.data*);
        */libnustd.a:*.o(.data*);
        */libnusys.a:*.o(.data*);

        /usr/lib/n64/nusys/nusys_rom.o(.*rodata*);
        BUILD_DIR/src/platform/n64/main.o(.*rodata*);
        */libultra_rom.a:*.o(.*rodata*);
        */libnustd.a:*.o(.*rodata*);
        */libnusys.a:*.o(.*rodata*);
        */libgcc.a:*.o(.*rodata*);
    }
    END_SEG(code)

    BEGIN_NOLOAD(code)
    {
        /usr/lib/n64/nusys/nusys_rom.o(COMMON);
        /usr/lib/n64/nusys/nusys_rom.o(.scommon);
        /usr/lib/n64/nusys/nusys_rom.o(.*bss*);
        BUILD_DIR/src/platform/n64/main.o(COMMON);
        BUILD_DIR/src/platform/n64/main.o(.scommon);
        BUILD_DIR/src/platform/n64/main.o(.*bss*);
        */libultra_rom.a:*.o(COMMON);
        */libultra_rom.a:*.o(.scommon);
        */libultra_rom.a:*.o(.*bss*);
        */libnustd.a:*.o(COMMON);
        */libnustd.a:*.o(.scommon);
        */libnustd.a:*.o(.*bss*);
        */libnusys.a:*.o(COMMON);
        */libnusys.a:*.o(.scommon);
        */libnusys.a:*.o(.*bss*);
    }
    END_NOLOAD(code)

    /DISCARD/ :
    {
        *(*);
    }
}
