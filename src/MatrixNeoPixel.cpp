/*
 * MatrixNeoPixel.cpp
 *
 * Implements basic functions for NeoPixel matrix. Tested with 8x8 matrix.
 *
 *  SUMMARY
 *  You need to install "Adafruit NeoPixel" library under "Tools -> Manage Libraries..." or "Ctrl+Shift+I" -> use "neoPixel" as filter string.
 *
 *  Copyright (C) 2019  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
 *
 *  This file is part of NeoPatterns https://github.com/ArminJo/NeoPatterns.
 *
 *  NeoPatterns is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/gpl.html>.
 *
 */

//#define TRACE
//#define DEBUG
#include "MatrixNeoPixel.h"

// Demo 8x8 heart graphics
const uint8_t heart8x8[] PROGMEM = { 0x66, 0xFF, 0xFF, 0xFF, 0x7E, 0x3C, 0x18, 0x00 };

MatrixNeoPixel::MatrixNeoPixel() :
        NeoPixel() {

    init();
}

void MatrixNeoPixel::init() {
    Rows = 0;
    Columns = 0;
#ifndef SUPPORT_ONLY_DEFAULT_GEOMETRY
    Geometry = NEO_MATRIX_DEFAULT_GEOMETRY;
    LayoutMappingFunction = NULL;
#endif
}

MatrixNeoPixel::MatrixNeoPixel(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry, uint8_t aTypeOfPixel) : // @suppress("Class members should be properly initialized")
        NeoPixel(aColumns * aRows, aPin, aTypeOfPixel) {

    Rows = aRows;
    Columns = aColumns;
#ifndef SUPPORT_ONLY_DEFAULT_GEOMETRY
    Geometry = aMatrixGeometry;
    LayoutMappingFunction = NULL;
#endif
}

/*
 * Returns false if no memory available
 */
bool MatrixNeoPixel::init(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry, uint8_t aTypeOfPixel) {
    bool tRetval = NeoPixel::init(aColumns * aRows, aPin, aTypeOfPixel);

    Rows = aRows;
    Columns = aColumns;
#ifndef SUPPORT_ONLY_DEFAULT_GEOMETRY
    Geometry = aMatrixGeometry;
    LayoutMappingFunction = NULL;
#endif
    return tRetval;
}

#ifndef SUPPORT_ONLY_DEFAULT_GEOMETRY
void MatrixNeoPixel::setLayoutMappingFunction(uint16_t (*aLayoutMappingFunction)(uint8_t, uint8_t, uint8_t, uint8_t)) {
    LayoutMappingFunction = aLayoutMappingFunction;
}
#endif

void MatrixNeoPixel::setMatrixPixelColor(uint8_t aColumnX, uint8_t aRowY, uint8_t aRed, uint8_t aGreen, uint8_t aBlue) {
    if (aColumnX < Columns && aRowY < Rows) {
#ifdef TRACE
        printPin();
        Serial.print(F("set x="));
        Serial.print(aColumnX);
        Serial.print(F(" y="));
        Serial.print(aRowY);
        Serial.print(F(" n="));
        Serial.print(LayoutMapping(aColumnX, aRowY));
        Serial.print(F(" Color="));
        Serial.print(aRed);
        Serial.print('|');
        Serial.print(aGreen);
        Serial.print('|');
        Serial.println(aBlue);
#endif
#ifndef SUPPORT_ONLY_DEFAULT_GEOMETRY
        if (LayoutMappingFunction == NULL) {
            setPixelColor(LayoutMapping(aColumnX, aRowY), aRed, aGreen, aBlue);
        } else {
            setPixelColor(LayoutMappingFunction(aColumnX, aRowY, Columns, Rows), aRed, aGreen, aBlue);
        }
#else
    setPixelColor((Columns * (Rows - aRowY) - aColumnX) - 1, aRed, aGreen, aBlue);
#endif
#ifdef TRACE
    } else {
        printPin();
        Serial.print(F("skip x="));
        Serial.print(aColumnX);
        Serial.print(F(" y="));
        Serial.println(aRowY);
#endif
    }
}

