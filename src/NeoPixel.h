/*
 * NeoPixel.h
 *
 * Implements extensions to Adafruit_NeoPixel functions
 *
 *  Copyright (C) 2019-2022  Armin Joachimsmeyer
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
 *                    MatrixNeoPixelVU \
 *                                     ,o--> MatrixNeoPixel (virtual) \
 * MatrixSnake --> MatrixNeoPatterns  <                                o--> NeoPixel --> Adafruit_NeoPixel
 *                                     `o--> NeoPatterns    (virtual) /
 *
 */

#ifndef _NEOPATTERNS_NEOPIXEL_H
#define _NEOPATTERNS_NEOPIXEL_H

// To support various debug levels set in different sources
#include "DebugLevel.h"

// This does not work in Arduino IDE for "Generating function prototypes..."
//#if ! __has_include("Adafruit_NeoPixel.h")
//#error This NeoPixel library requires the "Adafruit NeoPixel" library. Please install it via the Arduino library manager.
//#endif

#include "Adafruit_NeoPixel.h" // Click here to get the library: http://librarymanager/All#Adafruit_NeoPixel
#include "Colors.h"

//#define DO_NOT_SUPPORT_RGBW // saves up to 428 bytes additional program memory for the AllPatternsOnMultiDevices() example.
#if !defined(DO_NOT_SUPPORT_RGBW)
// Support rgbw colors for pattern.
// Deactivate this, if you do NOT need RGBW support and want to save program memory (max 300 bytes).
#define _SUPPORT_RGBW // Introduced to avoid double negations below using #if ! defined(DO_NOT_SUPPORT_RGBW)
#endif

//#define DO_NOT_SUPPORT_BRIGHTNESS // saves up to 428 bytes additional program memory for the AllPatternsOnMultiDevices() example.
#if !defined(DO_NOT_SUPPORT_BRIGHTNESS)
#define SUPPORT_BRIGHTNESS // Introduced to avoid double negations

//#define DO_NOT_SUPPORT_NO_ZERO_BRIGHTNESS // If activated, disables writing of zero only if brightness or color is zero. Saves up to 144 bytes ...
#  if !defined(DO_NOT_SUPPORT_NO_ZERO_BRIGHTNESS)
// // Pixel is set to zero, only if brightness or input color is zero, otherwise it is clipped at e.g. 0x000100
#define SUPPORT_NO_ZERO_BRIGHTNESS // Introduced to avoid double negations
#  endif
#endif
#define MAX_BRIGHTNESS  0xFF // is internally stored as 0

#if defined(_SUPPORT_RGBW)
uint8_t getWhitePart(color32_t color);
uint8_t White(color32_t color) __attribute__ ((deprecated ("Renamed to getWhitePart()"))); // deprecated
#endif
uint8_t getRedPart(color32_t color);
uint8_t getGreenPart(color32_t color);
uint8_t getBluePart(color32_t color);

uint8_t Red(color32_t color) __attribute__ ((deprecated ("Renamed to getRedPart()"))); // deprecated
uint8_t Green(color32_t color) __attribute__ ((deprecated ("Renamed to getGreenPart()"))); // deprecated
uint8_t Blue(color32_t color) __attribute__ ((deprecated ("Renamed to getBluePart()"))); // deprecated

class NeoPixel: public Adafruit_NeoPixel {
public:
    NeoPixel();
    NeoPixel(uint16_t aNumberOfPixels, uint8_t aPin, neoPixelType aTypeOfPixel);
    void AdafruitNeoPixelIinit(uint16_t aNumberOfPixels, uint16_t aPin, neoPixelType aTypeOfPixel);
    bool init(uint16_t aNumberOfPixels, uint8_t aPin, neoPixelType aTypeOfPixel);
    NeoPixel(NeoPixel *aUnderlyingNeoPixelObject, uint16_t aPixelOffset, uint16_t aNumberOfPixels,
            bool aEnableShowOfUnderlyingPixel = true);
    void init(NeoPixel *aUnderlyingNeoPixelObject, uint16_t aPixelOffset, uint16_t aNumberOfPixels,
            bool aEnableShowOfUnderlyingPixel = true);

    void printInfo(Print *aSerial);
    void printContent(Print *aSerial);
    void printConnectionInfo(Print *aSerial);
    void printPin(Print *aSerial);

    // To enable more than one pattern on the same strip
    void setPixelBuffer(uint8_t *aNewPixelBufferPointer);

    // To move the start index of a NeoPixel object
    void setPixelOffsetForPartialNeoPixel(uint16_t aPixelOffset);
    /*
     * Extensions to Adafruit_NeoPixel functions
     */
    void begin();
    void begin(uint8_t aBrightness, bool aEnableBrightnessNonZeroMode = false);
    void show();
    // Version with error message
    bool begin(Print *aSerial);
    bool begin(Print *aSerial, uint8_t aBrightness, bool aEnableBrightnessNonZeroMode = false);

