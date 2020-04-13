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

/*
 * Class inheritance diagram. We have virtual inheritance, since MatrixNeoPatterns needs only one member set of NeoPixel
 *
 *                    MatrixNeoPixelVU \
 *                                     ,o--> MatrixNeoPixel (virtual) \
 * MatrixSnake --> MatrixNeoPatterns  <                                o--> NeoPixel --> Adafruit_NeoPixel
 *                                     `o--> NeoPatterns    (virtual) /
 *
 */

#ifndef SRC_LIB_NEOPATTERNS_NEOPIXEL_H_
#define SRC_LIB_NEOPATTERNS_NEOPIXEL_H_

#include "Adafruit_NeoPixel.h"
#include "Colors.h"


#if ! defined(DO_NOT_SUPPORT_RGBW)
// Support rgbw colors for pattern. Requires additional program space.
// Comment this out, if you do NOT need RGBW support and want to save program space.
#define SUPPORT_RGBW
#endif

#ifdef SUPPORT_RGBW
uint8_t White(color32_t color);
#endif
uint8_t Red(color32_t color);
uint8_t Green(color32_t color);
uint8_t Blue(color32_t color);

class NeoPixel: public Adafruit_NeoPixel {
public:
    NeoPixel();
    NeoPixel(uint16_t aNumberOfPixels, uint8_t aPin, uint8_t aTypeOfPixel);
    void AdafruitNeoPixelIinit(uint16_t aNumberOfPixels, uint16_t aPin, neoPixelType aTypeOfPixel);
    bool init(uint16_t aNumberOfPixels, uint8_t aPin, uint8_t aTypeOfPixel);
    NeoPixel(NeoPixel * aUnderlyingNeoPixelObject, uint16_t aPixelOffset, uint16_t aNumberOfPixels,
            bool aEnableShowOfUnderlyingPixel = true);
    void init(NeoPixel * aUnderlyingNeoPixelObject, uint16_t aPixelOffset, uint16_t aNumberOfPixels,
                bool aEnableShowOfUnderlyingPixel = true);

    void printInfo(Print * aSerial);

    // To enable more than one pattern on the same strip
    void setPixelBuffer(uint8_t * aNewPixelBufferPointer);

    // To move the start index of a NeoPixel object
    void setPixelOffsetForPartialNeoPixel( uint16_t aPixelOffset);
    /*
     * Extensions to Adafruit_NeoPixel functions
     */
    void begin();
    void show();
    // Version with error message
    bool begin(Print * aSerial);

    void ColorSet(color32_t aColor);
    color32_t getPixelColor(uint16_t aPixelIndex);
    uint32_t dimColor(color32_t aColor);
    void resetBrightnessValue();
    uint8_t getBytesPerPixel();
    neoPixelType getType();
    uint16_t getPixelBufferSize();
    void storePixelBuffer(uint8_t * aPixelBufferPointerDestination);
    void restorePixelBuffer(uint8_t * aPixelBufferPointerSource);

    // Functions to support PixelOffset
    void clear(void);
    void clearPixel(uint16_t aPixelIndex);
    void setPixelColor(uint16_t aPixelIndex, uint8_t aRed, uint8_t aGreen, uint8_t aBlue);
#ifdef SUPPORT_RGBW
    void setPixelColor(uint16_t aPixelIndex, uint8_t aRed, uint8_t aGreen, uint8_t aBlue, uint8_t aWhite);
#endif
    void setPixelColor(uint16_t aPixelIndex, uint32_t aColor);
    void fillWithRainbow(uint8_t aWheelStartPos, bool aStartAtTop = false);
    void drawBar(uint16_t aBarLength, color32_t aColor, bool aDrawFromBottom = true);
    void drawBarFromColorArray(uint16_t aBarLength, color32_t * aColorArrayPtr, bool aDrawFromBottom = true);

    void addPixelColor(uint16_t aPixelIndex, uint8_t aRed, uint8_t aGreen, uint8_t aBlue);
    // Static functions
    static color32_t Wheel(uint8_t aWheelPos);
    static uint8_t gamma32(uint8_t aLinearBrightnessValue);
    static uint8_t gamma32WithSpecialZero(uint8_t aLinearBrightnessValue);
    static color32_t convertLinearToGamma32Color(color32_t aLinearBrightnessColor);
    static color32_t dimColorWithGamma32(color32_t aLinearBrightnessColor, uint8_t aBrightness, bool doSpecialZero = false);

    void TestWS2812Resolution();

#ifdef SUPPORT_RGBW
    uint8_t BytesPerPixel;  // can be 3 or 4
#else
#define BytesPerPixel 3
#endif
    uint8_t PixelFlags;
    uint16_t PixelOffset; // The offset of the pattern on the underlying pixel buffer to enable partial patterns overlays
    NeoPixel * UnderlyingNeoPixelObject; // The underlying NeoPixel for partial patterns overlays
};

#define PIXEL_FLAG_IS_PARTIAL_NEOPIXEL                       0x01 // enables partial patterns overlays and uses show() of UnderlyingNeoPixelObject
#define PIXEL_FLAG_DISABLE_SHOW_OF_UNDERLYING_PIXEL_OBJECT   0x02 // use negative logic because evaluation is simpler then
/*
 * Flag for NeoPattern. This disables the initial asynchronous show() for a new pattern, but enables show() if called by synchronous callback.
 * This behavior is needed to avoid disturbing other libraries, which cannot handle the time when interrupt is disabled for show() e.g. the Servo library.
 * The asynchronous call is detected by checking if the current pattern is not PATTERN_NONE.
 */
#define PIXEL_FLAG_SHOW_ONLY_AT_UPDATE                       0x04
// Used for some demo handler
#define PIXEL_FLAG_GEOMETRY_CIRCLE                           0x80 // in contrast to bar

extern const uint8_t GammaTable32[32] PROGMEM;

#endif /* SRC_LIB_NEOPATTERNS_NEOPIXEL_H_ */

#pragma once
