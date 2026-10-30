/*
 *  LCDBigNumbers.hpp
 *
 *  Arduino library to write big numbers on a 1602 or 2004 LCD.
 *
 *  A big digit is NumberWidth columns wide and NumberHeight rows high.
 *
 *  Accepted input values:
 *  - Byte 0x00 to 0x09 or ASCII '0' to '9': rendered as a big digit.
 *  - ' ' (space): rendered as a blank with the width of a digit.
 *  - '-', '.', ':': rendered as special characters, which are always one column wide.
 *  - '|' and '_' (ONE_COLUMN_SPACE_CHARACTER and ONE_COLUMN_HYPHEN_CHARACTER): rendered as a one column blank and a one column hyphen.
 *  All other values are skipped.
 *
 *  Gap:
 *  For some fonts, a one column gap is inserted after each digit, space and '-'.
 *  '.', ':', '|' and '_' are never followed by a gap.
 *  The gap is cleared (written as blank), unless the character ends at the last column of the display.
 *  In this case, the gap would be outside of the display and is not written.
 *
 *  Return value of writeBigNumber():
 *  The size_t value is NOT the value of the characters consumed, but the value of the columns consumed / required for the printed string.
 *  All non printable / skipped characters add 10 to the return value to make the error of using them more prominent.
 *  Keep in mind, that the AVR and other print() implementations will return the numbers of the characters consumed.
 *
 *
 * Copyright (C) 2022-2026 Armin Joachimsmeyer
 *
 * This file is part of LCDBigNumbers https://github.com/ArminJo/LCDBigNumbers.
 *
 * LCDBigNumbers is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/gpl-3.0.html>.
 */

#ifndef _LCD_BIG_NUMBERS_HPP
#define _LCD_BIG_NUMBERS_HPP

#include <Arduino.h>

/*
 * Two special character mappings.
 */
#define ONE_COLUMN_SPACE_CHARACTER      '|' // This input character is printed as a one column space. A normal space is printed as a space but with the width of a number.
#define ONE_COLUMN_SPACE_STRING         "|"

#define ONE_COLUMN_HYPHEN_CHARACTER      '_' // This input character is printed as a one column hyphen. A normal hyphen / minus character is printed as a hyphen and can have a gap behind.
#define ONE_COLUMN_HYPHEN_STRING         "_"

#define VERSION_LCD_BIG_NUMBERS "1.3.0"
#define VERSION_LCD_BIG_NUMBERS_MAJOR 1
#define VERSION_LCD_BIG_NUMBERS_MINOR 3
#define VERSION_LCD_BIG_NUMBERS_PATCH 0
// The change log is at the README.md

/*
 * Macro to convert 3 version parts into an integer
 * To be used in preprocessor comparisons, such as #if VERSION_LCD_BIG_NUMBERS_HEX >= VERSION_HEX_VALUE(3, 0, 0)
 */
#if !defined(VERSION_HEX_VALUE)
#define VERSION_HEX_VALUE(major, minor, patch) (((major) << 16) | ((minor) << 8) | (patch))
#endif
#define VERSION_LCD_BIG_NUMBERS_HEX  VERSION_HEX_VALUE(VERSION_LCD_BIG_NUMBERS_MAJOR, VERSION_LCD_BIG_NUMBERS_MINOR, VERSION_LCD_BIG_NUMBERS_PATCH)

// START - This is a copy from https://github.com/ArminJo/Arduino-Utils/blob/master/src/LCDPrintUtils.hpp
//#define USE_PARALLEL_2004_LCD // Is default
//#define USE_PARALLEL_1602_LCD
//#define USE_SERIAL_2004_LCD
//#define USE_SERIAL_1602_LCD

#if !defined(USE_PARALLEL_2004_LCD) && !defined(USE_PARALLEL_1602_LCD) && !defined(USE_SERIAL_2004_LCD) && !defined(USE_SERIAL_1602_LCD)
#warning "No LCD type like USE_SERIAL_2004_LCD specified, therefore using USE_PARALLEL_2004_LCD as default"
#define USE_PARALLEL_2004_LCD    // Use parallel 2004 LCD as default
#endif

#if defined(USE_PARALLEL_2004_LCD) || defined(USE_PARALLEL_1602_LCD)
#define USE_PARALLEL_LCD
#include "LiquidCrystal.h"
#else
#define USE_SERIAL_LCD
#include "LiquidCrystal_I2C.hpp" // Here we use an enhanced version, which supports SoftI2CMaster
#endif

#if defined(USE_PARALLEL_1602_LCD) || defined(USE_SERIAL_1602_LCD)
#define LCD_COLUMNS     16
#define LCD_ROWS        2
#else
#define LCD_COLUMNS     20
#define LCD_ROWS        4
#endif
// END Copy from https://github.com/ArminJo/Arduino-Utils/blob/master/src/LCDPrintUtils.hpp

#define DEFAULT_TEST_DELAY  3000
#define NUMBER_OF_SPECIAL_CHARACTERS_IN_FONT_ARRAY  3

#define COLUMN_MASK     0x0C // Number of columns = shifted masked value + 1
#define ROW_MASK        0x03 // Number of rows = masked value + 1
#define VARIANT_MASK    0x30
// Numbers are created by using the above masks
#define BIG_NUMBERS_FONT_1_COLUMN_2_ROWS            0x01 // No variants available ;-)
#define BIG_NUMBERS_FONT_2_COLUMN_2_ROWS            0x05
#define BIG_NUMBERS_FONT_3_COLUMN_2_ROWS_VARIANT_1  0x09
#define BIG_NUMBERS_FONT_3_COLUMN_2_ROWS_VARIANT_2  0x19
#define BIG_NUMBERS_FONT_3_COLUMN_2_ROWS_VARIANT_3  0x29
#define BIG_NUMBERS_FONT_2_COLUMN_3_ROWS_VARIANT_1  0x06
#define BIG_NUMBERS_FONT_2_COLUMN_3_ROWS_VARIANT_2  0x16
#define BIG_NUMBERS_FONT_3_COLUMN_3_ROWS            0x0A
#define BIG_NUMBERS_FONT_3_COLUMN_4_ROWS_VARIANT_1  0x0B
#define BIG_NUMBERS_FONT_3_COLUMN_4_ROWS_VARIANT_2  0x1B

#define BIG_NUMBERS_FONT_1_COLUMN_2_ROWS_VARIANT_1  BIG_NUMBERS_FONT_1_COLUMN_2_ROWS // deprecated
#define BIG_NUMBERS_FONT_2_COLUMN_2_ROWS_VARIANT_1  BIG_NUMBERS_FONT_2_COLUMN_2_ROWS // deprecated
#define BIG_NUMBERS_FONT_3_COLUMN_3_ROWS_VARIANT_1  BIG_NUMBERS_FONT_3_COLUMN_3_ROWS // deprecated

