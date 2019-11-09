/*
 * NeoPixel.cpp
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

#include "NeoPixel.h"

//#define TRACE

NeoPixel::NeoPixel(uint16_t aNumberOfPixels, uint8_t aPin, uint8_t aTypeOfPixel) : // @suppress("Class members should be properly initialized")
        Adafruit_NeoPixel(aNumberOfPixels, aPin, aTypeOfPixel) {
    BytesPerPixel = ((wOffset == rOffset) ? 3 : 4);
    /* not really needed
     PixelOffset = 0;  // 8 byte Flash
     UnderlyingNeoPixelObject = NULL;
     PixelFlags = 0;
     */
}

/*
 * Used to create a NeoPixel, which operates on a segment of the underlying NeoPixel object.
 * This creates a new Adafruit_NeoPixel object and replaces the new pixel buffer with the underlying existing one.
 * ATTENTION!
 * @param aShowAllPixel - true = calls show function of the existing NeoPixel object (compatibility mode)
 *                        false = suppress calling the show function, since it makes no sense because show() would
 *                                set only the FIRST aNumberOfPixels of the underlying NeoPixelObject
 * ATTENTION. To save a lot of CPU time set aShowAllPixel to false and use only ExistingNeoPixelObject->show().
 * if(PartialNeoPixelBar.update()){
 *   UnderlyingNeoPixelObject->show();
 * }
 */
NeoPixel::NeoPixel(NeoPixel * aUnderlyingNeoPixelObject, uint16_t aPixelOffset, uint16_t aNumberOfPixels,
        bool aEnableShowOfUnderlyingPixel) :
        Adafruit_NeoPixel(aNumberOfPixels, aUnderlyingNeoPixelObject->getPin(), aUnderlyingNeoPixelObject->getType()) {
    UnderlyingNeoPixelObject = aUnderlyingNeoPixelObject;
    BytesPerPixel = aUnderlyingNeoPixelObject->BytesPerPixel;
    PixelOffset = aPixelOffset;
    PixelFlags = PIXEL_FLAG_IS_PARTIAL_PIXEL;
    if (!aEnableShowOfUnderlyingPixel) {
        PixelFlags = PIXEL_FLAG_IS_PARTIAL_PIXEL | PIXEL_FLAG_DISABLE_SHOW_OF_UNDERLYING_PIXEL_OBJECT;
    }
    /*
     * Replace buffer with existing one
     */
    setPixelBuffer(aUnderlyingNeoPixelObject->getPixels());
}

void NeoPixel::begin() {
    Adafruit_NeoPixel::begin();
}

/*
 * begin function, which prints error message if aSerial is not NULL
 * Returns false if no memory available
 */
bool NeoPixel::begin(Stream * aSerial) {
    Adafruit_NeoPixel::begin();
    if (numLEDs == 0) {
        if (aSerial != NULL) {
            aSerial->print(F("ERROR Not enough free memory available for Pattern at pin "));
            aSerial->println(getPin());
        }
        return false;
    }
    return true;
}

/*
 * Handles the DISABLE_SHOW_OF_UNDERLYING_PIXEL_OBJECT flag
 */
void NeoPixel::show() {
    if ((PixelFlags & PIXEL_FLAG_DISABLE_SHOW_OF_UNDERLYING_PIXEL_OBJECT) == 0) {
        if (PixelFlags & PIXEL_FLAG_IS_PARTIAL_PIXEL) {
#ifdef TRACE
            Serial.println("Underlying->show");
#endif
            UnderlyingNeoPixelObject->Adafruit_NeoPixel::show();
        } else {
#ifdef TRACE
            Serial.println("show");
#endif
            Adafruit_NeoPixel::show();
        }
    }
}

uint8_t NeoPixel::getBytesPerPixel() {
    return BytesPerPixel;
}

neoPixelType NeoPixel::getType() {
#ifdef NEO_KHZ400
    neoPixelType tReturnValue = (wOffset << 6 | rOffset << 4 | gOffset << 2 | bOffset);
    if (is800KHz) {
        tReturnValue |= 0x100;
    }
#else
    neoPixelType tReturnValue=(wOffset << 6 | rOffset << 4 | gOffset << 2 | bOffset);
#endif
    return tReturnValue;
}

/*
 *
 * Free old pixel buffer and set new value.
 * Needed if you want to have more than one patterns on the same strip.
 */
void NeoPixel::setPixelBuffer(uint8_t * aNewPixelBufferPointer) {
    if (pixels) {
        free(pixels);
    }
    pixels = aNewPixelBufferPointer;
}

uint16_t NeoPixel::getPixelBufferSize() {
    return numBytes;
}

void NeoPixel::storePixelBuffer(uint8_t * aPixelBufferPointerDestination) {
    memcpy(aPixelBufferPointerDestination, pixels, numBytes);
}

