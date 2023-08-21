#include <inttypes.h>

#if defined(FONT_8X8)
#  if defined(__AVR__)
#include <avr/pgmspace.h>
const uint8_t font_PGM[] PROGMEM =
#  else
const uint8_t font[] =
#  endif
{
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // 0x20
  0x30,0x78,0x78,0x30,0x30,0x00,0x30,0x00, // 0x21
  0x6C,0x6C,0x6C,0x00,0x00,0x00,0x00,0x00, // 0x22
  0x6C,0x6C,0xFE,0x6C,0xFE,0x6C,0x6C,0x00, // 0x23
  0x30,0x7C,0xC0,0x78,0x0C,0xF8,0x30,0x00, // 0x24
  0x00,0xC6,0xCC,0x18,0x30,0x66,0xC6,0x00, // 0x25
  0x38,0x6C,0x38,0x76,0xDC,0xCC,0x76,0x00, // 0x26
  0x60,0x60,0xC0,0x00,0x00,0x00,0x00,0x00, // 0x27
  0x18,0x30,0x60,0x60,0x60,0x30,0x18,0x00, // 0x28
  0x60,0x30,0x18,0x18,0x18,0x30,0x60,0x00, // 0x29
  0x00,0x66,0x3C,0xFF,0x3C,0x66,0x00,0x00, // 0x2A
  0x00,0x30,0x30,0xFC,0x30,0x30,0x00,0x00, // 0x2B
  0x00,0x00,0x00,0x00,0x00,0x70,0x30,0x60, // 0x2C
  0x00,0x00,0x00,0xFC,0x00,0x00,0x00,0x00, // 0x2D
  0x00,0x00,0x00,0x00,0x00,0x30,0x30,0x00, // 0x2E
  0x06,0x0C,0x18,0x30,0x60,0xC0,0x80,0x00, // 0x2F
  0x78,0xCC,0xDC,0xFC,0xEC,0xCC,0x78,0x00, // 0x30
  0x30,0xF0,0x30,0x30,0x30,0x30,0xFC,0x00, // 0x31
  0x78,0xCC,0x0C,0x38,0x60,0xCC,0xFC,0x00, // 0x32
  0x78,0xCC,0x0C,0x38,0x0C,0xCC,0x78,0x00, // 0x33
  0x1C,0x3C,0x6C,0xCC,0xFE,0x0C,0x0C,0x00, // 0x34
  0xFC,0xC0,0xF8,0x0C,0x0C,0xCC,0x78,0x00, // 0x35
  0x38,0x60,0xC0,0xF8,0xCC,0xCC,0x78,0x00, // 0x36
  0xFC,0xCC,0x0C,0x18,0x30,0x60,0x60,0x00, // 0x37
  0x78,0xCC,0xCC,0x78,0xCC,0xCC,0x78,0x00, // 0x38
  0x78,0xCC,0xCC,0x7C,0x0C,0x18,0x70,0x00, // 0x39
  0x00,0x00,0x30,0x30,0x00,0x30,0x30,0x00, // 0x3A
  0x00,0x00,0x30,0x30,0x00,0x70,0x30,0x60, // 0x3B
  0x18,0x30,0x60,0xC0,0x60,0x30,0x18,0x00, // 0x3C
  0x00,0x00,0xFC,0x00,0xFC,0x00,0x00,0x00, // 0x3D
  0x60,0x30,0x18,0x0C,0x18,0x30,0x60,0x00, // 0x3E
  0x78,0xCC,0x0C,0x18,0x30,0x00,0x30,0x00, // 0x3F
  0x7C,0xC6,0xDE,0xDE,0xDE,0xC0,0x78,0x00, // 0x40
  0x30,0x78,0xCC,0xCC,0xFC,0xCC,0xCC,0x00, // 0x41
  0xFC,0x66,0x66,0x7C,0x66,0x66,0xFC,0x00, // 0x42
  0x3C,0x66,0xC0,0xC0,0xC0,0x66,0x3C,0x00, // 0x43
  0xFC,0x6C,0x66,0x66,0x66,0x6C,0xFC,0x00, // 0x44
  0xFE,0x62,0x68,0x78,0x68,0x62,0xFE,0x00, // 0x45
  0xFE,0x62,0x68,0x78,0x68,0x60,0xF0,0x00, // 0x46
  0x3C,0x66,0xC0,0xC0,0xCE,0x66,0x3E,0x00, // 0x47
  0xCC,0xCC,0xCC,0xFC,0xCC,0xCC,0xCC,0x00, // 0x48
  0x78,0x30,0x30,0x30,0x30,0x30,0x78,0x00, // 0x49
  0x1E,0x0C,0x0C,0x0C,0xCC,0xCC,0x78,0x00, // 0x4A
  0xE6,0x66,0x6C,0x78,0x6C,0x66,0xE6,0x00, // 0x4B
  0xF0,0x60,0x60,0x60,0x62,0x66,0xFE,0x00, // 0x4C
  0xC6,0xEE,0xFE,0xD6,0xC6,0xC6,0xC6,0x00, // 0x4D
  0xC6,0xE6,0xF6,0xDE,0xCE,0xC6,0xC6,0x00, // 0x4E
  0x38,0x6C,0xC6,0xC6,0xC6,0x6C,0x38,0x00, // 0x4F
  0xFC,0x66,0x66,0x7C,0x60,0x60,0xF0,0x00, // 0x50
  0x78,0xCC,0xCC,0xCC,0xDC,0x78,0x1C,0x00, // 0x51
  0xFC,0x66,0x66,0x7C,0x78,0x6C,0xE6,0x00, // 0x52
  0x78,0xCC,0xE0,0x38,0x1C,0xCC,0x78,0x00, // 0x53
  0xFC,0xB4,0x30,0x30,0x30,0x30,0x78,0x00, // 0x54
  0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xFC,0x00, // 0x55
  0xCC,0xCC,0xCC,0xCC,0xCC,0x78,0x30,0x00, // 0x56
  0xC6,0xC6,0xC6,0xD6,0xFE,0xEE,0xC6,0x00, // 0x57
  0xC6,0xC6,0x6C,0x38,0x6C,0xC6,0xC6,0x00, // 0x58
  0xCC,0xCC,0xCC,0x78,0x30,0x30,0x78,0x00, // 0x59
  0xFE,0xCC,0x98,0x30,0x62,0xC6,0xFE,0x00, // 0x5A
  0x78,0x60,0x60,0x60,0x60,0x60,0x78,0x00, // 0x5B
  0xC0,0x60,0x30,0x18,0x0C,0x06,0x02,0x00, // 0x5C
  0x78,0x18,0x18,0x18,0x18,0x18,0x78,0x00, // 0x5D
  0x10,0x38,0x6C,0xC6,0x00,0x00,0x00,0x00, // 0x5E
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF, // 0x5F
  0x30,0x30,0x18,0x00,0x00,0x00,0x00,0x00, // 0x60
  0x00,0x00,0x78,0x0C,0x7C,0xCC,0x76,0x00, // 0x61
  0xE0,0x60,0x7C,0x66,0x66,0x66,0xBC,0x00, // 0x62
  0x00,0x00,0x78,0xCC,0xC0,0xCC,0x78,0x00, // 0x63
  0x1C,0x0C,0x0C,0x7C,0xCC,0xCC,0x76,0x00, // 0x64
  0x00,0x00,0x78,0xCC,0xFC,0xC0,0x78,0x00, // 0x65
  0x38,0x6C,0x60,0xF0,0x60,0x60,0xF0,0x00, // 0x66
  0x00,0x00,0x76,0xCC,0xCC,0x7C,0x0C,0xF8, // 0x67
  0xE0,0x60,0x6C,0x76,0x66,0x66,0xE6,0x00, // 0x68
  0x30,0x00,0x70,0x30,0x30,0x30,0x78,0x00, // 0x69
  0x18,0x00,0x78,0x18,0x18,0x18,0xD8,0x70, // 0x6A
  0xE0,0x60,0x66,0x6C,0x78,0x6C,0xE6,0x00, // 0x6B
  0x70,0x30,0x30,0x30,0x30,0x30,0x78,0x00, // 0x6C
  0x00,0x00,0xEC,0xFE,0xD6,0xC6,0xC6,0x00, // 0x6D
  0x00,0x00,0xF8,0xCC,0xCC,0xCC,0xCC,0x00, // 0x6E
  0x00,0x00,0x78,0xCC,0xCC,0xCC,0x78,0x00, // 0x6F
  0x00,0x00,0xDC,0x66,0x66,0x7C,0x60,0xF0, // 0x70
  0x00,0x00,0x76,0xCC,0xCC,0x7C,0x0C,0x1E, // 0x71
  0x00,0x00,0xD8,0x6C,0x6C,0x60,0xF0,0x00, // 0x72
  0x00,0x00,0x7C,0xC0,0x78,0x0C,0xF8,0x00, // 0x73
  0x10,0x30,0x7C,0x30,0x30,0x34,0x18,0x00, // 0x74
  0x00,0x00,0xCC,0xCC,0xCC,0xCC,0x76,0x00, // 0x75
  0x00,0x00,0xCC,0xCC,0xCC,0x78,0x30,0x00, // 0x76
  0x00,0x00,0xC6,0xC6,0xD6,0xFE,0x6C,0x00, // 0x77
  0x00,0x00,0xC6,0x6C,0x38,0x6C,0xC6,0x00, // 0x78
  0x00,0x00,0xCC,0xCC,0xCC,0x7C,0x0C,0xF8, // 0x79
  0x00,0x00,0xFC,0x98,0x30,0x64,0xFC,0x00, // 0x7A
  0x1C,0x30,0x30,0xE0,0x30,0x30,0x1C,0x00, // 0x7B
  0x18,0x18,0x18,0x00,0x18,0x18,0x18,0x00, // 0x7C
  0xE0,0x30,0x30,0x1C,0x30,0x30,0xE0,0x00, // 0x7D
  0x76,0xDC,0x00,0x00,0x00,0x00,0x00,0x00, // 0x7E
  0x10,0x38,0x6C,0xC6,0xC6,0xC6,0xFE,0x00, // 0x7F
#if !defined(FONT_END7F)
  0x78,0xCC,0xC0,0xC0,0xCC,0x78,0x30,0x60, // 0x80
  0x00,0xCC,0x00,0xCC,0xCC,0xCC,0x7E,0x00, // 0x81
  0x18,0x30,0x78,0xCC,0xFC,0xC0,0x78,0x00, // 0x82
  0x7E,0xC3,0x3C,0x06,0x3E,0x66,0x3F,0x00, // 0x83
  0xCC,0x00,0x78,0x0C,0x7C,0xCC,0x7E,0x00, // 0x84
  0x60,0x30,0x78,0x0C,0x7C,0xCC,0x7E,0x00, // 0x85
  0x3C,0x66,0x3C,0x06,0x3E,0x66,0x3F,0x00, // 0x86
  0x00,0x78,0xCC,0xC0,0xCC,0x78,0x30,0x60, // 0x87
  0x7E,0xC3,0x3C,0x66,0x7E,0x60,0x3C,0x00, // 0x88
  0xCC,0x00,0x78,0xCC,0xFC,0xC0,0x78,0x00, // 0x89
  0x60,0x30,0x78,0xCC,0xFC,0xC0,0x78,0x00, // 0x8A
  0xCC,0x00,0x70,0x30,0x30,0x30,0x78,0x00, // 0x8B
  0x7C,0xC6,0x38,0x18,0x18,0x18,0x3C,0x00, // 0x8C
  0x60,0x30,0x70,0x30,0x30,0x30,0x78,0x00, // 0x8D
  0xCC,0x30,0x78,0xCC,0xCC,0xFC,0xCC,0x00, // 0x8E
  0x30,0x48,0x30,0x78,0xCC,0xFC,0xCC,0x00, // 0x8F
  0x18,0x30,0xFC,0x60,0x78,0x60,0xFC,0x00, // 0x90
  0x00,0x00,0x7F,0x0C,0x7F,0xCC,0x7F,0x00, // 0x91
  0x3E,0x6C,0xCC,0xFE,0xCC,0xCC,0xCE,0x00, // 0x92
  0x78,0xCC,0x00,0x78,0xCC,0xCC,0x78,0x00, // 0x93
  0x00,0xCC,0x00,0x78,0xCC,0xCC,0x78,0x00, // 0x94
  0x60,0x30,0x00,0x78,0xCC,0xCC,0x78,0x00, // 0x95
  0x78,0xCC,0x00,0xCC,0xCC,0xCC,0x7E,0x00, // 0x96
  0x60,0x30,0x00,0xCC,0xCC,0xCC,0x7E,0x00, // 0x97
  0x00,0xCC,0x00,0xCC,0xCC,0xFC,0x0C,0xF8, // 0x98
  0xC6,0x00,0x7C,0xC6,0xC6,0xC6,0x7C,0x00, // 0x99
  0xCC,0x00,0xCC,0xCC,0xCC,0xCC,0x78,0x00, // 0x9A
  0x00,0x00,0x7C,0xCE,0xD6,0xE6,0x7C,0x00, // 0x9B
  0x38,0x6C,0x64,0xF0,0x60,0xE6,0xFC,0x00, // 0x9C
  0x3A,0x6C,0xCE,0xD6,0xE6,0x6C,0xB8,0x00, // 0x9D
  0x00,0x00,0xCC,0x78,0x30,0x78,0xCC,0x00, // 0x9E
  0x0E,0x1B,0x18,0x7E,0x18,0x18,0xD8,0x70, // 0x9F
  0x18,0x30,0x78,0x0C,0x7C,0xCC,0x7E,0x00, // 0xA0
  0x18,0x30,0x70,0x30,0x30,0x30,0x78,0x00, // 0xA1
  0x0C,0x18,0x00,0x78,0xCC,0xCC,0x78,0x00, // 0xA2
  0x0C,0x18,0x00,0xCC,0xCC,0xCC,0x7E,0x00, // 0xA3
  0x76,0xDC,0x00,0xF8,0xCC,0xCC,0xCC,0x00, // 0xA4
  0x76,0xDC,0x00,0xEC,0xFC,0xDC,0xCC,0x00, // 0xA5
  0x3C,0x6C,0x6C,0x3E,0x00,0x7E,0x00,0x00, // 0xA6
  0x3C,0x66,0x66,0x3C,0x00,0x7E,0x00,0x00, // 0xA7
  0x30,0x00,0x30,0x60,0xC0,0xCC,0x78,0x00, // 0xA8
  0x3C,0x5A,0xA5,0xB9,0xA9,0x66,0x3C,0x00, // 0xA9
  0x00,0x00,0x00,0xFC,0x0C,0x0C,0x00,0x00, // 0xAA
  0xE6,0x6C,0x78,0x7E,0x63,0xCE,0x98,0x1F, // 0xAB
  0xE6,0x6C,0x78,0x73,0x67,0xCD,0x9F,0x03, // 0xAC
  0x00,0x18,0x00,0x18,0x18,0x3C,0x3C,0x18, // 0xAD
  0x00,0x33,0x66,0xCC,0x66,0x33,0x00,0x00, // 0xAE
  0x00,0xCC,0x66,0x33,0x66,0xCC,0x00,0x00, // 0xAF
  0x22,0x88,0x22,0x88,0x22,0x88,0x22,0x88, // 0xB0
  0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA, // 0xB1
  0xDD,0x77,0xDD,0x77,0xDD,0x77,0xDD,0x77, // 0xB2
  0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18, // 0xB3
  0x18,0x18,0x18,0x18,0xF8,0x18,0x18,0x18, // 0xB4
  0x0C,0x18,0x30,0x78,0xCC,0xFC,0xCC,0x00, // 0xB5
  0x78,0x84,0x30,0x78,0xCC,0xFC,0xCC,0x00, // 0xB6
  0xC0,0x60,0x30,0x78,0xCC,0xFC,0xCC,0x00, // 0xB7
  0x3C,0x42,0xB9,0xA1,0xB9,0x42,0x3C,0x00, // 0xB8
  0x36,0x36,0xF6,0x06,0xF6,0x36,0x36,0x36, // 0xB9
  0x36,0x36,0x36,0x36,0x36,0x36,0x36,0x36, // 0xBA
  0x00,0x00,0xFE,0x06,0xF6,0x36,0x36,0x36, // 0xBB
  0x36,0x36,0xF6,0x06,0xFE,0x00,0x00,0x00, // 0xBC
  0x18,0x18,0x7E,0xC0,0xC0,0x7E,0x18,0x18, // 0xBD
  0xCC,0xCC,0x78,0xFC,0x30,0xFC,0x30,0x30, // 0xBE
  0x00,0x00,0x00,0x00,0xF8,0x18,0x18,0x18, // 0xBF
  0x18,0x18,0x18,0x18,0x1F,0x00,0x00,0x00, // 0xC0
  0x18,0x18,0x18,0x18,0xFF,0x00,0x00,0x00, // 0xC1
  0x00,0x00,0x00,0x00,0xFF,0x18,0x18,0x18, // 0xC2
  0x18,0x18,0x18,0x18,0x1F,0x18,0x18,0x18, // 0xC3
  0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00, // 0xC4
  0x18,0x18,0x18,0x18,0xFF,0x18,0x18,0x18, // 0xC5
  0x76,0xDC,0x78,0x0C,0x7C,0xCC,0x7E,0x00, // 0xC6
  0x76,0xDC,0x30,0x78,0xCC,0xFC,0xCC,0x00, // 0xC7
  0x36,0x36,0x37,0x30,0x3F,0x00,0x00,0x00, // 0xC8
  0x00,0x00,0x3F,0x30,0x37,0x36,0x36,0x36, // 0xC9
  0x36,0x36,0xF7,0x00,0xFF,0x00,0x00,0x00, // 0xCA
  0x00,0x00,0xFF,0x00,0xF7,0x36,0x36,0x36, // 0xCB
  0x36,0x36,0x37,0x30,0x37,0x36,0x36,0x36, // 0xCC
  0x00,0x00,0xFF,0x00,0xFF,0x00,0x00,0x00, // 0xCD
  0x36,0x36,0xF7,0x00,0xF7,0x36,0x36,0x36, // 0xCE
  0x00,0x82,0xFE,0x6C,0x6C,0xFE,0x82,0x00, // 0xCF
  0xD8,0x70,0xD8,0x0C,0x3C,0x6C,0x38,0x00, // 0xD0
  0xFC,0x6C,0x66,0xF6,0x66,0x6C,0xFC,0x00, // 0xD1
  0x78,0x84,0xFC,0x60,0x78,0x60,0xFC,0x00, // 0xD2
  0xCC,0x00,0xFC,0x60,0x78,0x60,0xFC,0x00, // 0xD3
  0x60,0x30,0xFC,0x60,0x78,0x60,0xFC,0x00, // 0xD4
  0x00,0xC0,0x40,0xE0,0x00,0x00,0x00,0x00, // 0xD5
  0x18,0x30,0x78,0x30,0x30,0x30,0x78,0x00, // 0xD6
  0x78,0x84,0x78,0x30,0x30,0x30,0x78,0x00, // 0xD7
  0xCC,0x00,0x78,0x30,0x30,0x30,0x78,0x00, // 0xD8
  0x18,0x18,0x18,0x18,0xF8,0x00,0x00,0x00, // 0xD9
  0x00,0x00,0x00,0x00,0x1F,0x18,0x18,0x18, // 0xDA
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, // 0xDB
  0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF, // 0xDC
  0x18,0x18,0x18,0x00,0x18,0x18,0x18,0x00, // 0xDD
  0x60,0x30,0x78,0x30,0x30,0x30,0x78,0x00, // 0xDE
  0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00, // 0xDF
  0x18,0x30,0x7C,0xC6,0xC6,0xC6,0x7C,0x00, // 0xE0
  0x00,0x78,0xCC,0xF8,0xCC,0xF8,0xC0,0xC0, // 0xE1
  0x7C,0x82,0x7C,0xC6,0xC6,0xC6,0x7C,0x00, // 0xE2
  0x30,0x18,0x7C,0xC6,0xC6,0xC6,0x7C,0x00, // 0xE3
  0x76,0xDC,0x00,0x78,0xCC,0xCC,0x78,0x00, // 0xE4
  0x76,0xDC,0x7C,0xC6,0xC6,0xC6,0x7C,0x00, // 0xE5
  0x00,0x66,0x66,0x66,0x66,0x7C,0x60,0xC0, // 0xE6
  0x00,0xE0,0x78,0x6C,0x78,0x60,0xF0,0x00, // 0xE7
  0xF0,0x60,0x7C,0x66,0x7C,0x60,0xF0,0x00, // 0xE8
  0x18,0x30,0xCC,0xCC,0xCC,0xCC,0x78,0x00, // 0xE9
  0x78,0x84,0x00,0xCC,0xCC,0xCC,0x78,0x00, // 0xEA
  0x60,0x30,0xCC,0xCC,0xCC,0xCC,0x78,0x00, // 0xEB
  0x18,0x30,0x00,0xCC,0xCC,0xFC,0x0C,0xF8, // 0xEC
  0x18,0x30,0xCC,0xCC,0x78,0x30,0x78,0x00, // 0xED
  0x00,0xFC,0x00,0x00,0x00,0x00,0x00,0x00, // 0xEE
  0x18,0x30,0x00,0x00,0x00,0x00,0x00,0x00, // 0xEF
  0x00,0x00,0x00,0x00,0xFC,0x00,0x00,0x00, // 0xF0
  0x30,0x30,0xFC,0x30,0x30,0x00,0xFC,0x00, // 0xF1
  0x00,0x00,0x00,0xFC,0x00,0xFC,0x00,0x00, // 0xF2
  0xE6,0x2C,0x78,0x33,0xE7,0xCD,0x9F,0x03, // 0xF3
  0x7F,0xDB,0xDB,0x7B,0x1B,0x1B,0x1B,0x00, // 0xF4
  0x7E,0xC3,0x78,0xCC,0xCC,0x78,0x8C,0xF8, // 0xF5
  0x30,0x30,0x00,0xFC,0x00,0x30,0x30,0x00, // 0xF6
  0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x60, // 0xF7
  0x38,0x6C,0x6C,0x38,0x00,0x00,0x00,0x00, // 0xF8
  0x00,0xCC,0x00,0x00,0x00,0x00,0x00,0x00, // 0xF9
  0x00,0x00,0x00,0x00,0x18,0x00,0x00,0x00, // 0xFA
  0x38,0x78,0x18,0x18,0x7E,0x00,0x00,0x00, // 0xFB
  0x00,0x30,0x38,0x3C,0x3C,0x38,0x30,0x00, // 0xFC
  0x00,0x66,0x66,0x66,0x66,0x66,0x66,0x00, // 0xFD
  0x00,0x7E,0x7E,0x7E,0x7E,0x7E,0x7E,0x00, // 0xFE
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00  // 0xFF
#endif
};
#endif