#if defined(DEBUG)
#define LOCAL_DEBUG
#else
//#define LOCAL_DEBUG // To debug/understand the writeBigNumber() function - only for development
#endif

// !!! Must be without trailing comment and closed by @formatter:on
// @formatter:off

/*
 * 1 x 2
 * Broken link: http://www.picbasic.co.uk/forum/showthread.php?t=13376
 */
// 8 custom characters for 1 column font
const uint8_t bigNumbers1x2CustomPatterns_1[][8] PROGMEM = {
    {   0b11110, 0b10010, 0b10010, 0b10010, 0b10010, 0b10010, 0b10010, 0b11110}, // 0 Closed rectangle - used for: 8, 9
    {   0b11110, 0b10010, 0b10010, 0b10010, 0b10010, 0b10010, 0b10010, 0b10010}, // 1 Rectangle - open at bottom - 0
    {   0b10010, 0b10010, 0b10010, 0b10010, 0b10010, 0b10010, 0b10010, 0b11110}, // 2 Rectangle - open at top - 0, 4, 6, 8
    {   0b11110, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b11110}, // 3 Rectangle - open at left - 3
    {   0b11110, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11110}, // 4 Rectangle - open at right - 2, 5, 6
    {   0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010}, // 5 Right bar - 1, 4, 7
    {   0b11110, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010}, // 6 Top right - 2, 7
    {   0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b00010, 0b11110}  // 7 Right bottom - 3,5,9
};
const uint8_t bigNumbers1x2_1[2][13] PROGMEM = {                   // 2-line numbers
//    "-"   "."   ":"    0     1     2     3     4     5     6     7     8     9
    { 0x5F, 0xFE, 0xA5, 0x01, 0x05, 0x06, 0x03, 0x02, 0x04, 0x04, 0x06, 0x00, 0x00 },
    { 0xFE, 0x2E, 0xA5, 0x02, 0x05, 0x04, 0x07, 0x05, 0x07, 0x02, 0x05, 0x02, 0x07 }
};

/*
 * 2 x 2
 * https://www.alpenglowindustries.com/pages/the-big-numbers-go-marching-2x2-tutorial
 * https://github.com/AlpenglowIndustries/Alpenglow_BigNums2x2
 * Trek font, Requires 1 0xFF block for the special "0"
 */
const uint8_t bigNumbers2x2CustomPatterns_1[][8] PROGMEM = {
 { 0b11111, 0b11111, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000 }, // 0
 { 0b11000, 0b11000, 0b11000, 0b11000, 0b11000, 0b11000, 0b11000, 0b11000 }, // 1
 { 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111 }, // 2
 { 0b11111, 0b11111, 0b00011, 0b00011, 0b00011, 0b00011, 0b11111, 0b11111 }, // 3
 { 0b11111, 0b11111, 0b11000, 0b11000, 0b11000, 0b11000, 0b11111, 0b11111 }, // 4
 { 0b11111, 0b11111, 0b11000, 0b11000, 0b11000, 0b11000, 0b11000, 0b11000 }, // 5
 { 0b00011, 0b00011, 0b00011, 0b00011, 0b00011, 0b00011, 0b11111, 0b11111 }, // 6
 { 0b11000, 0b11000, 0b11000, 0b11000, 0b11000, 0b11000, 0b11111, 0b11111 } // 7
};
const uint8_t bigNumbers2x2_1[2][23] PROGMEM = {                   // 2-line numbers
//    "-"   "."   ":"       0          1          2          3          4          5          6          7          8          9
    { 0xFE, 0xFE, 0xA5, 0x05,0xFF, 0x00,0x01, 0x00,0x03, 0x00,0x03, 0x01,0x01, 0x04,0x00, 0x05,0x00, 0x00,0x03, 0x04,0x03, 0x04,0x03},
    { 0x00, 0x2E, 0xA5, 0x07,0x06, 0x02,0x07, 0x04,0x02, 0x02,0x03, 0x00,0x05, 0x02,0x03, 0x04,0x03, 0xFE,0x01, 0x04,0x03, 0x02,0x06}
};

/*
 * 3 x 2 - Variant 1
 * https://liudr.wordpress.com/2011/03/21/big-font/
 * Requires 0xFF blocks, but character 6 could be used for it
 */
const uint8_t bigNumbers3x2CustomPatterns_1[6][8] PROGMEM = {
  { 0b11111, 0b11111 ,0b00000 ,0b00000 ,0b00000 ,0b00000 ,0b00000 ,0b00000 }, // 0 Upper bar
  { 0b00000, 0b00000 ,0b00000 ,0b00000 ,0b00000 ,0b00000 ,0b11111 ,0b11111 }, // 1 Lower bar
  { 0b11111, 0b11111 ,0b00000 ,0b00000 ,0b00000 ,0b00000 ,0b11111 ,0b11111 }, // 2 Upper and lower bar
  { 0b00000 ,0b00000 ,0b00000 ,0b11111 ,0b11111 ,0b00000 ,0b00000 ,0b00000 }, // 3 Minus sign
  { 0b00000 ,0b00000 ,0b00000 ,0b00000 ,0b00000 ,0b01110 ,0b01110 ,0b01110 }, // 4 Decimal point
  { 0b00000 ,0b00000 ,0b01110 ,0b01110 ,0b01110 ,0b00000 ,0b00000 ,0b00000 }  // 5 Colon
};
const uint8_t bigNumbers3x2_1[2][33] PROGMEM = {               // 2-line numbers
//    "-"   "."   ":"         0               1               2               3               4               5               6               7               8               9
    { 0x01, 0xFE, 0x05, 0xFF,0x00,0xFF, 0x00,0xFF,0xFE, 0x02,0x02,0xFF, 0x00,0x02,0xFF, 0xFF,0x01,0xFF, 0xFF,0x02,0x02, 0xFF,0x02,0x02, 0x00,0x00,0xFF, 0xFF,0x02,0xFF, 0xFF,0x02,0xFF},
    { 0xFE, 0x04, 0x05, 0xFF,0x01,0xFF, 0x01,0xFF,0x01, 0xFF,0x01,0x01, 0x01,0x01,0xFF, 0xFE,0xFE,0xFF, 0x01,0x01,0xFF, 0xFF,0x01,0xFF, 0xFE,0xFE,0xFF, 0xFF,0x01,0xFF, 0x01,0x01,0xFF}
};


/*
 * 3 x 2 - Variant 2
 * Broken link: https://exploreembedded.com/wiki/Distance_Meter_with_Big_Fonts
 */
