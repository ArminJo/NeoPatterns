/*
 * MatrixNeoPatterns.h
 *
 *  SUMMARY
 *  You need to install "Adafruit NeoPixel" library under Sketch -> Include Library -> Manage Librarys... -> use "neoPixel" as filter string
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

#ifndef MATRIXNEOPATTERNS_H_
#define MATRIXNEOPATTERNS_H_

#include "NeoPatterns.h"

#define FLAG_TICKER_DATA_IN_FLASH 0x01 // Flag if DataPtr points to RAM or FLASH

#define PATTERN_TICKER              32
#define PATTERN_MOVE                33
#define PATTERN_MOVING_PICTURE      34

extern const uint8_t fontNumbers4x6[] PROGMEM; // the font for showing numbers
#define NUMBERS_FONT_WIDTH 4
#define NUMBERS_FONT_HEIGHT 6

#define COOLING  20
/*
 * The sum SHOULD be 0!!!
 * The Matrix which describes the contribution of each surrounding pixel to the next heat value.
 */
#define CONVOLUTION_MATRIX_SIZE 3
float const convolutionMatrix[CONVOLUTION_MATRIX_SIZE][CONVOLUTION_MATRIX_SIZE] =
        { { 0.02, 0.02, 0.02 } /*weights of values above*/, { 0.05, -1 /*own value*/, 0.05 }, { 0.05, 0.74, 0.05 }/*weights of values below*/};

#define mapXYToArray(x, y, XRowLength) (((y) * (XRowLength)) + (x))

// extension of NeoPattern Class approximately 85 Byte / object
class MatrixNeoPatterns: public NeoPatterns {
public:
    MatrixNeoPatterns(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry, uint8_t aTypeOfPixel,
            void (*aPatternCompletionCallback)(NeoPatterns*)=NULL);

    void setGeometry(uint8_t aRows, uint8_t aColoums);
    void setLayoutMappingFunction(uint16_t (*aLayoutMappingFunction)(uint8_t, uint8_t, uint8_t, uint8_t));

    uint16_t LayoutMapping(uint8_t aColumnX, uint8_t aRowY);

    void Fire(uint16_t aIntervalMillis, uint16_t repetitions = 100);

    void TickerPGM(const char* aStringPtrPGM, color32_t aForegroundColor, color32_t aBackgroundColor, uint16_t aIntervalMillis,
            uint8_t aDirection = DIRECTION_LEFT);
    void Ticker(const char* aStringPtr, color32_t aForegroundColor, color32_t aBackgroundColor, uint16_t aIntervalMillis,
            uint8_t aDirection =
            DIRECTION_LEFT);
    void TickerInit(const char* aStringPtr, color32_t aForegroundColor, color32_t aBackgroundColor, uint16_t aIntervalMillis,
            uint8_t aDirection =
            DIRECTION_LEFT, uint8_t aFlags = 0);

    void drawQuarterPattern(uint16_t aPatternValue, color32_t aForegroundColor, color32_t aBackgroundColor);

    void loadPicturePGM(const uint8_t* aGraphicsrrayPGM, int8_t aWidthOfGraphic, uint8_t aHeightOfGraphic,
            color32_t aForegroundColor, color32_t aBackgroundColor = COLOR32_BLACK, int8_t aXOffset = 0, int8_t aYOffset = 0,
            bool doPaddingRight = false, bool doPadding = false);

    void MovingPicturePGM(const uint8_t* aGraphics8x8Array, color32_t aForegroundColor, color32_t aBackgroundColor, int8_t aXOffset,
            int8_t aYOffset, uint16_t aSteps, uint16_t aIntervalMillis, uint8_t aDirection = DIRECTION_UP);
    //
    bool Update(bool doShow = true);
    void Move(uint8_t aDirection, uint16_t aSteps = 1, uint16_t aIntervalMillis = 70, bool aMoveDirect = true,
            color32_t aBackgroundColor = COLOR32_BLACK);
    void moveArrayContent(uint8_t aDirection);
    void moveArrayContent(uint8_t aDirection, color32_t aBackgroundColor);

