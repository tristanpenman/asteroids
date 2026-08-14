#include <nusys.h>

.set noat
.set noreorder
.set gp=64

.include "macros.inc"

.section .start, "ax"

glabel __start
    la $t0, _codeSegmentBssStart
    la $t1, _codeSegmentBssSize
bss_clear:
    addi $t1, $t1, -8
    sw $zero, ($t0)
    sw $zero, 4($t0)
    bnez $t1, bss_clear
    addi $t0, $t0, 8
    la $t2, nuBoot
    la $sp, NU_SPEC_BOOT_STACK
    jr $t2
    nop