const uint8_t bigNumbers3x2CustomPatterns_2[8][8] PROGMEM = {
{ 0b11100, 0b11110, 0b11110, 0b11110, 0b11110, 0b11110, 0b11110, 0b11100}, // 0 left bar
{ 0b00111, 0b01111, 0b01111, 0b01111, 0b01111, 0b01111, 0b01111, 0b00111}, // 1 right bar
{ 0b11111, 0b11111, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111}, // 2 upper and lower bar
{ 0b11110, 0b11100, 0b00000, 0b00000, 0b00000, 0b00000, 0b11000, 0b11100}, // 3 left upper and lower rounded
{ 0b01111, 0b00111, 0b00000, 0b00000, 0b00000, 0b00000, 0b00011, 0b00111}, // 4 right upper and lower rounded
{ 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111}, // 5 right lower
{ 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00111, 0b01111}, // 6 right lower rounded
{ 0b11111, 0b11111, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000}  // 7 upper bar
};
const uint8_t bigNumbers3x2_2[2][33] PROGMEM = {                   // 2-line numbers
//    "-"   "."   ":"         0               1               2               3               4               5               6               7               8               9
    { 0xFE, 0xFE, 0xA5, 0x01,0x07,0x00, 0xFE,0x00,0xFE, 0x04,0x02,0x00, 0x04,0x02,0x00, 0x01,0x05,0x00, 0x01,0x02,0x03, 0x01,0x02,0x03, 0x01,0x07,0x00, 0x01,0x02,0x00, 0x01,0x02,0x00},
    { 0x07, 0x06, 0xA5, 0x01,0x05,0x00, 0xFE,0x00,0xFE, 0x01,0x05,0x05, 0x06,0x05,0x00, 0xFE,0xFE,0x00, 0x06,0x05,0x00, 0x01,0x05,0x00, 0xFE,0xFE,0x00, 0x01,0x05,0x00, 0x06,0x05,0x00}
};

/*
 * 3 x 2 - Variant 3
 * https://forum.arduino.cc/t/display-3-character-wide-big-digits-on-16x2-lcd/905360 bottom of page
 * Requires 0xFF blocks
 */
const uint8_t bigNumbers3x2CustomPatterns_3[8][8] PROGMEM = {
  { 0b11111 ,0b11111 ,0b11111 ,0b00000 ,0b00000 ,0b00000 ,0b00000 ,0b00000 }, // 0 Upper bar
  { 0b00000 ,0b00000 ,0b00000 ,0b00000 ,0b00000 ,0b11111 ,0b11111 ,0b11111 }, // 1 Lower bar
  { 0b11111 ,0b11111 ,0b11111 ,0b00000 ,0b00000 ,0b00000 ,0b11111 ,0b11111 }, // 2 Upper and lower bar for 5,6
  { 0b11100 ,0b11100 ,0b11100 ,0b11100 ,0b11100 ,0b11100 ,0b11100 ,0b11100 }, // 3 Left bar
  { 0b00000 ,0b00000 ,0b00000 ,0b00000 ,0b00000 ,0b11100 ,0b11100 ,0b11100 }, // 4 Left lower bar for 2
  { 0b11100 ,0b11100 ,0b11100 ,0b00000 ,0b00000 ,0b00000 ,0b11100 ,0b11100 }, // 5 Left upper and lower bar for 5,6
  { 0b00000 ,0b00000 ,0b00000 ,0b00000 ,0b00000 ,0b01110 ,0b01110 ,0b01110 }, // 6 Decimal point
  { 0b00000 ,0b00000 ,0b01110 ,0b01110 ,0b01110 ,0b00000 ,0b00000 ,0b00000 }  // 7 Colon
};
const uint8_t bigNumbers3x2_3[2][33] PROGMEM = {               // 2-line numbers
//    "-"   "."   ":"         0               1               2               3               4               5               6               7               8               9
    { 0x01, 0xFE, 0x07, 0xFF,0x00,0x03, 0x00,0x03,0xFE, 0x02,0x02,0x03, 0x02,0x02,0x03, 0xFF,0x01,0x03, 0xFF,0x02,0x05, 0xFF,0x02,0x05, 0x00,0x00,0x03, 0xFF,0x02,0x03, 0xFF,0x02,0x03},
    { 0xFE, 0x06, 0x07, 0xFF,0x01,0x03, 0xFE,0x03,0xFE, 0xFF,0x01,0x04, 0x01,0x01,0x03, 0xFE,0xFE,0x03, 0x01,0x01,0x03, 0xFF,0x01,0x03, 0xFE,0xFE,0x03, 0xFF,0x01,0x03, 0xFE,0xFE,0x03}
};



/*
 * 2 x 3 - Variant 1 with space above
 * http://woodsgood.ca/projects/2015/01/16/large-numbers-on-small-displays/
 */
const uint8_t bigNumbers2x3CustomPatterns_1[8][8] PROGMEM = { { 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x07, 0x07 }, // char 0: bottom right
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x1C, 0x1C },     // char 1: bottom left
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F },     // char 2: bottom block
        { 0x07, 0x07, 0x07, 0x07, 0x07, 0x1F, 0x1F, 0x1F },     // char 3: right bottom block
        { 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1F, 0x1F, 0x1F },     // char 4: left bottom block
        { 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C },     // char 5: left bar
        { 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07 },     // char 6: right bar
        { 0x00, 0x00, 0x0E, 0x0E, 0x0E, 0x00, 0x00, 0x00 }      // char 7: 3x3 block for colon
};
const uint8_t bigNumbers2x3_1[3][23] PROGMEM = {                   // 3-line numbers
//    "-"   "."   ":"       0          1          2          3          4          5          6          7          8          9
    { 0xFE, 0xFE, 0xFE, 0x02,0x02, 0x00,0x01, 0x02,0x02, 0x02,0x02, 0x01,0x00, 0x02,0x02, 0x01,0xFE, 0x02,0x02, 0x02,0x02, 0x02,0x02},
    { 0x02, 0xFE, 0x07, 0x05,0x06, 0xFE,0x05, 0x02,0x03, 0x00,0x03, 0x04,0x03, 0x04,0x02, 0x04,0x02, 0xFE,0x06, 0x04,0x03, 0x04,0x03},
    { 0xFE, 0x01, 0x07, 0x04,0x03, 0x00,0x04, 0x04,0x02, 0x02,0x03, 0xFE,0x06, 0x02,0x03, 0x04,0x03, 0xFE,0x06, 0x04,0x03, 0xFE,0x06}
};

/*
 * 2 x 3 - Variant 2 with space below
 * http://woodsgood.ca/projects/2015/01/16/large-numbers-on-small-displays/
 */
const uint8_t bigNumbers2x3CustomPatterns_2[][8] PROGMEM = { { 0x07, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00  }, // char 0: top right
        { 0x1C, 0x1C, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00 },     // char 1: top left
        { 0x1F, 0x1F, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00 },     // char 2: top block
        { 0x1F, 0x1F, 0x1F, 0x07, 0x07, 0x07, 0x07, 0x07 },     // char 3: right top block
        { 0x1F, 0x1F, 0x1F, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C },     // char 4: left top block
        { 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C },     // char 5: left bar
        { 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07 },     // char 6: right bar
        { 0x00, 0x00, 0x0E, 0x0E, 0x0E, 0x00, 0x00, 0x00 }      // char 7: 3x3 block for colon
};

