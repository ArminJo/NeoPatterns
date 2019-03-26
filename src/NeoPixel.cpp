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

NeoPixel::NeoPixel(uint16_t aNumberOfPixels, uint8_t aPin, uint8_t aTypeOfPixel) :
        Adafruit_NeoPixel(aNumberOfPixels, aPin, aTypeOfPixel) {
    uint8_t twOffset = (aTypeOfPixel >> 6) & 0b11; // See notes in header file Adafruit_NeoPixel.h regarding R/G/B/W offsets
    uint8_t trOffset = (aTypeOfPixel >> 4) & 0b11;
    BytesPerPixel = ((twOffset == trOffset) ? 3 : 4);
}

void NeoPixel::begin() {
    Adafruit_NeoPixel::begin();
}

/*
 * Prints error message if aSerial is not NULL
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

void NeoPixel::resetBrightnessValue() {
    brightness = 0;
}

uint8_t NeoPixel::getBytesPerPixel() {
    return BytesPerPixel;
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

void NeoPixel::restorePixelBuffer(uint8_t * aPixelBufferPointerSource, bool aResetBrightness) {
    memcpy(pixels, aPixelBufferPointerSource, numBytes);
    if (aResetBrightness) {
        brightness = 0;
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
// Calculate 50% dimmed version of a color
uint32_t NeoPixel::DimColor(color32_t color) {
// Shift R, G and B components one bit to the right
    uint32_t dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
    return dimColor;
}

// Set all pixels to a color (synchronously)
void NeoPixel::ColorSet(color32_t color) {
    for (uint16_t i = 0; i < numLEDs; i++) {
        setPixelColor(i, color);
    }
}

// Input a value 0 to 255 to get a color value.
// The colors are a transition r - g - b - back to r.
color32_t NeoPixel::Wheel(uint8_t WheelPos) {
    WheelPos = 255 - WheelPos;
    if (WheelPos < 85) {
        return Color(255 - (WheelPos * 3), 0, WheelPos * 3);
    } else if (WheelPos < 170) {
        WheelPos -= 85;
        return Color(0, WheelPos * 3, 255 - (WheelPos * 3));
    } else {
        WheelPos -= 170;
        return Color(WheelPos * 3, 255 - (WheelPos * 3), 0);
    }
}

// from https://www.mikrocontroller.net/articles/LED-Fading
const uint8_t _gammaTable32[32] PROGMEM = { 0, 1, 2, 2, 2, 3, 3, 4, 5, 6, 7, 8, 10, 11, 13, 16, 19, 23, 27, 32, 38, 45, 54, 64, 76,
        91, 108, 128, 152, 181, 215, 255 };

/*
 * use mapping table with 32 entries (using 5 MSbits)
 */
uint8_t NeoPixel::gamma5(uint8_t aLinearBrightnessValue) {
    return pgm_read_byte(&_gammaTable32[(aLinearBrightnessValue / 8)]);
}

/*
 * Returns only 0 if value is 0.
 * Returns 1 for input 1 to 7.
 * used for snake tail, not to blank out the last elements of a tail with more than 32 elements
 */
uint8_t NeoPixel::gamma5WithSpecialZero(uint8_t aLinearBrightnessValue) {
    if (aLinearBrightnessValue <= 7 && aLinearBrightnessValue >= 1) {
        return 1;
    }
    return pgm_read_byte(&_gammaTable32[(aLinearBrightnessValue / 8)]);
}

color32_t NeoPixel::gamma5FromColor(color32_t aLinearBrightnessColor) {
    uint8_t tRed = pgm_read_byte(&_gammaTable32[(Red(aLinearBrightnessColor) / 8)]);
    uint8_t tGreen = pgm_read_byte(&_gammaTable32[(Green(aLinearBrightnessColor) / 8)]);
    uint8_t tBlue = pgm_read_byte(&_gammaTable32[(Blue(aLinearBrightnessColor) / 8)]);
    return Color(tRed, tGreen, tBlue);
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
 * outputs the 11 values 0,1,2,3,4,8,16,32,64,128,255 for every color -> 33 pixel
 */
void NeoPixel::TestWS2812Resolution() {

    /*
     * output 0 to 3 on RED
     */
    uint8_t tPosition = 0;
    for (int i = 0; i < 4; ++i) {
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
    for (int i = 0; i < 4; ++i) {
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
    for (int i = 0; i < 4; ++i) {
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
