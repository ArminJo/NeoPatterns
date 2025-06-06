/*
 * NeoPixel.hpp
 *
 * Implements extensions to Adafruit_NeoPixel functions
 *
 *  Copyright (C) 2019-2025  Armin Joachimsmeyer
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
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/gpl.html>.
 *
 */

#ifndef _NEOPIXEL_HPP
#define _NEOPIXEL_HPP

// This block must be located after the includes of other *.hpp files
//#define LOCAL_INFO  // This enables info output only for this file
//#define LOCAL_TRACE // This enables trace output only for this file - only for development
#include "LocalDebugLevelStart.h"

#include "NeoPixel.h"

NeoPixel::NeoPixel() :  // @suppress("Class members should be properly initialized")
        Adafruit_NeoPixel() {

#if defined(_SUPPORT_RGBW)
    BytesPerPixel = 0;
#endif
    PixelOffset = 0;
    PixelFlags = 0;
    numBytes = 0;
    Brightness = MAX_BRIGHTNESS;
}

NeoPixel::NeoPixel(uint16_t aNumberOfPixels, uint8_t aPin, neoPixelType aTypeOfPixel) : // @suppress("Class members should be properly initialized")
        Adafruit_NeoPixel(aNumberOfPixels, aPin, aTypeOfPixel) {

#if defined(_SUPPORT_RGBW)
    BytesPerPixel = ((wOffset == rOffset) ? 3 : 4);
#endif
    PixelOffset = 0;  // 8 byte Flash
    UnderlyingNeoPixelObject = this;
    PixelFlags = 0;
    Brightness = MAX_BRIGHTNESS;
}

/*
 * Substitute for missing init() of Adafruit_NeoPixel
 */
void NeoPixel::AdafruitNeoPixelIinit(uint16_t aNumberOfPixels, uint16_t aPin, neoPixelType aTypeOfPixel) {
    Adafruit_NeoPixel::updateType(aTypeOfPixel);
    Adafruit_NeoPixel::updateLength(aNumberOfPixels);
    Adafruit_NeoPixel::setPin(aPin);
}

/*
 * @return false if no memory available
 */
bool NeoPixel::init(uint16_t aNumberOfPixels, uint8_t aPin, neoPixelType aTypeOfPixel) {
    AdafruitNeoPixelIinit(aNumberOfPixels, aPin, aTypeOfPixel);
    Adafruit_NeoPixel::begin(); // sets pin to output

#if defined(_SUPPORT_RGBW)
    BytesPerPixel = ((wOffset == rOffset) ? 3 : 4);
#endif
    PixelOffset = 0;  // 8 byte Flash
    UnderlyingNeoPixelObject = this;
    PixelFlags = 0;
    return (numLEDs != 0);
}

/*
 * Used to create a NeoPixel, which operates on a segment of the underlying NeoPixel object.
 * This creates a new Adafruit_NeoPixel object and replaces the new pixel buffer with the underlying existing one.
 * ATTENTION!
 * @param aEnableShowOfUnderlyingPixel - true = calls show function of the underlying NeoPixel object if show() is called
 *                                         false = suppress calling the underlying show() function
 * It makes no sense to call show for segment because show() would set only the FIRST aNumberOfPixels of the underlying NeoPixelObject
 *
 * To save a lot of CPU time of unnecessary double updates set aEnableShowOfUnderlyingPixel to false and use only UnderlyingNeoPixelObject->show().
 * if(PartialNeoPixelBar.update()){
 *   UnderlyingNeoPixelObject->show();
 * }
 */
NeoPixel::NeoPixel(NeoPixel *aUnderlyingNeoPixelObject, uint16_t aPixelOffset, uint16_t aNumberOfPixels,
        bool aEnableShowOfUnderlyingPixel) :
        Adafruit_NeoPixel(
                (aNumberOfPixels > aUnderlyingNeoPixelObject->numLEDs) ? aUnderlyingNeoPixelObject->numLEDs : aNumberOfPixels,
                aUnderlyingNeoPixelObject->getPin(), aUnderlyingNeoPixelObject->getType()) {

    UnderlyingNeoPixelObject = aUnderlyingNeoPixelObject;
#if defined(_SUPPORT_RGBW)
    BytesPerPixel = aUnderlyingNeoPixelObject->BytesPerPixel;
#endif
    PixelOffset = aPixelOffset;
    Brightness = MAX_BRIGHTNESS;
    PixelFlags = PIXEL_FLAG_IS_PARTIAL_NEOPIXEL;
    if (!aEnableShowOfUnderlyingPixel) {
        PixelFlags = PIXEL_FLAG_IS_PARTIAL_NEOPIXEL | PIXEL_FLAG_DISABLE_SHOW_OF_UNDERLYING_PIXEL_OBJECT;
    }

    if (numLEDs == 0) {
        // malloc has not worked for creating Adafruit_NeoPixel.
        // But since we do not need the malloced buffer, just set the numLEDs and numBytes to the right values.
        numLEDs = aNumberOfPixels;
        numBytes = BytesPerPixel * aNumberOfPixels;
    }
    /*
     * Replace buffer with existing one
     */
    setPixelBuffer(aUnderlyingNeoPixelObject->getPixels());
}

