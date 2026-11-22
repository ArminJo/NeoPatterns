/*
 * MatrixNeoPixelVU.hpp
 *
 * Implements basic functions for VU display on NeoPixel matrix.
 *
 *  SUMMARY
 *  You need to install "Adafruit NeoPixel" library under "Tools -> Manage Libraries..." or "Ctrl+Shift+I" -> use "neoPixel" as filter string.
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
 */

#ifndef _MATRIX_NEOPIXEL_VU_HPP
#define _MATRIX_NEOPIXEL_VU_HPP

#include "MatrixNeoPixel.hpp"
#include "MatrixNeoPixelVU.h"
#include "NeoPixelVU.hpp" // for initBarColorRYG

// This block must be located after the includes of other *.hpp files
//#define LOCAL_DEBUG // This enables debug output only for this file - only for development
#include "LocalDebugLevelStart.h"

/*
 * Allocates the pointer array and one VUBar per column.
 * Prints an error message if aSerial is not nullptr.
 * @return false if not enough memory. Already allocated bars are freed in this case.
 */
bool MatrixNeoPixelVU::allocateVUBars(uint8_t aColumns, uint8_t aRows, uint8_t aDampingFactorForValue, uint8_t aFlags,
        Print *aSerial) {

    VUBarArray = new VUBar*[aColumns];
    if (VUBarArray == nullptr) {
        if (aSerial != nullptr) {
            aSerial->println(F("ERROR Not enough free memory available for VUBarArray"));
        }
        return false;
    }
    memset(VUBarArray, 0, aColumns * sizeof(VUBar*)); // after the nullptr check!

    for (uint_fast8_t i = 0; i < aColumns; i++) {
        // test for sufficient memory, because new VUBar() does not check for malloc() returning nullptr
        void *tMemory = malloc(sizeof(VUBar));
        if (tMemory == nullptr) {
            if (aSerial != nullptr) {
                aSerial->print(F("ERROR Not enough free memory available for VUBarArray object number "));
                aSerial->println(i);
            }
            freeVUBars();
            return false;
        }
        free(tMemory);
        VUBarArray[i] = new VUBar(aRows, aDampingFactorForValue, aFlags);
    }
    return true;
}

void MatrixNeoPixelVU::freeVUBars() {
    if (VUBarArray != nullptr) {
        for (uint_fast8_t i = 0; i < Columns; i++) {
            if (VUBarArray[i] != nullptr) {
                VUBarArray[i]->~VUBar(); // no delete, because older AVR cores lack operator delete(void*, size_t)
                free(VUBarArray[i]);
            }
        }
        delete[] VUBarArray;
        VUBarArray = nullptr;
    }
}

MatrixNeoPixelVU::~MatrixNeoPixelVU() {
    freeVUBars();
}

MatrixNeoPixelVU::MatrixNeoPixelVU() : // @suppress("Class members should be properly initialized")
        MatrixNeoPixel() {
    VUBarArray = nullptr;
}

MatrixNeoPixelVU::MatrixNeoPixelVU(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry,
        neoPixelType aTypeOfPixel) : // @suppress("Class members should be properly initialized")
        NeoPixel(aColumns * aRows, aPin, aTypeOfPixel), MatrixNeoPixel(aColumns, aRows, aPin, aMatrixGeometry, aTypeOfPixel) {
    VUBarArray = nullptr;
    BarColorUnion.BarColor = COLOR32_RED_HALF;
    allocateVUBars(aColumns, aRows, 0, 0, nullptr); // errors are reported later by begin(&Serial)
}

MatrixNeoPixelVU::MatrixNeoPixelVU(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry,
        neoPixelType aTypeOfPixel, // @suppress("Class members should be properly initialized")
        uint8_t aDampingFactorForValue, uint8_t aFlags, color32_t *aBarColorArrayPtr) :
        NeoPixel(aColumns * aRows, aPin, aTypeOfPixel), MatrixNeoPixel(aColumns, aRows, aPin, aMatrixGeometry, aTypeOfPixel) {
    VUBarArray = nullptr;
    BarColorUnion.BarColorArrayPtr = aBarColorArrayPtr;
    allocateVUBars(aColumns, aRows, aDampingFactorForValue, aFlags | FLAG_VU_NEOPIXEL_USE_BAR_COLOR_ARRAY, nullptr);
}

MatrixNeoPixelVU::MatrixNeoPixelVU(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry,
        neoPixelType aTypeOfPixel, // @suppress("Class members should be properly initialized")
        uint8_t aDampingFactorForValue, uint8_t aFlags, color16_t *aBarColor16ArrayPtr) :
        NeoPixel(aColumns * aRows, aPin, aTypeOfPixel), MatrixNeoPixel(aColumns, aRows, aPin, aMatrixGeometry, aTypeOfPixel) {
    VUBarArray = nullptr;
    BarColorUnion.BarColor16ArrayPtr = aBarColor16ArrayPtr;
    allocateVUBars(aColumns, aRows, aDampingFactorForValue,
            aFlags | FLAG_VU_NEOPIXEL_USE_BAR_COLOR_ARRAY | FLAG_VU_NEOPIXEL_USE_BAR_16_BIT_COLOR_ARRAY, nullptr);
}

