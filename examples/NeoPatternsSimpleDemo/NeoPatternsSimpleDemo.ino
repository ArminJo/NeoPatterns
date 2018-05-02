/*
 *  NeoPatterns144.cpp
 *
 *  Shows all patterns included in the NeoPixel library for NeoPixel stripes.
 *
 *  Just add your pattern code to the functions Pattern[1,2]() and Pattern[1,2]Update() in Neopatterns.cpp (line 588ff.)
 *  to realize and see your own patterns.
 *  Enable TEST_OWN_PATTERNS on line 38 to test your patterns.
 *
 *  You need to install "Adafruit NeoPixel" library under Sketch -> Include Library -> Manage Librarys... -> use "neoPixel" as filter string
 *
 *  Copyright (C) 2018  Armin Joachimsmeyer
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

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/gpl.html>.
 *
 */

#include <Arduino.h>

#include <NeoPatterns.h>

#define VERSION_EXAMPLE "1.0"

//#define TEST_OWN_PATTERNS

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN_STRIPE_16          8

// onComplete callback functions
void allPatterns(NeoPatterns * aLedsPtr);
void ownPatterns(NeoPatterns * aLedsPtr);

// construct the NeoPatterns instances
#ifdef TEST_OWN_PATTERNS
NeoPatterns stripe16 = NeoPatterns(16, PIN_STRIPE_16, NEO_GRB + NEO_KHZ800, &ownPatterns);
#else
NeoPatterns stripe16 = NeoPatterns(16, PIN_STRIPE_16, NEO_GRB + NEO_KHZ800, &allPatterns);
#endif

void setup() {
    Serial.begin(115200);
    Serial.println(F("START " __FILE__ "\r\nVersion " VERSION_EXAMPLE " from  " __DATE__));

    stripe16.begin(); // This initializes the NeoPixel library.
    stripe16.ColorWipe(COLOR32(0, 0, 02), 50, REVERSE); // Blue

    randomSeed(12345);

    Serial.println("started");
}

void loop() {
    stripe16.Update();
}

/*
 * Handler for testing your patterns
 */
void ownPatterns(NeoPatterns * aLedsPtr) {
    static int8_t sState = 0;

    uint8_t tDuration = random(20, 120);
    uint8_t tColor = random(255);

    switch (sState) {
    case 0:
        aLedsPtr->Pattern1(COLOR32_RED_HALF, NeoPatterns::Wheel(tColor), tDuration, FORWARD);
        break;

    case 1:
        aLedsPtr->Pattern2(COLOR32_RED_HALF, NeoPatterns::Wheel(tColor), tDuration, FORWARD);
        sState = -1; // Start from beginning
        break;

    default:
        Serial.println("ERROR");
        break;
    }

    sState++;
}

/*
 * Handler for all pattern
 */
void allPatterns(NeoPatterns * aLedsPtr) {
    static int8_t sState = 0;

    uint8_t tDuration = random(40, 81);
    uint8_t tColor = random(255);

    switch (sState) {
    case 0:
        aLedsPtr->Cylon(NeoPatterns::Wheel(tColor), tDuration, 2);
        break;
    case 1:
        aLedsPtr->Scanner(NeoPatterns::Wheel(tColor), tDuration);
        break;
    case 2:
        // Falling star
        aLedsPtr->Scanner(COLOR32_WHITE_HALF, tDuration / 2, 5);
        break;
    case 3:
        aLedsPtr->RainbowCycle(20);
        break;
    case 4:
        aLedsPtr->TheaterChase(COLOR32_WHITE_HALF, COLOR32_BLACK, tDuration + tDuration / 2); // White on Black
        break;
    case 5:
        aLedsPtr->TheaterChase(NeoPatterns::Wheel(tColor), NeoPatterns::Wheel(tColor + 0x80), tDuration + tDuration / 2); //
        break;
    case 6:
        aLedsPtr->Fade(COLOR32_RED, COLOR32_BLUE, 32, tDuration, REVERSE);
        break;
    case 7:
        aLedsPtr->ColorWipe(NeoPatterns::Wheel(tColor), tDuration);
        break;
    case 8:
        // start at both end
        aLedsPtr->Scanner(NeoPatterns::Wheel(tColor), tDuration / 2, 6);
        break;
    case 9:
        // Multiple falling star
        initFallingStar(aLedsPtr, COLOR32_WHITE_HALF, tDuration / 2, 3, &allPatterns);
        break;
    case 10:
        if (aLedsPtr->PatternsGeometry == GEOMETRY_BAR) {
            //Fire
            aLedsPtr->Fire(tDuration / 2, 150);
        } else {
            // start at both end
            aLedsPtr->Scanner(NeoPatterns::Wheel(tColor), tDuration, 6);
        }

        sState = -1; // Start from beginning
        break;
    default:
        Serial.println("ERROR");
        break;
    }

    sState++;
}
