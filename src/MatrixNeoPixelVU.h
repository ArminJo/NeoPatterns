/*
 * MatrixNeoPixelVU.h
 *
 *  Copyright (C) 2019-2026  Armin Joachimsmeyer
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
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/gpl-3.0.html>.
 *
 */

#ifndef _MATRIX_NEOPIXEL_VU_H
#define _MATRIX_NEOPIXEL_VU_H

#include "VUBar.h"
#include "MatrixNeoPixel.h"

class MatrixNeoPixelVU: public MatrixNeoPixel {
public:
    MatrixNeoPixelVU();
    MatrixNeoPixelVU(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry, neoPixelType aTypeOfPixel);
    bool init(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry, neoPixelType aTypeOfPixel);
    MatrixNeoPixelVU(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry, neoPixelType aTypeOfPixel,
            uint8_t aDampingFactorForValue, uint8_t aFlags, color32_t *aBarColorArrayPtr);
    MatrixNeoPixelVU(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry, neoPixelType aTypeOfPixel,
            uint8_t aDampingFactorForValue, uint8_t aFlags, color16_t *aBarColor16ArrayPtr);

    bool initMatrix(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry, neoPixelType aTypeOfPixel,
            Print *aSerial);

    bool init(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry, neoPixelType aTypeOfPixel,
            uint8_t aDampingFactorForValue, uint8_t aFlags, color32_t *aBarColorArrayPtr, Print *aSerial = nullptr);
    bool init(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry, neoPixelType aTypeOfPixel,
            uint8_t aDampingFactorForValue, uint8_t aFlags, color16_t *aBarColor16ArrayPtr, Print *aSerial = nullptr);
    bool init(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry, neoPixelType aTypeOfPixel,
            uint8_t aDampingFactorForValue, uint8_t aFlags, color32_t aBarColor, Print *aSerial = nullptr);
    ~MatrixNeoPixelVU();

    void begin();
    // Version with error message
    bool begin(Print *aSerial);

    void setBarAndPeak(uint8_t aColumnNumber, uint16_t aInputValue);
    void drawPeak(uint8_t aColumnNumber, color32_t aColor = COLOR32_RED);

    void initBarColorRYG();
    void initBarGreenToRed();
    void initBarColorWheel();
    void initBarColorRYG16();
    void initBarGreenToRed16();
    void initBarColorWheel16();

    union {
        color32_t BarColor;
        color32_t *BarColorArrayPtr; // Use this colors for drawing the bar
        color16_t *BarColor16ArrayPtr; // Use this 16 bit colors for drawing the bar
    } BarColorUnion;

    VUBar **VUBarArray; // Pointer to array of (column) VU bars

private:
    bool allocateVUBars(uint8_t aColumns, uint8_t aRows, uint8_t aDampingFactorForValue, uint8_t aFlags, Print *aSerial);
    void freeVUBars();
};

#endif // _MATRIX_NEOPIXEL_VU_H
