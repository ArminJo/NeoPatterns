/*
 * MatrixNeoPatterns.h
 *
 *  SUMMARY
 *  You need to install "Adafruit NeoPixel" library under "Tools -> Manage Libraries..." or "Ctrl+Shift+I" -> use "neoPixel" as filter string
 *  Extension are made to include more patterns and combined patterns and patterns for 8x8 NeoPixel matrix.
 *
 *  Copyright (C) 2018-2022  Armin Joachimsmeyer
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
 *  along with this program. If not, see <http://www.gnu.org/licenses/gpl.html>.
 *
 */

/*
 * Class inheritance diagram. We have virtual inheritance, since MatrixNeoPatterns needs only one member set of NeoPixel
 *
 *                                     ,o--> MatrixNeoPixel (virtual) \
 * MatrixSnake --> MatrixNeoPatterns  <                                o--> NeoPixel --> Adafruit_NeoPixel
 *                                     `o--> NeoPatterns    (virtual) /
 */

#ifndef _MATRIX_NEOPATTERNS_H
#define _MATRIX_NEOPATTERNS_H

#include "MatrixNeoPixel.h"

#if (!(defined(ENABLE_MATRIX_PATTERN_TICKER) || defined(ENABLE_MATRIX_PATTERN_MOVE) || defined(ENABLE_MATRIX_PATTERN_MOVING_PICTURE) \
|| defined(ENABLE_MATRIX_PATTERN_FIRE) || defined(ENABLE_MATRIX_PATTERN_SNOW) \
|| defined(ENABLE_NO_MATRIX_AND_NEO_PATTERN_BY_DEFAULT) ))
#define ENABLE_MATRIX_PATTERN_TICKER
#define ENABLE_MATRIX_PATTERN_MOVE
#define ENABLE_MATRIX_PATTERN_MOVING_PICTURE
#define ENABLE_MATRIX_PATTERN_FIRE
#define ENABLE_MATRIX_PATTERN_SNOW
#else
#define ENABLE_NO_NEO_PATTERN_BY_DEFAULT // must be before #include "NeoPatterns.h"
#endif

#include "NeoPatterns.h"

#define MATRIX_PATTERN_TICKER           (LAST_NEO_PATTERN + 1)
#define MATRIX_PATTERN_MOVE             (LAST_NEO_PATTERN + 2)
#define MATRIX_PATTERN_MOVING_PICTURE   (LAST_NEO_PATTERN + 3)
#define MATRIX_PATTERN_FIRE             (LAST_NEO_PATTERN + 4)
#define MATRIX_PATTERN_SNOW             (LAST_NEO_PATTERN + 5)
#define LAST_MATRIX_NEO_PATTERN         MATRIX_PATTERN_SNOW

#define FLAG_TICKER_DATA_IN_FLASH 0x01 // Flag if DataPtr points to RAM or FLASH. Only evaluated for AVR platform.

extern const uint8_t fontNumbers4x6[] PROGMEM; // the font for showing numbers
#define NUMBERS_FONT_WIDTH 4
#define NUMBERS_FONT_HEIGHT 6

#define MATRIX_FIRE_COOLING_PER_8_ROWS  15  // for 8 rows 10 to 25 are sensible with optimum around 15
/*
 * The sum SHOULD be 0!!!
 * The Matrix which describes the contribution of each surrounding pixel to the next heat value.
 */
#define CONVOLUTION_MATRIX_SIZE 3
//float const convolutionMatrix[CONVOLUTION_MATRIX_SIZE][CONVOLUTION_MATRIX_SIZE] =
//        { { 0.05, 0.74, 0.05 }/*weights of values below*/, { 0.05, -1 /*own value*/, 0.05 }, { 0.02, 0.02, 0.02 } /*weights of values above*/};
//int16_t const convolutionMatrixIntegerTimes256[CONVOLUTION_MATRIX_SIZE][CONVOLUTION_MATRIX_SIZE] =
//{ { 13, 189, 13 }/*weights of values below*/, { 13, -256 /*own value*/, 13 }, { 5, 5, 5 } /*weights of values above*/};
//int16_t const convolutionMatrixIntegerTimes256[CONVOLUTION_MATRIX_SIZE][CONVOLUTION_MATRIX_SIZE] =
//{  { 13, 90, 13 }/*weights of values below*/, { 6, -128 /*own value*/, 6 },{ 0, 0, 0 } /*weights of values above*/};
int16_t const convolutionMatrixIntegerTimes256[CONVOLUTION_MATRIX_SIZE][CONVOLUTION_MATRIX_SIZE] = {
        { 13, 102, 13 } /*weights of values below*/, { 0, -128 /*own value*/, 0 }, { 0, 0, 0 } /*weights of values above*/};

