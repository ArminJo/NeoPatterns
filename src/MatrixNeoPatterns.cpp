/*
 * MatrixNeoPatterns.cpp
 *
 * Implements functions especially for 8x8 NeoPixel matrix.
 *
 *  SUMMARY
 *  You need to install "Adafruit NeoPixel" library under "Tools -> Manage Libraries..." or "Ctrl+Shift+I" -> use "neoPixel" as filter string.
 *  Extension are made to include more patterns and combined patterns
 *  and patterns for nxn NeoPixel matrix (tested with 8x8 and 10x10).
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
//#define INFO
//#define WARN

#include "MatrixNeoPatterns.h"
#include "LongUnion.h"

// used for Ticker - modify line 12 of fonts.h to change font sizes
#include "fonts.h"

MatrixNeoPatterns::MatrixNeoPatterns() :  // @suppress("Class members should be properly initialized")
        NeoPixel(), MatrixNeoPixel(), NeoPatterns() {
    OnPatternComplete = NULL;
}

// Constructor - calls base-class constructor to initialize strip
MatrixNeoPatterns::MatrixNeoPatterns(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry, uint8_t aTypeOfPixel, // @suppress("Class members should be properly initialized")
        void (*aPatternCompletionCallback)(NeoPatterns*)) :
        NeoPixel((aColumns * aRows), aPin, aTypeOfPixel), MatrixNeoPixel(aColumns, aRows, aPin, aMatrixGeometry, aTypeOfPixel), NeoPatterns(
                (aColumns * aRows), aPin, aTypeOfPixel, NULL) {

    OnPatternComplete = aPatternCompletionCallback;
}

bool MatrixNeoPatterns::init(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry, uint8_t aTypeOfPixel,
        void (*aPatternCompletionCallback)(NeoPatterns*)) {
    MatrixNeoPixel::init(aColumns, aRows, aPin, aMatrixGeometry, aTypeOfPixel);
    bool tRetvalue = NeoPatterns::init((aColumns * aRows), aPin, aTypeOfPixel, NULL);

    OnPatternComplete = aPatternCompletionCallback;
    return tRetvalue;
}

// Update the pattern returns true if update has happened in order to give the caller a chance to manually change parameters (like color etc.)
bool MatrixNeoPatterns::update() {
    if ((millis() - lastUpdate) > Interval) {
        bool tPatternEnded = true;

        switch (ActivePattern) {
#ifndef DO_NOT_USE_MATRIX_FIRE_PATTERN
        case PATTERN_FIRE:
            tPatternEnded = FireMatrixUpdate();
            break;
#endif
        case PATTERN_TICKER:
            tPatternEnded = TickerUpdate();
            break;
        case PATTERN_MOVE:
            tPatternEnded = MoveUpdate();
            break;
        case PATTERN_MOVING_PICTURE:
            tPatternEnded = MovingPicturePGMUpdate();
            break;
#ifndef DO_NOT_USE_SNOW_PATTERN
        case PATTERN_SNOW:
            tPatternEnded = SnowUpdate();
            break;
#endif
        default:
            NeoPatterns::update();
            break;
        }
        if (!tPatternEnded) {
            show();
        }
        return true;
    }
    return false;
}

int16_t convolutionMatrixIntegerTimes256[3][3];
#define mapXYToArray(x, y, XColumns) (((y) * (XColumns)) + (x))

#ifndef DO_NOT_USE_MATRIX_FIRE_PATTERN
// The bottom line of fire which is set by random values every 4 updates
void MatrixNeoPatterns::setInitHeat() {
    LongUnion tRandomValue;
    uint_fast8_t tIndex = 0;
    while (true) {
        for (uint_fast8_t i = 0; i < 4; ++i) {
            /*
             * With one random call we get a uint32 -> 4 8 bit random values
             */
#if defined(ESP32)
//            tRandomValue.ULong = random(0xFFFFFFFFL); // this gives only constant 0xFFFFFFFF so use esp_random()
            tRandomValue.ULong = esp_random();
//            Serial.print(F("Random=0x"));
//            Serial.println(tRandomValue.ULong, HEX);
#elif defined(ESP8266)
            tRandomValue.ULong = rand();
#else
            tRandomValue.ULong = random();
#endif
            // Values from 40 to 255
            initalHeatLine[tIndex] = (tRandomValue.UBytes[i] - 40) + 40; // 40 is somewhat over represented
//            initalHeatLine[tIndex] = ((tRandomValue.UBytes[i] * (255 - 40)) >> 8) + 40; // the correct one - just in case
            tIndex++;
            if (tIndex >= Columns) {
#ifdef TRACE
                Serial.print(F("initalHeatLine="));
                for (uint8_t i = 0; i < Columns; ++i) {
                    Serial.print(initalHeatLine[i]);
                    Serial.print(' ');
                }
                Serial.println();
#endif
                return;
            }
        }
    }
}

/*
 * initialize for fire -> set all to zero
 */
