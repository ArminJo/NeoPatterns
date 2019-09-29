/*
 * NeoPixel.h
 *
 * Implements extensions to Adafruit_NeoPixel functions
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

/* Class inheritance diagram
 *                                     ,o--> MatrixNeoPixel \
 * MatrixSnake --> MatrixNeoPatterns  <                      o--> NeoPixel --> Adafruit_NeoPixel
 *                                     `o--> NeoPatterns    /
 */

#ifndef SRC_LIB_NEOPATTERNS_NEOPIXEL_H_
#define SRC_LIB_NEOPATTERNS_NEOPIXEL_H_

#include "Adafruit_NeoPixel.h"
#include "Colors.h"

uint8_t Red(color32_t color);
uint8_t Green(color32_t color);
uint8_t Blue(color32_t color);

class NeoPixel: public Adafruit_NeoPixel {
public:
    NeoPixel(uint16_t aNumberOfPixels, uint8_t aPin, uint8_t aTypeOfPixel);
    NeoPixel(NeoPixel * aUnderlyingNeoPixelObject, uint16_t aPixelOffset, uint16_t aNumberOfPixels, bool aEnableShowOfUnderlyingPixel = true);

    // To enable more than one pattern on the same strip
    void setPixelBuffer(uint8_t * aNewPixelBufferPointer);

    /*
     * Extensions to Adafruit_NeoPixel functions
     */
    void begin();
    // Version with error message
    bool begin(Stream * aSerial);

    void ColorSet(color32_t color);
    uint32_t DimColor(color32_t color);
    void resetBrightnessValue(); // resets internal brightness control value to full to support restoring of patterns while brightening
    uint8_t getBytesPerPixel();
    neoPixelType getType();
    uint16_t getPixelBufferSize();
    void storePixelBuffer(uint8_t * aPixelBufferPointerDestination);
    void restorePixelBuffer(uint8_t * aPixelBufferPointerSource, bool aResetBrightness = true);

    // Functions to support PixelOffset
    void clear(void);
    void setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b);
    void setPixelColor(uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w);
    void setPixelColor(uint16_t n, uint32_t c);

    color32_t addPixelColor(uint16_t aPixelIndex, uint8_t aRed, uint8_t aGreen, uint8_t aBlue);
    // Static functions
    static color32_t Wheel(uint8_t WheelPos);
    static uint8_t gamma5(uint8_t aLinearBrightnessValue);
    static uint8_t gamma5WithSpecialZero(uint8_t aLinearBrightnessValue);
    static color32_t gamma5FromColor(color32_t aLinearBrightnessColor);

    void TestWS2812Resolution();

    uint8_t BytesPerPixel;  // can be 3 or 4
#define IS_PARTIAL_PIXEL                        0x01 // enables partial patterns overlays and uses show() of UnderlyingNeoPixelObject
#define DISABLE_SHOW_OF_UNDERLYING_PIXEL_OBJECT 0x02 // use negative logic because evaluation is simpler then
    uint8_t PixelFlags;
    uint16_t PixelOffset; // The offset of the pattern on the underlying pixel buffer to enable partial patterns overlays
    NeoPixel * UnderlyingNeoPixelObject; // The underlying NeoPixel for partial patterns overlays
};

#endif /* SRC_LIB_NEOPATTERNS_NEOPIXEL_H_ */

#pragma once
