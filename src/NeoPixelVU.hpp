/*
 * NeoPixelVU.hpp
 *
 * Uses VUBar to compute new bar length and peak position values and then draws them according to the bar color scheme.
 * VU Bar length is restricted to 256 pixels
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

#ifndef _NEOPIXEL_VU_HPP
#define _NEOPIXEL_VU_HPP

#include "NeoPixel.hpp"
#include "NeoPixelVU.h"

#include "VUBar.hpp"

// This block must be located after the includes of other *.hpp files
//#define LOCAL_DEBUG // This enables debug output only for this file - only for development
//#define LOCAL_TRACE // This enables trace output only for this file - only for development
#include "LocalDebugLevelStart.h"

NeoPixelVU::NeoPixelVU() : // @suppress("Class members should be properly initialized")
        VUBar() {
    NeoPixelBar = nullptr;
}

NeoPixelVU::NeoPixelVU(uint8_t aPin, NeoPixel *aNeoPixelPtr) { // @suppress("Class members should be properly initialized")
    init(aPin, aNeoPixelPtr);
}

void NeoPixelVU::init(uint8_t aPin, NeoPixel *aNeoPixelPtr) {
    VUBar::init(aNeoPixelPtr->numPixels(), 0, 0);

    Pin = aPin;
    NeoPixelBar = aNeoPixelPtr;
    BarColorUnion.BarColor = COLOR32_RED_HALF;
}

/**
 * @param aDampingFactorForValue The shift amount of the underlying EMA low pass filter.
 *                               if 0 then use value direct and not dampened
 */
NeoPixelVU::NeoPixelVU(uint8_t aPin, NeoPixel *aNeoPixelPtr, uint8_t aDampingFactorForValue, // @suppress("Class members should be properly initialized")
        uint8_t aFlags, color32_t *aBarColorArrayPtr) {
    init(aPin, aNeoPixelPtr, aDampingFactorForValue, aFlags, aBarColorArrayPtr);
}
NeoPixelVU::NeoPixelVU(uint8_t aPin, NeoPixel *aNeoPixelPtr, uint8_t aDampingFactorForValue, // @suppress("Class members should be properly initialized")
        uint8_t aFlags, color16_t *aBarColor16ArrayPtr) {
    init(aPin, aNeoPixelPtr, aDampingFactorForValue, aFlags, aBarColor16ArrayPtr);
}

NeoPixelVU::NeoPixelVU(uint8_t aPin, NeoPixel *aNeoPixelPtr, uint8_t aDampingFactorForValue, // @suppress("Class members should be properly initialized")
        uint8_t aFlags, color32_t *aBarColorArrayPtr, uint16_t aValueForFullBar) {
    init(aPin, aNeoPixelPtr, aDampingFactorForValue, aFlags, aBarColorArrayPtr, aValueForFullBar);
}
NeoPixelVU::NeoPixelVU(uint8_t aPin, NeoPixel *aNeoPixelPtr, uint8_t aDampingFactorForValue, // @suppress("Class members should be properly initialized")
        uint8_t aFlags, color16_t *aBarColor16ArrayPtr, uint16_t aValueForFullBar) {
    init(aPin, aNeoPixelPtr, aDampingFactorForValue, aFlags, aBarColor16ArrayPtr, aValueForFullBar);
}

NeoPixelVU::NeoPixelVU(uint8_t aPin, NeoPixel *aNeoPixelPtr, uint8_t aDampingFactorForValue, // @suppress("Class members should be properly initialized")
        uint8_t aFlags, color32_t aBarColor) {
    init(aPin, aNeoPixelPtr, aDampingFactorForValue, aFlags, aBarColor);
}
NeoPixelVU::NeoPixelVU(uint8_t aPin, NeoPixel *aNeoPixelPtr, uint8_t aDampingFactorForValue, // @suppress("Class members should be properly initialized")
        uint8_t aFlags, color32_t aBarColor, uint16_t aValueForFullBar) {
    init(aPin, aNeoPixelPtr, aDampingFactorForValue, aFlags, aBarColor, aValueForFullBar);
}