void MatrixNeoPixel::addMatrixPixelColor(uint8_t aColumnX, uint8_t aRowY, uint8_t aRed, uint8_t aGreen, uint8_t aBlue) {
    if (aColumnX < Columns && aRowY < Rows) {
#ifdef TRACE
        printPin();
        Serial.print(F("set x="));
        Serial.print(aColumnX);
        Serial.print(F(" y="));
        Serial.print(aRowY);
        Serial.print(F(" n="));
        Serial.print(LayoutMapping(aColumnX, aRowY));
        Serial.print(F(" Color="));
        Serial.print(aRed);
        Serial.print('|');
        Serial.print(aGreen);
        Serial.print('|');
        Serial.println(aBlue);
#endif
#ifndef SUPPORT_ONLY_DEFAULT_GEOMETRY
        if (LayoutMappingFunction == NULL) {
            addPixelColor(LayoutMapping(aColumnX, aRowY), aRed, aGreen, aBlue);
        } else {
            addPixelColor(LayoutMappingFunction(aColumnX, aRowY, Columns, Rows), aRed, aGreen, aBlue);
        }
#else
    addPixelColor((Columns * (Rows - aRowY) - aColumnX) - 1, aRed, aGreen, aBlue);
#endif
#ifdef TRACE
    } else {
        printPin();
        Serial.print(F("skip x="));
        Serial.print(aColumnX);
        Serial.print(F(" y="));
        Serial.println(aRowY);
#endif
    }
}

/*
 * If LayoutMappingFunction is not set, use ZTypeMapping.
 */
void MatrixNeoPixel::setMatrixPixelColor(uint8_t aColumnX, uint8_t aRowY, color32_t a32BitColor) {
    if (aColumnX < Columns && aRowY < Rows) {
#ifdef TRACE
        printPin();
        Serial.print(F("set x="));
        Serial.print(aColumnX);
        Serial.print(F(" y="));
        Serial.print(aRowY);
        Serial.print(F(" n="));
        Serial.print(LayoutMapping(aColumnX, aRowY));
        Serial.print(F(" color="));
        Serial.println(a32BitColor, HEX);
#endif
#ifndef SUPPORT_ONLY_DEFAULT_GEOMETRY
        if (LayoutMappingFunction == NULL) {
            setPixelColor(LayoutMapping(aColumnX, aRowY), a32BitColor);
        } else {
            setPixelColor(LayoutMappingFunction(aColumnX, aRowY, Columns, Rows), a32BitColor);
        }
#else
        setPixelColor((Columns * (Rows - aRowY) - aColumnX) - 1, a32BitColor);
#endif
#ifdef TRACE
    } else {
        printPin();
        Serial.print(F("skip x="));
        Serial.print(aColumnX);
        Serial.print(F(" y="));
        Serial.println(aRowY);
#endif
    }
}

void MatrixNeoPixel::addMatrixPixelColor(uint8_t aColumnX, uint8_t aRowY, color32_t a32BitColor) {
    if (aColumnX < Columns && aRowY < Rows) {
#ifdef TRACE
        printPin();
        Serial.print(F("set x="));
        Serial.print(aColumnX);
        Serial.print(F(" y="));
        Serial.print(aRowY);
        Serial.print(F(" n="));
        Serial.print(LayoutMapping(aColumnX, aRowY));
        Serial.print(F(" color="));
        Serial.println(a32BitColor, HEX);
#endif
#ifndef SUPPORT_ONLY_DEFAULT_GEOMETRY
        if (LayoutMappingFunction == NULL) {
            addPixelColor(LayoutMapping(aColumnX, aRowY), a32BitColor);
        } else {
            addPixelColor(LayoutMappingFunction(aColumnX, aRowY, Columns, Rows), a32BitColor);
        }
#else
        addPixelColor((Columns * (Rows - aRowY) - aColumnX) - 1, a32BitColor);
#endif
#ifdef TRACE
    } else {
        printPin();
        Serial.print(F("skip x="));
        Serial.print(aColumnX);
        Serial.print(F(" y="));
        Serial.println(aRowY);
#endif
    }
}
/*
 * If LayoutMappingFunction is not set, use ZTypeMapping.
 */
uint32_t MatrixNeoPixel::getMatrixPixelColor(uint8_t aColumnX, uint8_t aRowY) {
    uint32_t tColor;
#ifndef SUPPORT_ONLY_DEFAULT_GEOMETRY
    if (LayoutMappingFunction == NULL) {
        tColor = getPixelColor(LayoutMapping(aColumnX, aRowY));
    } else {
        tColor = getPixelColor(LayoutMappingFunction(aColumnX, aRowY, Columns, Rows));
    }
#else
    tColor = getPixelColor((Columns * (Rows - aRowY) - aColumnX) - 1);
#endif
#ifdef TRACE
    printPin();
    Serial.print(F("set x="));
    Serial.print(aColumnX);
    Serial.print(F(" y="));
    Serial.print(aRowY);
    Serial.print(F(" n="));
    Serial.print(LayoutMapping(aColumnX, aRowY));
    Serial.print(F(" color="));
    Serial.println(tColor, HEX);
#endif
    return tColor;
}