    void ColorSet(color32_t aColor) __attribute__ ((deprecated ("Renamed to setColor()"))); // deprecated;
    void setColor(color32_t aColor);
    color32_t getPixelColor(uint16_t aPixelIndex);
    uint8_t getBytesPerPixel();
    neoPixelType getType();
    uint16_t getPixelBufferSize();
    uint16_t getNumberOfPixels();
    void storePixelBuffer(uint8_t *aPixelBufferPointerDestination);
    void restorePixelBuffer(uint8_t *aPixelBufferPointerSource);

    // Functions to support PixelOffset
    void clear(void);
    void clearPixel(uint16_t aPixelIndex);
    void setPixelColor(uint16_t aPixelIndex, uint8_t aRed, uint8_t aGreen, uint8_t aBlue);
#if defined(_SUPPORT_RGBW)
    void setPixelColor(uint16_t aPixelIndex, uint8_t aRed, uint8_t aGreen, uint8_t aBlue, uint8_t aWhite);
#endif
    void setPixelColor(uint16_t aPixelIndex, color32_t aColor);
    void setBrightnessValue(uint8_t aBrightness);           // Sets the brightness used by Neopixel drawing functions
    void setAdafruitBrightnessValue(uint8_t aBrightness);   // Convenience function to set the brightness used by the (unused) Adafruit drawing functions
    void setBrightnessNonZeroMode(bool aEnableBrightnessNonZeroMode);
    void fillWithRainbow(uint8_t aWheelStartPos, bool aStartAtTop = false);
    void drawBar(uint16_t aBarLength, color32_t aColor, bool aDrawFromBottom = true);
    void fillRegion(color32_t aColor, uint16_t aRegionFirst, uint16_t aRegionLength);
    void drawBarFromColorArray(uint16_t aBarLength, color32_t *aColorArrayPtr, bool aDrawFromBottom = true);

    void addPixelColor(uint16_t aPixelIndex, uint8_t aRed, uint8_t aGreen, uint8_t aBlue);
    void addPixelColor(uint16_t aPixelIndex, color32_t aColor);
    // Static functions
    static uint32_t dimColor(color32_t aColor);
    void dimPixelColor(uint16_t aPixelIndex);
    static color32_t Wheel(uint8_t aWheelPos);
    static uint8_t gamma5(uint8_t aLinearBrightnessValue);
    static uint8_t gamma5WithSpecialZero(uint8_t aLinearBrightnessValue);
    static color32_t convertLinearToGamma5Color(color32_t aLinearBrightnessColor);
    static color32_t dimColorWithGamma5(color32_t aLinearBrightnessColor, uint8_t aBrightness, bool doSpecialZero = false);

    // deprecated
    static uint8_t gamma32(uint8_t aLinearBrightnessValue) __attribute__ ((deprecated ("Renamed to gamma5().")));
    static color32_t dimColorWithGamma32(color32_t aLinearBrightnessColor, uint8_t aBrightness, bool doSpecialZero = false)
            __attribute__ ((deprecated ("Renamed to dimColorWithGamma5().")));

    void TestWS2812Resolution();

#if defined(_SUPPORT_RGBW)
    uint8_t BytesPerPixel;  // can be 3 or 4
#else
#define BytesPerPixel 3
#endif
    uint8_t PixelFlags;
    uint16_t PixelOffset;               // The offset of the pattern on the underlying pixel buffer to enable partial patterns overlays
    NeoPixel *UnderlyingNeoPixelObject; // The underlying NeoPixel object for partial patterns overlays, otherwise the object itself
    uint8_t Brightness;                 // NeoPixel brightness instead of the Adafruit brightness, which is store as effective brightness + 1 :-(.
};

#define PIXEL_FLAG_IS_PARTIAL_NEOPIXEL                       0x01 // enables partial patterns overlays and uses show() of UnderlyingNeoPixelObject
#define PIXEL_FLAG_DISABLE_SHOW_OF_UNDERLYING_PIXEL_OBJECT   0x02 // use negative logic because evaluation is simpler then
/*
 * Flag for NeoPattern. This disables the initial asynchronous show() for a new pattern, but enables show() if called by synchronous callback.
 * This behavior is required to avoid disturbing other libraries, which cannot handle the time when interrupt is disabled for show() e.g. the Servo library.
 * The asynchronous call is detected by checking if the current pattern is not PATTERN_NONE.
 */
#define PIXEL_FLAG_SHOW_ONLY_AT_UPDATE                       0x04
#define PIXEL_FLAG_USE_NON_ZERO_BRIGHTNESS                   0x08 // Pixel is set to zero, only if brightness or input color is zero, otherwise it is clipped at e.g. 0x000100
// Used for some demo handler
#define PIXEL_FLAG_GEOMETRY_CIRCLE                           0x80 // in contrast to bar

extern const uint8_t GammaTable32[32] PROGMEM;

#endif /* _NEOPATTERNS_NEOPIXEL_H */
