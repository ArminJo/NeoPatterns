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

#include "MatrixNeoPixel.h"

// Demo 8x8 heart graphics
const uint8_t heart8x8[] PROGMEM = { 0x66, 0xFF, 0xFF, 0xFF, 0x7E, 0x3C, 0x18, 0x00 };

MatrixNeoPixel::MatrixNeoPixel(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry, uint8_t aTypeOfPixel) : // @suppress("Class members should be properly initialized")
        NeoPixel(aColumns * aRows, aPin, aTypeOfPixel) {
    Rows = aRows;
    Columns = aColumns;
    Geometry = aMatrixGeometry;
    LayoutMappingFunction = NULL;
}

void MatrixNeoPixel::setLayoutMappingFunction(uint16_t (*aLayoutMappingFunction)(uint8_t, uint8_t, uint8_t, uint8_t)) {
    LayoutMappingFunction = aLayoutMappingFunction;
}

void MatrixNeoPixel::setMatrixPixelColor(uint8_t x, uint8_t y, uint8_t aRed, uint8_t aGreen, uint8_t aBlue) {
#ifdef DEBUG
    Serial.print(F("set x="));
    Serial.print(x);
    Serial.print(F(" y="));
    Serial.print(y);
    Serial.print(F(" n="));
    Serial.print(LayoutMapping(x, y));
    Serial.print(F(" color="));
    Serial.println(a32BitColor, HEX);
#endif
    if (LayoutMappingFunction == NULL) {
        setPixelColor(LayoutMapping(x, y), aRed, aGreen, aBlue);
    } else {
        setPixelColor(LayoutMappingFunction(x, y, Columns, Rows), aRed, aGreen, aBlue);
    }
}

/*
 * If LayoutMappingFunction is not set, use ZTypeMapping.
 */
void MatrixNeoPixel::setMatrixPixelColor(uint8_t x, uint8_t y, color32_t a32BitColor) {
#ifdef DEBUG
    Serial.print(F("set x="));
    Serial.print(x);
    Serial.print(F(" y="));
    Serial.print(y);
    Serial.print(F(" n="));
    Serial.print(LayoutMapping(x, y));
    Serial.print(F(" color="));
    Serial.println(a32BitColor, HEX);
#endif
    if (LayoutMappingFunction == NULL) {
        setPixelColor(LayoutMapping(x, y), a32BitColor);
    } else {
        setPixelColor(LayoutMappingFunction(x, y, Columns, Rows), a32BitColor);
    }
}

/*
 * If LayoutMappingFunction is not set, use ZTypeMapping.
 */
uint32_t MatrixNeoPixel::getMatrixPixelColor(uint8_t x, uint8_t y) {
    uint32_t tColor;
    if (LayoutMappingFunction == NULL) {
        tColor = getPixelColor(LayoutMapping(x, y));
    } else {
        tColor = getPixelColor(LayoutMappingFunction(x, y, Columns, Rows));
    }
#ifdef DEBUG
    Serial.print(F("set x="));
    Serial.print(x);
    Serial.print(F(" y="));
    Serial.print(y);
    Serial.print(F(" n="));
    Serial.print(LayoutMapping(x, y));
    Serial.print(F(" color="));
    Serial.println(tColor, HEX);
#endif
    return tColor;
}

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
                tRetvalue = (Columns * tRows - aColumnX) - 1; // From Right to Left -1 since we start at 0
            } else {
                tRetvalue = (Columns * (tRows - 1)) + aColumnX; // From Left to Right
            }
        } else {
            // ZIGZAG here
            if ((Geometry & NEO_MATRIX_RIGHT) == NEO_MATRIX_RIGHT) {
                tCompareValue = 0;
            } else {
                tCompareValue = 1;
            }
            if ((tRows & 0x01) != tCompareValue) { // equivalent to ((tRows - 1) % 2 == tCompareValue)
                tRetvalue = (Columns * tRows - aColumnX) - 1; // From Right to Left -1 since we start at 0
            } else {
                tRetvalue = (Columns * (tRows - 1)) + aColumnX;  // From Left to Right
            }
        }
    }
    return tRetvalue;
}

/*
 * For displaying a one color graphic.
 *
 * Graphic is stored in a byte array where array[0] is the upper line and the lowest bit is the rightmost pixel.
 * Smaller graphics can be displayed by using aNumberOfVerticalLinesToProcess < 8 which ignores left / highest bits of 8bit line
 *
 * aHeightOfGraphic how much bytes are a graphic
 * aYOffset positive -> picture is shift up and truncated and bottom is padded with aBackgroundColor
 *          negative picture is shift down and truncated and top lines are padded with aBackgroundColor
 * aXOffset positive -> picture is shift right and left lines are padded -> rightmost part of graphic is not displayed
 *          negative -> picture is shift left and right lines are padded
 * doPaddingRight -> if  doPadding == false only do padding right from graphic. Needed for (last) character while moving left.
 * doPadding true -> fill bottom, top and right lines with background color
 */
