/*
 *  AllPatternOnOneStrip.cpp
 *
 *  Shows all patterns included in the NeoPixel library for NeoPixel strips.
 *
 *  Just add your pattern code to the functions Pattern[1,2]() and Pattern[1,2]Update() in Neopatterns.cpp (line 588ff.)
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

#define VERSION_EXAMPLE "1.0"

// Uncomment this to test your own pattern implementation on line 70 ff.
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
    Serial.begin(115200);
    while (!Serial)
        ; //delay for Leonardo
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ "\r\nVersion " VERSION_EXAMPLE " from " __DATE__));

    bar16.begin(); // This initializes the NeoPixel library.
    bar16.ColorWipe(COLOR32(0, 0, 02), 50, REVERSE); // light Blue

    Serial.println("started");
    delay(500);
}

void loop() {
    bar16.Update();
    delay(10);
}

#ifdef TEST_USER_PATTERNS
/*
 * Put your own pattern code here
 */

// Initialize for Pattern1
// set all pixel to aColor1 and let a pixel of color2 move through
void UserPattern1(NeoPatterns * aNeoPatterns, color32_t aColor1, color32_t aColor2, uint8_t aInterval, uint8_t aDirection) {
    aNeoPatterns->ActivePattern = PATTERN_USER_PATTERN1;
    aNeoPatterns->Interval = aInterval;
    aNeoPatterns->Color1 = aColor1;
    aNeoPatterns->Color2 = aColor2;
    aNeoPatterns->Direction = aDirection;
    aNeoPatterns->TotalStepCounter = aNeoPatterns->numPixels();
}

void UserPattern1Update(NeoPatterns * aNeoPatterns, bool aDoUpdate) {
    /*
     * Sample implementation
     */
    for (uint16_t i = 0; i < aNeoPatterns->numPixels(); i++) {
        if (i == aNeoPatterns->Index) {
            aNeoPatterns->setPixelColor(i, aNeoPatterns->Color2);
        } else {
            aNeoPatterns->setPixelColor(i, aNeoPatterns->Color1);
        }
    }
    if (aDoUpdate) {
        aNeoPatterns->NextIndexAndDecrementTotalStepCounter();
    }
}

// clear all pixel and let a pixel of color2 move up and down
void UserPattern2(NeoPatterns * aNeoPatterns, color32_t aColor1, color32_t aColor2, uint8_t aInterval, uint8_t aDirection) {
    /*
     * Sample implementation
     */
    aNeoPatterns->ActivePattern = PATTERN_USER_PATTERN2;
    aNeoPatterns->Interval = aInterval;
    aNeoPatterns->Color1 = aColor1;
    aNeoPatterns->Color2 = aColor2;
    aNeoPatterns->Direction = aDirection;
    aNeoPatterns->Index = 0;
    aNeoPatterns->TotalStepCounter = (2 * aNeoPatterns->numPixels()) - 1; // up and down but do nor use upper pixel twice
}

void UserPattern2Update(NeoPatterns * aNeoPatterns, bool aDoUpdate) {
    /*
     * Sample implementation
     */
    for (uint16_t i = 0; i < aNeoPatterns->numPixels(); i++) {
        if (i == aNeoPatterns->Index) {
            aNeoPatterns->setPixelColor(i, aNeoPatterns->Color2);
        } else {
            aNeoPatterns->setPixelColor(i, COLOR32_BLACK);
        }
    }

    if (aDoUpdate) {
        aNeoPatterns->NextIndexAndDecrementTotalStepCounter();
        if (aNeoPatterns->Index == aNeoPatterns->numPixels()) {
            // change direction
            aNeoPatterns->Direction = DIRECTION_DOWN;
            // do nor use upper pixel twice
            aNeoPatterns->Index--;
        }
    }
}

/*
 * Handler for testing your own patterns
 */
void ownPatterns(NeoPatterns * aLedsPtr) {
    static int8_t sState = 0;

    uint8_t tDuration = random(20, 120);
    uint8_t tColor = random(255);

    switch (sState) {
    case 0:
        UserPattern1(aLedsPtr, COLOR32_RED_HALF, NeoPatterns::Wheel(tColor), tDuration, FORWARD);
        break;

    case 1:
        UserPattern2(aLedsPtr, COLOR32_RED_HALF, NeoPatterns::Wheel(tColor), tDuration, FORWARD);
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

    Serial.print("Pin=");
    Serial.print(aLedsPtr->getPin());
    Serial.print(" Length=");
    Serial.print(aLedsPtr->numPixels());
    Serial.print(" State=");
    Serial.print(sState);

    switch (sState) {
    case 0:
        // simple scanner
        aLedsPtr->clear();
        aLedsPtr->ScannerExtended(NeoPatterns::Wheel(tColor), 5, tDuration, 2, FLAG_SCANNER_EXT_CYLON);
        break;
    case 1:
        // rocket and falling star - 2 times bouncing
        aLedsPtr->ScannerExtended(NeoPatterns::Wheel(tColor), 7, tDuration, 2,
        FLAG_SCANNER_EXT_ROCKET | FLAG_SCANNER_EXT_START_AT_BOTH_ENDS, (tDuration & DIRECTION_DOWN));
        break;
    case 2:
        // 1 times rocket or falling star
        aLedsPtr->clear();
        aLedsPtr->ScannerExtended(COLOR32_WHITE_HALF, 7, tDuration / 2, 0, FLAG_SCANNER_EXT_VANISH_COMPLETE,
                (tDuration & DIRECTION_DOWN));
        break;
    case 3:
        aLedsPtr->RainbowCycle(20);
        break;
    case 4:
        aLedsPtr->Stripes(COLOR32_WHITE_HALF, 5, NeoPatterns::Wheel(tColor), 3, tDuration, 2 * aLedsPtr->numPixels(),
                (tDuration & DIRECTION_DOWN));
        break;
    case 5:
        // old TheaterChase
        aLedsPtr->Stripes(NeoPatterns::Wheel(tColor), 1, NeoPatterns::Wheel(tColor + 0x80), 2, tDuration / 2,
                2 * aLedsPtr->numPixels(), (tDuration & DIRECTION_DOWN));
        break;
    case 6:
        aLedsPtr->Fade(COLOR32_RED, COLOR32_BLUE, 32, tDuration);
        break;
    case 7:
        aLedsPtr->ColorWipe(NeoPatterns::Wheel(tColor), tDuration);
        break;
    case 8:
        // clear pattern
        aLedsPtr->ColorWipe(COLOR32_BLACK, tDuration, FLAG_DO_NOT_CLEAR, DIRECTION_DOWN);
        break;
    case 9:
        // Multiple falling star
        initMultipleFallingStars(aLedsPtr, COLOR32_WHITE_HALF, 7, tDuration / 2, 3, &allPatterns);
        break;
    case 10:
        if (aLedsPtr->PatternsGeometry == GEOMETRY_BAR) {
            //Fire
            aLedsPtr->Fire(tDuration / 2, 150);
        } else {
            // start at both end
            aLedsPtr->ScannerExtended(NeoPatterns::Wheel(tColor), 5, tDuration, 0,
            FLAG_SCANNER_EXT_START_AT_BOTH_ENDS | FLAG_SCANNER_EXT_VANISH_COMPLETE);
        }

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