const uint8_t bigNumbers2x3_2[][23] PROGMEM = {                   // 3-line numbers
//    "-"   "."   ":"       0          1          2          3          4          5          6          7          8          9
    { 0xFE, 0xFE, 0x07, 0x04,0x03, 0x00,0x05, 0x02,0x03, 0x02,0x03, 0x05,0x06, 0x04,0x02, 0x05,0xFE, 0x02,0x03, 0x04,0x03, 0x04,0x03},
    { 0x02, 0xFE, 0x07, 0x05,0x06, 0xFE,0x05, 0x04,0x02, 0x00,0x03, 0x02,0x03, 0x02,0x03, 0x04,0x03, 0xFE,0x06, 0x04,0x03, 0x02,0x03},
    { 0xFE, 0x01, 0xFE, 0x02,0x02, 0x00,0x02, 0x02,0x02, 0x02,0x02, 0xFE,0x00, 0x02,0x02, 0x02,0x02, 0xFE,0x00, 0x02,0x02, 0xFE,0x00}
};

/*
 * 3 x 3
 * http://woodsgood.ca/projects/2015/03/06/3-4-line-big-font-numerals/
 * Requires 0xFF blocks
 */
const uint8_t bigNumbers3x3And3x4CustomPatterns_1[][8] PROGMEM = { { 0x01, 0x07, 0x0F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F }, // char 0: bottom right triangle
        { 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F },     // char 1: bottom block
        { 0x10, 0x1C, 0x1E, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F },     // char 2: bottom left triangle
        { 0x1F, 0x0F, 0x07, 0x01, 0x00, 0x00, 0x00, 0x00 },     // char 3: top right triangle
        { 0x1F, 0x1E, 0x1C, 0x10, 0x00, 0x00, 0x00, 0x00 },     // char 4: top left triangle
        { 0x1F, 0x1F, 0x1F, 0x1F, 0x00, 0x00, 0x00, 0x00 },     // char 5: upper block
        { 0x10, 0x1C, 0x1E, 0x1F, 0x00, 0x00, 0x00, 0x00 },     // char 6: full top left triangle Used only once in 7
//        { 0x1F, 0x1F, 0x1E, 0x1C, 0x18, 0x10, 0x00, 0x00 },     // char 6: variant of full top left triangle
        { 0x01, 0x07, 0x0F, 0x1F, 0x00, 0x00, 0x00, 0x00 }      // char 7: top right triangle
};

const uint8_t bigNumbers3x3_1[3][33] PROGMEM = {   // 3-line numbers
//    "-"   "."   ":"         0               1               2               3               4              5               6                7               8               9
    { 0xFE, 0xFE, 0x01, 0x00,0x05,0x02, 0x07,0xFF,0xFE, 0x07,0x05,0x02, 0x07,0x05,0x02, 0xFF,0xFE,0xFF, 0xFF,0x05,0x05, 0x00,0x05,0x06, 0x05,0x05,0xFF, 0x00,0x05,0x02, 0x00,0x05,0x02},
    { 0x05, 0xFE, 0x01, 0xFF,0xFE,0xFF, 0xFE,0xFF,0xFE, 0x00,0x05,0x05, 0xFE,0x05,0xFF, 0x05,0x05,0xFF, 0x05,0x05,0xFF, 0xFF,0x05,0x02, 0xFE,0x00,0x04, 0xFF,0x05,0xFF, 0x03,0x05,0xFF},
    { 0xFE, 0x05, 0xFE, 0x03,0x05,0x04, 0xFE,0x05,0xFE, 0x05,0x05,0x05, 0x03,0x05,0x04, 0xFE,0xFE,0x05, 0x03,0x05,0x04, 0x03,0x05,0x04, 0xFE,0x05,0xFE, 0x03,0x05,0x04, 0x03,0x05,0x04}
};

/*
 * 3 x 4 - Variant 1
 * http://woodsgood.ca/projects/2015/03/06/3-4-line-big-font-numerals/
 */
const uint8_t bigNumbers3x4_1[4][33] PROGMEM = {                   // 4-line numbers
//    "-"   "."   ":"         0               1               2               3               4              5               6                7               8               9
    { 0xFE, 0xFE, 0xFE, 0x00,0x05,0x02, 0x07,0xFF,0xFE, 0x07,0x05,0x02, 0x07,0x05,0x02, 0xFF,0xFE,0xFF, 0xFF,0x05,0x05, 0x00,0x05,0x02, 0x05,0x05,0xFF, 0x00,0x05,0x02, 0x00,0x05,0x02},
    { 0x01, 0xFE, 0x05, 0xFF,0xFE,0xFF, 0xFE,0xFF,0xFE, 0x01,0x01,0xFF, 0xFE,0x01,0xFF, 0xFF,0x01,0xFF, 0xFF,0x01,0x01, 0xFF,0x01,0x01, 0xFE,0x00,0x04, 0xFF,0x01,0xFF, 0xFF,0x01,0xFF},
    { 0xFE, 0xFE, 0x01, 0xFF,0xFE,0xFF, 0xFE,0xFF,0xFE, 0xFF,0xFE,0xFE, 0xFE,0xFE,0xFF, 0xFE,0xFE,0xFF, 0xFE,0xFE,0xFF, 0xFF,0xFE,0xFF, 0xFE,0xFF,0xFE, 0xFF,0xFE,0xFF, 0xFE,0xFE,0xFF},
    { 0xFE, 0x05, 0xFE, 0x03,0x05,0x04, 0xFE,0x05,0xFE, 0x05,0x05,0x05, 0x03,0x05,0x04, 0xFE,0xFE,0x05, 0x03,0x05,0x04, 0x03,0x05,0x04, 0xFE,0x05,0xFE, 0x03,0x05,0x04, 0x03,0x05,0x04}
};

/*
 * 3 x 4 - Variant 2
 * https://forum.arduino.cc/t/wie-bekommt-man-solch-grosse-zahlen-hin/986148/12
 * Requires 0xFF blocks
 */
const uint8_t bigNumbers3x4CustomPatterns_2[][8] PROGMEM = {
{ 0b00000, 0b00000, 0b00000, 0b00000, 0b00001, 0b00111, 0b01111, 0b11111 }, // char 0: bottom right triangle
{ 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b11111, 0b11111, 0b11111 }, // char 1: bottom block
{ 0b00000, 0b00000, 0b00000, 0b00000, 0b10000, 0b11100, 0b11110, 0b11111 }, // char 2: bottom left triangle
{ 0b11111, 0b01111, 0b00111, 0b00001, 0b00000, 0b00000, 0b00000, 0b00000 }, // char 3: top right triangle
{ 0b11111, 0b11111, 0b11111, 0b11111, 0b00000, 0b00000, 0b00000, 0b00000 }, // char 4: upper block
{ 0b11111, 0b11110, 0b11100, 0b10000, 0b00000, 0b00000, 0b00000, 0b00000 }, // char 5: top left triangle
{ 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b01111, 0b00111, 0b00001 }, // char 6: full top right triangle
{ 0b11111, 0b11111, 0b11111, 0b11111, 0b11111, 0b11110, 0b11100, 0b10000 }  // char 7: full top left triangle
};