/*
 * Common part of all init() functions
 */
bool MatrixNeoPixelVU::initMatrix(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry, neoPixelType aTypeOfPixel,
        Print *aSerial) {
    freeVUBars(); // must be before MatrixNeoPixel::init(), because it requires the old value of Columns
    if (!MatrixNeoPixel::init(aColumns, aRows, aPin, aMatrixGeometry, aTypeOfPixel)) {
        if (aSerial != nullptr) {
            aSerial->print(F("ERROR Not enough free memory available for VU matrix at pin "));
            aSerial->println(getPin());
        }
        return false;
    }
    return true;
}

bool MatrixNeoPixelVU::init(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry, neoPixelType aTypeOfPixel,
        uint8_t aDampingFactorForValue, uint8_t aFlags, color32_t *aBarColorArrayPtr, Print *aSerial) {
    if (!initMatrix(aColumns, aRows, aPin, aMatrixGeometry, aTypeOfPixel, aSerial)) {
        return false;
    }
    BarColorUnion.BarColorArrayPtr = aBarColorArrayPtr; // One color template for all bars
    return allocateVUBars(aColumns, aRows, aDampingFactorForValue, aFlags | FLAG_VU_NEOPIXEL_USE_BAR_COLOR_ARRAY, aSerial);
}

bool MatrixNeoPixelVU::init(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry, neoPixelType aTypeOfPixel,
        uint8_t aDampingFactorForValue, uint8_t aFlags, color16_t *aBarColor16ArrayPtr, Print *aSerial) {
    if (!initMatrix(aColumns, aRows, aPin, aMatrixGeometry, aTypeOfPixel, aSerial)) {
        return false;
    }
    BarColorUnion.BarColor16ArrayPtr = aBarColor16ArrayPtr;
    return allocateVUBars(aColumns, aRows, aDampingFactorForValue,
            aFlags | FLAG_VU_NEOPIXEL_USE_BAR_COLOR_ARRAY | FLAG_VU_NEOPIXEL_USE_BAR_16_BIT_COLOR_ARRAY, aSerial);
}

bool MatrixNeoPixelVU::init(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry, neoPixelType aTypeOfPixel,
        uint8_t aDampingFactorForValue, uint8_t aFlags, color32_t aBarColor, Print *aSerial) {
    if (!initMatrix(aColumns, aRows, aPin, aMatrixGeometry, aTypeOfPixel, aSerial)) {
        return false;
    }
    BarColorUnion.BarColor = aBarColor;
    return allocateVUBars(aColumns, aRows, aDampingFactorForValue, aFlags, aSerial);
}

bool MatrixNeoPixelVU::init(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry, neoPixelType aTypeOfPixel) {
    if (!initMatrix(aColumns, aRows, aPin, aMatrixGeometry, aTypeOfPixel, nullptr)) {
        return false;
    }
    BarColorUnion.BarColor = COLOR32_RED_HALF;
    return allocateVUBars(aColumns, aRows, 0, 0, nullptr);
}

void MatrixNeoPixelVU::begin() {
    Adafruit_NeoPixel::begin();
}

/*
 * begin function, which prints error message if aSerial is not nullptr
 * Returns false if no memory available
 */
bool MatrixNeoPixelVU::begin(Print *aSerial) {

    Adafruit_NeoPixel::begin();
    if (numLEDs == 0) {
        if (aSerial != nullptr) {
            aSerial->print(F("ERROR Not enough free memory available for VU matrix at pin "));
            aSerial->println(getPin());
        }
        return false;
    }
    if (VUBarArray == nullptr) {
        if (aSerial != nullptr) {
            aSerial->println(F("ERROR Not enough free memory available for VUBarArray"));
        }
        return false;
    }

    for (uint_fast8_t i = 0; i < Columns; ++i) {
        if (VUBarArray[i] == nullptr) {
            if (aSerial != nullptr) {
                aSerial->print(F("ERROR Not enough free memory available for VUBarArray object number "));
                aSerial->println(i);
            }
            return false;
        }
    }
    return true;
}