void NeoPixel::init(NeoPixel *aUnderlyingNeoPixelObject, uint16_t aPixelOffset, uint16_t aNumberOfPixels,
        bool aEnableShowOfUnderlyingPixel) {
    if (aNumberOfPixels > aUnderlyingNeoPixelObject->numLEDs) {
        aNumberOfPixels = aUnderlyingNeoPixelObject->numLEDs;
    }
    AdafruitNeoPixelIinit(aNumberOfPixels, aUnderlyingNeoPixelObject->getPin(), aUnderlyingNeoPixelObject->getType());
    Adafruit_NeoPixel::begin(); // sets pin to output

    UnderlyingNeoPixelObject = aUnderlyingNeoPixelObject;
#if defined(_SUPPORT_RGBW)
    BytesPerPixel = aUnderlyingNeoPixelObject->BytesPerPixel;
#endif
    PixelOffset = aPixelOffset;
    Brightness = MAX_BRIGHTNESS;
    PixelFlags = PIXEL_FLAG_IS_PARTIAL_NEOPIXEL;
    if (!aEnableShowOfUnderlyingPixel) {
        PixelFlags = PIXEL_FLAG_IS_PARTIAL_NEOPIXEL | PIXEL_FLAG_DISABLE_SHOW_OF_UNDERLYING_PIXEL_OBJECT;
    }

    if (numLEDs == 0) {
        // malloc has not worked for creating Adafruit_NeoPixel.
        // But since we do not need the malloced buffer, just set the numLEDs and numBytes to the right values.
        numLEDs = aNumberOfPixels;
        numBytes = BytesPerPixel * aNumberOfPixels;
    }
    /*
     * Replace buffer with existing one
     */
    setPixelBuffer(aUnderlyingNeoPixelObject->getPixels());
}

/*
 * @param aEnableBrightnessNonZeroMode Pixel is set to zero, only if brightness or input color is zero, otherwise it is clipped at e.g. 0x000100
 */
void NeoPixel::begin(uint8_t aBrightness, bool aEnableBrightnessNonZeroMode) {
#if defined(SUPPORT_BRIGHTNESS)
    Brightness = aBrightness;
    if (aEnableBrightnessNonZeroMode) {
        PixelFlags |= PIXEL_FLAG_USE_NON_ZERO_BRIGHTNESS;
    }
#else
    (void) aBrightness;
    (void) aEnableBrightnessNonZeroMode;
#endif
    Adafruit_NeoPixel::begin();
}

void NeoPixel::begin() {
    Adafruit_NeoPixel::begin();
}

/*
 * begin function, which prints error message if aSerial is not nullptr
 * Returns false if no memory available
 */
bool NeoPixel::begin(Print *aSerial) {
    begin();
    if (numLEDs == 0) {
        if (aSerial != nullptr) {
            aSerial->print(F("ERROR Not enough free memory available for Pattern at pin "));
            aSerial->println(getPin());
        }
        return false;
    }
    return true;
}

bool NeoPixel::begin(Print *aSerial, uint8_t aBrightness, bool aEnableBrightnessNonZeroMode) {
    begin(aBrightness, aEnableBrightnessNonZeroMode);
    if (numLEDs == 0) {
        if (aSerial != nullptr) {
            aSerial->print(F("ERROR Not enough free memory available for Pattern at pin "));
            aSerial->println(getPin());
        }
        return false;
    }
    return true;
}

/*
 * Requires around 140 bytes of program space
 */
void NeoPixel::printConnectionInfo(Print *aSerial) {
    if (PixelFlags & PIXEL_FLAG_GEOMETRY_CIRCLE) {
        aSerial->print(F("Circular "));
    }
    aSerial->print(F("Neopixel of length "));
    aSerial->print(numLEDs);
    aSerial->print(F(" is attached to pin "));
    aSerial->print(pin);
    if (PixelOffset != 0) {
        aSerial->print(F(" with pixel offset "));
        aSerial->print(PixelOffset);
    }
    aSerial->println();
}

/*
 * For debugging purposes
 */
void NeoPixel::printPin(Print *aSerial) {
    aSerial->print(pin);
    if (PixelOffset != 0) {
        aSerial->print('|');
        aSerial->print(PixelOffset);
    }
    aSerial->print(' ');
}

void NeoPixel::printInfo(Print *aSerial) {
    aSerial->print(F("Pin="));
    aSerial->print(getPin());
    aSerial->print(F(" Offset="));
    aSerial->print(PixelOffset);

    aSerial->print(F(" PixelFlags=0x"));
    aSerial->print(PixelFlags, HEX);
    aSerial->print(F(" &underlying.NeoPixel=0x"));
    aSerial->print((uintptr_t) UnderlyingNeoPixelObject, HEX);
    aSerial->print(F(" &NeoPixel=0x"));
    aSerial->println((uintptr_t) this, HEX);
}

void NeoPixel::printContent(Print *aSerial) {
    aSerial->print(getPin());
    aSerial->print(F(" Colors="));
    for (uint16_t i = 0; i < numLEDs; i++) {
        aSerial->print(F(" 0x"));
        aSerial->print(getPixelColor(i), HEX);
    }
    aSerial->println();
}

/*
 * Handles the DISABLE_SHOW_OF_UNDERLYING_PIXEL_OBJECT flag
 */
void NeoPixel::show() {
    if (PixelFlags & PIXEL_FLAG_IS_PARTIAL_NEOPIXEL) {
        if ((PixelFlags & PIXEL_FLAG_DISABLE_SHOW_OF_UNDERLYING_PIXEL_OBJECT) == 0) {
#if defined(LOCAL_TRACE)
            printPin(&Serial);
            Serial.print(F("Underlying->show, brightness="));
            Serial.println(Brightness);
#endif
            UnderlyingNeoPixelObject->Adafruit_NeoPixel::show();
        }
    } else {
#if defined(LOCAL_TRACE)
        printPin(&Serial);
        Serial.print(F("Show, brightness="));
        Serial.println(Brightness);
#endif
        Adafruit_NeoPixel::show();
    }
}

uint8_t NeoPixel::getBytesPerPixel() {
    return BytesPerPixel;
}

neoPixelType NeoPixel::getType() {
#if defined(NEO_KHZ400)
    neoPixelType tReturnValue = (wOffset << 6 | rOffset << 4 | gOffset << 2 | bOffset);
    if (is800KHz) {
        tReturnValue |= 0x100;
    }
#else
    neoPixelType tReturnValue = (wOffset << 6 | rOffset << 4 | gOffset << 2 | bOffset);
#endif
    return tReturnValue;
}

/*
 * Move the start index of a partial NeoPixel object
 */