#define SNOW_BOTTOM_LINE_DIM_PRESCALER  20
// Bit-fields save 2 bytes RAM per flake but costs 120 bytes program memory
struct SnowFlakeInfoStruct {
#if defined(SNOW_SUPPORT_MORE_THAN_16_ROWS_AND_COLUMNS)
    uint8_t Period;
    uint8_t Counter;
    uint8_t Row; // starting with 0 / top
    uint8_t Column; // starting with 0 / left
#else
    uint8_t Period :4; // values from 8 to F. Fast flakes (period = 8 or 9) are in the foreground an therefore brighter;
    uint8_t Counter :4;
    uint8_t Row :4; // starting with 0 / top
    uint8_t Column :4; // starting with 0 / left
#endif
};

// extension of NeoPattern Class approximately 85 byte / object
class MatrixNeoPatterns: public MatrixNeoPixel, public NeoPatterns {
public:
    MatrixNeoPatterns();
    void init();
    MatrixNeoPatterns(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry, neoPixelType aTypeOfPixel,
            void (*aPatternCompletionCallback)(NeoPatterns*)=NULL);
    bool init(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry, neoPixelType aTypeOfPixel,
            void (*aPatternCompletionCallback)(NeoPatterns*)=NULL);

    void setGeometry(uint8_t aRows, uint8_t aColoums);

#if defined(ENABLE_MATRIX_PATTERN_TICKER)
    void TickerPGM(const char *aStringPtrPGM, color32_t aForegroundColor, color32_t aBackgroundColor, uint16_t aIntervalMillis,
            uint8_t aDirection = DIRECTION_LEFT);
    void Ticker(__FlashStringHelper *aStringPtrPGM, color32_t aForegroundColor, color32_t aBackgroundColor,
            uint16_t aIntervalMillis, uint8_t aDirection = DIRECTION_LEFT);
    void Ticker(const char *aStringPtr, color32_t aForegroundColor, color32_t aBackgroundColor, uint16_t aIntervalMillis,
            uint8_t aDirection = DIRECTION_LEFT);
    void TickerInit(const char *aStringPtr, color32_t aForegroundColor, color32_t aBackgroundColor, uint16_t aIntervalMillis,
            uint8_t aDirection = DIRECTION_LEFT, uint8_t aFlags = 0);
    bool TickerUpdate();
#endif

#if defined(ENABLE_MATRIX_PATTERN_MOVING_PICTURE)
    void MovingPicturePGM(const uint8_t *aGraphics8x8Array, color32_t aForegroundColor, color32_t aBackgroundColor, int8_t aXOffset,
            int8_t aYOffset, uint16_t aSteps, uint16_t aIntervalMillis, uint8_t aDirection = DIRECTION_UP);
    bool MovingPicturePGMUpdate();
#endif
#if defined(ENABLE_MATRIX_PATTERN_MOVE)
    void Move(uint8_t aDirection, uint16_t aNumberOfSteps = 1, uint16_t aIntervalMillis = 70, color32_t aBackgroundColor =
    COLOR32_BLACK);
    bool MoveUpdate();
#endif
    void moveArrayContent(uint8_t aDirection);
    void moveArrayContent(uint8_t aDirection, color32_t aBackgroundColor);

    bool update();
    bool update(uint8_t aBrightness);

#if defined(ENABLE_MATRIX_PATTERN_SNOW)
    bool Snow(uint16_t aNumberOfSteps = 500, uint16_t aIntervalMillis = 20);
    bool SnowUpdate();
    void SnowStop();

    void setRandomFlakeParameters(uint8_t aSnowFlakeIndex);
    void drawSnowFlake(uint8_t aSnowFlakeIndex);
    struct SnowFlakeInfoStruct *SnowFlakesArray;
#endif

    void showNumberOnMatrix(uint8_t aNumber, color32_t aColor);

#if defined(ENABLE_MATRIX_PATTERN_FIRE)
    bool Fire(uint16_t aNumberOfSteps = 200, uint16_t aIntervalMillis = 30);
    bool FireMatrixUpdate();
    void FireMatrixStop();
    void FireMatrixDealloc();
    /*
     * Two arrays for double buffering. Used for fire pattern
     * They have 1 pixel padding on each side for computation of convolution
     * Plus 1 extra bottom row for initial heat values
     */
    uint8_t *MatrixNew;
    uint8_t *MatrixOld;
#endif

    // for movingPicture and Ticker patterns
    const uint8_t *DataPtr; // can hold pointer to PGM or data space string or to PGM space 8x8 graphic array.
    int8_t GraphicsYOffset; // Offset of lower edge of graphic
    int8_t GraphicsXOffset; // Offset of left edge of graphic

    void setInitHeat();
};

void MatrixPatternsDemo(NeoPatterns *aLedsPtr);

void myLoadTest(MatrixNeoPatterns *aLedsPtr);

#endif // _MATRIX_NEOPATTERNS_H