#ifndef SUPPORT_ONLY_DEFAULT_GEOMETRY
/*
 *   Example Pixel mappings supported (bottom up mappings not shown here)
 *
 *   ProgressiveMapping                  ZigzagTypeMapping
 *  Regular        Mirrored           Regular        Mirrored
 *  Bottom/Right   Bottom/Left
 *  15 14 13 12    12 13 14 15        12 13 14 15    15 14 13 12
 *  11 10  9  8     8  9 10 11        11 10  9  8     8  9 10 11
 *   7  6  5  4     4  5  6  7         4  5  6  7     7  6  5  4
 *   3  2  1  0     0  1  2  3         3  2  1  0     0  1  2  3
 */

uint16_t MatrixNeoPixel::LayoutMapping(uint8_t aColumnX, uint8_t aRowY) {
    uint16_t tRetvalue = 0;
    uint8_t tCompareValue = 0;
    uint8_t tRows;
    if (aRowY > Rows) {
        aRowY = Rows;
    }
    if (aColumnX > Columns) {
        aColumnX = Columns;
    }
    if ((Geometry & NEO_MATRIX_COLUMNS) == NEO_MATRIX_COLUMNS) {
#ifdef ERROR
        Serial.print(F("NEO_MATRIX_COLUMNS not yet implemented. Try to rotate your Matrix and use NEO_MATRIX_ROWS."));
#endif

    } else {
        if ((Geometry & NEO_MATRIX_BOTTOM) == NEO_MATRIX_BOTTOM) {
            tRows = Rows - aRowY;
        } else {
            tRows = aRowY + 1;
        }
        if ((Geometry & NEO_MATRIX_SEQUENCE) == NEO_MATRIX_PROGRESSIVE) {
            if ((Geometry & NEO_MATRIX_RIGHT) == NEO_MATRIX_RIGHT) {
                tRetvalue = (Columns * tRows - aColumnX) - 1; // From right to left -1 since we start at 0
            } else {
                tRetvalue = (Columns * (tRows - 1)) + aColumnX; // From left to right
            }
        } else {
            // ZIGZAG here
            if ((Geometry & NEO_MATRIX_RIGHT) == NEO_MATRIX_RIGHT) {
                tCompareValue = 0;
            } else {
                tCompareValue = 1;
            }
            if ((tRows & 0x01) != tCompareValue) { // equivalent to ((tRows - 1) % 2 == tCompareValue)
                tRetvalue = (Columns * tRows - aColumnX) - 1; // From right to left -1 since we start at 0
            } else {
                tRetvalue = (Columns * (tRows - 1)) + aColumnX;  // From left to right
            }
        }
    }
    return tRetvalue;
}
#endif

/*
 * @param aDrawFromBottom -false: Bar is top down, i.e. it starts at the uppermost row (low pixel index!)
 */
void MatrixNeoPixel::drawBar(uint8_t aColumnX, uint16_t aBarLength, color32_t aColor, bool aDrawFromBottom) {

    for (uint8_t i = 0; i < Rows; i++) {
        bool tDrawPixel;
        // Since top left is (0,0) draw from top is like draw from bottom for simple bars
        if (aDrawFromBottom) {
            tDrawPixel = (i >= (Rows - aBarLength));
        } else {
            tDrawPixel = (i < aBarLength);
        }
        if (tDrawPixel) {
            setMatrixPixelColor(aColumnX, i, aColor);
        } else {
            // Clear pixel
            setMatrixPixelColor(aColumnX, i, COLOR32_BLACK);
        }

    }
}

/*
 * @param aColorArrayPtr - Address of a color array holding numLEDs color entries for the bar colors.
 * @param aDrawFromBottom -false: Bar is top down, i.e. it starts at the uppermost row (low pixel index!)
 */