void NeoPixel::setPixelOffsetForPartialNeoPixel(uint16_t aPixelOffset) {
    if (PixelFlags & PIXEL_FLAG_IS_PARTIAL_NEOPIXEL) {
        // clip aPixelOffset
        if (aPixelOffset > (UnderlyingNeoPixelObject->numLEDs - numLEDs)) {
            aPixelOffset = UnderlyingNeoPixelObject->numLEDs - numLEDs;
        }
        PixelOffset = aPixelOffset;
    }
}

/*
 *
 * Free old pixel buffer and set new value.
 * Required if you want to have more than one patterns on the same strip.
 */
void NeoPixel::setPixelBuffer(uint8_t *aNewPixelBufferPointer) {
    if (pixels) {
        free(pixels);
    }
    pixels = aNewPixelBufferPointer;
}

uint16_t NeoPixel::getPixelBufferSize() {
    return numBytes;
}

/*
 * The same as numPixels()
 */
uint16_t NeoPixel::getNumberOfPixels() {
    return numLEDs;
}

void NeoPixel::storePixelBuffer(uint8_t *aPixelBufferPointerDestination) {
    memcpy(aPixelBufferPointerDestination, pixels, numBytes);
}

void NeoPixel::restorePixelBuffer(uint8_t *aPixelBufferPointerSource) {
    memcpy(pixels, aPixelBufferPointerSource, numBytes);
}

/*
 * Clear allPixels
 */
void NeoPixel::clear(void) {
    memset(pixels + (BytesPerPixel * PixelOffset), 0, numBytes);
}

void NeoPixel::clearAndShow(void) {
    clear();
    show();
}

/*
 * Requires 50 bytes program memory, but is faster than using setPixelColor()
 */
void NeoPixel::clearPixel(uint16_t aPixelIndex) {
    if (aPixelIndex < numLEDs) {
        aPixelIndex += PixelOffset; // added line to support offsets
    }
    uint8_t *tPixelPtr = &pixels[aPixelIndex * BytesPerPixel];
    *tPixelPtr++ = 0;
    *tPixelPtr++ = 0;
    *tPixelPtr++ = 0;
#if defined(_SUPPORT_RGBW)
    if (BytesPerPixel == 4) {
        *tPixelPtr = 0;
    }
#endif
}

/*
 * Draws bar with length pixel and clears the other ones
 * @param aDrawFromBottom - false: Bar is top down, i.e. it starts at the highest pixel index
 */
void NeoPixel::drawBar(uint16_t aBarLength, color32_t aColor, bool aDrawFromBottom) {
    for (uint_fast16_t i = 0; i < numLEDs; i++) {
        bool tDrawPixel;
        if (aDrawFromBottom) {
            tDrawPixel = (i < aBarLength);
        } else {
            tDrawPixel = (i >= ((uint_fast16_t) numLEDs - aBarLength));
        }
        if (tDrawPixel) {
            setPixelColor(i, aColor);
        } else {
            // Clear pixel
            setPixelColor(i, COLOR32_BLACK);
        }
    }
}

/*
 * @param aColorArrayPtr - Address of a color array holding numLEDs color entries for the bar colors.
 * @param aDrawFromBottom - false: Bar is top down, i.e. it starts at the highest pixel index
 */