    void MoveUpdate();
    void FireMatrixUpdate();
    void MovingPicturePGMUpdate();
    void TickerUpdate();

    color32_t getMatrixPixelColor(uint8_t x, uint8_t y);
    //
    bool setMatrixPixelColor(uint8_t x, uint8_t y, color32_t a32BitColor);
    void showNumberOnMatrix(uint8_t aNumber, color32_t aColor);

    // Geometry of Matrix
    uint8_t Rows;       // Y Direction
    uint8_t Columns;    // X Direction

    // Two arrays for double buffering
    uint8_t * MatrixNew;
    uint8_t * MatrixOld;

    // for picture and ticker extension
    const uint8_t* DataPtr; // can hold pointer to PGM or data space string or to PGM space 8x8 graphic array.
    int8_t GraphicsYOffset;
    int8_t GraphicsXOffset;

    uint16_t (*LayoutMappingFunction)(uint8_t, uint8_t, uint8_t, uint8_t); // Pointer to function, which implements the mapping between X/Y and pixel number

    /*
     * Defines from Adafruit_NeoMatrix.h
     */
    // Matrix layout information is passed in the 'matrixType' parameter for
    // each constructor (the parameter immediately following is the LED type
    // from NeoPixel.h).
    // These define the layout for a single 'unified' matrix (e.g. one made
    // from NeoPixel strips, or a single NeoPixel shield), or for the pixels
    // within each matrix of a tiled display (e.g. multiple NeoPixel shields).
#define NEO_MATRIX_TOP         0x00 // Pixel 0 is at top of matrix
#define NEO_MATRIX_BOTTOM      0x01 // Pixel 0 is at bottom of matrix
#define NEO_MATRIX_LEFT        0x00 // Pixel 0 is at left of matrix
#define NEO_MATRIX_RIGHT       0x02 // Pixel 0 is at right of matrix
#define NEO_MATRIX_CORNER      0x03 // Bitmask for pixel 0 matrix corner
#define NEO_MATRIX_ROWS        0x00 // Matrix is row major (horizontal)
#define NEO_MATRIX_COLUMNS     0x04 // Matrix is column major (vertical)
#define NEO_MATRIX_AXIS        0x04 // Bitmask for row/column layout
#define NEO_MATRIX_PROGRESSIVE 0x00 // Same pixel order across each line
#define NEO_MATRIX_ZIGZAG      0x08 // Pixel order reverses between lines
#define NEO_MATRIX_SEQUENCE    0x08 // Bitmask for pixel line order
    uint8_t Geometry;    // Flags for geometry

};

/*
 * Example (fast) custom mapping functions
 */
uint16_t ProgressiveTypeBottomRightMapping(uint8_t aColumnX, uint8_t aRowY, uint8_t aColumnsTotal, uint8_t aRowsTotal);
uint16_t ProgressiveTypeBottomLeftMapping(uint8_t aColumnX, uint8_t aRowY, uint8_t aColumnsTotal, uint8_t aRowsTotal);
uint16_t ZigzagTypeBottomRightMapping(uint8_t aColumnX, uint8_t aRowY, uint8_t aColumnsTotal, uint8_t aRowsTotal);
uint16_t ZigzagTypeBottomLeftMapping(uint8_t aColumnX, uint8_t aRowY, uint8_t aColumnsTotal, uint8_t aRowsTotal);

#define HEART_WIDTH 8
#define HEART_HEIGHT 8
extern const uint8_t heart8x8[] PROGMEM;

void MatrixPatternsDemo(NeoPatterns * aLedsPtr);

void myLoadTest(MatrixNeoPatterns* aLedsPtr);
void mySnowflakeTest(MatrixNeoPatterns* aLedsPtr);

#endif /* MATRIXNEOPATTERNS_H_ */