void NeoPixel::restorePixelBuffer(uint8_t * aPixelBufferPointerSource) {
    memcpy(pixels, aPixelBufferPointerSource, numBytes);
}

void NeoPixel::clear(void) {
    memset(pixels + (BytesPerPixel * PixelOffset), 0, numBytes);
}

/*
 * @param aDrawFromBottom - false: Bar is top down, i.e. it starts at the highest pixel index
 */
void NeoPixel::drawBar(uint16_t aBarLength, color32_t aColor, bool aDrawFromBottom) {
    for (uint16_t i = 0; i < numLEDs; i++) {
        bool tDrawPixel;
        if (aDrawFromBottom) {
            tDrawPixel = (i < aBarLength);
        } else {
            tDrawPixel = (i >= (numLEDs - aBarLength));
        }
        if (tDrawPixel) {
            setPixelColor(i, aColor);
        } else {
            // Clear pixel
            setPixelColor(i, 0, 0, 0);
        }
    }
}

/*
 * @param aColorArrayPtr - Address of a color array holding numLEDs color entries for the bar colors.
 * @param aDrawFromBottom - false: Bar is top down, i.e. it starts at the highest pixel index
 */
void NeoPixel::drawBarFromColorArray(uint16_t aBarLength, color32_t * aColorArrayPtr, bool aDrawFromBottom) {
    for (uint16_t i = 0; i < numLEDs; i++) {
        bool tDrawPixel;
        if (aDrawFromBottom) {
            tDrawPixel = (i < aBarLength);
        } else {
            tDrawPixel = (i >= (numLEDs - aBarLength));
        }
        if (tDrawPixel) {
            if (aDrawFromBottom) {
                setPixelColor(i, aColorArrayPtr[(numLEDs - 1) - i]);
            } else {
                setPixelColor(i, aColorArrayPtr[i]);
            }
            setPixelColor(i, aColorArrayPtr[i]);
        } else {
            // Clear pixel
            clearPixel(i);
        }
    }
}

/*
 * Needs 50 bytes FLASH, but is faster
 */
void NeoPixel::clearPixel(uint16_t aPixelIndex) {
    if (aPixelIndex < numLEDs) {
        aPixelIndex += PixelOffset; // added line to support offsets
    }
    uint8_t * tPixelPtr = &pixels[aPixelIndex * BytesPerPixel];
    *tPixelPtr++ = 0;
    *tPixelPtr++ = 0;
    *tPixelPtr++ = 0;
    if (BytesPerPixel == 4) {
        *tPixelPtr = 0;
    }
}

/*
 * Checks for valid index / skips invalid ones
 */
void NeoPixel::setPixelColor(uint16_t aPixelIndex, uint8_t aRed, uint8_t aGreen, uint8_t aBlue) {
#ifdef TRACE
    Serial.print(F("Pixel="));
    Serial.print(aPixelIndex);
    Serial.print(F(" Offset="));
    Serial.print(PixelOffset);
    Serial.print(F(" Color="));
    Serial.print(aRed);
    Serial.print('|');
    Serial.print(aGreen);
    Serial.print('|');
    Serial.println(aBlue);
#endif
    if (aPixelIndex < numLEDs) {
        aPixelIndex += PixelOffset; // added line to support offsets
        uint8_t *p = &pixels[aPixelIndex * BytesPerPixel];
        if (BytesPerPixel == 4) {
            p[wOffset] = 0;        // But only R,G,B passed -- set W to 0
        }
        p[rOffset] = aRed;          // R,G,B always stored
        p[gOffset] = aGreen;
        p[bOffset] = aBlue;
    }
}

void NeoPixel::setPixelColor(uint16_t aPixelIndex, uint8_t aRed, uint8_t aGreen, uint8_t aBlue, uint8_t aWhite) {
#ifdef TRACE
    Serial.print(F("Pixel="));
    Serial.print(aPixelIndex);
    Serial.print(F(" Offset="));
    Serial.print(PixelOffset);
    Serial.print(F(" Color="));
    Serial.print(aRed);
    Serial.print('|');
    Serial.print(aGreen);
    Serial.print('|');
    Serial.print(aBlue);
    Serial.println('|');
#endif
    if (aPixelIndex < numLEDs) {
        aPixelIndex += PixelOffset; // added line to support offsets
        uint8_t *p = &pixels[aPixelIndex * BytesPerPixel];
        if (BytesPerPixel == 4) {
            p[wOffset] = aWhite;        // Store W
        }
        p[rOffset] = aRed;          // Store R,G,B
        p[gOffset] = aGreen;
        p[bOffset] = aBlue;
    }
}

/*
 *
 */