void MatrixNeoPatterns::Fire(uint16_t aNumberOfSteps, uint16_t aIntervalMillis) {
    Interval = aIntervalMillis;
    Direction = DIRECTION_UP;
    TotalStepCounter = aNumberOfSteps + 1;  // + 1 step for the last pattern to show
    PatternLength = (MATRIX_FIRE_COOLING_PER_8_ROWS * 8) / Rows; // cooling affects the height of fire, the greater the value the smaller the fire
    Index = 3; // to call setInitHeat(); at startup

    // just to be sure
    if (MatrixNew) {
        free(MatrixNew);
    }
    if (MatrixOld) {
        free(MatrixOld);
    }
    if (initalHeatLine) {
        free(initalHeatLine);
    }

    // They have 1 pixel padding on each side for computation of convolution
    MatrixNew = (uint8_t*) calloc((Rows + 2) * (Columns + 2), 1);
    MatrixOld = (uint8_t*) calloc((Rows + 2) * (Columns + 2), 1);
    initalHeatLine = (uint8_t*) calloc(Columns + 2, 1);

#ifdef INFO
    printPin();
    Serial.print(F("Starting Fire with refresh interval="));
    Serial.println(aIntervalMillis);
#endif
#ifdef DEBUG
    Serial.print(F("MatrixNew=0x"));
    Serial.println((uintptr_t) MatrixNew, HEX);
    Serial.print(F("MatrixOld=0x"));
    Serial.println((uintptr_t) MatrixOld, HEX);
#endif

    for (uint_fast8_t cy = 0; cy < CONVOLUTION_MATRIX_SIZE; cy++) {
        for (uint_fast8_t cx = 0; cx < CONVOLUTION_MATRIX_SIZE; cx++) {
            convolutionMatrixIntegerTimes256[cx][cy] = convolutionMatrix[cx][cy] * 256;
        }
    }

    FireMatrixUpdate();
    showPatternInitially();
    // must be after showPatternInitially(), since it needs the old value do detect asynchronous calling
    ActivePattern = PATTERN_FIRE;
}

/*
 * Fire for a 8x8 matrix works on a 10x10 temporary matrix (with borders for the convolution on each side)
 * Every 4 updates a new bottom heat line is generated.
 * Then the new matrix is computed with convolution from the old one.
 */
bool MatrixNeoPatterns::FireMatrixUpdate() {
    if (TotalStepCounter == 1) {
        /*
         * End of fire pattern -> cleanup before calling callback
         */
        free(MatrixOld);
        MatrixOld = NULL;
        free(MatrixNew);
        MatrixNew = NULL;
        free(initalHeatLine);
        initalHeatLine = NULL;
    }
    if (decrementTotalStepCounterAndSetNextIndex()) {
        return true;
    }

    if (Index == 4) {
        setInitHeat();
        Index = 0;
    }

    // First refresh (invisible) bottom line on every update
    for (uint8_t i = 1; i < Columns + 1; i++) {
        MatrixOld[mapXYToArray(i, Rows + 1, Columns + 2)] = initalHeatLine[i - 1];
    }

    /*
     * Process and Map center without padding from heat cells to LED colors.
     * Using pointer instead of indexing with x and y saves 1 ms!
     */
    for (uint8_t y = 1; y < Rows + 1; y++) {
        for (uint8_t x = 1; x < Columns + 1; x++) {
            // Convolution takes 11 milli seconds with float or 4 ms with integer
            long tConvolutionSumTimes256 = 0;
            // using pointers here saves 1 ms
            int16_t *convolutionMatrixIntegerTimes256Ptr = &convolutionMatrixIntegerTimes256[0][0];
            uint8_t *tFireMatrixOldPtr = &MatrixOld[mapXYToArray((x - 1), (y - 1), Columns + 2)];
            for (uint_fast8_t cy = 0; cy < CONVOLUTION_MATRIX_SIZE; cy++) {
                for (uint_fast8_t cx = 0; cx < CONVOLUTION_MATRIX_SIZE; cx++) {
                    tConvolutionSumTimes256 += *convolutionMatrixIntegerTimes256Ptr++ * *tFireMatrixOldPtr++;
//                    tConvolutionSumTimes256 += convolutionMatrixIntegerTimes256[cy][cx]
//                            * FireGrid8x8Old[map10x10((x - 1) + cx, (y - 1) + cy)];
                }
                tFireMatrixOldPtr += (Columns + 2) - 3;
            }

            uint8_t tNewHeatValue = MatrixOld[mapXYToArray(x, y, Columns + 2)] + (tConvolutionSumTimes256 / 256);

            /*
             * PatternLength contains the cooling
             * COOLING and clipping to zero
             */
            if (tNewHeatValue > PatternLength) {
                tNewHeatValue -= PatternLength;
            } else {
                tNewHeatValue = 0;
            }
            MatrixNew[mapXYToArray(x, y, Columns + 2)] = tNewHeatValue;

            // Heat color mapping
            setMatrixPixelColor(x - 1, y - 1, HeatColor(tNewHeatValue));
#ifdef TRACE
            printPin();
            Serial.print(F("x="));
            Serial.print(x);
            Serial.print(F(" y="));
            Serial.print(y);
            Serial.print(F(" map="));
            Serial.println(mapXYToArray(x, y, Columns + 2));
#endif

        }
    }

// toggle areas
    uint8_t *tPtr = MatrixNew;
    MatrixNew = MatrixOld;
    MatrixOld = tPtr;
    return false;
}
#endif // DO_NOT_USE_MATRIX_FIRE_PATTERN

#ifndef DO_NOT_USE_SNOW_PATTERN
// bright version
//color32_t SnowFlakeBrightnessMap[4] = { COLOR32_WHITE, COLOR32_WHITE_QUARTER, COLOR32_WHITE_16TH, COLOR32_WHITE_64TH };
// dimmed version
const color32_t SnowFlakeBrightnessMap[4] = { COLOR32_WHITE_HALF, COLOR32_WHITE_EIGHTH, COLOR32_WHITE_32TH, COLOR32_WHITE_128TH };

