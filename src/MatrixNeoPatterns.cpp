/*
 * MatrixNeoPatterns.cpp
 *
 * Implements functions especially for 8x8 NeoPixel matrix.
 *
 *  SUMMARY
 *  You need to install "Adafruit NeoPixel" library under Sketch -> Include Library -> Manage Librarys... -> use "neoPixel" as filter string
 *  Extension are made to include more patterns and combined patterns
 *  and patterns for nxn NeoPixel matrix (tested with 8x8 and 10x10).
 *
 *  This library includes a modified "Adafruit NeoPixel" library without private variables and a additional function
 *  to enable efficient Move()
 *
 *  Copyright (C) 2018  Armin Joachimsmeyer
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

#include <Arduino.h>

//#define TRACE
//#define DEBUG
#define INFO

EEMEM uint16_t sHighScoreEEPROM; // is reset if both right and left button are pressed at startup.

#include "MatrixNeoPatterns.h"

// used for Ticker - modify it to change font sizes
#include "fonts.h"

// Demo 8x8 graphics
const uint8_t heart8x8[] PROGMEM = { 0x66, 0xFF, 0xFF, 0xFF, 0x7E, 0x3C, 0x18, 0x00 };

// Constructor - calls base-class constructor to initialize strip
MatrixNeoPatterns::MatrixNeoPatterns(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry, uint8_t aTypeOfPixel, // @suppress("Class members should be properly initialized")
        void (*aPatternCompletionCallback)(NeoPatterns*)) :
        NeoPatterns(aColumns * aRows, aPin, aTypeOfPixel) {
    Rows = aRows;
    Columns = aColumns;
    Geometry = aMatrixGeometry;
    OnPatternComplete = aPatternCompletionCallback;
    LayoutMappingFunction = NULL;
}

void MatrixNeoPatterns::setLayoutMappingFunction(uint16_t (*aLayoutMappingFunction)(uint8_t, uint8_t, uint8_t, uint8_t)) {
    LayoutMappingFunction = aLayoutMappingFunction;
}

// Update the pattern returns true if update has happened in order to give the caller a chance to manually change parameters (like color etc.)
bool MatrixNeoPatterns::Update() {
    if ((millis() - lastUpdate) > Interval) {
        switch (ActivePattern) {
        case FIRE:
            FireMatrixUpdate();
            break;
        case TICKER:
            TickerUpdate();
            break;
        case MOVE:
            MoveUpdate();
            break;
        case MOVING_PICTURE:
            MovingPicturePGMUpdate();
            break;
        default:
            NeoPatterns::Update();
            break;
        }
        // time to update
        lastUpdate = millis();
        return true;
    }
    return false;
}

/*
 * If LayoutMappingFunction is not set, use ZTypeMapping.
 */
bool MatrixNeoPatterns::setMatrixPixelColor(uint8_t x, uint8_t y, color32_t a32BitColor) {
#ifdef DEBUG
    Serial.print(F("set x="));
    Serial.print(x);
    Serial.print(F(" y="));
    Serial.print(y);
    Serial.print(F(" n="));
    Serial.print(LayoutMapping(x, y));
    Serial.print(F(" color="));
    Serial.println(aColor, HEX);
#endif
    if (LayoutMappingFunction == NULL) {
        setPixelColor(LayoutMapping(x, y), a32BitColor);
    } else {
        setPixelColor(LayoutMappingFunction(x, y, Columns, Rows), a32BitColor);
    }
    return true;
}

/*
 * If LayoutMappingFunction is not set, use ZTypeMapping.
 */
uint32_t MatrixNeoPatterns::getMatrixPixelColor(uint8_t x, uint8_t y) {
    uint32_t tColor;
#ifdef DEBUG
    Serial.print(F("set x="));
    Serial.print(x);
    Serial.print(F(" y="));
    Serial.print(y);
    Serial.print(F(" n="));
    Serial.print(LayoutMapping(x, y));
    Serial.print(F(" color="));
    Serial.println(aColor, HEX);
#endif
    if (LayoutMappingFunction == NULL) {
        tColor = getPixelColor(LayoutMapping(x, y));
    } else {
        tColor = getPixelColor(LayoutMappingFunction(x, y, Columns, Rows));
    }
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

uint16_t MatrixNeoPatterns::LayoutMapping(uint8_t aColumnX, uint8_t aRowY) {
    uint16_t tRetvalue = 0;
    uint8_t tCompareValue = 0;
    uint8_t tRows;
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
            if (tRows % 2 != tCompareValue) { // equivalent to ((tRows - 1) % 2 == tCompareValue)
                tRetvalue = (Columns * tRows - aColumnX) - 1; // From Right to Left -1 since we start at 0
            } else {
                tRetvalue = (Columns * (tRows - 1)) + aColumnX;  // From Left to Right
            }
        }
    }
    return tRetvalue;
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
    return (aColumnsTotal * (aRowsTotal - aRowY) - aColumnX) - 1; // -1 since we start at 0
}