const uint8_t bigNumbers3x4_2[4][33] PROGMEM = {                         // 4-line numbers
//    "-"   "."   ":"         0               1               2               3               4              5               6                7               8               9
    { 0xFE, 0xFE, 0xFE, 0x00,0x01,0x02, 0x00,0x01,0xFE, 0x00,0x01,0x02, 0x00,0x01,0x02, 0x00,0xFE,0x01, 0x01,0x01,0x01, 0x00,0x01,0x02, 0x01,0x01,0x01, 0x00,0x01,0x02, 0x00,0x01,0x02},
    { 0x01, 0xFE, 0x04, 0xFF,0x00,0xFF, 0x05,0xFF,0xFE, 0x04,0x00,0x07, 0x04,0x00,0x07, 0xFF,0xFE,0xFF, 0x06,0x01,0x02, 0xFF,0x01,0x02, 0xFE,0x00,0x07, 0x06,0x01,0x07, 0x06,0x01,0xFF},
    { 0xFE, 0xFE, 0x01, 0xFF,0x05,0xFF, 0xFE,0xFF,0xFE, 0xFF,0x05,0xFE, 0x01,0x03,0xFF, 0x04,0x04,0xFF, 0x01,0xFE,0xFF, 0xFF,0xFE,0xFF, 0xFF,0x05,0xFE, 0xFF,0xFE,0xFF, 0xFE,0xFE,0xFF},
    { 0xFE, 0x04, 0xFE, 0x03,0x04,0x05, 0xFE,0x04,0xFE, 0x04,0x04,0x04, 0x03,0x04,0x05, 0xFE,0xFE,0x04, 0x03,0x04,0x05, 0x03,0x04,0x05, 0x04,0xFE,0xFE, 0x03,0x04,0x05, 0x03,0x04,0x05}
};

// 4x4: https://github.com/wa1hco/BigFont - Not implemented
// @formatter:on
class LCDBigNumbers: public Print { // @suppress("Class has a virtual method and non-virtual destructor")

public:
// If activated gives linker error: undefined reference to `operator delete(void*, unsigned int)'
// If disabled I get a Eclipse warning: Class 'LCDBigNumbers' has virtual method 'flush' but non-virtual destructor
//    virtual ~LCDBigNumbers() {}
#if defined(USE_PARALLEL_LCD)
    LiquidCrystal *LCD;
#else
    LiquidCrystal_I2C *LCD;
#endif
    uint8_t NumberWidth; // Width of the rendered number not including the optional blank gap
    uint8_t NumberHeight;
    const uint8_t (*bigNumbersCustomPatternsPGM)[8];
    uint8_t NumberOfCustomPatterns;
    const uint8_t *bigNumbersFont;
    bool forceGapBetweenNumbers;    // The default depends on the font used
    uint8_t upperLeftColumnIndex;   // Start of the next character
    uint8_t upperLeftRowIndex;      // Start of the next character

    /*
     *
     */
    void setBigNumberCursor(uint8_t aUpperLeftColumnIndex, uint8_t aUpperLeftRowIndex = 0) {
        upperLeftColumnIndex = aUpperLeftColumnIndex;
        upperLeftRowIndex = aUpperLeftRowIndex;
    }

    using Print::write;   // make write(const char*), write(buffer, size) visible again
    // We only overload write(uint8_t) here.
    size_t write(uint8_t aBigNumberValue) {
        return writeBigNumber(aBigNumberValue);
    }

    /*
     * Creates maximum 8 custom character used for generating big numbers
     * This also sets cursor to 0.0 by call to _createChar()
     */
    void createCustomChars() {
        for (uint_fast8_t i = 0; i < NumberOfCustomPatterns; i++) {
            _createChar(i, bigNumbersCustomPatternsPGM[i]);
        }
    }
    /*
     * Call it in setup AFTER LCD is initialized
     */
    void begin() {
        createCustomChars();
    }

    void enableGapBetweenNumbers() {
        forceGapBetweenNumbers = true;
    }
    void disableGapBetweenNumbers() {
        forceGapBetweenNumbers = false;
    }
    void setGapBetweenNumbers(bool aForceGapBetweenNumbers) {
        forceGapBetweenNumbers = aForceGapBetweenNumbers;
    }