void MatrixNeoPatterns::setRandomFlakeParameters(uint8_t aSnowFlakeIndex) {
    // random parameters for a snow flake
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
    // Hack to avoid using a union only for this purpose
#ifdef SNOW_SUPPORT_MORE_THAN_16_ROWS_AND_COLUMNS
    uint32_t *tSnowFakePtr = (uint16_t*) &SnowFlakesArray[aSnowFlakeIndex];
#else
    uint16_t *tSnowFakePtr = (uint16_t*) &SnowFlakesArray[aSnowFlakeIndex];
#endif
#if defined(ESP32)
//            tRandomValue.ULong = random(0xFFFFFFFFL); // this gives only constant 0xFFFFFFFF so use esp_random()
    *tSnowFakePtr = esp_random();
//            Serial.print(F("Random=0x"));
//            Serial.println(tRandomValue.ULong, HEX);
#elif defined(ESP8266)
    *tSnowFakePtr = rand();
#else
    *tSnowFakePtr = random();
#endif
    SnowFlakesArray[aSnowFlakeIndex].Period |= 0x08; // values from 8 to F -> factor 2
}

void MatrixNeoPatterns::drawSnowFlake(uint8_t aSnowFlakeIndex) {
    addMatrixPixelColor(SnowFlakesArray[aSnowFlakeIndex].Column, SnowFlakesArray[aSnowFlakeIndex].Row,
            SnowFlakeBrightnessMap[(SnowFlakesArray[aSnowFlakeIndex].Period >> 1) & 0x03]);
}

/*
 * Currently only a maximum of 16 rows and columns are supported
 */
void MatrixNeoPatterns::Snow(uint16_t aNumberOfSteps, uint16_t aIntervalMillis) {
    Interval = aIntervalMillis;
    TotalStepCounter = aNumberOfSteps + 1;  // + 1 step for the last pattern to show
    Direction = DIRECTION_UP; // for dim prescaler
    if (SnowFlakesArray) {
        free(SnowFlakesArray);
    }
    PatternLength = ((Rows * 2) / 5) * Columns;
    SnowFlakesArray = (struct SnowFlakeInfoStruct*) malloc(PatternLength * sizeof(struct SnowFlakeInfoStruct));
    for (int tSnowFlakeIndex = 0; tSnowFlakeIndex < PatternLength; ++tSnowFlakeIndex) {
        // random parameters for a snow flake
        setRandomFlakeParameters(tSnowFlakeIndex);
    }

#ifdef INFO
    printPin();
    Serial.print(F("Starting Snow with refresh interval="));
    Serial.println(aIntervalMillis);
#endif

    SnowUpdate();
    showPatternInitially();
    // must be after showPatternInitially(), since it needs the old value do detect asynchronous calling
    ActivePattern = PATTERN_SNOW;
}

bool MatrixNeoPatterns::SnowUpdate() {
    if (TotalStepCounter == 1) {
        /*
         * End of snow pattern -> cleanup before calling callback
         */
        free(SnowFlakesArray);
        SnowFlakesArray = NULL;
    }
    if (decrementTotalStepCounterAndSetNextIndex()) {
        return true;
    }

    /*
     *  clear all but the bottom row
     */
#ifndef SUPPORT_ONLY_DEFAULT_GEOMETRY
    // TODO 2 for loops instead of memset
    for (uint8_t x = 0; x < Columns; x++) {
        for (uint8_t y = 0; y < Rows - 1; y++) {
            setMatrixPixelColor(x, y, COLOR32_BLACK);
        }
    }
#else
    memset(pixels + (BytesPerPixel * Columns), 0, numBytes - (BytesPerPixel * Columns));
#endif

    /*
     * 1. Dim bottom row. reasonable prescaler is between 10 and 30
     */
    if (Index >= SNOW_BOTTOM_LINE_DIM_PRESCALER) {
        Index = 0;
        for (uint8_t i = 0; i < Columns; i++) {
#ifndef SUPPORT_ONLY_DEFAULT_GEOMETRY
            if (LayoutMappingFunction == NULL) {
                dimPixelColor(LayoutMapping(i, Rows - 1));
            } else {
                dimPixelColor(LayoutMappingFunction(i, Rows - 1, Columns, Rows));
            }
#else
        dimPixelColor(i);
#endif
        }
    }

    /*
     * 2. Do individual flake delay, move and draw all flakes
     */
    for (uint8_t tSnowFlakeIndex = 0; tSnowFlakeIndex < PatternLength; ++tSnowFlakeIndex) {
        uint8_t tCount = SnowFlakesArray[tSnowFlakeIndex].Counter;
        if (tCount == 0) {
            // Move flake
            tCount = SnowFlakesArray[tSnowFlakeIndex].Period;
            SnowFlakesArray[tSnowFlakeIndex].Row++; // move
        } else {
            // delay
            tCount--;
        }
        SnowFlakesArray[tSnowFlakeIndex].Counter = tCount;

        // set pixel
        drawSnowFlake(tSnowFlakeIndex);

#ifdef TRACE
        printPin();
        Serial.print(F("Index="));
        Serial.print(tSnowFlakeIndex);
        Serial.print(F(" Column="));
        Serial.print(SnowFlakesArray[tSnowFlakeIndex].Column);
        Serial.print(F(" Row="));
        Serial.print(SnowFlakesArray[tSnowFlakeIndex].Row);
        Serial.print(F(" Count="));
        Serial.print(tCount);
        Serial.print(F(" Period="));
        Serial.print(SnowFlakesArray[tSnowFlakeIndex].Period);
        Serial.print(F(" All=0x"));
        Serial.print(SnowFlakesArray[tSnowFlakeIndex].All, HEX);
        Serial.println();
#endif
    }
    /*
     * 3. Reinitialize all flakes, which were arrived at bottom row
     */
    for (uint8_t tSnowFlakeIndex = 0; tSnowFlakeIndex < PatternLength; ++tSnowFlakeIndex) {
        if (SnowFlakesArray[tSnowFlakeIndex].Row >= Rows - 1) {
            // Reinitialize
            setRandomFlakeParameters(tSnowFlakeIndex);
            // set row to 0
            SnowFlakesArray[tSnowFlakeIndex].Row = 0;
            uint8_t tTimeoutCounter = Columns;
            uint8_t tFlakeColumn = SnowFlakesArray[tSnowFlakeIndex].Column;
            /*
             * Try to place the new flake in a column where the three upper rows are empty.
             */
            while (getMatrixPixelColor(tFlakeColumn, 0) != COLOR32_BLACK || getMatrixPixelColor(tFlakeColumn, 1) != COLOR32_BLACK
                    || getMatrixPixelColor(tFlakeColumn, 2) != COLOR32_BLACK) {
                tFlakeColumn++;
                if (tFlakeColumn >= Columns) {
                    tFlakeColumn = 0;
                }
                tTimeoutCounter--;
                if (tTimeoutCounter == 0) {
                    break;
                }
            }
            SnowFlakesArray[tSnowFlakeIndex].Column = tFlakeColumn;

            // set pixel
            drawSnowFlake(tSnowFlakeIndex);

#ifdef TRACE
            Serial.print(F("Index="));
            Serial.print(tSnowFlakeIndex);
            Serial.print(F(" Column="));
            Serial.print(SnowFlakesArray[tSnowFlakeIndex].Column);
            Serial.print(F(" tTimeoutCounter="));
            Serial.print(tTimeoutCounter);
            Serial.println();
#endif
        }
    }

    return false;
}
#endif // DO_NOT_USE_SNOW_PATTERN