void MatrixNeoPixelVU::setBarAndPeak(uint8_t aColumnNumber, uint16_t aInputValue) {
    if (VUBarArray == nullptr || aColumnNumber >= Columns) {
        return;
    }
#if defined(LOCAL_DEBUG)
    printPin(&Serial);
    Serial.print(F("Column="));
    Serial.print(aColumnNumber);
    Serial.print(F(" Input="));
    Serial.print(aInputValue);
    Serial.print(F(" Flags=0x"));
    Serial.print(VUBarArray[aColumnNumber]->Flags, HEX);
    Serial.print(F(" &Array=0x"));
    Serial.println((uintptr_t) (BarColorUnion.BarColorArrayPtr), HEX);
#endif
    uint8_t tSignalHeight = VUBarArray[aColumnNumber]->computeNewSignalHeight(aInputValue);
    if (!(VUBarArray[aColumnNumber]->Flags & FLAG_VU_NEOPIXEL_USE_BAR_COLOR_ARRAY)) {
        drawBar(aColumnNumber, tSignalHeight, BarColorUnion.BarColor);
    } else if (VUBarArray[aColumnNumber]->Flags & FLAG_VU_NEOPIXEL_USE_BAR_16_BIT_COLOR_ARRAY) {
        drawBarFromColorArray(aColumnNumber, tSignalHeight, BarColorUnion.BarColor16ArrayPtr);
    } else {
        drawBarFromColorArray(aColumnNumber, tSignalHeight, BarColorUnion.BarColorArrayPtr);
    }
    VUBarArray[aColumnNumber]->computeNewPeakValue(tSignalHeight);
    drawPeak(aColumnNumber, PEAK_PIXEL_COLOR);
}

/*
 * Set peak pixel
 * @param aColor - if color is COLOR32_PEAK_RYG (COLOR32_BLACK), then use Red Yellow Green color schema
 *                 with < 1/2 green, then 1/4 yellow and 1/4 red, like NeoPixelVU::drawPeak() and initBarColorRYG()
 */
void MatrixNeoPixelVU::drawPeak(uint8_t aColumnNumber, color32_t aColor) {
    uint8_t tPeakValue = VUBarArray[aColumnNumber]->PeakValue;
    uint16_t tBarLength = VUBarArray[aColumnNumber]->BarLength;

    // If PeakValue == 0, do not render peak
    if (tPeakValue > 0 && tPeakValue <= tBarLength) {
        if (aColor == COLOR32_PEAK_RYG) {
            if (tPeakValue <= (tBarLength + 1) / 2) {
                aColor = COLOR32_GREEN;
            } else if (tPeakValue <= (tBarLength - (tBarLength / 4))) {
                aColor = COLOR32_YELLOW;
            } else {
                aColor = COLOR32_RED;
            }
        }
        // PeakValue 1 is the bottom row (Rows - 1), PeakValue == Rows is the top row 0
        setMatrixPixelColor(aColumnNumber, Rows - tPeakValue, aColor);
    }
}

/*
 * Initialize bar color array with < 1/2 green, then yellow and 1/4 in red
 */
void MatrixNeoPixelVU::initBarColorRYG() {
    if (VUBarArray != nullptr && VUBarArray[0]->has32BitColorArray()) {
        NeoPixelVU::initBarColorRYG(VUBarArray[0]->BarLength, BarColorUnion.BarColorArrayPtr);
    }
}

void MatrixNeoPixelVU::initBarGreenToRed() {
    if (VUBarArray != nullptr && VUBarArray[0]->has32BitColorArray()) {
        NeoPixelVU::initBarGreenToRed(VUBarArray[0]->BarLength, BarColorUnion.BarColorArrayPtr);
    }
}

void MatrixNeoPixelVU::initBarColorWheel() {
    if (VUBarArray != nullptr && VUBarArray[0]->has32BitColorArray()) {
        NeoPixelVU::initBarColorWheel(VUBarArray[0]->BarLength, BarColorUnion.BarColorArrayPtr);
    }
}
void MatrixNeoPixelVU::initBarColorRYG16() {
    if (VUBarArray != nullptr && VUBarArray[0]->has16BitColorArray()) {
        NeoPixelVU::initBarColorRYG16(VUBarArray[0]->BarLength, BarColorUnion.BarColor16ArrayPtr);
    }
}

void MatrixNeoPixelVU::initBarGreenToRed16() {
    if (VUBarArray != nullptr && VUBarArray[0]->has16BitColorArray()) {
        NeoPixelVU::initBarGreenToRed16(VUBarArray[0]->BarLength, BarColorUnion.BarColor16ArrayPtr);
    }
}

void MatrixNeoPixelVU::initBarColorWheel16() {
    if (VUBarArray != nullptr && VUBarArray[0]->has16BitColorArray()) {
        NeoPixelVU::initBarColorWheel16(VUBarArray[0]->BarLength, BarColorUnion.BarColor16ArrayPtr);
    }
}

#include "LocalDebugLevelEnd.h"
#endif // _MATRIX_NEOPIXEL_VU_HPP
