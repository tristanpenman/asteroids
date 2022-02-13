.include "macros.inc"

.section .data

glabel _audiopbankSegmentRomStart
.incbin "assets/audio.ptr"
.balign 16
glabel _audiopbankSegmentRomEnd

glabel _audiowbankSegmentRomStart
.incbin "assets/audio.wbk"
.balign 16
glabel _audiowbankSegmentRomEnd

glabel _audiosfxSegmentRomStart
.incbin "assets/audio.bfx"
.balign 16
glabel _audiosfxSegmentRomEnd