/*
 * Moving array content. Only moving out is performed e.g. for moving out the score-points of the snake game.
 * aSteps == 1 is equivalent to just calling moveArrayContent(aDirection, aBackgroundColor),
 */
void MatrixNeoPatterns::Move(uint8_t aDirection, uint16_t aNumberOfSteps, uint16_t aIntervalMillis, color32_t aBackgroundColor) {
    LongValue1.Color2 = aBackgroundColor;
    Direction = aDirection;
    Interval = aIntervalMillis;
    TotalStepCounter = aNumberOfSteps + 1; // +1 for the last step to show

#ifdef INFO
    printPin();
    Serial.print(F("Starting Move with refresh interval="));
    Serial.print(aIntervalMillis);
    Serial.print(F("ms. Direction="));
    Serial.println(aDirection);
#endif

    MoveUpdate();
    showPatternInitially();
    // must be after showPatternInitially(), since it needs the old value do detect asynchronous calling
    ActivePattern = PATTERN_MOVE;
}

bool MatrixNeoPatterns::MoveUpdate() {
#ifdef DEBUG
    printPin();
    Serial.print(F("MoveUpdate TotalSteps="));
    Serial.println(TotalStepCounter);
#endif
    if (decrementTotalStepCounter()) {
        return true;
    }
    moveArrayContent(Direction, LongValue1.Color2);
    return false;
}

/*
 * Display a 8x8 graphic with moving offsets.
 * Can be used for moving in and moving out in contrast to MOVE pattern, which is only capable of moving out.
 * direction FORWARD is from left to right
 * Currently only 8x8 graphics are supported
 */
void MatrixNeoPatterns::MovingPicturePGM(const uint8_t *aGraphics8x8ArrayPGM, color32_t aForegroundColor,
        color32_t aBackgroundColor, int8_t aGraphicsXOffset, int8_t aGraphicsYOffset, uint16_t aSteps, uint16_t aIntervalMillis,
        uint8_t aDirection) {
    DataPtr = aGraphics8x8ArrayPGM;
    Color1 = aForegroundColor;
    LongValue1.Color2 = aBackgroundColor;
    GraphicsXOffset = constrain(aGraphicsXOffset, -MAX_SUPPORTED_GRAPHICS_WIDTH, Columns);
    GraphicsYOffset = constrain(aGraphicsYOffset, -1, Rows + MAX_SUPPORTED_GRAPHICS_HEIGHT);
    Interval = aIntervalMillis;
    TotalStepCounter = aSteps + 1; // for the last pattern to show
    Direction = aDirection;

#ifdef INFO
    printPin();
    Serial.print(F("Starting MovingPicturePGM with refresh interval="));
    Serial.print(aIntervalMillis);
    Serial.print(F("ms. Direction="));
    Serial.println(aDirection);
#endif

    MovingPicturePGMUpdate();
    showPatternInitially();
    // must be after showPatternInitially(), since it needs the old value do detect asynchronous calling
    ActivePattern = PATTERN_MOVING_PICTURE;

}

bool MatrixNeoPatterns::MovingPicturePGMUpdate() {
    if (decrementTotalStepCounter()) {
        return true;
    }
    loadPicturePGM(DataPtr, 8, 8, Color1, LongValue1.Color2, GraphicsXOffset, GraphicsYOffset, true);
    if (Direction == DIRECTION_UP) {
        GraphicsYOffset--;
    } else if (Direction == DIRECTION_DOWN) {
        GraphicsYOffset++;
    } else if (Direction == DIRECTION_LEFT) {
        GraphicsXOffset++;
    } else if (Direction == DIRECTION_RIGHT) {
        GraphicsXOffset--;
    }
    return false;
}

