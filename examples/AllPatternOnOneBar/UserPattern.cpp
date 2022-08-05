/*
 *  UserPattern.cpp
 *
 *  Contains the user provided definitions of the functions UserPattern1() and UserPattern1().
 *  For convenience it also contains a test handler userPatternsHandler().
 *
 *  You need to install "Adafruit NeoPixel" library under "Tools -> Manage Libraries..." or "Ctrl+Shift+I" -> use "neoPixel" as filter string
 *
 *  Copyright (C) 2022  Armin Joachimsmeyer
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

#include <NeoPatterns.h>

#define INFO // if not defined, no Serial related code should be linked

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN_NEOPIXEL_BAR_16          3

/************************************************************************************************************
 * Put your own pattern code here
 * Provided are sample implementation not supporting initial direction DIRECTION_DOWN
 * Full implementation of this functions can be found in the NeoPatterns.cpp file of the NeoPatterns library
 ************************************************************************************************************/

/*
 * set all pixel to aColor1 and let a pixel of color2 move through
 * Starts with all pixel aColor1 and also ends with it.
 */
void UserPattern1(NeoPatterns *aNeoPatterns, color32_t aPixelColor, color32_t aBackgroundColor, uint16_t aIntervalMillis,
        uint8_t aDirection) {
    /*
     * Sample implementation not supporting DIRECTION_DOWN
     */
    aNeoPatterns->ActivePattern = PATTERN_USER_PATTERN1;
    aNeoPatterns->Interval = aIntervalMillis;
    aNeoPatterns->Color1 = aPixelColor;
    aNeoPatterns->LongValue1.BackgroundColor = aBackgroundColor;
    aNeoPatterns->Direction = aDirection;
    aNeoPatterns->TotalStepCounter = aNeoPatterns->numPixels() + 1;
    aNeoPatterns->setColor(aBackgroundColor);
    aNeoPatterns->show();
    aNeoPatterns->lastUpdate = millis();
}

/*
 * @return - true if pattern has ended, false if pattern has NOT ended
 */
bool UserPattern1Update(NeoPatterns *aNeoPatterns, bool aDoUpdate) {
    /*
     * Sample implementation not supporting initial direction DIRECTION_DOWN
     */
    if (aDoUpdate) {
        if (aNeoPatterns->decrementTotalStepCounterAndSetNextIndex()) {
            return true;
        }
    }

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
 * let a pixel of aColor move up and down
 * starts and ends with all pixel cleared
 */
void UserPattern2(NeoPatterns *aNeoPatterns, color32_t aColor, uint16_t aIntervalMillis, uint16_t aRepetitions,
        uint8_t aDirection) {
    /*
     * Sample implementation not supporting DIRECTION_DOWN
     */
    aNeoPatterns->ActivePattern = PATTERN_USER_PATTERN2;
    aNeoPatterns->Interval = aIntervalMillis;
    aNeoPatterns->Color1 = aColor;
    aNeoPatterns->Direction = aDirection;
    aNeoPatterns->Index = 0;
    // *2 for up and down. (aNeoPatterns->numPixels() - 1) do not use end pixel twice.
    // +1 for the initial pattern with end pixel. + 2 for the first and last clear pattern.
    aNeoPatterns->TotalStepCounter = ((aRepetitions + 1) * 2 * (aNeoPatterns->numPixels() - 1)) + 1 + 2;
    aNeoPatterns->clear();
    aNeoPatterns->show();
    aNeoPatterns->lastUpdate = millis();
}

/*
 * @return - true if pattern has ended, false if pattern has NOT ended
 */
bool UserPattern2Update(NeoPatterns *aNeoPatterns, bool aDoUpdate) {
    /*
     * Sample implementation
     */
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

    switch (sState) {
    case 0:
        UserPattern1(aLedsPtr, COLOR32_RED_HALF, NeoPatterns::Wheel(tColor), tDuration, DIRECTION_UP);
        break;

    case 1:
        UserPattern2(aLedsPtr, NeoPatterns::Wheel(tColor), tDuration, tRepetitions, DIRECTION_UP);
        sState = -1; // Start from beginning
        break;

    default:
        Serial.println("ERROR");
        break;
    }

    sState++;
}
