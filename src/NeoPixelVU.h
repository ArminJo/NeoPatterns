/*
 * NeoPixelVU.h
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

#ifndef _NEOPIXEL_VU_H
#define _NEOPIXEL_VU_H

#include <stdint.h>

#include "NeoPixel.h"
#include "VUBar.h"
#include "Colors.h"

#define COLOR32_PEAK_RYG        COLOR32_BLACK // if color is COLOR32_BLACK, then use Red Yellow Green color schema
#if !defined(PEAK_PIXEL_COLOR)
#define PEAK_PIXEL_COLOR        COLOR32_RED
//#define PEAK_PIXEL_COLOR      COLOR32_PEAK_RYG
#endif
/*
 * NeoPixelVU does not inherit from NeoPixel, it has only a pointer to the parent NeoPixel object,
 * since all NeoPixelVU of one size can share the same NeoPixel object for drawing, because no pattern persistence is needed.
 * Only thing we must do is change the output pin of the parent NeoPixel object.
 *
 * 22 bytes RAM
 */
class NeoPixelVU: public VUBar {
public:
    NeoPixelVU();
    NeoPixelVU(uint8_t aPin, NeoPixel *aNeoPixelPtr);
    void init(uint8_t aPin, NeoPixel *aNeoPixelPtr);
    NeoPixelVU(uint8_t aPin, NeoPixel *aNeoPixelPtr, uint8_t aDampingFactorForValue, uint8_t aFlags, color32_t *aBarColorArrayPtr);
    NeoPixelVU(uint8_t aPin, NeoPixel *aNeoPixelPtr, uint8_t aDampingFactorForValue, uint8_t aFlags,
            color16_t *aBarColor16ArrayPtr);
    NeoPixelVU(uint8_t aPin, NeoPixel *aNeoPixelPtr, uint8_t aDampingFactorForValue, uint8_t aFlags, color32_t *aBarColorArrayPtr,
            uint16_t aValueForFullBar);
    NeoPixelVU(uint8_t aPin, NeoPixel *aNeoPixelPtr, uint8_t aDampingFactorForValue, uint8_t aFlags, color16_t *aBarColor16ArrayPtr,
            uint16_t aValueForFullBar);
    NeoPixelVU(uint8_t aPin, NeoPixel *aNeoPixelPtr, uint8_t aDampingFactorForValue, uint8_t aFlags, color32_t aBarColor);
    NeoPixelVU(uint8_t aPin, NeoPixel *aNeoPixelPtr, uint8_t aDampingFactorForValue, uint8_t aFlags, color32_t aBarColor,
            uint16_t aValueForFullBar);
    void init(uint8_t aPin, NeoPixel *aNeoPixelPtr, uint8_t aDampingFactorForValue, uint8_t aFlags, color32_t *aBarColorArrayPtr);
    void init(uint8_t aPin, NeoPixel *aNeoPixelPtr, uint8_t aDampingFactorForValue, uint8_t aFlags, color16_t *aBarColor16ArrayPtr);
    void init(uint8_t aPin, NeoPixel *aNeoPixelPtr, uint8_t aDampingFactorForValue, uint8_t aFlags, color32_t *aBarColorArrayPtr,
            uint16_t aValueForFullBar);
    void init(uint8_t aPin, NeoPixel *aNeoPixelPtr, uint8_t aDampingFactorForValue, uint8_t aFlags, color16_t *aBarColor16ArrayPtr,
            uint16_t aValueForFullBar);
    void init(uint8_t aPin, NeoPixel *aNeoPixelPtr, uint8_t aDampingFactorForValue, uint8_t aFlags, color32_t aBarColor);
    void init(uint8_t aPin, NeoPixel *aNeoPixelPtr, uint8_t aDampingFactorForValue, uint8_t aFlags, color32_t aBarColor,
            uint16_t aValueForFullBar);

    void setBarAndPeakAndShow(uint16_t aInputValue);
    void drawPeak(color32_t aColor = COLOR32_RED); // if color is COLOR32_BLACK, then use Red Yellow Green color schema
    void show();
    void initBarColorRYG();
    void initBarGreenToRed();
    void initBarColorWheel();
    void initBarColorRYG16();
    void initBarGreenToRed16();
    void initBarColorWheel16();

    NeoPixel *NeoPixelBar;

    union {
        color32_t BarColor;
        color32_t *BarColorArrayPtr; // Use this colors for drawing the bar
        color16_t *BarColor16ArrayPtr; // Use this colors for drawing the bar
    } BarColorUnion;

    uint8_t Pin; // required for re-using one NeoPixel object for different NeoPixelVU

    // static functions for use in MatrixNeoPixelVU
    static void initBarColorRYG(uint8_t aBarLength, color32_t *aBarColorArrayPtr);
    static void initBarGreenToRed(uint8_t aBarLength, color32_t *aBarColorArrayPtr);
    static void initBarColorWheel(uint8_t aBarLength, color32_t *aBarColorArrayPtr);
    // For 16 bit color
    static void initBarColorRYG16(uint8_t aBarLength, color16_t *aBarColor16ArrayPtr);
    static void initBarGreenToRed16(uint8_t aBarLength, color16_t *aBarColor16ArrayPtr);
    static void initBarColorWheel16(uint8_t aBarLength, color16_t *aBarColor16ArrayPtr);
};

#endif // _NEOPIXEL_VU_H