void MatrixNeoPatterns::showNumberOnMatrix(uint8_t aNumber, color32_t aColor) {
    char tStringBuffer[4];
    utoa(aNumber, tStringBuffer, 10); // number is aligned left
    uint8_t tStringLength = strlen(tStringBuffer);
    int8_t tXOffset = Columns - NUMBERS_FONT_WIDTH;
    clear();
    for (int8_t i = tStringLength; i > 0; i--) {
        loadPicturePGM(&fontNumbers4x6[(tStringBuffer[i - 1] - '0') * NUMBERS_FONT_HEIGHT], NUMBERS_FONT_WIDTH,
        NUMBERS_FONT_HEIGHT, aColor, COLOR32_BLACK, tXOffset, NUMBERS_FONT_HEIGHT);
        tXOffset -= NUMBERS_FONT_WIDTH;
        if (tXOffset < -3) {
            break;
        }
    }
    show();
}

/*
 * 103(UP/DOWN) or 129 (LEFT/RIGHT) usec for moving 8x8
 * show takes ca. 1000 us
 * assumes Z type mapping
 */
void MatrixNeoPatterns::moveArrayContent(uint8_t aDirection) {
#ifndef SUPPORT_ONLY_DEFAULT_GEOMETRY
    if (LayoutMappingFunction != NULL) {
#  ifdef WARN
        Serial.print(F("moveArrayContent with one parameter does not support other than Z type mappings."));
#  endif
    } else
#  endif
    {
        uint8_t tBytesToSkipForOneRow = Columns * BytesPerPixel;

#  ifdef TRACE
        printPin();
        Serial.print(F("moveArrayContent Direction="));
        Serial.print(aDirection);
        Serial.print(F(" NumPixels="));
        Serial.print(numPixels());
        Serial.print(F(" BytesPerPixel="));
        Serial.print(BytesPerPixel);
        Serial.print(F(" BytesToSkipForOneRow="));
        Serial.println(tBytesToSkipForOneRow);
#  endif

        if (aDirection == DIRECTION_UP) {
            memmove(pixels + tBytesToSkipForOneRow, pixels, numBytes - tBytesToSkipForOneRow);
            memset(pixels, 0, tBytesToSkipForOneRow);
        } else if (aDirection == DIRECTION_DOWN) {
            memmove(pixels, pixels + tBytesToSkipForOneRow, numBytes - tBytesToSkipForOneRow);
            memset(pixels + (numBytes - tBytesToSkipForOneRow), 0, tBytesToSkipForOneRow);
        } else if (aDirection == DIRECTION_LEFT) {
            memmove(pixels + BytesPerPixel, pixels, numBytes - BytesPerPixel);
            // clear right column in each row
            for (uint_fast8_t i = 0; i < Rows; ++i) {
                memset(pixels, 0, BytesPerPixel);
                pixels += tBytesToSkipForOneRow;
            }
        } else if (aDirection == DIRECTION_RIGHT) {
            memmove(pixels, pixels + BytesPerPixel, numBytes - BytesPerPixel);
            pixels = pixels + tBytesToSkipForOneRow - BytesPerPixel;
            // clear left column in each row
            for (uint_fast8_t i = 0; i < Rows; ++i) {
                memset(pixels, 0, BytesPerPixel);
                pixels += tBytesToSkipForOneRow;
            }
        }
    }
}

/*
 *  156/136(UP/DOWN) or 246 (LEFT/RIGHT)  usec for moving 8x8
 */