void MatrixNeoPixel::drawBarFromColorArray(uint8_t aColumnX, uint16_t aBarLength, color32_t *aColorArrayPtr, bool aDrawFromBottom) {

    uint8_t j = Rows - 1;
    for (uint8_t i = 0; i < Rows; i++) {
        bool tDrawPixel;

        // Since top left is (0,0) draw from top is like draw from bottom for simple bars
        if (aDrawFromBottom) {
            tDrawPixel = (i >= (Rows - aBarLength));
        } else {
            tDrawPixel = (i < aBarLength);
        }
        if (tDrawPixel) {
            if (aDrawFromBottom) {
                setMatrixPixelColor(aColumnX, i, aColorArrayPtr[j]);
            } else {
                setMatrixPixelColor(aColumnX, i, aColorArrayPtr[i]);
            }
        } else {
            // Clear pixel
            setMatrixPixelColor(aColumnX, i, COLOR32_BLACK);
        }
        j--;
    }
}

/*
 * For displaying a one color graphic with maximum width of 8.
 * @param aGraphicsArrayPtr pointer to the first byte of graphics which is the top row of the graphics
 * Graphic is stored in a byte array where array[0] is the upper line and the lowest bit is the rightmost pixel.
 * Smaller graphics can be displayed by using aNumberOfVerticalLinesToProcess < 8 which ignores left / highest bits of 8bit line
 *
 * aHeightOfGraphic how much bytes are a graphic
 * aYOffset positive -> picture is shift up and truncated and bottom is padded with aBackgroundColor
 *          negative picture is shift down and truncated and top lines are padded with aBackgroundColor
 * aXOffset positive -> picture is shift right and left lines are padded -> rightmost part of graphic is not displayed
 *          negative -> picture is shift left and right lines are padded
 * doPaddingRight -> if  doPadding == false only do padding right from graphic. Required for (last) character while moving left.
 * doPadding true -> fill bottom, top and right lines with background color
 */
void MatrixNeoPixel::loadPicturePGM(const uint8_t *aGraphicsArrayPtrPGM, int8_t aWidthOfGraphic, uint8_t aHeightOfGraphic,
        color32_t aForegroundColor, color32_t aBackgroundColor, int8_t aXOffset, int8_t aYOffset, bool doPadding) {
    loadPicture(aGraphicsArrayPtrPGM, aWidthOfGraphic, aHeightOfGraphic, aForegroundColor, aBackgroundColor, aXOffset, aYOffset,
            doPadding, true);
}
void MatrixNeoPixel::loadPicture(const uint8_t *aGraphicsArrayPtr, int8_t aWidthOfGraphic, uint8_t aHeightOfGraphic,
        color32_t aForegroundColor, color32_t aBackgroundColor, int8_t aXOffset, int8_t aYOffset, bool doPadding, bool IsPGMData) {

#ifdef DEBUG
    printPin();
    Serial.print(F("loadPicture aGraphicsPtr="));
    Serial.print((uintptr_t) aGraphicsArrayPtr, HEX);
    Serial.print(F(" aXOffset="));
    Serial.print(aXOffset);
    Serial.print(F(" aYOffset="));
    Serial.print(aYOffset);
    Serial.print(F(" doPadding="));
    Serial.println(doPadding);
#endif

    const uint8_t *tGraphicsPointer = aGraphicsArrayPtr;
    aYOffset -= (aHeightOfGraphic - 1); // top of graphic has lower offset than bottom line

    /*
     * Use the lower aWidthOfGraphic bits of a 8 bit bitmask
     */
    uint8_t tBitmaskToStart = 0x80 >> (8 - aWidthOfGraphic); // 8 are width of one byte (of the byte array of picture)

    /*
     * Process all horizontal lines from top line to bottom line (+2 for optional padding above and below) -> YOffset is increasing
     */
    for (int8_t y = (aYOffset - 1); y <= (aYOffset + aHeightOfGraphic); ++y) {
        if (y >= 0) {
            uint8_t tLineBitPattern;

            if (y == (aYOffset + aHeightOfGraphic) || y == (aYOffset - 1)) {
                // extra line for optional padding
                if (doPadding) {
                    // set dummy blank line for padding below
                    tLineBitPattern = 0;
                } else {
                    continue;
                }
            } else {
                /*
                 * Get pattern for the current line
                 */
                if (IsPGMData) {
                    tLineBitPattern = pgm_read_byte(tGraphicsPointer);
                } else {
                    tLineBitPattern = *tGraphicsPointer;
                }
            }

            uint8_t tCurrentBitmask = tBitmaskToStart; // shifting mask

#ifdef TRACE
        Serial.print(F(" tGraphicsPointer="));
        Serial.print((uintptr_t) tGraphicsPointer, HEX);
        Serial.print(F(" tLineBitPattern="));
        Serial.println(tLineBitPattern, HEX);
#endif
            /*
             * Process one horizontal line
             */
            for (int8_t x = aXOffset; x < (aXOffset + aWidthOfGraphic); ++x) {
#ifdef TRACE
            Serial.print(F(" tBitmask="));
            Serial.print(tCurrentBitmask);
            Serial.print(F(" x="));
            Serial.print(x);
            Serial.print(F(" y="));
            Serial.println(y);
#endif

                if (x >= 0) {
                    // x < Columns is checked in setMatrixPixelColor()
                    if (tCurrentBitmask & tLineBitPattern) {
                        // bit in pattern is 1
                        setMatrixPixelColor(x, y, aForegroundColor);
                    } else {
                        setMatrixPixelColor(x, y, aBackgroundColor);
                    }
                }
                tCurrentBitmask = tCurrentBitmask >> 1;
            }
            if (doPadding) {
                // do padding right
                setMatrixPixelColor(aXOffset + aWidthOfGraphic, y, aBackgroundColor);
            }
        }
        if (y != (aYOffset - 1)) {
            tGraphicsPointer++;
        }
    }
}

