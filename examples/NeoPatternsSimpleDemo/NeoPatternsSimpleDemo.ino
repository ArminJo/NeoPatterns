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
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/gpl.html>.
 *
 */

#include <Arduino.h>

#include <NeoPatterns.hpp>

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN_NEOPIXEL_BAR_16          3

// onComplete callback functions
void TwoPatterns(NeoPatterns *aLedsPtr);

// The NeoPatterns instances
NeoPatterns bar16 = NeoPatterns(16, PIN_NEOPIXEL_BAR_16, NEO_GRB + NEO_KHZ800, &TwoPatterns);

void setup() {
    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) /*stm32duino*/|| defined(USBCON) /*STM32_stm32*/ \
    || defined(SERIALUSB_PID)  || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_attiny3217)
    delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_NEOPATTERNS));
    bar16.printConnectionInfo(&Serial);

    bar16.begin(); // This sets the pin.

    /*
     * First pattern ColorWipe
     */
    Serial.println("ColorWipe");
    bar16.ColorWipe(COLOR32(0, 0, 4), 50, DIRECTION_DOWN); // light Blue
    bar16.updateShowAndWaitForPatternToStop();
    delay(500);

    /*
     * Second pattern - RainbowCycle
     */
    Serial.println("RainbowCycle");
    bar16.RainbowCycle(10);
    bar16.updateShowAndWaitForPatternToStop();
    delay(500);

    /*
     * Third pattern - rocket
     */
    Serial.println("Rocket");
    bar16.ScannerExtended(COLOR32_WHITE_HALF, 7, 20, 0, FLAG_SCANNER_EXT_VANISH_COMPLETE);
    bar16.updateShowAndWaitForPatternToStop();
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

    uint8_t tDuration = random8(20, 121);
    uint8_t tColorWheelIndex1 = random8();
    uint8_t tColorWheelIndex2 = random8();

    switch (sState) {
    case 0:
        // Scanner - use random mode and direction
        aLedsPtr->ScannerExtended(NeoPatterns::Wheel(tColorWheelIndex1), 4, tDuration, 2,
                (tColorWheelIndex2 & FLAG_SCANNER_EXT_CYLON) | (tColorWheelIndex2 & FLAG_SCANNER_EXT_VANISH_COMPLETE)
                        | (tColorWheelIndex2 & FLAG_SCANNER_EXT_START_AT_BOTH_ENDS), (tColorWheelIndex1 & DIRECTION_DOWN));
        break;

    case 1:
        // Stripes - use random direction
        aLedsPtr->Stripes(NeoPatterns::Wheel(tColorWheelIndex1), 5, NeoPatterns::Wheel(tColorWheelIndex2), 3,
                2 * aLedsPtr->numPixels(), tDuration, (tColorWheelIndex2 & DIRECTION_DOWN));
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