    /*
     * Reset cursor, set new font values, but do not create custom chars
     * Therefore it can be used by constructor (before LCD is initialized).
     */
    void _setFont(const uint8_t aBigNumberFontIdentifier) {
        setBigNumberCursor(0); // Reset cursor
        NumberWidth = ((aBigNumberFontIdentifier & COLUMN_MASK) >> 2) + 1;
        NumberHeight = (aBigNumberFontIdentifier & ROW_MASK) + 1;
//        uint8_t FontVariant = ((aBigNumberFontIdentifier & VARIANT_MASK) >> 4) + 1; // not really used
        NumberOfCustomPatterns = 8;
        forceGapBetweenNumbers = true;
        switch (aBigNumberFontIdentifier) {
        case BIG_NUMBERS_FONT_1_COLUMN_2_ROWS:
            bigNumbersCustomPatternsPGM = bigNumbers1x2CustomPatterns_1;
            bigNumbersFont = (const uint8_t*) bigNumbers1x2_1;
            forceGapBetweenNumbers = false;
            break;
        case BIG_NUMBERS_FONT_2_COLUMN_2_ROWS:
            bigNumbersCustomPatternsPGM = bigNumbers2x2CustomPatterns_1;
            bigNumbersFont = (const uint8_t*) bigNumbers2x2_1;
            break;
        case BIG_NUMBERS_FONT_3_COLUMN_2_ROWS_VARIANT_1:
            bigNumbersCustomPatternsPGM = bigNumbers3x2CustomPatterns_1;
            bigNumbersFont = (const uint8_t*) bigNumbers3x2_1;
            NumberOfCustomPatterns = 6;
            break;
        case BIG_NUMBERS_FONT_3_COLUMN_2_ROWS_VARIANT_2:
            bigNumbersCustomPatternsPGM = bigNumbers3x2CustomPatterns_2;
            bigNumbersFont = (const uint8_t*) bigNumbers3x2_2;
            forceGapBetweenNumbers = false;
            break;
        case BIG_NUMBERS_FONT_3_COLUMN_2_ROWS_VARIANT_3:
            bigNumbersCustomPatternsPGM = bigNumbers3x2CustomPatterns_3;
            bigNumbersFont = (const uint8_t*) bigNumbers3x2_3;
            forceGapBetweenNumbers = false;
            break;
#if LCD_ROWS <= 2
        default:
            // ERROR: NumberHeight is greater than 2 for a 2 line display -> fallback to 2x2 font
            bigNumbersCustomPatternsPGM = bigNumbers2x2CustomPatterns_1;
            bigNumbersFont = (const uint8_t*) bigNumbers2x2_1;
            NumberWidth = 2;
            NumberHeight = 2;
            break;
#else
        case BIG_NUMBERS_FONT_2_COLUMN_3_ROWS_VARIANT_1:
            bigNumbersCustomPatternsPGM = bigNumbers2x3CustomPatterns_1;
            bigNumbersFont = (const uint8_t*) bigNumbers2x3_1;
            break;
        case BIG_NUMBERS_FONT_2_COLUMN_3_ROWS_VARIANT_2:
            bigNumbersCustomPatternsPGM = bigNumbers2x3CustomPatterns_2;
            bigNumbersFont = (const uint8_t*) bigNumbers2x3_2;
            break;
        case BIG_NUMBERS_FONT_3_COLUMN_3_ROWS:
            bigNumbersCustomPatternsPGM = bigNumbers3x3And3x4CustomPatterns_1;
            bigNumbersFont = (const uint8_t*) bigNumbers3x3_1;
            break;
        case BIG_NUMBERS_FONT_3_COLUMN_4_ROWS_VARIANT_1:
            bigNumbersCustomPatternsPGM = bigNumbers3x3And3x4CustomPatterns_1;
            bigNumbersFont = (const uint8_t*) bigNumbers3x4_1;
            break;
        case BIG_NUMBERS_FONT_3_COLUMN_4_ROWS_VARIANT_2:
            bigNumbersCustomPatternsPGM = bigNumbers3x4CustomPatterns_2;
            bigNumbersFont = (const uint8_t*) bigNumbers3x4_2;
            break;
        default:
            // fallback to 2x2 font
            bigNumbersCustomPatternsPGM = bigNumbers2x2CustomPatterns_1;
            bigNumbersFont = (const uint8_t*) bigNumbers2x2_1;
            NumberWidth = 2;
            NumberHeight = 2;
            break;
#endif
        }
    }

    /*
     * Function to reconfigure existing object to hold (another) font and to create the custom characters required
     */
    void loadFont(const uint8_t aBigNumberFontIdentifier) {
        _setFont(aBigNumberFontIdentifier);
        createCustomChars();
    }

#if defined(USE_PARALLEL_LCD)
    LCDBigNumbers(LiquidCrystal *aLCD, const uint8_t aBigNumberFontIdentifier) :
#else
    LCDBigNumbers(LiquidCrystal_I2C *aLCD, const uint8_t aBigNumberFontIdentifier) :
#endif
                    LCD(aLCD) {
        _setFont(aBigNumberFontIdentifier); // cannot call begin here, because LCD is not initialized and we can not set custom fonts here
    }

    /*
     * Like LiquidCrystal::createChar(uint8_t location, uint8_t charmap[])
     * but with PROGMEM input
     * This also sets cursor to 0.0
     */
    void _createChar(uint_fast8_t aCharacterIndex, const uint8_t *CustomPatternMapPGM) {
        aCharacterIndex &= 0x7; // we only have 8 locations 0-7
        LCD->command(LCD_SETCGRAMADDR | (aCharacterIndex << 3));
        for (uint_fast8_t i = 0; i < 8; i++) {
            LCD->write(pgm_read_byte(CustomPatternMapPGM++));
        }
        LCD->command(LCD_SETDDRAMADDR); // set cursor to 0.0, this avoids overwriting CGRAM by next write() command.
    }

    /**
     * Draws a big digit of size aNumberWidth x aNumberHeight at cursor position.
     * For some fonts, a one column gap is inserted after each digit, space and '-'
     * The gap is cleared, if not at the (last + 1) column!
     * Special characters "-.:|_" always have the width of 1 and only '-' can have a gap behind.
     * @param aNumber - byte 0x00 to 0x09 or ASCII number '0' to '9' or space or one of these special characters: "-.:|_"
     * @return  The number of columns written (1 to 4 currently) or 10 for all skipped characters.
     */
    size_t writeBigNumber(uint8_t aNumberOrSpecialCharacter) {
        uint_fast8_t tFontArrayOffset = 0;
        uint_fast8_t tCharacterWidth = 1;
        /*
         * First 3 entries are the special characters
         */
        if (aNumberOrSpecialCharacter == '-' || aNumberOrSpecialCharacter == ONE_COLUMN_HYPHEN_CHARACTER) {
            // here we have the initial values: tFontArrayOffset = 0
        } else if (aNumberOrSpecialCharacter == '.') {
            tFontArrayOffset = 1;
        } else if (aNumberOrSpecialCharacter == ':') {
            tFontArrayOffset = 2;
        } else if (aNumberOrSpecialCharacter == ' ') {
            tCharacterWidth = NumberWidth; // Prepare to draw a normal space as a space with the width of a number
        } else if (aNumberOrSpecialCharacter == ONE_COLUMN_SPACE_CHARACTER) {
            // Prepare to draw a one column space here
            aNumberOrSpecialCharacter = ' ';
        } else {
            /*
             * Here we have numbers only
             */
            if (aNumberOrSpecialCharacter > 9) {
                // if not byte 0x00 to 0x09, convert ASCII character to number
                aNumberOrSpecialCharacter -= '0';
            }
            if (aNumberOrSpecialCharacter > 9) {
                // If we do not have a number 0 to 9 now, we skip it
                // The correct return value of 0 is interpreted as an error and returning 1 may lead to some hard to find inconsistencies and buffer overflows. Thus we return 10.
                return 10;
            }
            tCharacterWidth = NumberWidth;
            tFontArrayOffset = NUMBER_OF_SPECIAL_CHARACTERS_IN_FONT_ARRAY + (aNumberOrSpecialCharacter * tCharacterWidth);
        }
#if defined(LOCAL_DEBUG)
        Serial.print(F("Number="));
        Serial.print(aNumberOrSpecialCharacter);
        Serial.print(F(" CharacterWidth="));
        Serial.print(tCharacterWidth);
        Serial.print(F(" FontArrayOffset="));
        Serial.print(tFontArrayOffset);
        Serial.print(F(" ColunmOffset="));
        Serial.println(upperLeftColumnIndex);
#endif
        const uint8_t *tArrayPtr = bigNumbersFont + tFontArrayOffset;
        // Render character row by row
        for (uint_fast8_t tRow = 0; tRow < NumberHeight; tRow++) {
            LCD->setCursor(upperLeftColumnIndex, upperLeftRowIndex + tRow);
            // Render all columns in a row
            for (uint_fast8_t i = 0; i < tCharacterWidth; i++) {
                uint8_t tCharacterIndex;
                if (aNumberOrSpecialCharacter == ' ') {
                    tCharacterIndex = ' '; // Blank
                } else {
                    tCharacterIndex = pgm_read_byte(tArrayPtr);
                }
                LCD->write(tCharacterIndex);
                tArrayPtr++; // Prepare for next column, we do not use i
#if defined(LOCAL_DEBUG)
                Serial.print(F(" 0x"));
                Serial.print(tCharacterIndex, HEX);
                Serial.print(F(" 0x"));
                Serial.print((uintptr_t) tArrayPtr, HEX);
#endif
            }
            tArrayPtr += NUMBER_OF_SPECIAL_CHARACTERS_IN_FONT_ARRAY + (NumberWidth - tCharacterWidth) + (9 * NumberWidth); // Next array row
#if defined(LOCAL_DEBUG)
            Serial.print('|');
#endif
        }
        upperLeftColumnIndex += tCharacterWidth; // now we are at the index for the next number or the optional blank gap

        /*
         * Implement the gap after the number character
         */
        if (forceGapBetweenNumbers && (NumberWidth == 1 || tCharacterWidth > 1 || aNumberOrSpecialCharacter == '-')) {
            if (LCD_COLUMNS > upperLeftColumnIndex) {
                // We are not at the last column, so clear the gap after the number
                for (uint_fast8_t tRow = 0; tRow < NumberHeight; tRow++) {
                    LCD->setCursor(upperLeftColumnIndex, upperLeftRowIndex + tRow);
                    LCD->write(' '); // Blank
                }
                tCharacterWidth++;
            }
            upperLeftColumnIndex++; // This provides one column gap between numbers, but not between special characters.
        }

#if defined(LOCAL_DEBUG)
        Serial.println();
#endif
        return tCharacterWidth;
    }

