#ifndef fonts_h
#define fonts_h

#if !defined(__AVR__) && ! defined(PROGMEM)
#define PROGMEM
#endif

// All font data from Benedikt K.
// http://www.mikrocontroller.net/topic/54860

// Fonts all have <blanks> at bottom line (except for descenders (like in p and q) and padded from leftmost column
//Font selection (select only one font)
//#define FONT_4X6
//#define FONT_5X8
//#define FONT_5X12
#define FONT_6X8
//#define FONT_6X10
//#define FONT_7X12
//#define FONT_8X8
//#define FONT_8X12
//#define FONT_8X14
//#define FONT_10X16
//#define FONT_12X16
//#define FONT_12X20
//#define FONT_16X26

//if defined char range 0x20-0x7F otherwise 0x20-0xFF
#define FONT_END7F

#define FONT_START (0x20) //first character

#if defined(FONT_4X6)
# define FONT_WIDTH   (4)
# define FONT_HEIGHT  (6)
#elif defined(FONT_5X8)
# define FONT_WIDTH   (5)
# define FONT_HEIGHT  (8)
#elif defined(FONT_5X12)
# define FONT_WIDTH   (5)
# define FONT_HEIGHT (12)
#elif defined(FONT_6X8)
# define FONT_WIDTH   (6)
# define FONT_HEIGHT  (8)
#elif defined(FONT_6X10)
# define FONT_WIDTH   (6)
# define FONT_HEIGHT (10)
#elif defined(FONT_7X12)
# define FONT_WIDTH   (7)
# define FONT_HEIGHT (12)
#elif defined(FONT_8X8)
# define FONT_WIDTH   (8)
# define FONT_HEIGHT  (8)
#elif defined(FONT_8X12)
# define FONT_WIDTH   (8)
# define FONT_HEIGHT (12)
#elif defined(FONT_8X14)
# define FONT_WIDTH   (8)
# define FONT_HEIGHT (14)
#elif defined(FONT_10X16)
# define FONT_WIDTH  (10)
# define FONT_HEIGHT (16)
#elif defined(FONT_12X16)
# define FONT_WIDTH  (12)
# define FONT_HEIGHT (16)
#elif defined(FONT_12X20)
# define FONT_WIDTH  (12)
# define FONT_HEIGHT (20)
#elif defined(FONT_16X26)
# define FONT_WIDTH  (16)
# define FONT_HEIGHT (26)
#else
# warning No font defined
#endif

extern const uint8_t font_PGM[];

#endif //fonts_h

#pragma once

