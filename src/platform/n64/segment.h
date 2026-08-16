#ifndef ASTEROIDS_PLATFORM_N64_SEGMENT_H
#define ASTEROIDS_PLATFORM_N64_SEGMENT_H

extern u8 _audiopbankSegmentRomStart[];
extern u8 _audiopbankSegmentRomEnd[];
extern u8 _audiowbankSegmentRomStart[];
extern u8 _audiosfxSegmentRomStart[];
extern u8 _audiosfxSegmentRomEnd[];

#define AUDIO_POINTER_BANK_START _audiopbankSegmentRomStart
#define AUDIO_POINTER_BANK_SIZE \
    ((u32)(_audiopbankSegmentRomEnd - _audiopbankSegmentRomStart))
#define AUDIO_WAVE_BANK_START _audiowbankSegmentRomStart
#define AUDIO_EFFECTS_BANK_START _audiosfxSegmentRomStart
#define AUDIO_EFFECTS_BANK_SIZE \
    ((u32)(_audiosfxSegmentRomEnd - _audiosfxSegmentRomStart))

#endif