    /**
     * Draws a big digit of size aNumberWidth x aNumberHeight
     * @param aNumber - byte 0x00 to 0x09 or ASCII number '0' to '9' or space or one of these special characters: "-.:|_"
     * @param aUpperLeftColumnIndex - Starts with 0, no check!
     * @param aUpperLeftRowIndex - Starts with 0, no check!
     */
    size_t writeAt(uint8_t aNumber, uint8_t aUpperLeftColumnIndex, uint8_t aUpperLeftRowIndex = 0) {
        setBigNumberCursor(aUpperLeftColumnIndex, aUpperLeftRowIndex);
        return writeBigNumber(aNumber);
    }

};

/**
 * Print all fonts, used in screenshots, using one object
 * BIG_NUMBERS_FONT_1_COLUMN_2_ROWS, BIG_NUMBERS_FONT_2_COLUMN_2_ROWS,
 * BIG_NUMBERS_FONT_3_COLUMN_2_ROWS_VARIANT_1, BIG_NUMBERS_FONT_3_COLUMN_2_ROWS_VARIANT_2, BIG_NUMBERS_FONT_3_COLUMN_2_ROWS_VARIANT_3,
 * BIG_NUMBERS_FONT_2_COLUMN_3_ROWS_VARIANT_1, BIG_NUMBERS_FONT_2_COLUMN_3_ROWS_VARIANT_2, BIG_NUMBERS_FONT_3_COLUMN_3_ROWS,
 * BIG_NUMBERS_FONT_3_COLUMN_4_ROWS_VARIANT_1, BIG_NUMBERS_FONT_3_COLUMN_4_ROWS_VARIANT_2
 *
 * Strings rendered are:
 * "01234"
 * "56789" ONE_COLUMN_SPACE_STRING ONE_COLUMN_HYPHEN_STRING ".:"
 * "-42.38:"
 */
