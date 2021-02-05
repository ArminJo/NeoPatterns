/*
 * MatrixNeoPatterns.h
 *
 *  SUMMARY
 *  You need to install "Adafruit NeoPixel" library under "Tools -> Manage Libraries..." or "Ctrl+Shift+I" -> use "neoPixel" as filter string
 *  Extension are made to include more patterns and combined patterns and patterns for 8x8 NeoPixel matrix.
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

/*
 * Class inheritance diagram. We have virtual inheritance, since MatrixNeoPatterns needs only one member set of NeoPixel
 *
 *                                     ,o--> MatrixNeoPixel (virtual) \
 * MatrixSnake --> MatrixNeoPatterns  <                                o--> NeoPixel --> Adafruit_NeoPixel
 *                                     `o--> NeoPatterns    (virtual) /
 */

#ifndef MATRIXNEOPATTERNS_H_
#define MATRIXNEOPATTERNS_H_

#include "MatrixNeoPixel.h"
#include "NeoPatterns.h"

#define FLAG_TICKER_DATA_IN_FLASH 0x01 // Flag if DataPtr points to RAM or FLASH. Only evaluated for AVR platform.

#define PATTERN_TICKER              (LAST_NEO_PATTERN + 1)
#define PATTERN_MOVE                (LAST_NEO_PATTERN + 2)
#define PATTERN_MOVING_PICTURE      (LAST_NEO_PATTERN + 3)
#define PATTERN_SNOW                (LAST_NEO_PATTERN + 4)
#define LAST_MATRIX_NEO_PATTERN     PATTERN_SNOW

//#define DO_NOT_USE_MATRIX_FIRE_PATTERN
//#define DO_NOT_USE_SNOW_PATTERN

extern const uint8_t fontNumbers4x6[] PROGMEM; // the font for showing numbers
#define NUMBERS_FONT_WIDTH 4
#define NUMBERS_FONT_HEIGHT 6

#define MATRIX_FIRE_COOLING_PER_8_ROWS  15  // for 8 rows 10 to 25 are sensible with optimum around 15
/*
 * The sum SHOULD be 0!!!
 * The Matrix which describes the contribution of each surrounding pixel to the next heat value.
 */
#define CONVOLUTION_MATRIX_SIZE 3
float const convolutionMatrix[CONVOLUTION_MATRIX_SIZE][CONVOLUTION_MATRIX_SIZE] =
        { { 0.02, 0.02, 0.02 } /*weights of values above*/, { 0.05, -1 /*own value*/, 0.05 }, { 0.05, 0.74, 0.05 }/*weights of values below*/};

#define SNOW_BOTTOM_LINE_DIM_PRESCALER  20
// Bit-fields save 2 bytes RAM per flake but costs 120 bytes FLASH
struct SnowFlakeInfoStruct {
#ifdef SNOW_SUPPORT_MORE_THAN_16_ROWS_AND_COLUMNS
    uint8_t Period;
    uint8_t Counter;
    uint8_t Row; // starting with 0 / top
    uint8_t Column; // starting with 0 / left
#else
    // fast flakes (period = 4) are in the foreground an therefore brighter;
    uint8_t Period :4;
    uint8_t Counter :4;
    uint8_t Row :4; // starting with 0 / top
    uint8_t Column :4; // starting with 0 / left
#endif
};

// extension of NeoPattern Class approximately 85 Byte / object
class MatrixNeoPatterns: public MatrixNeoPixel, public NeoPatterns {
public:
    MatrixNeoPatterns();
    void init();
    MatrixNeoPatterns(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry, uint8_t aTypeOfPixel,
            void (*aPatternCompletionCallback)(NeoPatterns*)=NULL);
    bool init(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry, uint8_t aTypeOfPixel,
            void (*aPatternCompletionCallback)(NeoPatterns*)=NULL);

    void setGeometry(uint8_t aRows, uint8_t aColoums);



    void TickerPGM(const char *aStringPtrPGM, color32_t aForegroundColor, color32_t aBackgroundColor, uint16_t aIntervalMillis,
            uint8_t aDirection = DIRECTION_LEFT);
    void Ticker(__FlashStringHelper *aStringPtrPGM, color32_t aForegroundColor, color32_t aBackgroundColor,
            uint16_t aIntervalMillis, uint8_t aDirection = DIRECTION_LEFT);
    void Ticker(const char *aStringPtr, color32_t aForegroundColor, color32_t aBackgroundColor, uint16_t aIntervalMillis,
            uint8_t aDirection = DIRECTION_LEFT);
    void TickerInit(const char *aStringPtr, color32_t aForegroundColor, color32_t aBackgroundColor, uint16_t aIntervalMillis,
            uint8_t aDirection = DIRECTION_LEFT, uint8_t aFlags = 0);

    void MovingPicturePGM(const uint8_t *aGraphics8x8Array, color32_t aForegroundColor, color32_t aBackgroundColor, int8_t aXOffset,
            int8_t aYOffset, uint16_t aSteps, uint16_t aIntervalMillis, uint8_t aDirection = DIRECTION_UP);
    //
    void Move(uint8_t aDirection, uint16_t aNumberOfSteps = 1, uint16_t aIntervalMillis = 70, color32_t aBackgroundColor =
    COLOR32_BLACK);
    bool MoveUpdate();
    void moveArrayContent(uint8_t aDirection);
    void moveArrayContent(uint8_t aDirection, color32_t aBackgroundColor);

    bool update();

#ifndef DO_NOT_USE_SNOW_PATTERN
    void Snow(uint16_t aNumberOfSteps = 500, uint16_t aIntervalMillis = 20);
    bool SnowUpdate();
    void setRandomFlakeParameters(uint8_t aSnowFlakeIndex);
    void drawSnowFlake(uint8_t aSnowFlakeIndex);
    struct SnowFlakeInfoStruct *SnowFlakesArray;
#endif

    bool MovingPicturePGMUpdate();
    bool TickerUpdate();

    void showNumberOnMatrix(uint8_t aNumber, color32_t aColor);

#ifndef DO_NOT_USE_MATRIX_FIRE_PATTERN
    void Fire(uint16_t aNumberOfSteps = 200, uint16_t aIntervalMillis = 30);
    bool FireMatrixUpdate();
    /*
     * Two arrays for double buffering. Used for fire pattern
     * They have 1 pixel padding on each side for computation of convolution
     * Plus 1 extra bottom row for initial heat values
     */
    uint8_t *MatrixNew;
    uint8_t *MatrixOld;
    uint8_t *initalHeatLine;
#endif

    // for movingPicture and Ticker patterns
    const uint8_t *DataPtr; // can hold pointer to PGM or data space string or to PGM space 8x8 graphic array.
    int8_t GraphicsYOffset; // Offset of lower edge of graphic
    int8_t GraphicsXOffset; // Offset of left edge of graphic

    void setInitHeat();
};

void MatrixPatternsDemo(NeoPatterns *aLedsPtr);

void myLoadTest(MatrixNeoPatterns *aLedsPtr);

#endif /* MATRIXNEOPATTERNS_H_ */

#pragma once