/*
 * For displaying a one color graphic with maximum width of 16.
 *
 * Graphic is stored in a word array where array[0] is the upper line and the lowest bit is the rightmost pixel.
 * Smaller graphics can be displayed by using aNumberOfVerticalLinesToProcess < 16 which ignores left / highest bits of 16bit line
 */
void MatrixNeoPixel::loadPicture(const uint16_t *aGraphicsArrayPtr, int8_t aWidthOfGraphic, uint8_t aHeightOfGraphic,
        color32_t aForegroundColor, color32_t aBackgroundColor, int8_t aXOffset, int8_t aYOffset, bool doPadding, bool IsPGMData) {

#ifdef DEBUG
    printPin();
    Serial.print(F("loadPicture aGraphicsPtr="));
    Serial.print((uintptr_t) aGraphicsArrayPtr, HEX);
    Serial.print(F(" aXOffset="));
    Serial.print(aXOffset);
    Serial.print(F(" aYOffset="));
    Serial.println(aYOffset);
#endif

    const uint16_t *tGraphicsPointer = aGraphicsArrayPtr;

    aYOffset -= (aHeightOfGraphic - 1); // top of graphic has lower offset than bottom line
    aXOffset = constrain(aXOffset, -MAX_SUPPORTED_GRAPHICS_WIDTH, Columns);
    aYOffset = constrain(aYOffset, -1, Rows + MAX_SUPPORTED_GRAPHICS_HEIGHT);

    /*
     * Use the lower aWidthOfGraphic bits of a 16bit bitmask
     */
    uint16_t tBitmaskToStart = 0x8000 >> (16 - aWidthOfGraphic); // 16 are width of one word (of the word array of picture)

    /*
     * Process all horizontal lines from top line to bottom line (+2 for optional padding above and below) -> YOffset is increasing
     */
    for (int8_t y = (aYOffset - 1); y <= (aYOffset + aHeightOfGraphic); ++y) {
        if (y >= 0) {
            uint8_t tLineBitPattern;

            if (y == (aYOffset + aHeightOfGraphic) || y == (aYOffset - 1)) {
                // extra line for optional padding
                if (doPadding) {
                    // set dummy blank line for padding below
                    tLineBitPattern = 0;
                } else {
                    continue;
                }
            } else {
                /*
                 * Get pattern for the current line
                 */
                if (IsPGMData) {
                    tLineBitPattern = pgm_read_byte(tGraphicsPointer);
                } else {
                    tLineBitPattern = *tGraphicsPointer;
                }
            }
            uint16_t tCurrentBitmask = tBitmaskToStart; // shifting mask

#ifdef TRACE
            Serial.print(F("tGraphicsPointer="));
            Serial.print((uintptr_t) tGraphicsPointer, HEX);
            Serial.print(F(" tLineBitPattern="));
            Serial.println(tLineBitPattern, HEX);
#endif
            /*
             * Process one horizontal line
             */
            for (int8_t x = aXOffset; x < (aXOffset + aWidthOfGraphic); ++x) {
#ifdef TRACE
                Serial.print(F(" tBitmask="));
                Serial.print(tCurrentBitmask);
                Serial.print(F(" x="));
                Serial.print(x);
                Serial.print(F(" y="));
                Serial.println(y);
#endif
                if (x >= 0) {
                    // x < Columns is checked in setMatrixPixelColor()
                    if (tCurrentBitmask & tLineBitPattern) {
                        // bit in pattern is 1
                        setMatrixPixelColor(x, y, aForegroundColor);
                    } else {
                        setMatrixPixelColor(x, y, aBackgroundColor);
                    }
                }
                tCurrentBitmask = tCurrentBitmask >> 1;
            }
            if (doPadding) {
                // do padding right
                setMatrixPixelColor(aXOffset + aWidthOfGraphic, y, aBackgroundColor);
            }
        }

        if (y != (aYOffset - 1)) {
            tGraphicsPointer++;
        }
    }
}

