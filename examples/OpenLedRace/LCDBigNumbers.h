/*
 *  LCDBigNumbers.h
 *
 *  The plain include file without sources.
 *  !!! Do not include (and is not required) if LCDBigNumbers.hpp is included!!!
 *
 *  Arduino library to write big numbers on a 1602 or 2004 LCD.
 *
 *  Copyright (C) 2022-2026  Armin Joachimsmeyer
 *
 *  This file is part of LCDBigNumbers https://github.com/ArminJo/LCDBigNumbers.
 *
 *  LCDBigNumbers is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/gpl-3.0.html>.
 */

#if !defined(_LCD_BIG_NUMBERS_HPP) // include only if hpp was not included before
#if !defined(_LCD_BIG_NUMBERS_H)
#define _LCD_BIG_NUMBERS_H

#include <Arduino.h>
#define ONE_COLUMN_SPACE_CHARACTER      '|' // This input character is printed as a one column space. A normal space is printed as a space but with the width of a number.
#define ONE_COLUMN_SPACE_STRING         "|"

#define ONE_COLUMN_HYPHEN_CHARACTER      '_' // This input character is printed as a one column hyphen. A normal hyphen / minus character is printed as a hyphen and can have a gap behind.
#define ONE_COLUMN_HYPHEN_STRING         "_"

#if !defined(USE_PARALLEL_2004_LCD) && !defined(USE_PARALLEL_1602_LCD) && !defined(USE_SERIAL_2004_LCD) && !defined(USE_SERIAL_1602_LCD)
#define USE_PARALLEL_2004_LCD    // Use parallel 2004 LCD as default
#endif

#if defined(USE_PARALLEL_2004_LCD) || defined(USE_PARALLEL_1602_LCD)
#define USE_PARALLEL_LCD
#include <LiquidCrystal.h>
#else
#include "LiquidCrystal_I2C.h"  // Use an up to date library version which has the init method
#endif

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

class LCDBigNumbers: public Print {

public:
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

#if defined(USE_PARALLEL_LCD)
    LCDBigNumbers(LiquidCrystal *aLCD, const uint8_t aBigNumberFontIdentifier);
#else
    LCDBigNumbers(LiquidCrystal_I2C *aLCD, const uint8_t aBigNumberFontIdentifier);
#endif

    void begin(); // Call it in setup AFTER LCD is initialized
    void createCustomChars();
    void loadFont(const uint8_t aBigNumberFontIdentifier);

    using Print::write;   // make write(const char*), write(buffer, size) visible again
    size_t write(uint8_t aBigNumberValue);
    size_t writeAt(uint8_t aNumber, uint8_t aUpperLeftColumnIndex, uint8_t aUpperLeftRowIndex = 0);
    size_t writeBigNumber(uint8_t aNumberOrSpecialCharacter);
    void setBigNumberCursor(uint8_t aUpperLeftColumnIndex, uint8_t aUpperLeftRowIndex = 0);

    void enableGapBetweenNumbers();
    void disableGapBetweenNumbers();
    void setGapBetweenNumbers(bool aForceGapBetweenNumbers);
};

void testBigNumbers(LCDBigNumbers *aBigNumberLCD);

#endif // _LCD_BIG_NUMBERS_H
#endif // _LCD_BIG_NUMBERS_HPP