void MatrixNeoPixel::loadPicturePGM(const uint8_t* aGraphicsArrayPtrPGM, int8_t aWidthOfGraphic, uint8_t aHeightOfGraphic,
        color32_t aForegroundColor, color32_t aBackgroundColor, int8_t aXOffset, int8_t aYOffset, bool doPaddingRight,
        bool doPadding) {
    loadPicture(aGraphicsArrayPtrPGM, aWidthOfGraphic, aHeightOfGraphic, aForegroundColor, aBackgroundColor, aXOffset, aYOffset,
            doPaddingRight, doPadding, true);
}
void MatrixNeoPixel::loadPicture(const uint8_t* aGraphicsArrayPtr, int8_t aWidthOfGraphic, uint8_t aHeightOfGraphic,
        color32_t aForegroundColor, color32_t aBackgroundColor, int8_t aXOffset, int8_t aYOffset, bool doPaddingRight,
        bool doPadding, bool IsPGMData) {

#ifdef DEBUG
    Serial.print(F("aGraphicsPtr="));
    Serial.print((uint16_t) aGraphicsArrayPtr, HEX);
    Serial.print(F(" aXOffset="));
    Serial.print(aXOffset);
    Serial.print(F(" aYOffset="));
    Serial.println(aYOffset);
#endif

    int tYposition = 0;
    const uint8_t* tGraphicsPointer = aGraphicsArrayPtr;

    uint8_t tLinesFromGraphic = aHeightOfGraphic;
    aXOffset = constrain(aXOffset, -Rows, Rows);
    aYOffset = constrain(aYOffset, -Columns, Columns);

    /*
     * YOffset handling
     */
    if (aYOffset > 0) {
        // shift up
        tLinesFromGraphic -= aYOffset;
        tGraphicsPointer += aYOffset;
        if (doPadding) {
            // fill bottom lines with background color
            for (uint8_t y = tLinesFromGraphic; y < Rows; ++y) {
                for (uint8_t x = 0; x < Columns; ++x) {
                    setMatrixPixelColor(x, y, aBackgroundColor);
                }
            }
        }
    } else if (aYOffset < 0) {
        // shift down
        if (doPadding) {
            // fill top lines with background color
            for (uint8_t y = 0; y < -aYOffset; ++y) {
                for (uint8_t x = 0; x < Columns; ++x) {
                    setMatrixPixelColor(x, y, aBackgroundColor);
                }
            }
        }
        tYposition += -aYOffset;
    }

    /*
     * XOffset handling
     * Use the lower 8 bit of a 16bit bitmask
     */
    uint16_t tBitmaskToStart = 0x0080 >> (8 - aWidthOfGraphic); // 8 are width of one byte (of the byte array of picture)
    if (aXOffset > 0) {
        tBitmaskToStart = tBitmaskToStart << aXOffset;
    } else if (aXOffset < 0) {
        tBitmaskToStart = tBitmaskToStart >> -aXOffset;
    }

    for (uint8_t i = 0; i < tLinesFromGraphic; ++i) {
        uint8_t tLineBitPattern;
        if (IsPGMData) {
            tLineBitPattern = pgm_read_byte(tGraphicsPointer);
        } else {
            tLineBitPattern = *tGraphicsPointer;
        }
        uint16_t tBitmaskExtended = tBitmaskToStart; // shifting mask

#ifdef TRACE
        Serial.print(F("tGraphicsPointer="));
        Serial.print((uint16_t) tGraphicsPointer, HEX);
        Serial.print(F(" tLineBitPattern="));
        Serial.println(tLineBitPattern, HEX);
#endif
        /*
         * Process one horizontal line
         */
        for (int8_t x = 0; x < Rows; ++x) {
            uint8_t tBitmask = tBitmaskExtended & 0xFF;
#ifdef TRACE
            Serial.print(F(" tBitmask="));
            Serial.print(tBitmask);
            Serial.print(F(" x="));
            Serial.print(x);
            Serial.print(F(" y="));
            Serial.println(tYposition);
#endif
            if (tBitmask != 0x00 && x >= aXOffset) {
                if (tBitmask & tLineBitPattern) {
                    // bit in pattern is 1
                    setMatrixPixelColor(x, tYposition, aForegroundColor);
                } else {
                    setMatrixPixelColor(x, tYposition, aBackgroundColor);
                }
            } else if (doPadding) {
                // padding
                setMatrixPixelColor(x, tYposition, aBackgroundColor);
            } else if (doPaddingRight && tBitmask == 0x00 && x >= aXOffset) {
                setMatrixPixelColor(x, tYposition, aBackgroundColor);
            }
            tBitmaskExtended = tBitmaskExtended >> 1;
        }
        tGraphicsPointer++;
        tYposition++;
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
                uint8_t greenC = NeoPixel::gamma5(green);
                uint8_t blueC = NeoPixel::gamma5(blue);
                uint8_t redC = NeoPixel::gamma5(red);

#ifdef TRACE
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