void NeoPixel::setPixelColor(uint16_t aPixelIndex, uint32_t aColor) {
#ifdef TRACE
    Serial.print(F("Pixel="));
    Serial.print(aPixelIndex);
    Serial.print(F(" Offset="));
    Serial.print(PixelOffset);
    Serial.print(F(" Color=0x"));
    Serial.println(aColor, HEX);
#endif
    if (aPixelIndex < numLEDs) {
        aPixelIndex += PixelOffset; // added line to support offsets
        uint8_t *p, r = (uint8_t) (aColor >> 16), g = (uint8_t) (aColor >> 8), b = (uint8_t) aColor;
        p = &pixels[aPixelIndex * BytesPerPixel];
        if (BytesPerPixel == 4) {
            uint8_t w = (uint8_t) (aColor >> 24);
            p[wOffset] = w;
        }
        p[rOffset] = r;
        p[gOffset] = g;
        p[bOffset] = b;
    }
}

/*
 * adds color to existing one and clip to white (255)
 */
color32_t NeoPixel::addPixelColor(uint16_t aPixelIndex, uint8_t aRed, uint8_t aGreen, uint8_t aBlue) {
    color32_t tOldColor = getPixelColor(aPixelIndex);
    if (tOldColor != 0) {
        uint8_t tRed = Red(tOldColor) + aRed;
        if (tRed < aRed) {
            // clip overflow
            tRed = 255;
        }
        uint8_t tGreen = Green(tOldColor) + aGreen;
        if (tGreen < aGreen) {
            tGreen = 255;
        }
        uint8_t tBlue = Blue(tOldColor) + aBlue;
        if (tBlue < aBlue) {
            tBlue = 255;
        }
        return Color(tRed, tGreen, tBlue);
    }
    return Color(aRed, aGreen, aBlue);
}

// Set all pixels to a color (synchronously)
void NeoPixel::ColorSet(color32_t aColor) {
    // This is faster but costs 82 bytes program space
//    if (BytesPerPixel == 3) {
//        setPixelColor(0, aColor);
//        memcpy(&pixels[(PixelOffset + 1) * 3], &pixels[PixelOffset * 3], (numLEDs - 1) * 3);
//    } else {
    for (uint16_t i = 0; i < numLEDs; i++) {
        setPixelColor(i, aColor);
    }
//    }
}

color32_t NeoPixel::getPixelColor(uint16_t aPixelIndex) {
    uint8_t * tPixelPointer = &pixels[(aPixelIndex + PixelOffset) * 3];
    if (BytesPerPixel == 3) {
        return (uint32_t) tPixelPointer[rOffset] << 16 | (uint32_t) tPixelPointer[gOffset] << 8 | tPixelPointer[bOffset];
    } else {
        return (uint32_t) tPixelPointer[wOffset] << 24 | (uint32_t) tPixelPointer[rOffset] << 16 | tPixelPointer[gOffset] << 8
                | tPixelPointer[bOffset];
    }
}

// Calculate 50% dimmed version of a color
uint32_t NeoPixel::dimColor(color32_t aColor) {
// Shift R, G and B components one bit to the right
    uint32_t dimColor = Color(Red(aColor) >> 1, Green(aColor) >> 1, Blue(aColor) >> 1);
    return dimColor;
}

// Input a value 0 to 255 to get a color value.
// The colors are a transition r - g - b - back to r.
color32_t NeoPixel::Wheel(uint8_t aWheelPos) {
    aWheelPos = 255 - aWheelPos;
    if (aWheelPos < 85) {
        return Color(255 - (aWheelPos * 3), 0, aWheelPos * 3);
    } else if (aWheelPos < 170) {
        aWheelPos -= 85;
        return Color(0, aWheelPos * 3, 255 - (aWheelPos * 3));
    } else {
        aWheelPos -= 170;
        return Color(aWheelPos * 3, 255 - (aWheelPos * 3), 0);
    }
}

// from https://www.mikrocontroller.net/articles/LED-Fading
const uint8_t GammaTable32[32] PROGMEM = { 0, 1, 2, 2, 2, 3, 3, 4, 5, 6, 7, 8, 10, 11, 13, 16, 19, 23, 27, 32, 38, 45, 54, 64, 76,
        91, 108, 128, 152, 181, 215, 255 };

/*
 * use mapping table with 32 entries (using 5 MSbits)
 * @param aLinearBrightnessValue - from 0 to 255
 */
uint8_t NeoPixel::gamma32(uint8_t aLinearBrightnessValue) {
    return pgm_read_byte(&GammaTable32[(aLinearBrightnessValue / 8)]);
}

/*
 * Returns only 0 if value is 0.
 * Returns 1 for input 1 to 7.
 * used for snake tail, not to blank out the last elements of a tail with more than 32 elements
 */