void testBigNumbers(LCDBigNumbers *aBigNumberLCD) {
    /*
     * 1 X 2
     */
    aBigNumberLCD->LCD->clear(); // Clear display
    aBigNumberLCD->loadFont(BIG_NUMBERS_FONT_1_COLUMN_2_ROWS);
    aBigNumberLCD->print(F("0123456789" ONE_COLUMN_SPACE_STRING ONE_COLUMN_HYPHEN_STRING ".:")); // Special and normal space are the same size here, we have an 1 column font

#if LCD_ROWS >2
    aBigNumberLCD->setBigNumberCursor(0, 2);
    aBigNumberLCD->print(F("-42.38:"));
    delay(DEFAULT_TEST_DELAY);
#endif

    /*
     * 2 X 2
     */
    aBigNumberLCD->LCD->clear(); // Clear display
    // Reconfigure existing object to hold another font
    aBigNumberLCD->loadFont(BIG_NUMBERS_FONT_2_COLUMN_2_ROWS);
    aBigNumberLCD->print(F("0123a4Z  1")); // Renders as "01234  " Test that regular characters are not printed by LCDBigNumber and test overflow handling
#if LCD_ROWS <= 2
    delay(DEFAULT_TEST_DELAY);
    aBigNumberLCD->setBigNumberCursor(0);
    aBigNumberLCD->print(F("56789"));
    delay(DEFAULT_TEST_DELAY);
    aBigNumberLCD->setBigNumberCursor(0);
    aBigNumberLCD->print(F("-.:"));
#else
    aBigNumberLCD->setBigNumberCursor(0, 2);
    aBigNumberLCD->print(F("56789" ONE_COLUMN_SPACE_STRING ONE_COLUMN_HYPHEN_STRING ".:"));
#endif
    delay(DEFAULT_TEST_DELAY);

    aBigNumberLCD->LCD->clear(); // Clear display
    aBigNumberLCD->setBigNumberCursor(0, 0);
    aBigNumberLCD->print(F("-42.38:"));
    delay(DEFAULT_TEST_DELAY);

    /*
     * 3 X 2 1. variant
     */
    aBigNumberLCD->LCD->clear(); // Clear display
    aBigNumberLCD->loadFont(BIG_NUMBERS_FONT_3_COLUMN_2_ROWS_VARIANT_1);

#if LCD_ROWS <= 2
    aBigNumberLCD->print(F("0123"));
    delay(DEFAULT_TEST_DELAY);
    aBigNumberLCD->setBigNumberCursor(0);
    aBigNumberLCD->print(F("4567"));
    delay(DEFAULT_TEST_DELAY);
    aBigNumberLCD->setBigNumberCursor(0);
    aBigNumberLCD->print(F("89" ONE_COLUMN_SPACE_STRING "-.:"));
#else
    aBigNumberLCD->print(F("01234"));
    aBigNumberLCD->setBigNumberCursor(0, 2);
    aBigNumberLCD->print(F("56789"));
    delay(DEFAULT_TEST_DELAY);

    aBigNumberLCD->LCD->clear(); // Clear display
    aBigNumberLCD->setBigNumberCursor(0);
    aBigNumberLCD->print(F("-42.38:"));
#endif

    delay(DEFAULT_TEST_DELAY);

    /*
     * 3 X 2 2. variant
     */
    aBigNumberLCD->LCD->clear(); // Clear display
    aBigNumberLCD->loadFont(BIG_NUMBERS_FONT_3_COLUMN_2_ROWS_VARIANT_2);
    aBigNumberLCD->print(F("01234"));
#if LCD_ROWS <= 2
    delay(DEFAULT_TEST_DELAY);
    aBigNumberLCD->setBigNumberCursor(0);
    aBigNumberLCD->print(F("56789"));
    delay(DEFAULT_TEST_DELAY);
    aBigNumberLCD->setBigNumberCursor(0);
    aBigNumberLCD->print(F("-.:"));
#else
    aBigNumberLCD->setBigNumberCursor(0, 2);
    aBigNumberLCD->print(F("56789" ONE_COLUMN_SPACE_STRING ONE_COLUMN_HYPHEN_STRING ".:"));
#endif
    delay(DEFAULT_TEST_DELAY);

    aBigNumberLCD->LCD->clear(); // Clear display
    aBigNumberLCD->setBigNumberCursor(0);
    aBigNumberLCD->print(F("-42.38:"));
    delay(DEFAULT_TEST_DELAY);

    /*
     * 3 X 2 3. variant
     */
    aBigNumberLCD->LCD->clear(); // Clear display
    aBigNumberLCD->loadFont(BIG_NUMBERS_FONT_3_COLUMN_2_ROWS_VARIANT_3);
    aBigNumberLCD->print(F("01234"));
#if LCD_ROWS <= 2
    delay(DEFAULT_TEST_DELAY);
    aBigNumberLCD->setBigNumberCursor(0);
    aBigNumberLCD->print(F("56789"));
    delay(DEFAULT_TEST_DELAY);
    aBigNumberLCD->setBigNumberCursor(0);
    aBigNumberLCD->print(F("-.:"));
#else
    aBigNumberLCD->setBigNumberCursor(0, 2);
    aBigNumberLCD->print(F("56789" ONE_COLUMN_SPACE_STRING ONE_COLUMN_HYPHEN_STRING ".:"));
#endif
    delay(DEFAULT_TEST_DELAY);

    aBigNumberLCD->LCD->clear(); // Clear display
    aBigNumberLCD->setBigNumberCursor(0);
    aBigNumberLCD->print(F("-42.38:"));
    delay(DEFAULT_TEST_DELAY);

#if LCD_ROWS > 2
    /**********************************************************************************
     * 3 line numbers
     **********************************************************************************/
    /*
     * 2 X 3 Space above
     */
    aBigNumberLCD->LCD->clear(); // Clear display
    aBigNumberLCD->loadFont(BIG_NUMBERS_FONT_2_COLUMN_3_ROWS_VARIANT_1);
    aBigNumberLCD->setBigNumberCursor(0, 1);
    aBigNumberLCD->print(F("01234"));
    delay(DEFAULT_TEST_DELAY);
    aBigNumberLCD->setBigNumberCursor(0, 1);
    aBigNumberLCD->print(F("56789" ONE_COLUMN_SPACE_STRING ONE_COLUMN_HYPHEN_STRING ".:"));
    delay(DEFAULT_TEST_DELAY);

    aBigNumberLCD->LCD->clear(); // Clear display
    aBigNumberLCD->setBigNumberCursor(0, 1);
    aBigNumberLCD->print(F("-42.38:"));
    delay(DEFAULT_TEST_DELAY);

    /*
     * 2 X 3 Space below
     */
    aBigNumberLCD->LCD->clear(); // Clear display
    aBigNumberLCD->loadFont( BIG_NUMBERS_FONT_2_COLUMN_3_ROWS_VARIANT_2);
    aBigNumberLCD->setBigNumberCursor(0, 1);
    aBigNumberLCD->print(F("01234"));
    delay(DEFAULT_TEST_DELAY);
    aBigNumberLCD->setBigNumberCursor(0, 1);
    aBigNumberLCD->print(F("56789" ONE_COLUMN_SPACE_STRING ONE_COLUMN_HYPHEN_STRING ".:"));
    delay(DEFAULT_TEST_DELAY);

    aBigNumberLCD->LCD->clear(); // Clear display
    aBigNumberLCD->setBigNumberCursor(0, 1);
    aBigNumberLCD->print(F("-42.38:"));
    delay(DEFAULT_TEST_DELAY);

    /*
     * 3 X 3 Space below
     */
    aBigNumberLCD->LCD->clear(); // Clear display
    aBigNumberLCD->loadFont(BIG_NUMBERS_FONT_3_COLUMN_3_ROWS);
    aBigNumberLCD->setBigNumberCursor(0, 1);
    aBigNumberLCD->print(F("01234"));
    delay(DEFAULT_TEST_DELAY);
    aBigNumberLCD->setBigNumberCursor(0, 1);
    aBigNumberLCD->print(F("56789"));
    delay(DEFAULT_TEST_DELAY);

    aBigNumberLCD->LCD->clear(); // Clear display
    aBigNumberLCD->setBigNumberCursor(0, 1);
    aBigNumberLCD->print(F("-42.38:"));
    delay(DEFAULT_TEST_DELAY);

    /****************
     * 4 line numbers
     ****************/
    /*
     * 3 X 4
     */
    aBigNumberLCD->LCD->clear(); // Clear display
    aBigNumberLCD->loadFont( BIG_NUMBERS_FONT_3_COLUMN_4_ROWS_VARIANT_1);
    aBigNumberLCD->print(F("01234"));
    delay(DEFAULT_TEST_DELAY);
    aBigNumberLCD->setBigNumberCursor(0);
    aBigNumberLCD->print(F("56789"));
    delay(DEFAULT_TEST_DELAY);

    aBigNumberLCD->LCD->clear(); // Clear display
    aBigNumberLCD->setBigNumberCursor(0);
    aBigNumberLCD->print(F("-42.38:"));
    delay(DEFAULT_TEST_DELAY);

    /*
     * Variant 2
     */
    aBigNumberLCD->LCD->clear(); // Clear display
    aBigNumberLCD->loadFont( BIG_NUMBERS_FONT_3_COLUMN_4_ROWS_VARIANT_2);
    aBigNumberLCD->print(F("01234"));
    delay(DEFAULT_TEST_DELAY);
    aBigNumberLCD->setBigNumberCursor(0);
    aBigNumberLCD->print(F("56789"));
    delay(DEFAULT_TEST_DELAY);

    aBigNumberLCD->LCD->clear(); // Clear display
    aBigNumberLCD->setBigNumberCursor(0);
    aBigNumberLCD->print(F("-42.38:"));
    delay(DEFAULT_TEST_DELAY);
#endif // LCD_ROWS > 2
}

#if defined(LOCAL_DEBUG)
#undef LOCAL_DEBUG
#endif
#endif // _LCD_BIG_NUMBERS_HPP