/*
 * Draws a pattern which is designed for 7x7 matrix on a 8x8 display
 * The pattern is coded as 4 nibbles in a 16 bit value.
 * The bit mapping to pixels is shown here. B = Border Pixel which is left blank
 *
 * LSB0  1  2  3  2  1  LSB0 B
 *    4  5  6  7  6  5  4    B
 *    8  9 10 11 10  9  8    B
 *   12 13 14 MSB 14 13 12   B
 *    8  9 10 11 10  9  8    B
 *    4  5  6  7  6  5  4    B
 * LSB0  1  2  3  2  1  LSB0 B
 *    B  B  B  B  B  B  B    B
 */
void MatrixNeoPixel::drawQuarterPatternOdd(uint16_t aPatternValue, color32_t aForegroundColor, color32_t aBackgroundColor) {
    uint16_t tBitMask = 0x0001;
#ifdef TRACE
    printPin();
        Serial.print(F("drawQuarterPatternOdd aPatternValue=0x"));
        Serial.println(aPatternValue, HEX);
#endif
    for (uint8_t tPixelY = 0; tPixelY < 4; ++tPixelY) {
        /*
         * Write one line and the above mirrored line
         * The middle line is written twice.
         */
        uint8_t tPixelX;
        for (tPixelX = 0; tPixelX < 4; ++tPixelX) {
            if (tBitMask & aPatternValue) {
                setMatrixPixelColor(tPixelX, tPixelY, aForegroundColor);
                setMatrixPixelColor(tPixelX, 6 - tPixelY, aForegroundColor);
            } else {
                setMatrixPixelColor(tPixelX, tPixelY, aBackgroundColor);
                setMatrixPixelColor(tPixelX, 6 - tPixelY, aBackgroundColor);
            }
            tBitMask <<= 1;
        }
        tBitMask >>= 1;
        if (tPixelY == 3) {
            // restore shifted out bit
            tBitMask = 0x8000;
        }
        /*
         * now mirror the pattern
         */
        for (; tPixelX < 7; ++tPixelX) {
            tBitMask >>= 1;
            if (tBitMask & aPatternValue) {
                setMatrixPixelColor(tPixelX, tPixelY, aForegroundColor);
                setMatrixPixelColor(tPixelX, 6 - tPixelY, aForegroundColor);
            } else {
                setMatrixPixelColor(tPixelX, tPixelY, aBackgroundColor);
                setMatrixPixelColor(tPixelX, 6 - tPixelY, aBackgroundColor);
            }
        }

        // clear 8. pixel
        setMatrixPixelColor(7, tPixelY, aBackgroundColor);
        setMatrixPixelColor(7, 7 - tPixelY, aBackgroundColor);

        // prepare for next line
        tBitMask <<= 4;
    }
//clear bottom line
    for (uint8_t tPixelX = 0; tPixelX < 7; ++tPixelX) {
        setMatrixPixelColor(tPixelX, 7, aBackgroundColor);
    }
    show();
}

/*
 * Draws a pattern on a 8x8 display
 * The pattern is coded as 4 nibbles in a 16 bit value.
 * The bit mapping to pixels is shown here.
 *
 * LSB0  1  2  3  3  2  1  LSB0
 *    4  5  6  7  7  6  5  4
 *    8  9 10 11 11 10  9  8
 *   12 13 14 MSBMSB 14 13 12
 *   12 13 14 MSBMSB 14 13 12
 *    8  9 10 11 11 10  9  8
 *    4  5  6  7  7  6  5  4
 * LSB0  1  2  3  3  2  1  LSB0
 */