uint8_t NeoPixel::gamma32WithSpecialZero(uint8_t aLinearBrightnessValue) {
    if (aLinearBrightnessValue <= 7 && aLinearBrightnessValue >= 1) {
        return 1;
    }
    return pgm_read_byte(&GammaTable32[(aLinearBrightnessValue / 8)]);
}

/*
 * Using gamma table with 32 entries
 */
color32_t NeoPixel::convertLinearToGamma32Color(color32_t aLinearBrightnessColor) {
    uint8_t tRed = pgm_read_byte(&GammaTable32[(Red(aLinearBrightnessColor) / 8)]);
    uint8_t tGreen = pgm_read_byte(&GammaTable32[(Green(aLinearBrightnessColor) / 8)]);
    uint8_t tBlue = pgm_read_byte(&GammaTable32[(Blue(aLinearBrightnessColor) / 8)]);
    return Color(tRed, tGreen, tBlue);
}

/*
 * aBrightnessIndex 0 = black 255 = full
 * doSpecialZero -> tGamma32Brightness is only zero if aBrightness is zero and 1 for aBrightness 1 to 16
 */
color32_t NeoPixel::dimColorWithGamma32(color32_t aLinearBrightnessColor, uint8_t aBrightness, bool doSpecialZero) {
    uint8_t tRedDimmed = (uint8_t) (aLinearBrightnessColor >> 16);
    uint8_t tGreenDimmed = (uint8_t) (aLinearBrightnessColor >> 8);
    uint8_t tBlueDimmed = (uint8_t) aLinearBrightnessColor;

    uint8_t tGamma32Brightness;
    if (doSpecialZero) {
        tGamma32Brightness = gamma32WithSpecialZero(aBrightness);
    } else {
        tGamma32Brightness = gamma32(aBrightness);
    }

    // (tRedDimmed + 1) since (255 * 1) >> 8 gives 0 (and not 1)
    tRedDimmed = ((tRedDimmed + 1) * tGamma32Brightness) >> 8;
    tGreenDimmed = ((tGreenDimmed + 1) * tGamma32Brightness) >> 8;
    tBlueDimmed = ((tBlueDimmed + 1) * tGamma32Brightness) >> 8;

#ifdef TRACE
    Serial.print(F("dimColorWithGamma32 aBrightness="));
    Serial.print(aBrightness);
    Serial.print(F(" Gamma="));
    Serial.print(tGamma32Brightness);
    Serial.print(F(" 0x"));
    Serial.print(aLinearBrightnessColor, HEX);
    Serial.print(F("->0x"));
    Serial.println(((uint32_t) tRedDimmed << 16) | ((uint32_t) tGreenDimmed << 8) | tBlueDimmed, HEX);
#endif

    return ((uint32_t) tRedDimmed << 16) | ((uint32_t) tGreenDimmed << 8) | tBlueDimmed;
}

// Returns the Red component of a 32-bit color
uint8_t Red(color32_t color) {
    return (color >> 16) & 0xFF;
}

// Returns the Green component of a 32-bit color
uint8_t Green(color32_t color) {
    return (color >> 8) & 0xFF;
}

// Returns the Blue component of a 32-bit color
uint8_t Blue(color32_t color) {
    return color & 0xFF;
}

/*
 * Test WS2812 resolution
 * outputs the 10 values 1,2,3,4,8,16,32,64,128,255 for every color -> 33 pixel
 */
void NeoPixel::TestWS2812Resolution() {

    /*
     * output 1 to 3 on RED
     */
    uint8_t tPosition = 0;
    for (int i = 1; i < 4; ++i) {
        setPixelColor(tPosition++, i, 0, 0);
    }

    /*
     * output 4, 8, 16 to 128,255 on RED
     */
    uint8_t tExponentialValue = 4;
    for (int i = 0; i < 6; ++i) {
        setPixelColor(tPosition++, tExponentialValue, 0, 0);
        tExponentialValue = tExponentialValue << 1;
    }
    setPixelColor(tPosition++, 255, 0, 0);

    /*
     * The same for green
     */
    for (int i = 1; i < 4; ++i) {
        setPixelColor(tPosition++, 0, i, 0);
    }
    tExponentialValue = 4;
    for (int i = 0; i < 6; ++i) {
        setPixelColor(tPosition++, 0, tExponentialValue, 0);
        tExponentialValue = tExponentialValue << 1;
    }
    setPixelColor(tPosition++, 0, 255, 0);

    /*
     * And blue
     */
    for (int i = 1; i < 4; ++i) {
        setPixelColor(tPosition++, 0, 0, i);
    }
    tExponentialValue = 4;
    for (int i = 0; i < 6; ++i) {
        setPixelColor(tPosition++, 0, 0, tExponentialValue);
        tExponentialValue = tExponentialValue << 1;
    }
    setPixelColor(tPosition++, 0, 0, 255);
    show();
}
