/*
 *  AllPatternOnOneBar.cpp
 *
 *  Shows all NeoPatterns included in the NeoPatterns library randomly on one 16 pixel bar.
 *
 *  You need to install "Adafruit NeoPixel" library under "Tools -> Manage Libraries..." or "Ctrl+Shift+I" -> use "neoPixel" as filter string
 *
 *  Copyright (C) 2018-2022  Armin Joachimsmeyer
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

#include <Arduino.h>

#include <NeoPatterns.hpp>

#define INFO // if not defined, no Serial related code should be linked

//#define TEST_USER_PATTERNS // Activate this to test your own pattern implementation on line 74 ff.

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN_NEOPIXEL_BAR_16          3

// onComplete callback handler for all patterns
void allPatterns(NeoPatterns *aLedsPtr);

// construct the NeoPatterns instances
#if defined(TEST_USER_PATTERNS)
void userPatternsHandler(NeoPatterns *aLedsPtr);
NeoPatterns bar16 = NeoPatterns(16, PIN_NEOPIXEL_BAR_16, NEO_GRB + NEO_KHZ800, &userPatternsHandler);
#else
NeoPatterns bar16 = NeoPatterns(16, PIN_NEOPIXEL_BAR_16, NEO_GRB + NEO_KHZ800, &allPatterns);
#endif

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

#if defined(INFO)
    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) /*stm32duino*/|| defined(USBCON) /*STM32_stm32*/|| defined(SERIALUSB_PID) || defined(ARDUINO_attiny3217)
    delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_NEOPATTERNS));
    bar16.printConnectionInfo(&Serial);
#endif

    bar16.begin(); // This initializes the NeoPixel library.
    bar16.ColorWipe(COLOR32(0, 0, 02), 50, 0, DIRECTION_DOWN); // light Blue

#if defined(INFO)
    Serial.println("started");
#endif
    delay(500);
}

void loop() {
    bar16.update();
    delay(10);
}

/*
 * onComplete callback handler for all patterns
 */
void allPatterns(NeoPatterns *aLedsPtr) {
    static int8_t sState = 0;

    uint8_t tDuration = random(40, 81);
    uint8_t tColor = random(255);

#if defined(INFO)
    Serial.print("Pin=");
    Serial.print(aLedsPtr->getPin());
    Serial.print(" Length=");
    Serial.print(aLedsPtr->numPixels());
    Serial.print(" State=");
    Serial.print(sState);
#endif

    switch (sState) {
    case 0:
        // Cylon
        aLedsPtr->ScannerExtended(NeoPatterns::Wheel(tColor), 5, tDuration, 2, FLAG_SCANNER_EXT_CYLON);
        break;
    case 1:
        // Heartbeat
        aLedsPtr->Heartbeat(NeoPatterns::Wheel(tColor), tDuration / 2, 2);
        break;
    case 2:
        // rocket and falling star - 2 times bouncing
        aLedsPtr->ScannerExtended(NeoPatterns::Wheel(tColor), 7, tDuration, 2,
        FLAG_SCANNER_EXT_ROCKET | FLAG_SCANNER_EXT_START_AT_BOTH_ENDS, (tDuration & DIRECTION_DOWN));
        break;
    case 3:
        aLedsPtr->Stripes(NeoPatterns::Wheel(tColor), 5, NeoPatterns::Wheel(tColor + 0x80), 3, 2 * aLedsPtr->numPixels(),
                tDuration * 2, (tDuration & DIRECTION_DOWN));
        break;
    case 4:
        aLedsPtr->RainbowCycle(tDuration / 4, (tDuration & DIRECTION_DOWN));
        break;
    case 5:
        // old TheaterChase
        aLedsPtr->Stripes(NeoPatterns::Wheel(tColor), 1, NeoPatterns::Wheel(tColor + 0x80), 2, 2 * aLedsPtr->numPixels(),
                tDuration * 2, (tDuration & DIRECTION_DOWN));
        break;
    case 6:
        aLedsPtr->Fade(NeoPatterns::Wheel(tColor), NeoPatterns::Wheel(tColor + 0x80), 64, tDuration);
        break;
    case 7:
        aLedsPtr->ColorWipe(NeoPatterns::Wheel(tColor), tDuration);
        break;
    case 8:
        // clear existing color wipe
        aLedsPtr->ColorWipe(COLOR32_BLACK, tDuration, FLAG_DO_NOT_CLEAR, DIRECTION_DOWN);
        break;
    case 9:
        // Multiple falling star
        initMultipleFallingStars(aLedsPtr, COLOR32_WHITE_HALF, 7, tDuration / 2, 3, &allPatterns);
        break;
    case 10:
        if ((aLedsPtr->PixelFlags & PIXEL_FLAG_GEOMETRY_CIRCLE) == 0) {
            //Fire
            aLedsPtr->Fire(tDuration * 2, tDuration / 2);
        } else {
            // start at both end
            aLedsPtr->ScannerExtended(NeoPatterns::Wheel(tColor), 5, tDuration, 0,
            FLAG_SCANNER_EXT_START_AT_BOTH_ENDS | FLAG_SCANNER_EXT_VANISH_COMPLETE);
        }

        sState = -1; // Start from beginning
        break;
    default:
#if defined(INFO)
        Serial.println("ERROR");
#endif
        break;
    }

#if defined(INFO)
    Serial.print(" ActivePattern=");
    aLedsPtr->printPatternName(aLedsPtr->ActivePattern, &Serial);
    Serial.print("|");
    Serial.print(aLedsPtr->ActivePattern);
    Serial.println();
#endif

    sState++;
}
