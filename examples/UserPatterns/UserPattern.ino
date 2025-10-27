/*
 *  UserPattern.cpp
 *
 *  User pattern must at least implement the following functions:
 *  bool UserPattern1Update(NeoPatterns *aNeoPatterns, bool aDoUpdate);
 *
 *  This example contains the definitions of all functions for UserPattern1 and UserPattern2.
 *  For convenience it also contains a test handler userPatternsHandler(),
 *  which switches between both patterns using different colors.
 *
 *  You need to install "Adafruit NeoPixel" library under "Tools -> Manage Libraries..." or "Ctrl+Shift+I" -> use "neoPixel" as filter string
 *
 *  Copyright (C) 2022-2024  Armin Joachimsmeyer
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

#define ENABLE_PATTERN_USER_PATTERN1
#define ENABLE_PATTERN_USER_PATTERN2
#define ENABLE_PATTERN_COLOR_WIPE
#define DEBUG

#include <NeoPatterns.hpp>

#define INFO // if not defined, no Serial related code should be linked

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN_NEOPIXEL_BAR_16          3

// onComplete callback handler for all patterns
void userPatternsHandler(NeoPatterns *aLedsPtr);

NeoPatterns bar16 = NeoPatterns(16, PIN_NEOPIXEL_BAR_16, NEO_GRB + NEO_KHZ800, &userPatternsHandler);

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) /*stm32duino*/|| defined(USBCON) /*STM32_stm32*/ \
    || defined(SERIALUSB_PID)  || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_attiny3217)
    delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_NEOPATTERNS));
    bar16.printConnectionInfo(&Serial);

    bar16.begin(); // This initializes the NeoPixel library.
    bar16.ColorWipe(COLOR32(0, 0, 02), 50, false, DIRECTION_DOWN); // light Blue

    Serial.println("started");

    delay(500);
}

void loop() {
    bar16.update();
    delay(10);
}

/************************************************************************************************************
 * Put your own pattern code here
 * Provided are sample implementations
 ************************************************************************************************************/
/*
 * Set all pixel to aBackgroundColor and let a pixel of Color1 move through.
 * Starts with all pixel aColor1 and also ends with it.
 */
void UserPattern1(NeoPatterns *aNeoPatterns, color32_t aPixelColor, color32_t aBackgroundColor, uint16_t aIntervalMillis,
        uint8_t aDirection) {
    aNeoPatterns->Interval = aIntervalMillis;
    aNeoPatterns->Color1 = aPixelColor;
    aNeoPatterns->LongValue1.BackgroundColor = aBackgroundColor;
    aNeoPatterns->Direction = aDirection;
    aNeoPatterns->TotalStepCounter = aNeoPatterns->numPixels() + 2; // + 2 for the first and last pattern
    if (aDirection == DIRECTION_UP) {
        aNeoPatterns->Index = -1;
    } else {
        aNeoPatterns->Index = aNeoPatterns->numPixels();
    }

    aNeoPatterns->setColor(aBackgroundColor);
    aNeoPatterns->showPatternInitially();
// must be after showPatternInitially(), since it requires the old value do detect asynchronous calling
    aNeoPatterns->ActivePattern = PATTERN_USER_PATTERN1;
#if defined(TRACE)
    aNeoPatterns->printInfo(&Serial, true);
#endif
}

/*
 * @return - true if pattern has ended, false if pattern has NOT ended
 */
bool UserPattern1Update(NeoPatterns *aNeoPatterns, bool aDoUpdate) {
    if (aDoUpdate) {
        Serial.println("User pattern 1 updated");
        Serial.flush();

        // decrementTotalStepCounterAndSetNextIndex() calls callback and returns true if pattern ended
        if (aNeoPatterns->decrementTotalStepCounterAndSetNextIndex()) {
            Serial.println("User pattern 1 ended");
            Serial.flush();
            return true;
        }
    }

    /*
     * Refresh pattern
     */
    for (unsigned int i = 0; i < aNeoPatterns->numPixels(); i++) {
        if (i == aNeoPatterns->Index) {
            aNeoPatterns->setPixelColor(i, aNeoPatterns->Color1);
        } else {
            aNeoPatterns->setPixelColor(i, aNeoPatterns->LongValue1.BackgroundColor);
        }
    }
    return false;
}

