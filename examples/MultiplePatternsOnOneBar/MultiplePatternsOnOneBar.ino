/*
 *  MultiplePatternsOnOneBar.cpp
 *
 *  Runs 3 patterns simultaneously on a 24 NeoPixel bar.
 *
 *  You need to install "Adafruit NeoPixel" library under "Tools -> Manage Libraries..." or "Ctrl+Shift+I" -> use "neoPixel" as filter string
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
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/gpl.html>.
 *
 */

#include <Arduino.h>

#include <NeoPatterns.h>

#define VERSION_EXAMPLE "1.0"

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN_NEOPIXEL  2

// onComplete callback functions
void MultiPatterns(NeoPatterns * aLedsPtr);

// construct the NeoPatterns instances
NeoPatterns NeoPixelBar24 = NeoPatterns(24, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
// false -> do not allow show on partial NeoPixel bar
NeoPatterns LowerNeoPixelBar = NeoPatterns(&NeoPixelBar24, 0, 8, &MultiPatterns, false);
NeoPatterns MiddleNeoPixelBar = NeoPatterns(&NeoPixelBar24, 8, 8, &MultiPatterns, false);
NeoPatterns UpperNeoPixelBar = NeoPatterns(&NeoPixelBar24, 16, 8, &MultiPatterns, false);

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);
    while (!Serial)
        ; //delay for Leonardo
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ "\r\nVersion " VERSION_EXAMPLE " from  " __DATE__));

    NeoPixelBar24.begin(); // This sets the output pin.

    /*
     * Start the patterns
     */
    LowerNeoPixelBar.ColorWipe(COLOR32_GREEN_QUARTER, 80);
    MiddleNeoPixelBar.ScannerExtended(COLOR32_BLUE_HALF, 2, 60, 2, FLAG_SCANNER_EXT_ROCKET | FLAG_SCANNER_EXT_START_AT_BOTH_ENDS);
    UpperNeoPixelBar.ColorWipe(COLOR32_RED_QUARTER, 80, 0, DIRECTION_DOWN);
    Serial.println(F("Started"));
}

void loop() {
    /*
     * Cannot do this in one if statement, because evaluation will stop after the first true.
     */
    bool tMustUpdate = LowerNeoPixelBar.update();
    tMustUpdate |= MiddleNeoPixelBar.update();
    tMustUpdate |= UpperNeoPixelBar.update();
    if (tMustUpdate) {
        NeoPixelBar24.show();
    }
    delay(10);
}

/*
 * Handler for multi pattern
 */
void MultiPatterns(NeoPatterns * aLedsPtr) {
    static int8_t sState = 1; // start with delay

    /*
     * Wait for the last pattern to end
     */
    if (LowerNeoPixelBar.ActivePattern == PATTERN_NONE && MiddleNeoPixelBar.ActivePattern == PATTERN_NONE
            && UpperNeoPixelBar.ActivePattern == PATTERN_NONE) {

        uint16_t tInterval = random(40, 200);

        if ((sState & 1) == 1) {
            /*
             * Insert a random delay if sState is odd
             */
            aLedsPtr->Delay(random(200, 1000)); // to separate each pattern
            sState++;
            return;
        }

        int8_t tState = sState / 2;
        uint32_t tRandom = random();

        switch (tState) {
        case 0:
            // falling stars
            initMultipleFallingStars(&LowerNeoPixelBar, COLOR32_WHITE_HALF, 4, tInterval / 4, 2, &MultiPatterns, DIRECTION_UP);
            Serial.println(F("Clear"));

            MiddleNeoPixelBar.clear();
            initMultipleFallingStars(&UpperNeoPixelBar, COLOR32_WHITE_HALF, 4, tInterval / 4, 2, &MultiPatterns);
            break;
        case 1:
            LowerNeoPixelBar.ColorWipe(COLOR32_GREEN_QUARTER, tInterval, 0, DIRECTION_DOWN);
            MiddleNeoPixelBar.ScannerExtended(COLOR32_BLUE_HALF, 2, tInterval, 5,
            FLAG_SCANNER_EXT_CYLON | FLAG_SCANNER_EXT_VANISH_COMPLETE);
            UpperNeoPixelBar.ColorWipe(COLOR32_RED_QUARTER, tInterval);
            break;
        case 2:
            LowerNeoPixelBar.ColorWipe(COLOR32_BLACK, tInterval, FLAG_DO_NOT_CLEAR);
            UpperNeoPixelBar.ColorWipe(COLOR32_BLACK, tInterval, FLAG_DO_NOT_CLEAR, DIRECTION_DOWN);
            break;
        case 3:
            LowerNeoPixelBar.Stripes(COLOR32_BLACK, 12, NeoPatterns::Wheel(tRandom), 12, tInterval, 48, tRandom & DIRECTION_DOWN);
            MiddleNeoPixelBar.ScannerExtended(COLOR32_BLUE_HALF, 2, tInterval, 2,
            FLAG_SCANNER_EXT_ROCKET | FLAG_SCANNER_EXT_START_AT_BOTH_ENDS);

            UpperNeoPixelBar.Stripes(COLOR32_BLACK, 12, NeoPatterns::Wheel(tRandom), 12, tInterval, 48,
                    OppositeDirection(tRandom & DIRECTION_DOWN));
            sState = -2; // Start from beginning
            break;
        default:
            Serial.println("ERROR");
            break;
        }

        Serial.print("Pin=");
        Serial.print(aLedsPtr->getPin());
        Serial.print(" Length=");
        Serial.print(aLedsPtr->numPixels());
        Serial.print(" ActivePattern=");
        aLedsPtr->printPatternName(aLedsPtr->ActivePattern, &Serial);
        Serial.print("|");
        Serial.print(aLedsPtr->ActivePattern);
        Serial.print(" Interval=");
        Serial.print(tInterval);
        Serial.print(" StateFast=");
        Serial.println(tState);

        sState++;
    }
}