void MatrixNeoPatterns::moveArrayContent(uint8_t aDirection, color32_t aBackgroundColor) {
#ifndef SUPPORT_ONLY_DEFAULT_GEOMETRY
    if (Geometry == NEO_MATRIX_DEFAULT_GEOMETRY) {
#endif
        /*
         * Use fast memmove() function here
         */
        uint16_t tNumBytes = numBytes;
        uint8_t tBytesToSkipForOneRow = Columns * BytesPerPixel;

#ifdef TRACE
        printPin();
        Serial.print(F("moveArrayContent Direction="));
        Serial.print(aDirection);
        Serial.print(F(" NumPixels="));
        Serial.print(numLEDs);
        Serial.print(F(" PixelColorStorageSize="));
        Serial.print(BytesPerPixel);
        Serial.print(F(" BytesToSkipForOneRow="));
        Serial.println(tBytesToSkipForOneRow);
#endif
        if (aDirection == DIRECTION_UP) {
            /*
             * Copy all LED data one row up, which is at a higher index
             */
            memmove(pixels + tBytesToSkipForOneRow, pixels, tNumBytes - tBytesToSkipForOneRow);
            for (uint8_t i = 0; i < Columns; ++i) {
                // set bottom line to background
                setPixelColor(i, aBackgroundColor);
            }
        } else if (aDirection == DIRECTION_DOWN) {
            memmove(pixels, pixels + tBytesToSkipForOneRow, tNumBytes - tBytesToSkipForOneRow);
            for (uint16_t i = numLEDs - Columns; i < numLEDs; ++i) {
                setPixelColor(i, aBackgroundColor);
            }
        } else if (aDirection == DIRECTION_LEFT) {
            memmove(pixels + BytesPerPixel, pixels, tNumBytes - BytesPerPixel);
            for (uint16_t i = 0; i < numLEDs; i += Columns) {
                setPixelColor(i, aBackgroundColor);
            }
        } else if (aDirection == DIRECTION_RIGHT) {
            memmove(pixels, pixels + BytesPerPixel, tNumBytes - BytesPerPixel);
            for (uint16_t i = Columns - 1; i <= numLEDs; i += Columns) {
                setPixelColor(i, aBackgroundColor);
            }
        }
#ifndef SUPPORT_ONLY_DEFAULT_GEOMETRY
    } else {
        /*
         * Use slower setMatrixPixelColor() function in all other cases
         */
        for (uint8_t y = 0; y < Rows; ++y) {
            for (uint8_t x = 0; x < Columns; ++x) {
                if (aDirection == DIRECTION_UP) {
                    if (y != (Rows - 1)) {
                        /*
                         * Copy data from row+1 to row
                         */
                        setMatrixPixelColor(x, y, getMatrixPixelColor(x, y + 1));
                    } else {
                        // set bottom line (y == (Rows - 1)) to background
                        setMatrixPixelColor(x, y, aBackgroundColor);
                    }
                } else if (aDirection == DIRECTION_DOWN) {
                    if (y != (Rows - 1)) {
                        /*
                         * Copy data from row to row+1 and start at Rows-1 in order to avoid overwriting data
                         */
                        setMatrixPixelColor(x, (Rows - 1) - y, getMatrixPixelColor(x, (Rows - 2) - y));
                    } else {
                        // set top line to background
                        setMatrixPixelColor(x, 0, aBackgroundColor);
                    }
                } else if (aDirection == DIRECTION_LEFT) {
                    if (x != (Columns - 1)) {
                        setMatrixPixelColor(x, y, getMatrixPixelColor(x + 1, y));
                    } else {
                        // set right column to background
                        setMatrixPixelColor(x, y, aBackgroundColor);
                    }
                } else if (aDirection == DIRECTION_RIGHT) {
                    if (x != (Columns - 1)) {
                        setMatrixPixelColor(((Columns - 1) - x), y, getMatrixPixelColor(((Columns - 1) - x) - 1, y));
                    } else {
                        // set left column to background
                        setMatrixPixelColor(x, y, aBackgroundColor);
                    }
                }
            }
        }
    }
#endif
}

/*
 * only directions DIRECTION_LEFT,DIRECTION_NONE and DIRECTION_UP are supported yet
 */
void MatrixNeoPatterns::Ticker(const char *aStringPtr, color32_t aForegroundColor, color32_t aBackgroundColor,
        uint16_t aIntervalMillis, uint8_t aDirection) {
    TickerInit(aStringPtr, aForegroundColor, aBackgroundColor, aIntervalMillis, aDirection);
}

void MatrixNeoPatterns::TickerPGM(const char *aStringPtrPGM, color32_t aForegroundColor, color32_t aBackgroundColor,
        uint16_t aIntervalMillis, uint8_t aDirection) {
    TickerInit(aStringPtrPGM, aForegroundColor, aBackgroundColor, aIntervalMillis, aDirection, FLAG_TICKER_DATA_IN_FLASH);
}

void MatrixNeoPatterns::Ticker(__FlashStringHelper *aStringPtrPGM, color32_t aForegroundColor, color32_t aBackgroundColor,
        uint16_t aIntervalMillis, uint8_t aDirection) {
    TickerInit(reinterpret_cast<const char*>(aStringPtrPGM), aForegroundColor, aBackgroundColor, aIntervalMillis, aDirection,
    FLAG_TICKER_DATA_IN_FLASH);
}

/*
 * Position the first character sensible
 */
void MatrixNeoPatterns::TickerInit(const char *aStringPtr, color32_t aForegroundColor, color32_t aBackgroundColor,
        uint16_t aIntervalMillis, uint8_t aDirection, uint8_t aFlags) {
    ActivePattern = PATTERN_TICKER;
    PatternFlags = aFlags;
    Interval = aIntervalMillis;

    DataPtr = (const uint8_t*) aStringPtr;
    Color1 = aForegroundColor;
    LongValue1.Color2 = aBackgroundColor;
    Direction = aDirection;

#ifdef INFO
    printPin();
    Serial.print(F("Starting Ticker with refresh interval="));
    Serial.print(aIntervalMillis);
    Serial.print(F("ms. Text=\""));
    if (PatternFlags & FLAG_TICKER_DATA_IN_FLASH) {
        Serial.print(reinterpret_cast<const __FlashStringHelper*>(aStringPtr));
    } else {
        Serial.print(aStringPtr);
    }
    Serial.print(F("\" Direction="));
    Serial.println(aDirection);
#endif
    /*
     * Try to position fonts or string at middle
     * Set start position for first character (to move in)
     */
    if (aDirection == DIRECTION_LEFT) {
        GraphicsXOffset = Columns;
        GraphicsYOffset = (Rows - 1) - ((Rows - FONT_HEIGHT) / 2);
    } else if (aDirection == DIRECTION_UP) {
        GraphicsXOffset = (Columns - FONT_WIDTH) / 2; // positive
        GraphicsYOffset = Rows + FONT_HEIGHT;
    } else if (aDirection == DIRECTION_NONE) {
        int tRemainingColumns;
#if defined(__AVR__)
        if (PatternFlags & FLAG_TICKER_DATA_IN_FLASH) {
            tRemainingColumns = Columns - (strlen_P(reinterpret_cast<const char*>(DataPtr)) * FONT_WIDTH);
        } else {
            tRemainingColumns = Columns - (strlen(reinterpret_cast<const char*>(DataPtr)) * FONT_WIDTH);
        }
#else
        tRemainingColumns = Columns - (strlen(reinterpret_cast<const char*>(DataPtr)) * FONT_WIDTH);
#endif
        if (tRemainingColumns > 0) {
            GraphicsXOffset = tRemainingColumns / 2;
        } else {
            GraphicsXOffset = 0; // Not enough space for complete string so start left
        }
        GraphicsYOffset = (Rows - 1) - ((Rows - FONT_HEIGHT) / 2);
    } else {
#ifdef WARN
        Serial.println(F("Direction="));
        Serial.print(aDirection);
        Serial.println(F(" not supported (yet)"));
#endif
        aDirection = DIRECTION_LEFT;
    }
}