void MatrixNeoPixel::drawQuarterPatternEven(uint16_t aPatternValue, color32_t aForegroundColor, color32_t aBackgroundColor) {
    uint16_t tBitMask = 0x0001;
#ifdef TRACE
    printPin();
        Serial.print(F("drawQuarterPatternEven aPatternValue=0x"));
        Serial.println(aPatternValue, HEX);
#endif
    for (uint8_t tPixelY = 0; tPixelY < 4; ++tPixelY) {
        /*
         * Write one line and the above mirrored line
         */
        uint8_t tPixelX;
        for (tPixelX = 0; tPixelX < 4; ++tPixelX) {
            if (tBitMask & aPatternValue) {
                setMatrixPixelColor(tPixelX, tPixelY, aForegroundColor);
                setMatrixPixelColor(tPixelX, 7 - tPixelY, aForegroundColor);
            } else {
                setMatrixPixelColor(tPixelX, tPixelY, aBackgroundColor);
                setMatrixPixelColor(tPixelX, 7 - tPixelY, aBackgroundColor);
            }
            tBitMask <<= 1;
        }
        tBitMask >>= 1;
        if (tPixelY == 3) {
            // restore shifted out bit

            tBitMask = 0x8000;
        }
        /*
         * now mirror the pattern
         */
        for (; tPixelX < 8; ++tPixelX) {
            if (tBitMask & aPatternValue) {
                setMatrixPixelColor(tPixelX, tPixelY, aForegroundColor);
                setMatrixPixelColor(tPixelX, 7 - tPixelY, aForegroundColor);
            } else {
                setMatrixPixelColor(tPixelX, tPixelY, aBackgroundColor);
                setMatrixPixelColor(tPixelX, 7 - tPixelY, aBackgroundColor);
            }
            tBitMask >>= 1;
        }
        if (tPixelY == 0) {
            // restore shifted out bit
            tBitMask = 0x0010;
        } else {
            // prepare for next line
            tBitMask <<= 5;
        }
    }
    show();
}

/*
 * Show different colors on the upper left half of matrix.
 * Lower left is red, upper left is green, upper right is blue.
 * The lower right half are the same colors but each color is gamma corrected
 */
void MatrixNeoPixel::drawAllColors() {
    for (uint8_t y = 0; y < Rows; y++) {
        for (uint8_t x = 0; x < Columns; x++) {
            // check for upper half
            if (x + y < Columns) {
                // linear rising values from 0 to 255
                uint8_t xAscending = (255 * x) / (Columns - 1);
                uint8_t yAscending = (255 * y) / (Rows - 1);

                // linear descending values from decending to 0 in descending steps
                // y==0 => 255, 219, 183, 146, 110, 73, 37, 0
                // y==1 => 219,    ,    ,    ,    ,   ,  0
                // y==1 => 183,    ,    ,    ,    ,  0
                uint8_t xDescendingSpecial = 255 - ((255 * (x + y)) / (Columns - 1));

                uint8_t blue = xAscending;
                uint8_t green = xDescendingSpecial;
                uint8_t red = yAscending;

                // Gamma corrected values
                uint8_t greenC = NeoPixel::gamma32(green);
                uint8_t blueC = NeoPixel::gamma32(blue);
                uint8_t redC = NeoPixel::gamma32(red);

#ifdef TRACE
                    printPin();
                    Serial.print(F("x="));
                    Serial.print(x);
                    Serial.print(F(" y="));
                    Serial.print(y);
                    Serial.print(F(" red="));
                    Serial.print(red);
                    Serial.print(F(" green="));
                    Serial.print(green);
                    Serial.print(F(" blue="));
                    Serial.println(blue);
#endif
                // set values
                setMatrixPixelColor(x, y, red, green, blue);

                // do not overwrite values at diagonal
                if (x + y < (Columns - 1)) {
                    // set gamma corrected values at lower right
                    // We must switch x and y here to fit the two patterns
                    setMatrixPixelColor((Rows - 1) - y, (Columns - 1) - x, redC, greenC, blueC);
                }
            }
        }
    }
    show();
}

/*
 * Show different colors on the matrix.
 * Bottom is red, upper left is green, upper right is blue.
 * This function is more simple and has more red related colors.
 * By switching colors also green and blue related colors can be shown.
 */