uint16_t ProgressiveTypeBottomLeftMapping(uint8_t aColumnX, uint8_t aRowY, uint8_t aColumnsTotal, uint8_t aRowsTotal) {
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
    if (aRowY % 2) {
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
    if (aRowY % 2) {
        // second, fourth, sixth . . . row from bottom
        return (aColumnsTotal * (aRowsTotal - aRowY) - aColumnX) - 1; // -1 since we start at 0
    } else {
        // first, third, fifth . . . row from bottom
        return (aColumnsTotal * ((aRowsTotal - aRowY) - 1)) + aColumnX;
    }
}

int convolutionMatrixIntegerTimes256[3][3];

// The bottom line of fire which is set by random values every 4 updates
uint8_t sInitialHeat[8];

void setInitHeat() {
    for (int i = 0; i < 8;) {
        uint8_t tVal = random(5, 250);
        sInitialHeat[i++] = tVal;
//        sInitialHeat[i++] = tVal;
    }
}

// initialize for fire -> set all to zero
void MatrixNeoPatterns::Fire(uint16_t aIntervalMillis, uint16_t aRepetitions) {
    ActivePattern = FIRE;
    Interval = aIntervalMillis;
    Direction = DIRECTION_UP;
    Index = aRepetitions;
// plus 1 pixel padding on each side
    if (MatrixNew) {
        free(MatrixNew);
    }
    if (MatrixOld) {
        free(MatrixOld);
    }
    MatrixNew = (uint8_t *) calloc((Rows + 2) * (Columns + 2), 1);
    MatrixOld = (uint8_t *) calloc((Rows + 2) * (Columns + 2), 1);

#ifdef INFO
    Serial.print(F("Starting Fire with refresh interval="));
    Serial.println(aIntervalMillis);
#endif
#ifdef DEBUG
    Serial.print("MatrixNew=0x");
    Serial.println((uint16_t) MatrixNew, HEX);
    Serial.print("MatrixOld=0x");
    Serial.println((uint16_t) MatrixOld, HEX);
#endif

    for (uint8_t cy = 0; cy < CONVOLUTION_MATRIX_SIZE; cy++) {
        for (uint8_t cx = 0; cx < CONVOLUTION_MATRIX_SIZE; cx++) {
            convolutionMatrixIntegerTimes256[cx][cy] = convolutionMatrix[cx][cy] * 256;
        }
    }
    setInitHeat();
}

void MatrixNeoPatterns::FireMatrixUpdate() {
    if (Index % 4 == 0) {
        setInitHeat();
    }
    for (int i = 1; i < Rows + 1; i++) {
        MatrixOld[mapXYToArray(i, Columns + 1, Rows + 2)] = sInitialHeat[i - 1];
    }

// Step 4.  Process and Map center without padding from heat cells to LED colors
// using pointer instead of indexing with x and y costs 1 ms!
    for (uint8_t y = 1; y < Columns + 1; y++) {
        for (uint8_t x = 1; x < Rows + 1; x++) {
            // Convolution
            long tConvolutionSumTimes256 = 0;
            // using pointers here saves 1 ms
            int * convolutionMatrixIntegerTimes256Ptr = &convolutionMatrixIntegerTimes256[0][0];
            uint8_t * tFireMatrixOldPtr = &MatrixOld[mapXYToArray((x - 1), (y - 1), Rows + 2)];
            for (uint8_t cy = 0; cy < CONVOLUTION_MATRIX_SIZE; cy++) {
                for (uint8_t cx = 0; cx < CONVOLUTION_MATRIX_SIZE; cx++) {
                    tConvolutionSumTimes256 += *convolutionMatrixIntegerTimes256Ptr++ * *tFireMatrixOldPtr++;
//                    tConvolutionSumTimes256 += convolutionMatrixIntegerTimes256[cy][cx]
//                            * FireGrid8x8Old[map10x10((x - 1) + cx, (y - 1) + cy)];
                }
                tFireMatrixOldPtr += (Rows + 2) - 3;
            }

            uint8_t tNewVal = MatrixOld[mapXYToArray(x, y, Rows + 2)] + (tConvolutionSumTimes256 / 256);
            // - COOLING and clipping to zero
            if (tNewVal > COOLING) {
                tNewVal -= COOLING;
            } else {
                tNewVal = 0;
            }
            MatrixNew[mapXYToArray(x, y, Rows + 2)] = tNewVal;

            // Heat color mapping
            setMatrixPixelColor(x - 1, y - 1, HeatColor(tNewVal));
#ifdef DEBUG
            Serial.print("x=");
            Serial.print(x);
            Serial.print(" y=");
            Serial.print(y);
            Serial.print(" map=");
            Serial.println(mapXYToArray(x, y, Rows + 2));
#endif

        }
    }
// takes 11 millis with float or 4 ms with integer
//    Serial.print("t=");
//    Serial.println(millis() - tMillis);
// toggle areas
    uint8_t * tPtr = MatrixNew;
    MatrixNew = MatrixOld;
    MatrixOld = tPtr;

    show();
    Index--;
    if (Index == 0) {

        free(MatrixOld);
        MatrixOld = NULL;
        free(MatrixNew);
        MatrixNew = NULL;

        if (OnPatternComplete != NULL) {
            OnPatternComplete(this); // call the completion callback
        }
    }
}

/*
 * direction FORWARD is from left to right
 * Currently only 8x8 Graphics are supported
 */
void MatrixNeoPatterns::MovingPicturePGM(const uint8_t* aGraphics8x8ArrayPGM, color32_t aForegroundColor,
        color32_t aBackgroundColor, int8_t aGraphicsXOffset, int8_t aGraphicsYOffset, uint16_t aSteps, uint16_t aIntervalMillis,
        uint8_t aDirection) {
    ActivePattern = MOVING_PICTURE;
    DataPtr = aGraphics8x8ArrayPGM;
    Color1 = aForegroundColor;
    Color2 = aBackgroundColor;
    GraphicsXOffset = checkAndTruncateParamValue(aGraphicsXOffset, Rows, -Rows);
    GraphicsYOffset = checkAndTruncateParamValue(aGraphicsYOffset, Columns, -Columns);
    Interval = aIntervalMillis;
    TotalSteps = aSteps;
    Index = 0;
    Direction = aDirection;
#ifdef INFO
    Serial.print(F("Starting MovingPicturePGM with refresh interval="));
    Serial.print(aIntervalMillis);
    Serial.print(F("ms. Direction="));
    Serial.println(aDirection);
#endif
}

void MatrixNeoPatterns::MovingPicturePGMUpdate() {
    loadPicturePGM(DataPtr, 8, 8, Color1, Color2, GraphicsXOffset, GraphicsYOffset, false, true);
    if (Direction == DIRECTION_UP) {
        GraphicsYOffset++;
    } else if (Direction == DIRECTION_DOWN) {
        GraphicsYOffset--;
    } else if (Direction == DIRECTION_LEFT) {
        GraphicsXOffset++;
    } else if (Direction == DIRECTION_RIGHT) {
        GraphicsXOffset--;
    }
    Index++;
    if (Index >= TotalSteps) {
        if (OnPatternComplete != NULL) {
            OnPatternComplete(this); // call the completion callback
        }
    }
}

void MatrixNeoPatterns::showNumberOnMatrix(uint8_t aNumber, color32_t aColor) {
    char tStringBuffer[4];
    utoa(aNumber, tStringBuffer, 10); // number is aligned left
    uint8_t tStringLength = strlen(tStringBuffer);
    int8_t tXOffset = Columns - NUMBERS_FONT_WIDTH;
    clear();
    for (int8_t i = tStringLength; i > 0; i--) {
        loadPicturePGM(&fontNumbers4x6[(tStringBuffer[i - 1] - '0') * NUMBERS_FONT_HEIGHT], NUMBERS_FONT_WIDTH,
        NUMBERS_FONT_HEIGHT, aColor, COLOR32_BLACK, tXOffset);
        tXOffset -= NUMBERS_FONT_WIDTH;
        if (tXOffset < -3) {
            break;
        }
    }
}

/*
 * 103(UP/DOWN) or 129 (LEFT/RIGHT) usec for moving 8x8 without show()
 * show takes ca. 1000 us
 * assumes Z type mapping
 */
void MatrixNeoPatterns::moveArrayContent(uint8_t aDirection) {
    if (LayoutMappingFunction != NULL) {
        Serial.print(F("moveArrayContent with one parameter does not support other than Z type mappings."));
    } else {
        uint8_t * tPixels = getPixels();

        uint16_t tNumBytes = PixelColorStorageSize * numPixels();
        uint8_t tBytesToSkipForOneRow = Columns * PixelColorStorageSize;

#ifdef TRACE
        Serial.print("moveArrayContent aDirection=");
        Serial.print(aDirection);
        Serial.print(" tNumPixels=");
        Serial.print(tNumPixels);
        Serial.print(" PixelColorStorageSize=");
        Serial.print(PixelColorStorageSize);
        Serial.print(" tBytesToSkipForOneRow=");
        Serial.println(tBytesToSkipForOneRow);
#endif

        if (aDirection == DIRECTION_UP) {
            memmove(tPixels + tBytesToSkipForOneRow, tPixels, tNumBytes - tBytesToSkipForOneRow);
            memset(tPixels, 0, tBytesToSkipForOneRow);
        } else if (aDirection == DIRECTION_DOWN) {
            memmove(tPixels, tPixels + tBytesToSkipForOneRow, tNumBytes - tBytesToSkipForOneRow);
            memset(tPixels + (tNumBytes - tBytesToSkipForOneRow), 0, tBytesToSkipForOneRow);
        } else if (aDirection == DIRECTION_LEFT) {
            memmove(tPixels + PixelColorStorageSize, tPixels, tNumBytes - PixelColorStorageSize);
            for (int i = 0; i < Rows; ++i) {
                memset(tPixels, 0, PixelColorStorageSize);
                tPixels += tBytesToSkipForOneRow;
            }
        } else if (aDirection == DIRECTION_RIGHT) {
            memmove(tPixels, tPixels + PixelColorStorageSize, tNumBytes - PixelColorStorageSize);
            tPixels = tPixels + tBytesToSkipForOneRow - PixelColorStorageSize;
            for (int i = 0; i < Rows; ++i) {
                memset(tPixels, 0, PixelColorStorageSize);
                tPixels += tBytesToSkipForOneRow;
            }
        }
        show();
    }
}

/*
 *  156/136(UP/DOWN) or 246 (LEFT/RIGHT)  usec for moving 8x8 without show()
 *  assumes LED Rows are in X direction
 */
void MatrixNeoPatterns::moveArrayContent(uint8_t aDirection, color32_t aBackgroundColor) {
    uint16_t tNumPixels = numPixels();
    if (Geometry == (NEO_MATRIX_BOTTOM | NEO_MATRIX_RIGHT | NEO_MATRIX_ROWS | NEO_MATRIX_PROGRESSIVE)) {
        uint8_t * tPixels = getPixels();

        uint16_t tNumBytes = tNumPixels * PixelColorStorageSize;
        uint8_t tBytesToSkipForOneRow = Columns * PixelColorStorageSize;

#ifdef TRACE
        Serial.print("moveArrayContent aDirection=");
        Serial.print(aDirection);
        Serial.print(" tNumPixels=");
        Serial.print(tNumPixels);
        Serial.print(" PixelColorStorageSize=");
        Serial.print(PixelColorStorageSize);
        Serial.print(" tBytesToSkipForOneRow=");
        Serial.println(tBytesToSkipForOneRow);
#endif
        if (aDirection == DIRECTION_UP) {
            memmove(tPixels + tBytesToSkipForOneRow, tPixels, tNumBytes - tBytesToSkipForOneRow);
            for (uint8_t i = 0; i < Rows; ++i) {
                setPixelColor(i, aBackgroundColor);
            }
        } else if (aDirection == DIRECTION_DOWN) {
            memmove(tPixels, tPixels + tBytesToSkipForOneRow, tNumBytes - tBytesToSkipForOneRow);
            for (uint16_t i = tNumPixels - Columns; i < tNumPixels; ++i) {
                setPixelColor(i, aBackgroundColor);
            }
        } else if (aDirection == DIRECTION_LEFT) {
            memmove(tPixels + PixelColorStorageSize, tPixels, tNumBytes - PixelColorStorageSize);
            for (uint16_t i = 0; i < tNumPixels; i += Columns) {
                setPixelColor(i, aBackgroundColor);
            }
        } else if (aDirection == DIRECTION_RIGHT) {
            memmove(tPixels, tPixels + PixelColorStorageSize, tNumBytes - PixelColorStorageSize);
            for (uint16_t i = Columns - 1; i <= tNumPixels; i += Columns) {
                setPixelColor(i, aBackgroundColor);
            }
        }
    } else {
        for (uint8_t y = 0; y < Rows; ++y) {
            for (uint8_t x = 0; x < Columns; ++x) {
                if (aDirection == DIRECTION_UP) {
                    if (y != (Rows - 1)) {
                        setMatrixPixelColor(x, y, getMatrixPixelColor(x, y + 1));
                    } else {
                        //padding lower line
                        setMatrixPixelColor(x, (Rows - 1), aBackgroundColor);
                    }
                } else if (aDirection == DIRECTION_DOWN) {
                    if (y != (Rows - 1)) {
                        setMatrixPixelColor(x, (Rows - 1) - y, getMatrixPixelColor(x, (Rows - 2) - y));
                    } else {
                        //padding upper line
                        setMatrixPixelColor(x, 0, aBackgroundColor);
                    }
                } else if (aDirection == DIRECTION_LEFT) {
                    if (x != (Columns - 1)) {
                        setMatrixPixelColor(x, y, getMatrixPixelColor(x + 1, y));
                    } else {
                        //padding right
                        setMatrixPixelColor(x, y, aBackgroundColor);
                    }
                } else if (aDirection == DIRECTION_RIGHT) {
                    if (x != (Columns - 1)) {
                        setMatrixPixelColor(((Columns - 1) - x), y, getMatrixPixelColor(((Columns - 1) - x) - 1, y));
                    } else {
                        //padding left
                        setMatrixPixelColor(x, y, aBackgroundColor);
                    }
                }
            }
        }
    }
    show();
}

/*
 * Moving for arrays with 0 at lower right and horizontal rows
 * aSteps == 1 is equivalent to just calling moveArrayContent()
 */
void MatrixNeoPatterns::Move(uint8_t aDirection, uint16_t aSteps, uint16_t aIntervalMillis, bool aMoveDirect,
        color32_t aBackgroundColor) {
    if (aMoveDirect) {
        moveArrayContent(aDirection, aBackgroundColor);
        aSteps--;
    }
    if (aSteps > 0) {
        Color2 = aBackgroundColor;
        Direction = aDirection;
        Interval = aIntervalMillis;
        Index = aSteps;
        ActivePattern = MOVE;
#ifdef INFO
        Serial.print(F("Starting Move with refresh interval="));
        Serial.print(aIntervalMillis);
        Serial.print(F("ms. Direction="));
        Serial.println(aDirection);
#endif
    }
}

void MatrixNeoPatterns::MoveUpdate() {
#ifdef DEBUG
    Serial.print("MoveUpdate TotalSteps=");
    Serial.println(TotalSteps);
#endif
    moveArrayContent(Direction, Color2);
    Index--;
    if (Index == 0) {
        if (OnPatternComplete != NULL) {
            OnPatternComplete(this); // call the completion callback
        }
    }
}

/*
 * only directions DIRECTION_LEFT and DIRECTION_UP are supported yet
 */
void MatrixNeoPatterns::Ticker(const char* aStringPtr, color32_t aForegroundColor, color32_t aBackgroundColor,
        uint16_t aIntervalMillis, uint8_t aDirection) {
    TickerInit(aStringPtr, aForegroundColor, aBackgroundColor, aIntervalMillis, aDirection);
}
void MatrixNeoPatterns::TickerPGM(const char* aStringPtrPGM, color32_t aForegroundColor, color32_t aBackgroundColor,
        uint16_t aIntervalMillis, uint8_t aDirection) {
    TickerInit(aStringPtrPGM, aForegroundColor, aBackgroundColor, aIntervalMillis, aDirection,
    FLAG_TICKER_DATA_IN_FLASH);
}

void MatrixNeoPatterns::TickerInit(const char* aStringPtr, color32_t aForegroundColor, color32_t aBackgroundColor,
        uint16_t aIntervalMillis, uint8_t aDirection, uint8_t aFlags) {
    ActivePattern = TICKER;
    Flags = aFlags;
    Interval = aIntervalMillis;

    DataPtr = (const uint8_t*) aStringPtr;
    Color1 = aForegroundColor;
    Color2 = aBackgroundColor;
    Direction = aDirection;

#ifdef INFO
    Serial.print(F("Starting Ticker with refresh interval="));
    Serial.print(aIntervalMillis);
    Serial.print(F("ms. Text=\""));
    if (Flags & FLAG_TICKER_DATA_IN_FLASH) {
        Serial.print(reinterpret_cast<const __FlashStringHelper *>(aStringPtr));
    } else {
        Serial.print(aStringPtr);
    }
    Serial.print(F("\" Direction="));
    Serial.println(aDirection);
#endif
// try to position fonts at middle
    GraphicsXOffset = (Rows - FONT_WIDTH) / 2; // positive
    GraphicsYOffset = (FONT_HEIGHT - Columns) / 2; // negative since character must be shifted down
// Set start position for first character to move in
    if (aDirection == DIRECTION_LEFT) {
        GraphicsXOffset = Columns;
    } else if (aDirection == DIRECTION_UP) {
        GraphicsYOffset = -Rows;
    } else {
        Serial.println(F("Direction="));
        Serial.print(aDirection);
        Serial.println(F(" not supported (yet)"));
        aDirection = DIRECTION_LEFT;
    }
}

void MatrixNeoPatterns::TickerUpdate() {
    char tChar;
    char tNextChar;
// left character
    if (Flags & FLAG_TICKER_DATA_IN_FLASH) {
        tChar = pgm_read_byte(DataPtr);
        tNextChar = pgm_read_byte(DataPtr + 1);
    } else {
        tChar = *DataPtr;
        tNextChar = *(DataPtr + 1);
    }

#ifdef DEBUG
    Serial.print(F("tChar="));
    Serial.print(tChar);
    Serial.print(F(" tNextChar="));
    Serial.println(tNextChar);
#endif

    const uint8_t* tGraphics8x8ArrayPtr = &font_PGM[(tChar - FONT_START) * FONT_HEIGHT];
    loadPicturePGM(tGraphics8x8ArrayPtr, FONT_WIDTH, FONT_HEIGHT, Color1, Color2, GraphicsXOffset, GraphicsYOffset,
            (tNextChar == '\0'));
    if (tNextChar != '\0') {
        // check if next character is moving in
        if (Direction == DIRECTION_LEFT && (GraphicsXOffset + FONT_WIDTH) < Columns) {
            char tNextNextChar;
            if (Flags & FLAG_TICKER_DATA_IN_FLASH) {
                tNextNextChar = pgm_read_byte(DataPtr + 2);
            } else {
                tNextNextChar = *(DataPtr + 2);
            }
            /*
             * Display next character at right
             */
            tGraphics8x8ArrayPtr = &font_PGM[(tNextChar - FONT_START) * FONT_HEIGHT];
            loadPicturePGM(tGraphics8x8ArrayPtr, FONT_WIDTH, FONT_HEIGHT, Color1, Color2, GraphicsXOffset + FONT_WIDTH,
                    GraphicsYOffset, (tNextNextChar == '\0'));
            // check if next/next character can be displayed (needed for small font width)
            // do not check for next/next/next char != '\0' here!
            if ((GraphicsXOffset + (2 * FONT_WIDTH)) < Columns) {
                if (tNextNextChar != '\0') {
                    tGraphics8x8ArrayPtr = &font_PGM[(tNextNextChar - FONT_START) * FONT_HEIGHT];
                    loadPicturePGM(tGraphics8x8ArrayPtr, FONT_WIDTH, FONT_HEIGHT, Color1, Color2,
                            GraphicsXOffset + (2 * FONT_WIDTH), GraphicsYOffset, false);
                }
            }
        }
        if (Direction == DIRECTION_UP && (GraphicsYOffset - FONT_HEIGHT) > -Columns) {
            tGraphics8x8ArrayPtr = &font_PGM[(tNextChar - FONT_START) * FONT_HEIGHT];
            loadPicturePGM(tGraphics8x8ArrayPtr, FONT_WIDTH, FONT_HEIGHT, Color1, Color2, GraphicsXOffset,
                    GraphicsYOffset - FONT_HEIGHT, false);
        }
    }
    /*
     * character is moved out of matrix, so switch to next one
     */
    if (GraphicsXOffset == -FONT_WIDTH || GraphicsYOffset == FONT_HEIGHT) {

        if (tNextChar == '\0') {
            if (OnPatternComplete != NULL) {
                OnPatternComplete(this); // call the completion callback
                return;
            }
        }
        // switch to next character
        DataPtr++;
        if (Direction == DIRECTION_LEFT) {
            GraphicsXOffset = 0;
        } else if (Direction == DIRECTION_UP) {
            GraphicsYOffset = 0;
        }
    }

// shift offsets
    if (Direction == DIRECTION_LEFT) {
        GraphicsXOffset--;
    } else if (Direction == DIRECTION_UP) {
        GraphicsYOffset++;
    }
}

/*
 * Graphic is stored in a byte array where array[0] is the upper line and the lowest bit is the rightmost pixel.
 * Smaller graphics can be displayed by using aNumberOfVerticalLinesToProcess < 8 which ignores left / highest bits of 8bit line
 *
 * aHeightOfGraphic how much bytes are a graphic
 * aYOffset positive -> picture is shift up and truncated and bottom is padded with aBackgroundColor
 *          negative picture is shift down and truncated and top lines are padded with aBackgroundColor
 * aXOffset positive -> picture is shift right and left lines are padded -> rightmost part of graphic is not displayed
 *          negative -> picture is shift left and right lines are padded
 * aNumberOfVerticalLinesToProcess lines from rightmost to process in pattern. Is needed for small font which have leftmost lines padded blank.
 * doPadding true -> fill bottom, top and right lines with background color
 */
void MatrixNeoPatterns::loadPicturePGM(const uint8_t* aGraphicsArrayPtr, int8_t aWidthOfGraphic, uint8_t aHeightOfGraphic,
        color32_t aForegroundColor, color32_t aBackgroundColor, int8_t aXOffset, int8_t aYOffset, bool doPaddingRight,
        bool doPadding) {

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
    checkAndTruncateParamValue(aXOffset, Rows, -Rows);
    checkAndTruncateParamValue(aYOffset, Columns, -Columns);

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
        uint8_t tLineBitPattern = pgm_read_byte(tGraphicsPointer);
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

    show();
}

/*
 * Sample callback handler for MatrixNeoPatterns
 */
void MatrixPatternsDemo(NeoPatterns * aLedsPtr) {
    MatrixNeoPatterns* tLedsPtr = (MatrixNeoPatterns*) aLedsPtr;
    static int8_t sState = 0;
    static uint8_t sHeartDirection = DIRECTION_DOWN;
    static int8_t sTickerDirection = DIRECTION_LEFT;

    Serial.print("sState=");
    Serial.println(sState);

    /*
     * implement a delay between each case
     */
    if (sState % 2 == 1) {
        aLedsPtr->Delay(100); // to separate each pattern - not really needed here
        sState++;
        return;
    }

    uint8_t tState = sState / 2;
    uint8_t tYOffset;
    uint8_t tSteps = HEART_HEIGHT + 1 + (tLedsPtr->Rows - HEART_HEIGHT) / 2;
    uint8_t tXOffset = (tLedsPtr->Columns - HEART_WIDTH) / 2;

    switch (tState) {
    case 0:
        tLedsPtr->TickerPGM(PSTR("I love Neopixel"), NeoPatterns::Wheel(0), COLOR32_BLACK, 80, sTickerDirection);
        sTickerDirection--;
        if (sTickerDirection < 0) {
            sTickerDirection = DIRECTION_LEFT;
        }
        break;
    case 1:
        tYOffset = HEART_HEIGHT;
        if (sHeartDirection == DIRECTION_UP) {
            tYOffset = -HEART_HEIGHT;
        }
        // move in
        tLedsPtr->MovingPicturePGM(heart8x8, COLOR32_RED_HALF, COLOR32_BLACK, tXOffset, tYOffset, tSteps, 100, sHeartDirection);
        break;

    case 2:
// Next 4 cases show 2 heart beats
        aLedsPtr->ColorForSelection = aLedsPtr->Color1;
        aLedsPtr->ProcessSelectiveColor(&DimColor, 6, 40);
        break;
    case 3:
        aLedsPtr->ProcessSelectiveColor(&LightenColor, 6, 40);
        break;
    case 4:
        aLedsPtr->ProcessSelectiveColor(&DimColor, 6, 40);
        break;
    case 5:
        aLedsPtr->ProcessSelectiveColor(&LightenColor, 6, 40);
        break;

    case 6:
        // move out
        tLedsPtr->Move(sHeartDirection, tLedsPtr->Rows, 100, true);
// change direction for next time
        if (sHeartDirection == DIRECTION_DOWN) {
            sHeartDirection = DIRECTION_UP;
        } else {
            sHeartDirection = DIRECTION_DOWN;
        }
        break;
    case 7:
        aLedsPtr->Delay(1500);
        break;
    case 8:
        tLedsPtr->Fire(100, 50);
        break;
    case 9:
        aLedsPtr->clear(); // Clear matrix
        aLedsPtr->show();
        aLedsPtr->Delay(2000);
        // do not forget sState = -1; in last sensible case
        sState = -1; // Start from beginning
        break;
    case 10:
        break;
    case 11:
// safety net
        sState = -1; // Start from beginning
        break;

// EXAMPLE ACTIONS for case
//        aLedsPtr->Delay(40000);
//        aLedsPtr->TickerPGM(PSTR("I love you"), COLOR32_RED, COLOR32_BLACK, 50, DIRECTION_LEFT);
//        aLedsPtr->MovingPicturePGM(heart8x8, COLOR32_RED, COLOR32_BLACK, 0, -8, 9, 100, DIRECTION_UP);
//        aLedsPtr->MovingPicturePGM(heart8x8, COLOR32_RED, COLOR32_BLACK, 0, 0, 9, 100, DIRECTION_UP);
//        aLedsPtr->MovingPicturePGM(heart8x8, COLOR32_RED, COLOR32_BLACK, 0, 8, 9, 100, DIRECTION_DOWN);
//        aLedsPtr->MovingPicturePGM(heart8x8, COLOR32_RED, COLOR32_BLACK, 0, 0, 9, 100, DIRECTION_DOWN);
//        aLedsPtr->MovingPicturePGM(heart8x8, COLOR32_RED, COLOR32_BLACK, -8, 0, 9, 100, DIRECTION_LEFT);
//        aLedsPtr->MovingPicturePGM(heart8x8, COLOR32_RED, COLOR32_BLACK, 0, 0, 9, 100, DIRECTION_LEFT);
//        aLedsPtr->MovingPicturePGM(heart8x8, COLOR32_RED, COLOR32_BLACK, 8, 0, 9, 100, DIRECTION_RIGHT);
//        aLedsPtr->MovingPicturePGM(heart8x8, COLOR32_RED, COLOR32_BLACK, 0, 0, 9, 100, DIRECTION_RIGHT);
//        aLedsPtr->FadeSelective(COLOR32_RED, COLOR32_GREEN, 20, 40);

    default:
        aLedsPtr->Delay(1);
//        Serial.print("case ");
//        Serial.print(tState);
//        Serial.println(" not implemented");
        break;
    }

    sState++;
}

#define TEST_DELAY_MILLIS 2000
void myMoveTest1(MatrixNeoPatterns* aLedsPtr) {
    aLedsPtr->moveArrayContent(DIRECTION_UP, COLOR32_RED_HALF);
    delay(TEST_DELAY_MILLIS);
    aLedsPtr->moveArrayContent(DIRECTION_DOWN, COLOR32_BLUE_HALF);
    delay(TEST_DELAY_MILLIS);
    aLedsPtr->moveArrayContent(DIRECTION_LEFT, COLOR32_GREEN_HALF);
    delay(TEST_DELAY_MILLIS);
    aLedsPtr->moveArrayContent(DIRECTION_RIGHT);
    delay(TEST_DELAY_MILLIS);
}

void myMoveTest2(MatrixNeoPatterns* aLedsPtr) {
    aLedsPtr->moveArrayContent(DIRECTION_DOWN, COLOR32_BLUE_HALF);
    delay(TEST_DELAY_MILLIS);
    aLedsPtr->moveArrayContent(DIRECTION_UP, COLOR32_RED_HALF);
    delay(TEST_DELAY_MILLIS);
    aLedsPtr->moveArrayContent(DIRECTION_RIGHT);
    delay(TEST_DELAY_MILLIS);
    aLedsPtr->moveArrayContent(DIRECTION_LEFT, COLOR32_GREEN_HALF);
    delay(TEST_DELAY_MILLIS);
}

void myLoadTest(MatrixNeoPatterns* aLedsPtr) {
    aLedsPtr->loadPicturePGM(heart8x8, 8, 8, COLOR32_RED_HALF, COLOR32_BLACK, 0, 0, false, true);
    delay(TEST_DELAY_MILLIS);
    myMoveTest1(aLedsPtr);
    aLedsPtr->loadPicturePGM(heart8x8, 8, 8, COLOR32_RED_HALF, COLOR32_BLACK, 2, 0, false, true);
    delay(TEST_DELAY_MILLIS);
    myMoveTest2(aLedsPtr);
    aLedsPtr->loadPicturePGM(heart8x8, 8, 8, COLOR32_RED_HALF, COLOR32_BLACK, 0, 2, false, true);
    delay(TEST_DELAY_MILLIS);
    aLedsPtr->loadPicturePGM(heart8x8, 8, 8, COLOR32_RED_HALF, COLOR32_BLACK, -2, -2, false, true);
    delay(TEST_DELAY_MILLIS);
}

const uint8_t fontNumbers4x6[] PROGMEM = { 0x03, 0x05, 0x05, 0x05, 0x06, 0x00, // 0x30 0
        0x02, 0x06, 0x02, 0x02, 0x07, 0x00, // 0x31 1
        0x06, 0x01, 0x02, 0x04, 0x07, 0x00, // 0x32 2
        0x06, 0x01, 0x02, 0x01, 0x06, 0x00, // 0x33
        0x01, 0x05, 0x07, 0x01, 0x01, 0x00, // 0x34
        0x07, 0x04, 0x06, 0x01, 0x06, 0x00, // 0x35
        0x02, 0x04, 0x06, 0x05, 0x02, 0x00, // 0x36
        0x07, 0x01, 0x03, 0x02, 0x02, 0x00, // 0x37
        0x02, 0x05, 0x02, 0x05, 0x02, 0x00, // 0x38 8
        0x02, 0x05, 0x03, 0x01, 0x02, 0x00 // 0x39 9
        };