/*
 * Draw visible character(s) and handle shift out of first visible character, shift in of last visible character
 * GraphicsXOffset is x offset for left side of first char to display. If DIRECTION_LEFT it goes from 0 to -(FONT_WIDTH-1) and back to 0 for next first char
 */
bool MatrixNeoPatterns::TickerUpdate() {
    char tCurrentChar;
    char tNextChar;
    lastUpdate = millis();

    const uint8_t *tDataPtr = DataPtr; // points to current character processed

// get first 2 characters
#if defined(__AVR__)
    if (PatternFlags & FLAG_TICKER_DATA_IN_FLASH) {
        tCurrentChar = pgm_read_byte(tDataPtr++);
        tNextChar = pgm_read_byte(tDataPtr++);
    } else {
        tCurrentChar = *tDataPtr++;
        tNextChar = *(tDataPtr++);
    }
#else
    tCurrentChar = *tDataPtr++;
    tNextChar = *(tDataPtr++);
#endif

#if defined(DEBUG)
    char tFirstCurrentChar = tCurrentChar;
    char tFirstNextChar = tNextChar;
#endif

    // Required for testing of end of string
    bool isLastChar = (tNextChar == '\0');

#ifdef TRACE
    printPin();
#endif
    if (Direction == DIRECTION_LEFT || Direction == DIRECTION_NONE) {
        int8_t tGraphicsXOffset = GraphicsXOffset; // X offset of current char to be processed

        /*
         * Check if current character is visible
         */
        while (tGraphicsXOffset < Columns && tCurrentChar != '\0') {
#ifdef TRACE
            Serial.print(F("GraphicsXOffset="));
            Serial.print(tGraphicsXOffset);
            Serial.print(F(" CurrentChar="));
            Serial.print(tCurrentChar);
            Serial.print(F(" NextChar="));
            Serial.println(tNextChar);
#endif
            /*
             * Print character (using information about next char)
             */
            const uint8_t *tGraphics8x8ArrayPtr = &font_PGM[(tCurrentChar - FONT_START) * FONT_HEIGHT];
            loadPicturePGM(tGraphics8x8ArrayPtr, FONT_WIDTH, FONT_HEIGHT, Color1, LongValue1.Color2, tGraphicsXOffset,
                    GraphicsYOffset, (tNextChar == '\0'));

            tGraphicsXOffset += FONT_WIDTH; // update X offset for next character

            /*
             * Get next character
             */
            tCurrentChar = tNextChar;
#if defined(__AVR__)
            if (PatternFlags & FLAG_TICKER_DATA_IN_FLASH) {
                tNextChar = pgm_read_byte(tDataPtr++);
            } else {
                tNextChar = *(tDataPtr++);
            }
#else
            tNextChar = *(tDataPtr++);
#endif
        }
    } else if (Direction == DIRECTION_UP) {
        int8_t tGraphicsYOffset = GraphicsYOffset; // Y offset of current char to be processed

        /*
         * Check if current character is visible
         */
        while (tGraphicsYOffset < (Rows + FONT_HEIGHT - 1) && tCurrentChar != '\0') {
#ifdef TRACE
            Serial.print(F("GraphicsYOffset="));
            Serial.print(tGraphicsYOffset);
            Serial.print(F(" CurrentChar="));
            Serial.print(tCurrentChar);
            Serial.print(F(" NextChar="));
            Serial.println(tNextChar);
#endif
            /*
             * Print character (using information about next char)
             */
            const uint8_t *tGraphics8x8ArrayPtr = &font_PGM[(tCurrentChar - FONT_START) * FONT_HEIGHT];
            loadPicturePGM(tGraphics8x8ArrayPtr, FONT_WIDTH, FONT_HEIGHT, Color1, LongValue1.Color2, GraphicsXOffset,
                    tGraphicsYOffset, (tNextChar == '\0'));

            tGraphicsYOffset += FONT_HEIGHT; // update Y offset for next character

            /*
             * Get next character
             */
            tCurrentChar = tNextChar;
#if defined(__AVR__)
            if (PatternFlags & FLAG_TICKER_DATA_IN_FLASH) {
                tNextChar = pgm_read_byte(tDataPtr++);
            } else {
                tNextChar = *(tDataPtr++);
            }
#else
            tNextChar = *(tDataPtr++);
#endif
            }
    }

    if (Direction != DIRECTION_NONE) {
        /*
         * Check if first character is moved out of matrix, then switch to next one
         */
        if (GraphicsXOffset == -FONT_WIDTH || GraphicsYOffset < 0) {

            if (isLastChar) {
                show(); // show last vanished character and its padding
                if (OnPatternComplete != NULL) {
                    OnPatternComplete(this); // call the completion callback
                } else {
                    ActivePattern = PATTERN_NONE; // reset ActivePattern to enable polling for end of pattern.
                }
                return true;
            }
            // switch to next character
#if defined(DEBUG) && !defined(TRACE)
            printPin();
            Serial.print(F("Char "));
            Serial.print(tFirstCurrentChar);
            Serial.print(F(" -> "));
            Serial.println(tFirstNextChar);
#endif
            DataPtr++;
            if (Direction == DIRECTION_LEFT) {
                GraphicsXOffset = 0;
            } else if (Direction == DIRECTION_UP) {
                GraphicsYOffset = FONT_HEIGHT - 1;
            }
        }

// shift offsets
        if (Direction == DIRECTION_LEFT) {
            GraphicsXOffset--;  // from 0 to -(FONT_WIDTH-1) and back to 0
        } else if (Direction == DIRECTION_UP) {
            GraphicsYOffset--; // from (FONT_HEIGHT-1) to 0 and back to (FONT_HEIGHT-1)
        }
    }
    return false;
}

