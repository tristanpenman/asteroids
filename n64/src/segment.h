#ifndef SEGMENT_H
#define SEGMENT_H

extern u8 _codeSegmentStart[];
extern u8 _codeSegmentEnd[];

extern u8 _audiopbankSegmentRomStart[];
extern u8 _audiopbankSegmentRomEnd[];
extern u8 _audiowbankSegmentRomStart[];
extern u8 _audiowbankSegmentRomEnd[];
extern u8 _audiosfxSegmentRomStart[];
extern u8 _audiosfxSegmentRomEnd[];

#define WBANK_START _audiowbankSegmentRomStart
#define PBANK_START _audiopbankSegmentRomStart
#define PBANK_SIZE  _audiopbankSegmentRomEnd - _audiopbankSegmentRomStart
#define SFX_START   _audiosfxSegmentRomStart
#define SFX_SIZE    _audiosfxSegmentRomEnd - _audiosfxSegmentRomStart

#endif