void NeoPixelVU::init(uint8_t aPin, NeoPixel *aNeoPixelPtr, uint8_t aDampingFactorForValue, uint8_t aFlags,
        color32_t *aBarColorArrayPtr) {
    VUBar::init(aNeoPixelPtr->numPixels(), aDampingFactorForValue, aFlags | FLAG_VU_NEOPIXEL_USE_BAR_COLOR_ARRAY);

    Pin = aPin;
    NeoPixelBar = aNeoPixelPtr;
    BarColorUnion.BarColorArrayPtr = aBarColorArrayPtr;
}
void NeoPixelVU::init(uint8_t aPin, NeoPixel *aNeoPixelPtr, uint8_t aDampingFactorForValue, uint8_t aFlags,
        color16_t *aBarColor16ArrayPtr) {
    VUBar::init(aNeoPixelPtr->numPixels(), aDampingFactorForValue,
            aFlags | FLAG_VU_NEOPIXEL_USE_BAR_COLOR_ARRAY | FLAG_VU_NEOPIXEL_USE_BAR_16_BIT_COLOR_ARRAY);

    Pin = aPin;
    NeoPixelBar = aNeoPixelPtr;
    BarColorUnion.BarColor16ArrayPtr = aBarColor16ArrayPtr;
}
void NeoPixelVU::init(uint8_t aPin, NeoPixel *aNeoPixelPtr, uint8_t aDampingFactorForValue, uint8_t aFlags,
        color32_t *aBarColorArrayPtr, uint16_t aValueForFullBar) {
    VUBar::init(aNeoPixelPtr->numPixels(), aDampingFactorForValue, aFlags | FLAG_VU_NEOPIXEL_USE_BAR_COLOR_ARRAY, aValueForFullBar);

    Pin = aPin;
    NeoPixelBar = aNeoPixelPtr;
    BarColorUnion.BarColorArrayPtr = aBarColorArrayPtr;
}
void NeoPixelVU::init(uint8_t aPin, NeoPixel *aNeoPixelPtr, uint8_t aDampingFactorForValue, uint8_t aFlags,
        color16_t *aBarColor16ArrayPtr, uint16_t aValueForFullBar) {
    VUBar::init(aNeoPixelPtr->numPixels(), aDampingFactorForValue,
            aFlags | FLAG_VU_NEOPIXEL_USE_BAR_COLOR_ARRAY | FLAG_VU_NEOPIXEL_USE_BAR_16_BIT_COLOR_ARRAY, aValueForFullBar);

    Pin = aPin;
    NeoPixelBar = aNeoPixelPtr;
    BarColorUnion.BarColor16ArrayPtr = aBarColor16ArrayPtr;
}
void NeoPixelVU::init(uint8_t aPin, NeoPixel *aNeoPixelPtr, uint8_t aDampingFactorForValue, uint8_t aFlags, color32_t aBarColor) {
    VUBar::init(aNeoPixelPtr->numPixels(), aDampingFactorForValue, aFlags);

    Pin = aPin;
    NeoPixelBar = aNeoPixelPtr;
    BarColorUnion.BarColor = aBarColor;
}
void NeoPixelVU::init(uint8_t aPin, NeoPixel *aNeoPixelPtr, uint8_t aDampingFactorForValue, uint8_t aFlags, color32_t aBarColor,
        uint16_t aValueForFullBar) {
    VUBar::init(aNeoPixelPtr->numPixels(), aDampingFactorForValue, aFlags, aValueForFullBar);

    Pin = aPin;
    NeoPixelBar = aNeoPixelPtr;
    BarColorUnion.BarColor = aBarColor;
}

/*
 * Since we can use one NeoPixel object for all NeoPixelVU Objects we must switch pin before showing the result
 */
void NeoPixelVU::show() {
    if (NeoPixelBar != nullptr) {
#if defined(LOCAL_TRACE)
        Serial.print(NeoPixelBar->getPin());
        Serial.print(F("->"));
        Serial.println(Pin);
#endif
        NeoPixelBar->setPin(Pin);
        NeoPixelBar->show();
    }
}

/*
 * Set peak pixel
 * @param aColor - if color is COLOR32_BLACK, then use Red Yellow Green color schema
 */
void NeoPixelVU::drawPeak(color32_t aColor) {
    if (NeoPixelBar != nullptr) {
//        uint16_t tBarLength = BarLength; // Using local variables does NOT save program memory :-(
//        uint8_t tPeakValue = PeakValue;
        /*
         * If PeakValue == 0, do not render peak
         */
        if (PeakValue > 0 && PeakValue <= BarLength) {
            if (aColor != COLOR32_PEAK_RYG) {
                // show color peak
                NeoPixelBar->setPixelColor(PeakValue - 1, aColor);
            } else {
                // using Red Yellow Green color schema
                // with < 1/2 green, then 1/4 yellow and 1/4 red
                if (PeakValue <= (BarLength + 1) / 2) {
                    NeoPixelBar->setPixelColor(PeakValue - 1, COLOR32_GREEN);
                } else if (PeakValue <= (BarLength - (BarLength / 4))) {
                    NeoPixelBar->setPixelColor(PeakValue - 1, COLOR32_YELLOW);
                } else {
                    NeoPixelBar->setPixelColor(PeakValue - 1, COLOR32_RED);
                }
            }
        }
    }
}

void NeoPixelVU::setBarAndPeakAndShow(uint16_t aInputValue) {
    if (NeoPixelBar != nullptr) {
#if defined(LOCAL_DEBUG)
        NeoPixelBar->printPin(&Serial);
#endif
        uint8_t tSignalHeight = computeNewSignalHeight(aInputValue);
        if (!(Flags & FLAG_VU_NEOPIXEL_USE_BAR_COLOR_ARRAY)) {
            NeoPixelBar->drawBar(tSignalHeight, BarColorUnion.BarColor);
        } else if (Flags & FLAG_VU_NEOPIXEL_USE_BAR_16_BIT_COLOR_ARRAY) {
            NeoPixelBar->drawBarFromColorArray(tSignalHeight, BarColorUnion.BarColor16ArrayPtr);
        } else {
            NeoPixelBar->drawBarFromColorArray(tSignalHeight, BarColorUnion.BarColorArrayPtr);
        }
        computeNewPeakValue(tSignalHeight);
        drawPeak(PEAK_PIXEL_COLOR);
        show();
    }
}