void MatrixNeoPixel::drawAllColors2() {

    for (uint8_t y = 0; y < Rows; ++y) {
        for (uint8_t x = 0; x < Columns; ++x) {

            uint8_t yAscending = (255 * y) / (Rows - 1);
            uint8_t yDescending = 255 - yAscending;
            uint8_t xAscending = (yDescending * x) / (Columns - 1);

            uint8_t blue = xAscending;
            uint8_t green = yDescending - xAscending;
            uint8_t red = yAscending;

#ifdef TRACE
                printPin();
                Serial.print(F("x="));
                Serial.print(x);
                Serial.print(F(" y="));
                Serial.print(y);
                Serial.print(F(" red="));
                Serial.print(red);
                Serial.print(F(" green="));
                Serial.print(green);
                Serial.print(F(" blue="));
                Serial.println(blue);
#endif
            setMatrixPixelColor(x, y, red, green, blue);

        }
    }
    show();
}

/*
 * Examples for fast custom mappings
 */
/*
 * Map row and column to pixel index for pixel arrays which pixels are organized in rows in Z type
 * This is default geometry.
 *
 * Example of indexes of
 * an 8 x 8 Pixel array
 *                           4x4 array
 * 63 62 61 60 59 58 57 56
 * 55 54 53 52 51 50 49 48   15 14 13 12
 * ...                       11 10  9  8
 * 15 14 13 12 11 10  9  8    7  6  5  4
 *  7  6  5  4  3  2  1  0    3  2  1  0
 */
uint16_t ProgressiveTypeBottomRightMapping(uint8_t aColumnX, uint8_t aRowY, uint8_t aColumnsTotal, uint8_t aRowsTotal) {
    if (aRowY > aRowsTotal) {
        aRowY = aRowsTotal;
    }
    if (aColumnX > aColumnsTotal) {
        aColumnX = aColumnsTotal;
    }
    return (aColumnsTotal * (aRowsTotal - aRowY) - aColumnX) - 1; // -1 since we start at 0
}

/*
 * Non default mapping
 */
uint16_t ProgressiveTypeBottomLeftMapping(uint8_t aColumnX, uint8_t aRowY, uint8_t aColumnsTotal, uint8_t aRowsTotal) {
    if (aRowY > aRowsTotal) {
        aRowY = aRowsTotal;
    }
    if (aColumnX > aColumnsTotal) {
        aColumnX = aColumnsTotal;
    }
    return (aColumnsTotal * ((aRowsTotal - aRowY) - 1)) + aColumnX;
}

/*
 * Map row and column to pixel index for pixel arrays which pixels are organized in rows in Zigzag type
 *
 * Example of indexes of
 * an 8 x 8 Pixel array      ZigzagTypeMapping
 *                           Regular
 * 56 57 58 59 60 61 62 63
 * 55 54 53 52 51 50 49 48   12 13 14 15
 * ...                       11 10  9  8
 *  8  9 10 11 12 13 14 15    4  5  6  7
 *  7  6  5  4  3  2  1  0    3  2  1  0
 */
uint16_t ZigzagTypeBottomRightMapping(uint8_t aColumnX, uint8_t aRowY, uint8_t aColumnsTotal, uint8_t aRowsTotal) {
    if (aRowY > aRowsTotal) {
        aRowY = aRowsTotal;
    }
    if (aColumnX > aColumnsTotal) {
        aColumnX = aColumnsTotal;
    }
    if (aRowY & 0x01) {
        // second, fourth, sixth . . . row from bottom
        return (aColumnsTotal * ((aRowsTotal - aRowY) - 1)) + aColumnX;
    } else {
        // first, third, fifth . . . row from bottom
        return (aColumnsTotal * (aRowsTotal - aRowY) - aColumnX) - 1; // -1 since we start at 0
    }
}
/* ZigzagTypeMapping
 * Mirrored
 *
 * 15 14 13 12
 *  8  9 10 11
 *  7  6  5  4
 *  0  1  2  3
 */
uint16_t ZigzagTypeBottomLeftMapping(uint8_t aColumnX, uint8_t aRowY, uint8_t aColumnsTotal, uint8_t aRowsTotal) {
    if (aRowY > aRowsTotal) {
        aRowY = aRowsTotal;
    }
    if (aColumnX > aColumnsTotal) {
        aColumnX = aColumnsTotal;
    }
    if (aRowY & 0x01) {
        // second, fourth, sixth . . . row from bottom
        return (aColumnsTotal * (aRowsTotal - aRowY) - aColumnX) - 1; // -1 since we start at 0
    } else {
        // first, third, fifth . . . row from bottom
        return (aColumnsTotal * ((aRowsTotal - aRowY) - 1)) + aColumnX;
    }
}

