/*
 *  AllPatternOnOneStrip.cpp
 *
 *  Shows all patterns included in the NeoPixel library for NeoPixel strips.
 *
 *  Add your pattern code to the functions Pattern[1,2]() and Pattern[1,2]Update() in Neopatterns.cpp (line 588ff.)
 *  to realize and see your own patterns.
 *  Enable TEST_OWN_PATTERNS on line 38 to test your patterns.
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

#define VERSION_EXAMPLE "2.0"

#define INFO // if not defined, no Serial related code should be linked

// Comment this out to test your own pattern implementation on line 74 ff.
//#define TEST_USER_PATTERNS

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN_NEOPIXEL_BAR_16          3

// onComplete callback functions
void allPatterns(NeoPatterns * aLedsPtr);

// construct the NeoPatterns instances
#ifdef TEST_USER_PATTERNS
void ownPatterns(NeoPatterns * aLedsPtr);
NeoPatterns bar16 = NeoPatterns(16, PIN_NEOPIXEL_BAR_16, NEO_GRB + NEO_KHZ800, &ownPatterns);
#else
NeoPatterns bar16 = NeoPatterns(16, PIN_NEOPIXEL_BAR_16, NEO_GRB + NEO_KHZ800, &allPatterns);
#endif

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

#ifdef INFO
    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__)
    while (!Serial); //delay for Leonardo, but this loops forever for Maple Serial
#endif
#if defined(SERIAL_USB)
    delay(2000); // To be able to connect Serial monitor after reset and before first printout
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ "\r\nVersion " VERSION_EXAMPLE " from " __DATE__));
#endif

    bar16.begin(); // This initializes the NeoPixel library.
    bar16.ColorWipe(COLOR32(0, 0, 02), 50, 0, REVERSE); // light Blue

#ifdef INFO
    Serial.println("started");
#endif
    delay(500);
}

void loop() {
    bar16.update();
    delay(10);
}

#ifdef TEST_USER_PATTERNS

/************************************************************************************************************
 * Put your own pattern code here
 * Provided are sample implementation not supporting initial direction DIRECTION_DOWN
 * Full implementation of this functions can be found in the NeoPatterns.cpp file of the NeoPatterns library
 ************************************************************************************************************/

/*
 * set all pixel to aColor1 and let a pixel of color2 move through
 * Starts with all pixel aColor1 and also ends with it.
 */
void UserPattern1(NeoPatterns * aNeoPatterns, color32_t aPixelColor, color32_t aBackgroundColor, uint16_t aIntervalMillis,
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
    aNeoPatterns->ColorSet(aBackgroundColor);
    aNeoPatterns->show();
    aNeoPatterns->lastUpdate = millis();
}

/*
 * @return - true if pattern has ended, false if pattern has NOT ended
 */
bool UserPattern1Update(NeoPatterns * aNeoPatterns, bool aDoUpdate) {
    /*
     * Sample implementation not supporting initial direction DIRECTION_DOWN
     */
    if (aDoUpdate) {
        if (aNeoPatterns->decrementTotalStepCounterAndSetNextIndex()) {
            return true;
        }
    }

    for (uint16_t i = 0; i < aNeoPatterns->numPixels(); i++) {
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
void UserPattern2(NeoPatterns * aNeoPatterns, color32_t aColor, uint16_t aIntervalMillis, uint16_t aRepetitions,
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
bool UserPattern2Update(NeoPatterns * aNeoPatterns, bool aDoUpdate) {
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
 * Handler for testing your own patterns
 */
void ownPatterns(NeoPatterns * aLedsPtr) {
    static int8_t sState = 0;

    uint8_t tDuration = random(20, 120);
    uint8_t tColor = random(255);
    uint8_t tRepetitions = random(2);

    switch (sState) {
    case 0:
        UserPattern1(aLedsPtr, COLOR32_RED_HALF, NeoPatterns::Wheel(tColor), tDuration, FORWARD);
        break;

    case 1:
        UserPattern2(aLedsPtr, NeoPatterns::Wheel(tColor), tDuration, tRepetitions, FORWARD);
        sState = -1; // Start from beginning
        break;

    default:
        Serial.println("ERROR");
        break;
    }

    sState++;
}
#endif // TEST_USER_PATTERNS

/*
 * Handler for all pattern
 */
void allPatterns(NeoPatterns * aLedsPtr) {
    static int8_t sState = 0;

    uint8_t tDuration = random(40, 81);
    uint8_t tColor = random(255);

#ifdef INFO
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
#ifdef INFO
        Serial.println("ERROR");
#endif
        break;
    }

#ifdef INFO
    Serial.print(" ActivePattern=");
    aLedsPtr->printPatternName(aLedsPtr->ActivePattern, &Serial);
    Serial.print("|");
    Serial.print(aLedsPtr->ActivePattern);
    Serial.println();
#endif

    sState++;
}