/*
 * Sample callback handler for MatrixNeoPatterns
 */
void MatrixPatternsDemo(NeoPatterns *aLedsPtr) {
    MatrixNeoPatterns *tLedsPtr = (MatrixNeoPatterns*) aLedsPtr;
    static int8_t sState = 0;
    static uint8_t sHeartDirection = DIRECTION_DOWN;
    static int8_t sTickerDirection = DIRECTION_LEFT;

#ifdef INFO
    Serial.print(aLedsPtr->getPin());
    Serial.print(F(" State="));
    Serial.println(sState);
#endif
    /*
     * implement a delay between each case
     */
    if ((sState & 1) == 1) {
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
        tLedsPtr->TickerInit(PSTR("I love Neopixel"), NeoPatterns::Wheel(0), COLOR32_BLACK, 80, sTickerDirection,
        FLAG_TICKER_DATA_IN_FLASH);
        sTickerDirection--;
        if (sTickerDirection < 0) {
            sTickerDirection = DIRECTION_LEFT;
        }
        break;
    case 1:
        tYOffset = -1; // direction DOWN
        if (sHeartDirection == DIRECTION_UP) {
            tYOffset = tLedsPtr->Rows + HEART_HEIGHT;
        }
        // move in
        tLedsPtr->MovingPicturePGM(heart8x8, COLOR32_RED_HALF, COLOR32_BLACK, tXOffset, tYOffset, tSteps, 100, sHeartDirection);
        break;

    case 2:
// Next 4 cases show 2 heart beats
        aLedsPtr->ProcessSelectiveColor(aLedsPtr->Color1, &DimColor, 6, 40);
        break;
    case 3:
        aLedsPtr->ProcessSelectiveColor(aLedsPtr->Color1, &BrightenColor, 6, 40);
        break;
    case 4:
        aLedsPtr->ProcessSelectiveColor(aLedsPtr->Color1, &DimColor, 6, 40);
        break;
    case 5:
        aLedsPtr->ProcessSelectiveColor(aLedsPtr->Color1, &BrightenColor, 6, 40);
        break;

    case 6:
        // move out
        tLedsPtr->Move(sHeartDirection, tLedsPtr->Rows, 100);
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
        tLedsPtr->Fire(150, 30);
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
#ifdef WARN
        Serial.print(F("case "));
        Serial.print(tState);
        Serial.println(F(" not implemented"));
#endif
        break;
    }

    sState++;
}

#define TEST_DELAY_MILLIS 2000
void myMoveTest1(MatrixNeoPatterns *aLedsPtr) {
    aLedsPtr->moveArrayContent(DIRECTION_UP, COLOR32_RED_HALF);
    delay(TEST_DELAY_MILLIS);
    aLedsPtr->moveArrayContent(DIRECTION_DOWN, COLOR32_BLUE_HALF);
    delay(TEST_DELAY_MILLIS);
    aLedsPtr->moveArrayContent(DIRECTION_LEFT, COLOR32_GREEN_HALF);
    delay(TEST_DELAY_MILLIS);
    aLedsPtr->moveArrayContent(DIRECTION_RIGHT);
    delay(TEST_DELAY_MILLIS);
}

void myMoveTest2(MatrixNeoPatterns *aLedsPtr) {
    aLedsPtr->moveArrayContent(DIRECTION_DOWN, COLOR32_BLUE_HALF);
    delay(TEST_DELAY_MILLIS);
    aLedsPtr->moveArrayContent(DIRECTION_UP, COLOR32_RED_HALF);
    delay(TEST_DELAY_MILLIS);
    aLedsPtr->moveArrayContent(DIRECTION_RIGHT);
    delay(TEST_DELAY_MILLIS);
    aLedsPtr->moveArrayContent(DIRECTION_LEFT, COLOR32_GREEN_HALF);
    delay(TEST_DELAY_MILLIS);
}

void myLoadTest(MatrixNeoPatterns *aLedsPtr) {
    aLedsPtr->loadPicturePGM(heart8x8, 8, 8, COLOR32_RED_HALF, COLOR32_BLACK, 0, 0, true);
    delay(TEST_DELAY_MILLIS);
    myMoveTest1(aLedsPtr);
    aLedsPtr->loadPicturePGM(heart8x8, 8, 8, COLOR32_RED_HALF, COLOR32_BLACK, 2, 0, true);
    delay(TEST_DELAY_MILLIS);
    myMoveTest2(aLedsPtr);
    aLedsPtr->loadPicturePGM(heart8x8, 8, 8, COLOR32_RED_HALF, COLOR32_BLACK, 0, 2, true);
    delay(TEST_DELAY_MILLIS);
    aLedsPtr->loadPicturePGM(heart8x8, 8, 8, COLOR32_RED_HALF, COLOR32_BLACK, -2, -2, true);
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

