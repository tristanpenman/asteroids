.include "macros.inc"

.section .data

glabel _audiopbankSegmentRomStart
.incbin "n64/assets/audio.ptr"
.balign 16
glabel _audiopbankSegmentRomEnd

glabel _audiowbankSegmentRomStart
.incbin "n64/assets/audio.wbk"
.balign 16
glabel _audiowbankSegmentRomEnd

glabel _audiosfxSegmentRomStart
.incbin "n64/assets/audio.bfx"
.balign 16
glabel _audiosfxSegmentRomEnd
