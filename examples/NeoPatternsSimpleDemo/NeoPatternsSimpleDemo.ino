/*
 *  NeoPatternsSimpleDemo.cpp
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

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN_NEOPIXEL_BAR_16          3

// onComplete callback functions
void TwoPatterns(NeoPatterns *aLedsPtr);

// The NeoPatterns instances
NeoPatterns bar16 = NeoPatterns(16, PIN_NEOPIXEL_BAR_16, NEO_GRB + NEO_KHZ800, &TwoPatterns);

void setup() {
    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_USB) || defined(SERIAL_PORT_USBVIRTUAL)  || defined(ARDUINO_attiny3217)
    delay(2000); // To be able to connect Serial monitor after reset or power up and before first printout
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_NEOPATTERNS));

    bar16.begin(); // This sets the pin.

    /*
     * First pattern ColorWipe
     */
    Serial.println("ColorWipe");
    bar16.ColorWipe(COLOR32(0, 0, 4), 50, REVERSE); // light Blue
    bar16.updateAndWaitForPatternToStop();
    delay(500);

    /*
     * Second pattern - RainbowCycle
     */
    Serial.println("RainbowCycle");
    bar16.RainbowCycle(10);
    bar16.updateAndWaitForPatternToStop();
    delay(500);

    /*
     * Third pattern - rocket
     */
    Serial.println("Rocket");
    bar16.ScannerExtended(COLOR32_WHITE_HALF, 7, 20, 0, FLAG_SCANNER_EXT_VANISH_COMPLETE);
    bar16.updateAndWaitForPatternToStop();
    delay(500);

    /*
     * Now trigger the automatic patterns
     */
    TwoPatterns(&bar16);

    Serial.println("started");
    randomSeed(1234);
}

void loop() {
    bar16.update();
    delay(10);
}

/*
 * Simple handler for switching between 2 patterns
 */
void TwoPatterns(NeoPatterns *aLedsPtr) {
    static int8_t sState = 0;

#if defined(__AVR__)
    uint32_t tRandom = random();
#else
    uint32_t tRandom = random(__UINT32_MAX__);
#endif
    uint8_t tDuration = random(20, 120);
    uint8_t tColor1 = tRandom;
    uint8_t tColor2 = tRandom >> 8;

    switch (sState) {
    case 0:
        // Scanner - use random mode and direction

        aLedsPtr->ScannerExtended(NeoPatterns::Wheel(tColor1), 4, tDuration, 2,
                (tRandom & FLAG_SCANNER_EXT_CYLON) | (tRandom & FLAG_SCANNER_EXT_VANISH_COMPLETE)
                        | (tRandom & FLAG_SCANNER_EXT_START_AT_BOTH_ENDS), ((tRandom >> 8) & DIRECTION_DOWN));
        break;

    case 1:
        // Stripes - use random direction
        aLedsPtr->Stripes(NeoPatterns::Wheel(tColor1), 5, NeoPatterns::Wheel(tColor2), 3, 2 * aLedsPtr->numPixels(), tDuration,
                (tRandom & DIRECTION_DOWN));
        break;

    case 2:
        /*
         * Non blocking delay implemented as pattern :-)
         */
        aLedsPtr->Delay(1000);
        sState = -1; // Start from beginning
        break;

    default:
        Serial.println("ERROR");
        break;
    }

    Serial.print(" ActivePattern=");
    aLedsPtr->printPatternName(aLedsPtr->ActivePattern, &Serial);
    Serial.print("|");
    Serial.print(aLedsPtr->ActivePattern);
    Serial.println();

    sState++;
}
