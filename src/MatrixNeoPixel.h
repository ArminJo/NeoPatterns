/*
 * MatrixNeoPixel.h
 *
 * Implements basic functions for NeoPixel matrix. Tested with 8x8 matrix.
 * Origin (0,0) of x and y values is at the top left corner and the positive direction is right and DOWN.
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

/*
 * Class inheritance diagram. We have virtual inheritance, since MatrixNeoPatterns needs only one member set of NeoPixel
 *
 *                    MatrixNeoPixelVU \
 *                                     ,o--> MatrixNeoPixel (virtual) \
 * MatrixSnake --> MatrixNeoPatterns  <                                o--> NeoPixel --> Adafruit_NeoPixel
 *                                     `o--> NeoPatterns    (virtual) /
 *
 */

#ifndef SRC_LIB_NEOPATTERNS_MATRIXNEOPIXEL_H_
#define SRC_LIB_NEOPATTERNS_MATRIXNEOPIXEL_H_

#include "NeoPixel.h"

/*
 * If you have only default geometry (NEO_MATRIX_BOTTOM | NEO_MATRIX_RIGHT | NEO_MATRIX_ROWS | NEO_MATRIX_PROGRESSIVE),
 * i.e. Pixel 0 is at bottom right of matrix, matrix is row major (horizontal) and same pixel order across each line (no zig-zag)
 * then you can save program space by defining symbol `SUPPORT_ONLY_DEFAULT_GEOMETRY`.
 * This saves 560 bytes FLASH and 3 bytes RAM.
 */
//#define SUPPORT_ONLY_DEFAULT_GEOMETRY
//
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

#define NEO_MATRIX_DEFAULT_GEOMETRY   (NEO_MATRIX_BOTTOM | NEO_MATRIX_RIGHT | NEO_MATRIX_ROWS | NEO_MATRIX_PROGRESSIVE)

/*
 * For constrain function
 */
#define MAX_SUPPORTED_GRAPHICS_WIDTH    16 // see void loadPicture(const uint16_t *aGraphicsArrayPtr...
#define MAX_SUPPORTED_GRAPHICS_HEIGHT   16 // a guess

class MatrixNeoPixel: public virtual NeoPixel {
public:
    MatrixNeoPixel();
    void init();
    MatrixNeoPixel(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry, uint8_t aTypeOfPixel);
    bool init(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry, uint8_t aTypeOfPixel);

#ifndef SUPPORT_ONLY_DEFAULT_GEOMETRY
    void setLayoutMappingFunction(uint16_t (*aLayoutMappingFunction)(uint8_t, uint8_t, uint8_t, uint8_t));
    uint16_t LayoutMapping(uint8_t aColumnX, uint8_t aRowY);
#endif

    color32_t getMatrixPixelColor(uint8_t aColumnX, uint8_t aRowY);
    void setMatrixPixelColor(uint8_t aColumnX, uint8_t aRowY, color32_t a32BitColor);
    void setMatrixPixelColor(uint8_t aColumnX, uint8_t aRowY, uint8_t aRed, uint8_t aGreen, uint8_t aBlue);
    void addMatrixPixelColor(uint8_t aColumnX, uint8_t aRowY, uint8_t aRed, uint8_t aGreen, uint8_t aBlue);
    void addMatrixPixelColor(uint8_t aColumnX, uint8_t aRowY, color32_t aColor);

    void loadPicturePGM(const uint8_t *aGraphicsArrayPtrPGM, int8_t aWidthOfGraphic, uint8_t aHeightOfGraphic,
            color32_t aForegroundColor, color32_t aBackgroundColor, int8_t aXOffset, int8_t aYOffset, bool doPadding = false);

    void loadPicture(const uint8_t *aGraphicsArrayPtr, int8_t aWidthOfGraphic, uint8_t aHeightOfGraphic, color32_t aForegroundColor,
            color32_t aBackgroundColor, int8_t aXOffset, int8_t aYOffset, bool doPadding = false, bool IsPGMData = false);

    void loadPicture(const uint16_t *aGraphicsArrayPtr, int8_t aWidthOfGraphic, uint8_t aHeightOfGraphic,
            color32_t aForegroundColor, color32_t aBackgroundColor, int8_t aXOffset, int8_t aYOffset, bool doPadding = false,
            bool IsPGMData = false);

    void drawQuarterPatternOdd(uint16_t aPatternValue, color32_t aForegroundColor, color32_t aBackgroundColor);
    void drawQuarterPatternEven(uint16_t aPatternValue, color32_t aForegroundColor, color32_t aBackgroundColor);

    void drawAllColors();
    void drawAllColors2();

    void drawBar(uint8_t aColumnX, uint16_t aBarLength, color32_t aColor, bool aDrawFromBottom = true);
    void drawBarFromColorArray(uint8_t aColumnX, uint16_t aBarLength, color32_t *aColorArrayPtr, bool aDrawFromBottom = true);

    // Geometry of Matrix
    uint8_t Rows;       // Y Direction / size
    uint8_t Columns;    // X Direction / size

#ifndef SUPPORT_ONLY_DEFAULT_GEOMETRY
    uint8_t Geometry;    // Flags for geometry
    uint16_t (*LayoutMappingFunction)(uint8_t, uint8_t, uint8_t, uint8_t); // Pointer to function, which implements the mapping between X/Y and pixel number
#endif
};

#define HEART_WIDTH 8
#define HEART_HEIGHT 8
extern const uint8_t heart8x8[] PROGMEM;

/*
 * Example (fast) custom mapping functions
 */
uint16_t ProgressiveTypeBottomRightMapping(uint8_t aColumnX, uint8_t aRowY, uint8_t aColumnsTotal, uint8_t aRowsTotal);
uint16_t ProgressiveTypeBottomLeftMapping(uint8_t aColumnX, uint8_t aRowY, uint8_t aColumnsTotal, uint8_t aRowsTotal);
uint16_t ZigzagTypeBottomRightMapping(uint8_t aColumnX, uint8_t aRowY, uint8_t aColumnsTotal, uint8_t aRowsTotal);
uint16_t ZigzagTypeBottomLeftMapping(uint8_t aColumnX, uint8_t aRowY, uint8_t aColumnsTotal, uint8_t aRowsTotal);

#endif /* SRC_LIB_NEOPATTERNS_MATRIXNEOPIXEL_H_ */

#pragma once

