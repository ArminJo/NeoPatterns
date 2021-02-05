/*
 *  MultiplePatternsOnOneBar.cpp
 *
 *  Runs 3 8-pixel patterns simultaneously on a 24 NeoPixel bar.
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

//#define DEBUG

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN_NEOPIXEL  2

// onComplete callback functions
void MultiPatterns(NeoPatterns *aLedsPtr);

// construct the NeoPatterns instances
NeoPatterns NeoPixelBar24 = NeoPatterns(24, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
// false -> do not allow show on partial NeoPixel bar
NeoPatterns LowerNeoPixelBar = NeoPatterns(&NeoPixelBar24, 0, 8, false, &MultiPatterns);
NeoPatterns MiddleNeoPixelBar = NeoPatterns(&NeoPixelBar24, 8, 8, false, &MultiPatterns);
NeoPatterns UpperNeoPixelBar = NeoPatterns(&NeoPixelBar24, 16, 8, false, &MultiPatterns);

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_USB) || defined(SERIAL_PORT_USBVIRTUAL)  || defined(ARDUINO_attiny3217)
    delay(2000); // To be able to connect Serial monitor after reset or power up and before first printout
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_NEOPATTERNS));

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
void MultiPatterns(NeoPatterns *aLedsPtr) {
    static int8_t sState = 1; // start with delay

    /*
     * Wait for the last pattern to end
     */
    if (LowerNeoPixelBar.TotalStepCounter != 0 || MiddleNeoPixelBar.TotalStepCounter != 0
            || UpperNeoPixelBar.TotalStepCounter != 0) {
        aLedsPtr->ActivePattern = PATTERN_NONE;
#ifdef DEBUG
        Serial.print(F("Lower counter="));
        Serial.print(LowerNeoPixelBar.TotalStepCounter);
        Serial.print(F(" Middle counter="));
        Serial.print(MiddleNeoPixelBar.TotalStepCounter);
        Serial.print(F(" Upper counter="));
        Serial.print(UpperNeoPixelBar.TotalStepCounter);
        Serial.println(F(" One pattern is still active"));
#endif
    } else {

        uint16_t tInterval = random(40, 200);

        if ((sState & 1) == 1) {
            /*
             * Insert a random delay if sState is odd
             */
            uint16_t tDelay = random(200, 1000);
            aLedsPtr->Delay(tDelay); // to separate each pattern
            sState++;
            Serial.print(F("Insert delay of "));
            Serial.println(tDelay);
            return;
        }

        int8_t tState = sState / 2;
#if defined(__AVR__)
        uint32_t tRandom = random();
#else
        uint32_t tRandom = random(__UINT32_MAX__);
#endif
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

        Serial.print(F("Pin="));
        Serial.print(aLedsPtr->getPin());
        Serial.print(F(" Length="));
        Serial.print(aLedsPtr->numPixels());
        Serial.print(F(" ActivePattern="));
        aLedsPtr->printPatternName(aLedsPtr->ActivePattern, &Serial);
        Serial.print('|');
        Serial.print(aLedsPtr->ActivePattern);
        Serial.print(F(" Interval="));
        Serial.print(tInterval);
        Serial.print(F(" State="));
        Serial.println(tState);

        sState++;
    }
}