/*
 * Sample implementation
 * let a pixel of aColor move up and down
 * starts and ends with all pixel cleared
 */
void UserPattern2(NeoPatterns *aNeoPatterns, color32_t aColor, uint16_t aIntervalMillis, uint16_t aRepetitions,
        uint8_t aDirection) {

    aNeoPatterns->Interval = aIntervalMillis;
    aNeoPatterns->Color1 = aColor;
    aNeoPatterns->Direction = aDirection;
// *2 for up and down. (aNeoPatterns->numPixels() - 1) do not use end pixel twice.
// +1 for the initial pattern with end pixel. + 2 for the first and last clear pattern.
    aNeoPatterns->TotalStepCounter = ((aRepetitions + 1) * 2 * (aNeoPatterns->numPixels() - 1)) + 1 + 2;
    if (aDirection == DIRECTION_UP) {
        aNeoPatterns->Index = -1;
    } else {
        aNeoPatterns->Index = aNeoPatterns->numPixels();
    }

    aNeoPatterns->clear();
    aNeoPatterns->showPatternInitially();
// must be after showPatternInitially(), since it requires the old value do detect asynchronous calling
    aNeoPatterns->ActivePattern = PATTERN_USER_PATTERN2;
#if defined(TRACE)
    aNeoPatterns->printInfo(&Serial, true);
#endif
}

/*
 * @param aDoUpdate if true update the pattern by one step, otherwise only refresh it.
 * @return - true if pattern has ended, false if pattern has NOT ended
 */
bool UserPattern2Update(NeoPatterns *aNeoPatterns, bool aDoUpdate) {
    if (aDoUpdate) {
        // clear old pixel
        aNeoPatterns->setPixelColor(aNeoPatterns->Index, COLOR32_BLACK);

        if (aNeoPatterns->decrementTotalStepCounterAndSetNextIndex()) {
            return true;
        }
        /*
         * Next index
         */
        if (aNeoPatterns->Direction == DIRECTION_UP) {
            // do not use top pixel twice
            if (aNeoPatterns->Index == (aNeoPatterns->numPixels() - 1)) {
                aNeoPatterns->Direction = DIRECTION_DOWN;
            }
        } else {
            // do not use bottom pixel twice
            if (aNeoPatterns->Index == 0) {
                aNeoPatterns->Direction = DIRECTION_UP;
            }
        }
    }

    /*
     * Refresh pattern
     */
    if (aNeoPatterns->TotalStepCounter != 1) {
        // last pattern is clear
        aNeoPatterns->setPixelColor(aNeoPatterns->Index, aNeoPatterns->Color1);
    }
    return false;
}

/*
 * Handler for testing your user patterns
 */
void userPatternsHandler(NeoPatterns *aLedsPtr) {
    static int8_t sState = 0;

    uint8_t tDuration = random(20, 120);
    uint8_t tColor = random(255);
    uint8_t tRepetitions = random(2);
    Serial.print("sState=");
    Serial.println(sState);

    switch (sState) {
    case 0:
        Serial.println("Start user pattern 1");
        Serial.flush();
        // tRepetitions & DIRECTION_UP_DOWN_MASK leads to 0 or 2 which is UP and DOWN
        // dimColor() reduces color intensity to 50 %
        UserPattern1(aLedsPtr, COLOR32_RED_HALF, NeoPatterns::dimColor(NeoPatterns::Wheel(tColor)), tDuration, tRepetitions & DIRECTION_UP_DOWN_MASK);
        break;

    case 1:
        Serial.println("Start user pattern 2");
        UserPattern2(aLedsPtr, NeoPatterns::Wheel(tColor), tDuration, tRepetitions, DIRECTION_UP);
        sState = -1; // Start from beginning
        break;

    default:
        Serial.println("ERROR");
        break;
    }

    sState++;
}