/*
 * Initialize bar color array with < 1/2 green, then yellow and 1/4 in red
 */
void NeoPixelVU::initBarColorRYG() {
    if (has32BitColorArray()) {
        NeoPixelVU::initBarColorRYG(BarLength, BarColorUnion.BarColorArrayPtr);
    }
}

/*
 * set bar colors from Wheel(88)/green to Wheel(0)/red
 */
void NeoPixelVU::initBarGreenToRed() {
    if (has32BitColorArray()) {
        NeoPixelVU::initBarGreenToRed(BarLength, BarColorUnion.BarColorArrayPtr);
    }
}

void NeoPixelVU::initBarColorWheel() {
    if (has32BitColorArray()) {
        NeoPixelVU::initBarColorWheel(BarLength, BarColorUnion.BarColorArrayPtr);
    }
}
/*
 * 16 bit color versions
 */
void NeoPixelVU::initBarColorRYG16() {
    if (has16BitColorArray()) {
        NeoPixelVU::initBarColorRYG16(BarLength, BarColorUnion.BarColor16ArrayPtr);
    }
}
void NeoPixelVU::initBarGreenToRed16() {
    if (has16BitColorArray()) {
        NeoPixelVU::initBarGreenToRed16(BarLength, BarColorUnion.BarColor16ArrayPtr);
    }
}
void NeoPixelVU::initBarColorWheel16() {
    if (has16BitColorArray()) {
        NeoPixelVU::initBarColorWheel16(BarLength, BarColorUnion.BarColor16ArrayPtr);
    }
}

/*
 * Initialize bar color array with < 1/2 green, then 1/4 yellow and 1/4 red
 */
void NeoPixelVU::initBarColorRYG(uint8_t aBarLength, color32_t *aBarColorArrayPtr) {
    uint8_t i;
    for (i = 0; i < (aBarLength + 1) / 2; ++i) {
        *aBarColorArrayPtr++ = COLOR32_GREEN;
    }
    for (; i < aBarLength - (aBarLength / 4); ++i) {
        *aBarColorArrayPtr++ = COLOR32_YELLOW;
    }
    for (; i < aBarLength; ++i) {
        *aBarColorArrayPtr++ = COLOR32_RED;
    }
}

void NeoPixelVU::initBarColorRYG16(uint8_t aBarLength, color16_t *aBarColor16ArrayPtr) {
    uint8_t i;
    for (i = 0; i < (aBarLength + 1) / 2; ++i) {
        *aBarColor16ArrayPtr++ = COLOR16_GREEN;
    }
    for (; i < aBarLength - (aBarLength / 4); ++i) {
        *aBarColor16ArrayPtr++ = COLOR16_YELLOW;
    }
    for (; i < aBarLength; ++i) {
        *aBarColor16ArrayPtr++ = COLOR16_RED;
    }
}
/*
 * set bar colors from Wheel(88)/green to Wheel(0)/red
 */
void NeoPixelVU::initBarGreenToRed(uint8_t aBarLength, color32_t *aBarColorArrayPtr) {
    for (uint_fast8_t i = 0; i < aBarLength; ++i) {
        *aBarColorArrayPtr++ = NeoPixel::Wheel(map(i, 0, aBarLength - 1, 88, 0));
    }
}
void NeoPixelVU::initBarGreenToRed16(uint8_t aBarLength, color16_t *aBarColor16ArrayPtr) {
    for (uint_fast8_t i = 0; i < aBarLength; ++i) {
        *aBarColor16ArrayPtr++ = Color32ToColor16(NeoPixel::Wheel(map(i, 0, aBarLength - 1, 88, 0)));
    }
}

void NeoPixelVU::initBarColorWheel(uint8_t aBarLength, color32_t *aBarColorArrayPtr) {
    for (uint_fast8_t i = 0; i < aBarLength; ++i) {
        *aBarColorArrayPtr++ = NeoPixel::Wheel(map(i, 0, aBarLength - 1, 0, 255));
    }
}
void NeoPixelVU::initBarColorWheel16(uint8_t aBarLength, color16_t *aBarColor16ArrayPtr) {
    for (uint_fast8_t i = 0; i < aBarLength; ++i) {
        *aBarColor16ArrayPtr++ = Color32ToColor16(NeoPixel::Wheel(map(i, 0, aBarLength - 1, 0, 255)));
    }
}

#include "LocalDebugLevelEnd.h"
#endif // _NEOPIXEL_VU_HPP
