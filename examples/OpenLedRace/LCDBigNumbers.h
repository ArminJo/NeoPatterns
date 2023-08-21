/*
 *  LCDBigNumbers.h
 *
 *  The plain include file without sources.
 *
 *  Arduino library to write big numbers on a 1602 or 2004 LCD.
 *
 *  Copyright (C) 2022-2023  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
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

 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/gpl.html>.
 *
 */

#ifndef _LCD_BIG_NUMBERS_H
#define _LCD_BIG_NUMBERS_H

#include <Arduino.h>
#define ONE_COLUMN_SPACE_CHARACTER      '|' // This character is printed as a one column space. Normal spaces are printed as a space with the width of the number.
#define ONE_COLUMN_SPACE_STRING         "|" // This string is printed as a one column space. Normal spaces are printed as a space with the width of the number.

#define ONE_COLUMN_HYPHEN_CHARACTER      '_' // This input character is printed as a one column hyphen. Normal hyphen / minus are printed as a hyphen with the width of the number - 1.
#define ONE_COLUMN_HYPHEN_STRING         "_" // This input string is printed as a one column hyphen. Normal hyphen / minus are printed as a hyphen with the width of the number - 1.

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
#define BIG_NUMBERS_FONT_1_COLUMN_2_ROWS_VARIANT_1  0x01
#define BIG_NUMBERS_FONT_2_COLUMN_2_ROWS_VARIANT_1  0x05
#define BIG_NUMBERS_FONT_3_COLUMN_2_ROWS_VARIANT_1  0x09
#define BIG_NUMBERS_FONT_3_COLUMN_2_ROWS_VARIANT_2  0x19
#define BIG_NUMBERS_FONT_3_COLUMN_2_ROWS_VARIANT_3  0x29
#define BIG_NUMBERS_FONT_2_COLUMN_3_ROWS_VARIANT_1  0x06
#define BIG_NUMBERS_FONT_2_COLUMN_3_ROWS_VARIANT_2  0x16
#define BIG_NUMBERS_FONT_3_COLUMN_3_ROWS_VARIANT_1  0x0A
#define BIG_NUMBERS_FONT_3_COLUMN_4_ROWS_VARIANT_1  0x0B
#define BIG_NUMBERS_FONT_3_COLUMN_4_ROWS_VARIANT_2  0x1B

class LCDBigNumbers : public Print{

public:
#if defined(USE_PARALLEL_LCD)
    LiquidCrystal *LCD;
#else
    LiquidCrystal_I2C *LCD;
#endif
    const uint8_t NumberWidth;
    const uint8_t NumberHeight;
    uint8_t FontVariant;
    const uint8_t (*bigNumbersCustomPatterns)[8];
    uint8_t NumberOfCustomPatterns;
    const uint8_t *bigNumbersFont;
    bool forceGapBetweenNumbers;    // The default depends on the font used
    uint8_t upperLeftColumnIndex;   // Start of the next character
    uint8_t maximumColumnIndex;     // Maximum of columns to be written. Used to not clear the gap after a number which ends at the last column. ( 44 bytes program space)
    uint8_t upperLeftRowIndex;      // Start of the next character

#if defined(USE_PARALLEL_LCD)
    LCDBigNumbers(LiquidCrystal *aLCD, const uint8_t aBigNumberFontIdentifier);
#else
    LCDBigNumbers(LiquidCrystal_I2C *aLCD, const uint8_t aBigNumberFontIdentifier);
#endif

    //createChar with PROGMEM input
    void _createChar(uint8_t location, const uint8_t *charmap);

    void init(const uint8_t aBigNumberFontIdentifier); // Internal function to select the appropriate font arrays
    size_t write();
    size_t writeAt(uint8_t aNumber, uint8_t aUpperLeftColumnIndex, uint8_t aUpperLeftRowIndex = 0);
    size_t writeBigNumber(uint8_t aNumberOrSpecialCharacter);
    void setBigNumberCursor(uint8_t aUpperLeftColumnIndex, uint8_t aUpperLeftRowIndex = 0);

    void begin(); // Creates custom character used for generating big numbers
    void enableGapBetweenNumbers();
    void disableGapBetweenNumbers();
};

#if defined(USE_PARALLEL_LCD)
void printSpaces(LiquidCrystal *aLCD, uint_fast8_t aNumberOfSpacesToPrint);
#else
void printSpaces(LiquidCrystal_I2C *aLCD, uint_fast8_t aNumberOfSpacesToPrint);
#endif

#if defined(USE_PARALLEL_LCD)
void clearLine(LiquidCrystal *aLCD, uint_fast8_t aLineNumber);
#else
void clearLine(LiquidCrystal_I2C *aLCD, uint_fast8_t aLineNumber);
#endif

/*
 * Prints 2 or 4 character Hex value with leading zero
 */
#if defined(USE_PARALLEL_LCD)
size_t printHex(LiquidCrystal *aLCD, uint16_t aHexByteValue);
#else
size_t printHex(LiquidCrystal_I2C *aLCD, uint16_t aHexByteValue);
#endif

/*
 * On my 2004 LCD the custon characters are available under 0 to 7 and mirrored to 8 to 15
 * The characters 0x80 to 0x8F are blanks
 */
#if defined(USE_PARALLEL_LCD)
void showSpecialCharacters(LiquidCrystal *aLCD);
#else
void showSpecialCharacters(LiquidCrystal_I2C *aLCD);
#endif


#if defined(USE_PARALLEL_LCD)
void showCustomCharacters(LiquidCrystal *aLCD);
#else
void showCustomCharacters(LiquidCrystal_I2C *aLCD);
#endif

#if defined(USE_PARALLEL_LCD)
void testBigNumbers(LiquidCrystal *aLCD);
#else
void testBigNumbers(LiquidCrystal_I2C *aLCD);
#endif

#endif // _LCD_BIG_NUMBERS_H
