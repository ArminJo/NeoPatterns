/*
 *  NeoPatternsDemo.cpp
 *
 *  Shows all patterns for strips rings and matrixes included in the NeoPattern MatrixNeoPattern and Snake library.
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

#define VERSION_EXAMPLE "1.1"

#include <Arduino.h>

#include <MatrixSnake.h>
#ifdef __AVR__
#include <avr/power.h>
#include <avr/pgmspace.h>
#endif

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN_NEOPIXEL_BAR_24     2
#define PIN_NEOPIXEL_BAR_16     3

#define PIN_NEOPIXEL_RING_16    4
#define PIN_NEOPIXEL_RING_24    5
#define PIN_NEOPIXEL_RING_12    6

#define PIN_NEOPIXEL_MATRIX     8

// onComplete callback functions
void TestPatterns(NeoPatterns * aLedsPtr);

// construct the NeoPatterns instances
NeoPatterns bar16 = NeoPatterns(16, PIN_NEOPIXEL_BAR_16, NEO_GRB + NEO_KHZ800, &allPatternsRandomExample);
NeoPatterns bar24 = NeoPatterns(24, PIN_NEOPIXEL_BAR_24, NEO_GRB + NEO_KHZ800, &TestPatterns);
NeoPatterns ring12 = NeoPatterns(12, PIN_NEOPIXEL_RING_12, NEO_GRB + NEO_KHZ800, &allPatternsRandomExample);
NeoPatterns ring16 = NeoPatterns(16, PIN_NEOPIXEL_RING_16, NEO_GRB + NEO_KHZ800, &allPatternsRandomExample);
NeoPatterns ring24 = NeoPatterns(24, PIN_NEOPIXEL_RING_24, NEO_GRB + NEO_KHZ800, &allPatternsRandomExample);
/*
 * Specify your matrix geometry as 4th parameter.
 * ....BOTTOM ....RIGHT specify the position of the zeroth pixel.
 * See MatrixNeoPatterns.h for further explanation.
 */
MatrixSnake NeoPixelMatrix = MatrixSnake(8, 8, PIN_NEOPIXEL_MATRIX,
NEO_MATRIX_BOTTOM | NEO_MATRIX_RIGHT | NEO_MATRIX_ROWS | NEO_MATRIX_PROGRESSIVE, NEO_GRB + NEO_KHZ800, &MatrixAndSnakePatternsDemo);

void setup() {
    Serial.begin(115200);
    while (!Serial); //delay for Leonardo
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ "\r\nVersion " VERSION_EXAMPLE " from " __DATE__));

    bar16.begin(); // This initializes the NeoPixel library.
    bar24.begin(); // This initializes the NeoPixel library.
    ring12.begin(); // This initializes the NeoPixel library.
    ring16.begin(); // This initializes the NeoPixel library.
    ring24.begin(); // This initializes the NeoPixel library.
    NeoPixelMatrix.begin(); // This initializes the NeoPixel library.

    bar16.PatternsGeometry = GEOMETRY_BAR;
    bar24.PatternsGeometry = GEOMETRY_BAR;
    ring12.ColorWipe(COLOR32_PURPLE, 50);
    ring16.ColorWipe(COLOR32_RED, 50, DIRECTION_DOWN);
    ring24.ColorWipe(COLOR32_GREEN, 50);
    bar16.ColorWipe(COLOR32_BLUE, 50, DIRECTION_DOWN);
    bar24.Stripes(COLOR32_BLUE, 5, COLOR32_RED, 3, 50, 48);
//    bar24.ScannerExtended(COLOR32_BLUE, 5, 50, 1,
//            FLAG_SCANNER_EXT_ROCKET | FLAG_SCANNER_EXT_VANISH_COMPLETE | FLAG_SCANNER_EXT_START_AT_BOTH_ENDS);
    NeoPixelMatrix.clear(); // Clear matrix
    NeoPixelMatrix.show();
    NeoPixelMatrix.Delay(5000); // start later
    Serial.println("started");
}

uint8_t sWheelPosition = 0; // hold the color index for the changing ticker colors

void loop() {
    bar16.Update();
    bar24.Update();
    ring12.Update();
    ring16.Update();
    ring24.Update();
    if (NeoPixelMatrix.Update()) {
        if (NeoPixelMatrix.ActivePattern == PATTERN_TICKER) {
            // change color of ticker after each update
            NeoPixelMatrix.Color1 = NeoPatterns::Wheel(sWheelPosition);
            sWheelPosition += 4;
        } else if (NeoPixelMatrix.ActivePattern == PATTERN_SNAKE) {
            if (NeoPixelMatrix.Index == 4) {
                NeoPixelMatrix.Direction = DIRECTION_LEFT;
            } else if (NeoPixelMatrix.Index == 8) {
                NeoPixelMatrix.Direction = DIRECTION_DOWN;
            }
        }
    }
}

/*
 * Handler for testing fire patterns
 */
void TestPatterns(NeoPatterns * aLedsPtr) {
    static int8_t sState = 0;

    switch (sState) {
    case 0:
        aLedsPtr->Delay(10);
        break;
    case 1:
        aLedsPtr->Delay(10);
        break;
    case 2:
        aLedsPtr->ColorWipe(COLOR32_GREEN, 5);
        break;
    case 3:
        aLedsPtr->Delay(400);
        break;
    case 4:
        aLedsPtr->Fire(20, 400); // OK
        break;
    case 5:
        aLedsPtr->ColorWipe(COLOR32_GREEN, 5);
        break;
    case 6:
        aLedsPtr->Delay(400);
        break;
    case 7:
        aLedsPtr->Fire(30, 260); // OK
        break;
    case 8:
        // switches to random
        initMultipleFallingStars(aLedsPtr, COLOR32_WHITE_HALF, 30, 3, &allPatternsRandomExample);
        sState = -1; // Start from beginning
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
    Serial.print(aLedsPtr->ActivePattern);
    Serial.print(" State=");
    Serial.println(sState);

    sState++;

}