void NeoPixel::drawBarFromColorArray(uint16_t aBarLength, color32_t *aColorArrayPtr, bool aDrawFromBottom) {
    for (uint_fast16_t i = 0; i < numLEDs; i++) {
        bool tDrawPixel;
        if (aDrawFromBottom) {
            tDrawPixel = (i < aBarLength);
        } else {
            tDrawPixel = (i >= ((uint_fast16_t) numLEDs - aBarLength));
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
 * Sets the brightness used by Neopixel drawing functions
 * and thr brightness for the Adafruit functions
 * @param   aBrightness  Brightness setting, 0=minimum (off), 255=brightest.
 */
void NeoPixel::setBrightnessValue(uint8_t aBrightness) {
    Brightness = aBrightness;
    // set also Adafruit brightness value
    brightness = aBrightness + 1; // Overflow is intended, see setBrightness()
}

/*
 * Convenience function to set the brightness used by the (unused) Adafruit drawing functions
 * @param   aBrightness  Brightness setting, 0=minimum (off), 255=brightest.
 */
void NeoPixel::setAdafruitBrightnessValue(uint8_t aBrightness) {
    brightness = aBrightness + 1; // Overflow is intended, see setBrightness()
}

void NeoPixel::setBrightnessNonZeroMode(bool aEnableBrightnessNonZeroMode) {
    if (aEnableBrightnessNonZeroMode) {
        PixelFlags |= PIXEL_FLAG_USE_NON_ZERO_BRIGHTNESS;
    } else {
        PixelFlags &= ~PIXEL_FLAG_USE_NON_ZERO_BRIGHTNESS;
    }
}

/*
 * Checks for valid pixel index / skips invalid ones
 */
void NeoPixel::setPixelColor(uint16_t aPixelIndex, uint8_t aRed, uint8_t aGreen, uint8_t aBlue) {
#if defined(LOCAL_TRACE)
    printPin(&Serial);
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
        aPixelIndex += PixelOffset; // support offsets
        uint8_t *tPixelPtr = &pixels[aPixelIndex * BytesPerPixel];

#if defined(_SUPPORT_RGBW)
        if (BytesPerPixel == 4) {
            tPixelPtr[wOffset] = 0;        // But only R,G,B passed -- set W to 0
        }
#endif

#if defined(SUPPORT_BRIGHTNESS)
#  if defined(SUPPORT_NO_ZERO_BRIGHTNESS)
        uint8_t tMaxOffset;
        bool tColorWasNotBlack = true;
#  endif
        // brightness check and multiplication adds 48 bytes
        uint8_t tBrightness = Brightness;
        if (tBrightness != MAX_BRIGHTNESS) {
#  if defined(SUPPORT_NO_ZERO_BRIGHTNESS)
            // searching tMaxOffset and checking for zero below, costs another 72 bytes
            if (aRed == 0 && aGreen == 0 && aBlue == 0) {
                tColorWasNotBlack = false;
            } else {
                tMaxOffset = bOffset;
                uint8_t tMax = aBlue;
                if (aGreen > tMax) {
                    tMax = aGreen;
                    tMaxOffset = gOffset;
                }
                if (aRed > tMax) {
                    tMaxOffset = rOffset;
                }
#  endif
                // multiplication with rounding, otherwise brightness 1 is the same as brightness 0
                aRed = ((aRed * tBrightness) + 0x80) >> 8;
                aGreen = ((aGreen * tBrightness) + 0x80) >> 8;
                aBlue = ((aBlue * tBrightness) + 0x80) >> 8;
#  if defined(SUPPORT_NO_ZERO_BRIGHTNESS)
            }
#  endif
        }
#endif // defined(SUPPORT_BRIGHTNESS)

        tPixelPtr[rOffset] = aRed;          // R,G,B are always stored
        tPixelPtr[gOffset] = aGreen;
        tPixelPtr[bOffset] = aBlue;
#if defined(SUPPORT_BRIGHTNESS) && defined(SUPPORT_NO_ZERO_BRIGHTNESS)
        if ((PixelFlags & PIXEL_FLAG_USE_NON_ZERO_BRIGHTNESS) && tBrightness != 0 && tColorWasNotBlack) {
            if (aRed == 0 && aGreen == 0 && aBlue == 0) {
                // avoid that pixel is completely off
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
                tPixelPtr[tMaxOffset] = 1; // tMaxOffset is set here since if tBrightness = 255 and aColor != 0 then is one of red or green or blue != 0
#pragma GCC diagnostic pop
            }
        }
#endif
    }
}

#if defined(_SUPPORT_RGBW)
void NeoPixel::setPixelColor(uint16_t aPixelIndex, uint8_t aRed, uint8_t aGreen, uint8_t aBlue, uint8_t aWhite) {
#if defined(LOCAL_TRACE)
    printPin(&Serial);
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
    if (aPixelIndex < numLEDs && (BytesPerPixel == 4)) {
        aPixelIndex += PixelOffset; // support offsets
        uint8_t *tPixelPtr = &pixels[aPixelIndex * 4];

#if defined(SUPPORT_BRIGHTNESS)
        uint8_t tBrightness = Brightness;
#  if defined(SUPPORT_NO_ZERO_BRIGHTNESS)
        uint8_t tMaxOffset;
        bool tColorWasNotBlack = true;
        // brightness check and multiplication adds 48 bytes
#  endif
        if (tBrightness != MAX_BRIGHTNESS) {
#  if defined(SUPPORT_NO_ZERO_BRIGHTNESS)
            // searching tMaxOffset and checking for zero below, costs another 72 bytes
            if (aRed == 0 && aGreen == 0 && aBlue == 0) {
                tColorWasNotBlack = false;
            } else {
                tMaxOffset = bOffset;
                uint8_t tMax = aBlue;
                if (aGreen > tMax) {
                    tMax = aGreen;
                    tMaxOffset = gOffset;
                }
                if (aWhite > tMax) {
                    tMax = aWhite;
                    tMaxOffset = wOffset;
                }
                if (aRed > tMax) {
                    tMaxOffset = rOffset;
                }
#  endif
                // multiplication with rounding, otherwise brightness 1 is the same as brightness 0
                aRed = ((aRed * tBrightness) + 0x80) >> 8;
                aGreen = ((aGreen * tBrightness) + 0x80) >> 8;
                aBlue = ((aBlue * tBrightness) + 0x80) >> 8;
                aWhite = ((aWhite * tBrightness) + 0x80) >> 8;
#  if defined(SUPPORT_NO_ZERO_BRIGHTNESS)
            }
#  endif
        }
#endif // defined(SUPPORT_BRIGHTNESS)

        tPixelPtr[rOffset] = aRed;          // Store R,G,B,W
        tPixelPtr[gOffset] = aGreen;
        tPixelPtr[bOffset] = aBlue;
        tPixelPtr[wOffset] = aWhite;

#if defined(SUPPORT_BRIGHTNESS) && defined(SUPPORT_NO_ZERO_BRIGHTNESS)
        if ((PixelFlags & PIXEL_FLAG_USE_NON_ZERO_BRIGHTNESS) && tBrightness != 0 && tColorWasNotBlack) {
            if (aRed == 0 && aGreen == 0 && aBlue == 0 && aWhite == 0) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
                // avoid that pixel is completely off
                tPixelPtr[tMaxOffset] = 1;
#pragma GCC diagnostic pop
            }

        }
#endif
    }
}
#endif // _SUPPORT_RGBW

/*
 * Version with rounded brightness computation and special non zero brightness mode
 */
void NeoPixel::setPixelColor(uint16_t aPixelIndex, color32_t aColor) {

//    if(aColor == 0) {
//        // is faster and adds 78 bytes for clearPixel function
//        clearPixel(aPixelIndex);
//    }
    if (aPixelIndex < numLEDs) {
        aPixelIndex += PixelOffset; // support offsets, no check for overflow

        uint8_t tRed = (uint8_t) (aColor >> 16);
        uint8_t tGreen = (uint8_t) (aColor >> 8);
        uint8_t tBlue = (uint8_t) aColor;
        uint8_t *tPixelPtr = &pixels[aPixelIndex * BytesPerPixel];

#if defined(LOCAL_TRACE)
        printPin(&Serial);
        Serial.print(F("Pixel="));
        Serial.print(aPixelIndex);
        Serial.print(F(" Offset="));
        Serial.print(PixelOffset);
        Serial.print(F(" Color=0x"));
        Serial.print(aColor, HEX);
        Serial.print(F(" Brightness=0x"));
        Serial.println(Brightness);
#endif

#if defined(SUPPORT_BRIGHTNESS)
        uint8_t tBrightness = Brightness;
#  if defined(SUPPORT_NO_ZERO_BRIGHTNESS)
        uint8_t tMaxOffset;
#  endif
#endif

#if defined(_SUPPORT_RGBW)
        uint8_t tWhite;
        if (BytesPerPixel == 4) {
            tWhite = (uint8_t) (aColor >> 24);
#if defined(SUPPORT_BRIGHTNESS)
            if (tBrightness != MAX_BRIGHTNESS) {
                tWhite = (tWhite * tBrightness) >> 8;
            }
#endif
            tPixelPtr[wOffset] = tWhite;
        }
#endif

#if defined(SUPPORT_BRIGHTNESS)
        // brightness check and multiplication adds 68 (132 with RGBW) bytes
        if (tBrightness != MAX_BRIGHTNESS) {
#  if defined(SUPPORT_NO_ZERO_BRIGHTNESS)
            // searching tMaxOffset and checking for zero below, costs another 54 (84 with RGBW) bytes
            tMaxOffset = bOffset;
            uint8_t tMax = tBlue;
            if (tGreen > tMax) {
                // Here green is brighter than blue, set maximum to green
                tMax = tGreen;
                tMaxOffset = gOffset;
            }
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#    if defined(_SUPPORT_RGBW)
            if (BytesPerPixel == 4 && tWhite > tMax) {
                tMax = tWhite;
                tMaxOffset = wOffset;
            }
#    endif
            if (tRed > tMax) {
                tMaxOffset = rOffset;
            }
#  endif
            // Compute brightness with rounding, here tBrightness is < 0xFF :-)
            tRed = ((tRed * tBrightness) + 0x80) >> 8;
            tGreen = ((tGreen * tBrightness) + 0x80) >> 8;
            tBlue = ((tBlue * tBrightness) + 0x80) >> 8;
        }
#endif // defined(SUPPORT_BRIGHTNESS)
        tPixelPtr[rOffset] = tRed;
        tPixelPtr[gOffset] = tGreen;
        tPixelPtr[bOffset] = tBlue;

#if defined(SUPPORT_BRIGHTNESS) && defined(SUPPORT_NO_ZERO_BRIGHTNESS)
        if ((PixelFlags & PIXEL_FLAG_USE_NON_ZERO_BRIGHTNESS) && tBrightness != 0 && aColor != 0) {
#  if defined(_SUPPORT_RGBW)
            if (tRed == 0 && tGreen == 0 && tBlue == 0 && (BytesPerPixel != 4 || (BytesPerPixel == 4 && tWhite == 0))) {
#  else
            if (tRed == 0 && tGreen == 0 && tBlue == 0) {
#  endif
#if defined(LOCAL_TRACE)
                printPin(&Serial);
                Serial.print(F("MaxOffset="));
                Serial.println(tMaxOffset);
#endif
                // avoid that pixel is completely off, but prefer blue if it has the same value as one of the other colors
                // I.e. white (x,x,x) changes to blue (0,0,1) if brightness is too low.
                tPixelPtr[tMaxOffset] = 1; // tMaxOffset is set here since if tBrightness = 255 and aColor != 0 then is one of red or green or blue != 0
#pragma GCC diagnostic pop

            }
        }
#endif
    }
}

/*
 * Adds color to existing one and clip to white (MAX_BRIGHTNESS)
 */
void NeoPixel::addPixelColor(uint16_t aPixelIndex, color32_t aColor) {
    if (aPixelIndex < numLEDs) {
        aPixelIndex += PixelOffset; // support offsets
        color32_t tOldColor = getPixelColor(aPixelIndex);
        if (tOldColor == 0) {
            setPixelColor(aPixelIndex, aColor);
        } else {
            uint8_t tRed = getRedPart(tOldColor) + (uint8_t) (aColor >> 16);
            if (tRed < (uint8_t) (aColor >> 16)) {
                // clip overflow
                tRed = MAX_BRIGHTNESS;
            }
            uint8_t tGreen = getGreenPart(tOldColor) + (uint8_t) (aColor >> 8);
            if (tGreen < (uint8_t) (aColor >> 8)) {
                tGreen = MAX_BRIGHTNESS;
            }
            uint8_t tBlue = getBluePart(tOldColor) + (uint8_t) aColor;
            if (tBlue < (uint8_t) aColor) {
                tBlue = MAX_BRIGHTNESS;
            }
            setPixelColor(aPixelIndex, tRed, tGreen, tBlue);
        }
    }
}

void NeoPixel::addPixelColor(uint16_t aPixelIndex, uint8_t aRed, uint8_t aGreen, uint8_t aBlue) {
    if (aPixelIndex < numLEDs) {
        aPixelIndex += PixelOffset; // support offsets
        color32_t tOldColor = getPixelColor(aPixelIndex);
        if (tOldColor == 0) {
            setPixelColor(aPixelIndex, aRed, aGreen, aBlue);
        } else {
            uint8_t tRed = getRedPart(tOldColor) + aRed;
            if (tRed < aRed) {
                // clip overflow
                tRed = MAX_BRIGHTNESS;
            }
            uint8_t tGreen = getGreenPart(tOldColor) + aGreen;
            if (tGreen < aGreen) {
                tGreen = MAX_BRIGHTNESS;
            }
            uint8_t tBlue = getBluePart(tOldColor) + aBlue;
            if (tBlue < aBlue) {
                tBlue = MAX_BRIGHTNESS;
            }
            setPixelColor(aPixelIndex, tRed, tGreen, tBlue);
        }
    }
}

// Set all pixels to a color (synchronously)
void NeoPixel::setColor(color32_t aColor) {
// This is faster but costs 94 bytes, or 54 bytes program memory if DO_NOT_SUPPORT_RGBW is defined
    if (BytesPerPixel == 3) {
        setPixelColor(0, aColor);
        memcpy(&pixels[(PixelOffset + 1) * 3], &pixels[PixelOffset * 3], (numLEDs - 1) * 3);
    } else {
        // if DO_NOT_SUPPORT_RGBW is defined, this code is removed by the compiler :-)
        for (uint_fast16_t i = 0; i < numLEDs; i++) {
            setPixelColor(i, aColor);
        }
    }
}
// deprecated
void NeoPixel::ColorSet(color32_t aColor) {
    setColor(aColor);
}

void NeoPixel::fillRegion(color32_t aColor, uint16_t aRegionStartIndext, uint16_t aRegionLength) {
    if (aRegionStartIndext + aRegionLength <= numLEDs) {
        for (uint_fast16_t i = aRegionStartIndext; i < aRegionStartIndext + aRegionLength; i++) {
            setPixelColor(i, aColor);
        }
    }
}

/*
 * Does no parameter checking!
 */
void NeoPixel::copyRegion(uint16_t aSourcePixelIndex, uint16_t aTargetPixelIndex, uint16_t aLength, bool aDoReverseCopy) {
    uint8_t *tSourcePixelPtr = &pixels[aSourcePixelIndex * BytesPerPixel];
    uint8_t *tTargetPixelPtr = &pixels[aTargetPixelIndex * BytesPerPixel];
    for (uint_fast16_t i = 0; i < aLength; i++) {
        // copy data for one pixel
        if (BytesPerPixel == 3) {
            *tTargetPixelPtr++ = *tSourcePixelPtr++;
            *tTargetPixelPtr++ = *tSourcePixelPtr++;
            *tTargetPixelPtr++ = *tSourcePixelPtr++;
#if defined(_SUPPORT_RGBW)
        } else {
            for (uint_fast16_t j = 0; j < BytesPerPixel; j++) {
                *tTargetPixelPtr++ = *tSourcePixelPtr++;
            }
#endif
        }
        if (aDoReverseCopy) {
            tTargetPixelPtr -= 2 * BytesPerPixel;
        }
    }
}
color32_t NeoPixel::getPixelColor(uint16_t aPixelIndex) {
    uint8_t *tPixelPointer = &pixels[(aPixelIndex + PixelOffset) * BytesPerPixel];
    if (BytesPerPixel == 3) {
        return (uint32_t) tPixelPointer[rOffset] << 16 | (uint32_t) tPixelPointer[gOffset] << 8 | tPixelPointer[bOffset];
    }
#if defined(_SUPPORT_RGBW)
    return (uint32_t) tPixelPointer[wOffset] << 24 | (uint32_t) tPixelPointer[rOffset] << 16 | tPixelPointer[gOffset] << 8
            | tPixelPointer[bOffset];
#endif
}

// Set 50% dimmed value of current color
void NeoPixel::dimPixelColor(uint16_t aPixelIndex) {
    uint8_t *tPixelPointer = &pixels[(aPixelIndex + PixelOffset) * BytesPerPixel];
    for (uint_fast8_t i = 0; i < BytesPerPixel; ++i) {
        *tPixelPointer = *tPixelPointer >> 1;
        tPixelPointer++;
    }
}

// Calculate 50% dimmed version of a color not using gamma
uint32_t NeoPixel::dimColor(color32_t aColor) {
// Shift R, G and B components one bit to the right
#if defined(_SUPPORT_RGBW)
    uint32_t dimColor = Color(getRedPart(aColor) >> 1, getGreenPart(aColor) >> 1, getBluePart(aColor) >> 1,
            getWhitePart(aColor) >> 1);
#else
    uint32_t dimColor = Color(getRedPart(aColor) >> 1, getGreenPart(aColor) >> 1, getBluePart(aColor) >> 1);
#endif
    return dimColor;
}

// Input a value 0 to 255 to get a color value.
// The colors are a transition red -> green -> blue -> back to red.
color32_t NeoPixel::Wheel(uint8_t aWheelPos) {
    aWheelPos = MAX_WHEEL_POSITION - aWheelPos;
    if (aWheelPos < 85) {
        return Color(MAX_WHEEL_POSITION - (aWheelPos * 3), 0, aWheelPos * 3);
    } else if (aWheelPos < 170) {
        aWheelPos -= 85;
        return Color(0, aWheelPos * 3, MAX_WHEEL_POSITION - (aWheelPos * 3));
    } else {
        aWheelPos -= 170;
        return Color(aWheelPos * 3, MAX_WHEEL_POSITION - (aWheelPos * 3), 0);
    }
}

/*
 * @param aStartAtTop if true the first color is written at index numLEDs
 */
void NeoPixel::fillWithRainbow(uint8_t aRainbowWheelStartPos, bool aStartAtTop) {
    uint16_t tWheelIndexHighResolution = aRainbowWheelStartPos << 8; // upper byte is the integer part used for Wheel(), lower byte is the fractional part
    uint16_t tWheelIndexHighResolutionDelta = 0x10000 / numLEDs;
    for (uint_fast16_t i = 0; i < numLEDs; i++) {
        if (aStartAtTop) {
            setPixelColor(numLEDs - 1 - i, Wheel(tWheelIndexHighResolution >> 8));
        } else {
            setPixelColor(i, Wheel(tWheelIndexHighResolution >> 8));
        }
        tWheelIndexHighResolution += tWheelIndexHighResolutionDelta;
    }
}

// from https://www.mikrocontroller.net/articles/LED-Fading pwmtable_8D
//const uint8_t GammaTable32[32] PROGMEM = { 0, 1, 2, 2, 2, 3, 3, 4, 5, 6, 7, 8, 10, 11, 13, 16, 19, 23, 27, 32, 38, 45, 54, 64, 76,
//        91, 108, 128, 152, 181, 215, 255 };

// Excel: y= round(31*power(1.121725454;x);0) values are chosen, that we have y for 1 as 0.7
const uint8_t GammaTable32[32] PROGMEM = { 0, 1, 1, 1, 1, 2, 2, 2, 3, 3, 4, 5, 6, 7, 9, 11, 13, 16, 20, 24, 29, 36, 43, 55, 64, 78,
        95, 116, 141, 172, 209, MAX_BRIGHTNESS };
/*
 * Use mapping table with 32 entries (using 5 MostSignificantBits)
 * @param aLinearBrightnessValue - from 0 to 255
 */
uint8_t NeoPixel::gamma5(uint8_t aLinearBrightnessValue) {
    return pgm_read_byte(&GammaTable32[(aLinearBrightnessValue / 8)]);
}
// deprecated
uint8_t NeoPixel::gamma32(uint8_t aLinearBrightnessValue) {
    return gamma5(aLinearBrightnessValue);
}

/*
 * The same as gamma5() but returns 1 for input 1 to 7 and 0 only if value is 0.
 * Can be used in conjunction with PIXEL_FLAG_USE_NON_ZERO_BRIGHTNESS, not to blank out pixels which are heavily dimmed
 */
uint8_t NeoPixel::gamma5WithSpecialZero(uint8_t aLinearBrightnessValue) {
    if (aLinearBrightnessValue <= 7 && aLinearBrightnessValue >= 1) {
        return 1;
    }
    return pgm_read_byte(&GammaTable32[(aLinearBrightnessValue / 8)]);
}

/*
 * Using gamma table with 32 entries
 */
color32_t NeoPixel::convertLinearToGamma5Color(color32_t aLinearBrightnessColor) {
    uint8_t tRed = pgm_read_byte(&GammaTable32[(getRedPart(aLinearBrightnessColor) / 8)]);
    uint8_t tGreen = pgm_read_byte(&GammaTable32[(getGreenPart(aLinearBrightnessColor) / 8)]);
    uint8_t tBlue = pgm_read_byte(&GammaTable32[(getBluePart(aLinearBrightnessColor) / 8)]);
#if defined(_SUPPORT_RGBW)
    uint8_t tWhite = pgm_read_byte(&GammaTable32[(getWhitePart(aLinearBrightnessColor) / 8)]);
    return Color(tRed, tGreen, tBlue, tWhite);
#else
    return Color(tRed, tGreen, tBlue);
#endif
}

/*
 * aBrightness 0 = black, 8 = 1, 16 to 32 = 2, 40 = 3, ... 255 = full
 * doSpecialZero -> tGamma32Brightness returns only 0 if aBrightness value is 0. Returns 1 for aBrightness 1 to 7 (and for 8 to 15).
 */
color32_t NeoPixel::dimColorWithGamma5(color32_t aLinearBrightnessColor, uint8_t aBrightness, bool doSpecialZero) {
#if defined(_SUPPORT_RGBW)
    uint8_t tWhiteDimmed = (uint8_t) (aLinearBrightnessColor >> 24);
#endif
    uint8_t tRedDimmed = (uint8_t) (aLinearBrightnessColor >> 16);
    uint8_t tGreenDimmed = (uint8_t) (aLinearBrightnessColor >> 8);
    uint8_t tBlueDimmed = (uint8_t) aLinearBrightnessColor;

    uint8_t tGamma5Brightness;
    if (doSpecialZero) {
        tGamma5Brightness = gamma5WithSpecialZero(aBrightness);
    } else {
        tGamma5Brightness = gamma5(aBrightness);
    }

// (tRedDimmed + 1) since (255 * 1) >> 8 gives 0 (and not 1)
    tRedDimmed = ((tRedDimmed + 1) * tGamma5Brightness) >> 8;
    tGreenDimmed = ((tGreenDimmed + 1) * tGamma5Brightness) >> 8;
    tBlueDimmed = ((tBlueDimmed + 1) * tGamma5Brightness) >> 8;
#if defined(_SUPPORT_RGBW)
    tWhiteDimmed = ((tWhiteDimmed + 1) * tGamma5Brightness) >> 8;
#endif

#if defined(LOCAL_TRACE)
            Serial.print(F("dimColorWithGamma5 aBrightness="));
            Serial.print(aBrightness);
            Serial.print(F(" Gamma="));
            Serial.print(tGamma5Brightness);
            Serial.print(F(" 0x"));
            Serial.print(aLinearBrightnessColor, HEX);
            Serial.print(F("->0x"));
#  if defined(_SUPPORT_RGBW)
            Serial.println(((uint32_t) tWhiteDimmed << 24) | ((uint32_t) tRedDimmed << 16) | ((uint32_t) tGreenDimmed << 8) | tBlueDimmed, HEX);
#  else
            Serial.println(((uint32_t) tRedDimmed << 16) | ((uint32_t) tGreenDimmed << 8) | tBlueDimmed, HEX);
#  endif
#endif
#if defined(_SUPPORT_RGBW)
    return ((uint32_t) tWhiteDimmed << 24) | ((uint32_t) tRedDimmed << 16) | ((uint32_t) tGreenDimmed << 8) | tBlueDimmed;
#else
    return ((uint32_t) tRedDimmed << 16) | ((uint32_t) tGreenDimmed << 8) | tBlueDimmed;
#endif
}

// deprecated, use dimColorWithGamma5()
color32_t NeoPixel::dimColorWithGamma32(color32_t aLinearBrightnessColor, uint8_t aBrightness, bool doSpecialZero) {
    return dimColorWithGamma5(aLinearBrightnessColor, aBrightness, doSpecialZero);
} // end deprecated

/*
 * Returns the White part of a 32-bit color
 * Replaces the old White(color32_t color) function
 */
uint8_t getWhitePart(color32_t color) {
    return (color >> 24) & 0xFF;
}

// Returns the getRed part of a 32-bit color
uint8_t getRedPart(color32_t color) {
    return (color >> 16) & 0xFF;
}

// Returns the getGreen part of a 32-bit color
uint8_t getGreenPart(color32_t color) {
    return (color >> 8) & 0xFF;
}

// Returns the Blue part of a 32-bit color
uint8_t getBluePart(color32_t color) {
    return color & 0xFF;
}

// deprecated
// Returns the White part of a 32-bit color
uint8_t White(color32_t color) {
    return (color >> 24) & 0xFF;
}

// Returns the getRed part of a 32-bit color
uint8_t Red(color32_t color) {
    return (color >> 16) & 0xFF;
}

// Returns the getGreen part of a 32-bit color
uint8_t Green(color32_t color) {
    return (color >> 8) & 0xFF;
}

// Returns the Blue part of a 32-bit color
uint8_t Blue(color32_t color) {
    return color & 0xFF;
}
// end deprecated

#include "ADCUtils.h"

/*
 * Get the actual (even) length of the strip (which only can be lower than the current length)
 * and adjust pixel buffer to the new length.
 * Uses ADC and the VCC voltage drop to determine the actual length of a strip.
 *
 * Based on the idea of Tim: https://cpldcpu.com/2014/11/16/ws2812_length/
 *
 * @return  The actual length of the strip
 *          0 if length could not be determined
 *
 * Could be improved by:
 * 1. Check at position slight below first loop value, how may pixels are required for a voltage drop.
 *    Currently we assume a voltage drop at one pixel.
 * 2  Use this number to compute start of strip found at second slow run.
 *
 * We use getVCCVoltageMillivoltSimple(), since it is faster and smaller,
 * because we must not check for ADC reference and channel switching here.
 */
#if !defined(DELTA_MILLIVOLT_FOR_PIXEL_DETECTION)
#define DELTA_MILLIVOLT_FOR_PIXEL_DETECTION         40 // We have a resolution of 20 mV at the 328P at 5 volt
#endif
#if !defined(TEST_PATTERN_LENGTH_FOR_PIXEL_DETECTION)
#define TEST_PATTERN_LENGTH_FOR_PIXEL_DETECTION     1  // Adjust this value if test pattern length for successful detection is greater than 1
#endif
uint16_t NeoPixel::getAndAdjustActualNeopixelLenghtSimple() {
#if defined(ADC_UTILS_ARE_AVAILABLE)
    /*
     * First set ADC reference and channel and clear strip
     */
    getVCCVoltageMillivoltSimple(); // to set ADC channel and reference
    clear();
    show();
    delay(50);
    uint16_t tStartMillivolt = getVCCVoltageMillivoltSimple(); // it is faster to use this simple version
#  if defined(LOCAL_INFO)
    Serial.print(F("Start VCC="));
    Serial.print(tStartMillivolt);
    Serial.println(" mV");
#  endif

    /*
     * First run is fast run and uses
     */
    int tLedIndex = numLEDs - 1;
    /*
     * Use every 4. pixel for the first run.
     * This gives 2 white pixel on a small 8 pixel bar, thus reliable supporting these small bars too.
     */
    uint_fast8_t tStepWidth = 4;
    for (unsigned int i = 0; i <= 4; i += 4) {
        /*
         * This loop / check runs twice
         * first from end of strip using every 4. pixel and no delay
         * Second from above of last result using every pixel and a delay of 4 ms
         */
        for (; tLedIndex >= 0; tLedIndex -= tStepWidth) {
            setPixelColor(tLedIndex, COLOR32_WHITE);
            show();
            delay(i);
            uint16_t tCurrentMillivolt = getVCCVoltageMillivoltSimple(); // We have a resolution of 20 mV at the 328P at 5 volt
            if (tCurrentMillivolt < tStartMillivolt - DELTA_MILLIVOLT_FOR_PIXEL_DETECTION) {
                tLedIndex++;
                break;
            }
        }
        /*
         * 2. run is slow run from starting nearby
         */
        clear();
        show();
        if (i != 0) {
            // exit 2 loop run here
            int tActualNeopixelLength = tLedIndex + TEST_PATTERN_LENGTH_FOR_PIXEL_DETECTION;
            // update length to next even number
            tActualNeopixelLength = (tActualNeopixelLength + 1) & ~0x01;
            if (tActualNeopixelLength != 0) {
                updateLength(tActualNeopixelLength);
            }
            return tActualNeopixelLength;
        }
        delay(20);

        /*
         * Prepare for 2. loop
         * Go back 8 steps of the first loop, but clip at end of strip
         */
        tLedIndex = tLedIndex + 36; // 8 * 4 = 36
        if (tLedIndex > (int) numLEDs - 1) {
            tLedIndex = numLEDs - 1;
        }
        tStepWidth = 1;
    }
#endif
    return 0;
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
    setPixelColor(tPosition++, MAX_BRIGHTNESS, 0, 0);

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
    setPixelColor(tPosition++, 0, MAX_BRIGHTNESS, 0);

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
    setPixelColor(tPosition++, 0, 0, MAX_BRIGHTNESS);
    show();
}

/***********************************************************************************************
 * From FastLED random8.h https://github.com/FastLED/FastLED/blob/master/src/lib8tion/random8.h
 ***********************************************************************************************/
uint16_t rand16seed;

/// Multiplier value for pseudo-random number generation
#define FASTLED_RAND16_2053 ((uint16_t)(2053))
/// Increment value for pseudo-random number generation
#define FASTLED_RAND16_13849 ((uint16_t)(13849))

#if defined(LIB8_ATTINY)
/// Multiplies a value by the pseudo-random multiplier
#define APPLY_FASTLED_RAND16_2053(x) (x << 11) + (x << 2) + x
#else
/// Multiplies a value by the pseudo-random multiplier
#define APPLY_FASTLED_RAND16_2053(x) (x * FASTLED_RAND16_2053)
#endif

uint8_t random8() {
    rand16seed = APPLY_FASTLED_RAND16_2053(rand16seed) + FASTLED_RAND16_13849;
// return the sum of the high and low bytes, for better
//  mixing and non-sequential correlation
    return (uint8_t) (((uint8_t) (rand16seed & 0xFF)) + ((uint8_t) (rand16seed >> 8)));
}

/*
 * @return values from 0 to (excluded) lim
 */
uint8_t random8(uint8_t lim) {
    uint8_t r = random8();
    r = (r * lim) >> 8;
    return r;
}

/*
 * @return values from 0 to (excluded) lim
 */
uint8_t random8(uint8_t min, uint8_t lim) {
    uint8_t delta = lim - min;
    uint8_t r = random8(delta) + min;
    return r;
}
#include "LocalDebugLevelEnd.h"
#endif // _NEOPIXEL_HPP
