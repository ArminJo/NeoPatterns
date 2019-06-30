/*
 * SnowFlakes.cpp
 *
 *  First shows fixed snowflake pattern on a 8x8 Matrix and then creates random patterns.
 *  Inspired by https://learn.adafruit.com/neopixel-matrix-snowflake-sweater
 *
 *  You need to install "Adafruit NeoPixel" library under "Tools -> Manage Libraries..." or "Ctrl+Shift+I" -> use "neoPixel" as filter string
 *
 *  Copyright (C) 2019  Armin Joachimsmeyer
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

#define VERSION_EXAMPLE "1.0"

#include <Arduino.h>
#include <MatrixNeoPatterns.h>

#define PIN_NEOPIXEL_MATRIX   8

/*
 * Specify your matrix geometry as 4th parameter.
 * ....BOTTOM ....RIGHT specify the position of the zeroth pixel.
 * See MatrixNeoPatterns.h for further explanation.
 */
MatrixNeoPatterns NeoPixelMatrix = MatrixNeoPatterns(8, 8, PIN_NEOPIXEL_MATRIX,
NEO_MATRIX_BOTTOM | NEO_MATRIX_RIGHT | NEO_MATRIX_ROWS | NEO_MATRIX_PROGRESSIVE,
NEO_GRB + NEO_KHZ800, NULL);

uint8_t * sPixelBuffer;

#define TEST_DELAY_MILLIS 2000

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);
    while (!Serial)
        ; //delay for Leonardo
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ "\r\nVersion " VERSION_EXAMPLE " from " __DATE__));

//    bar24.begin();
    // This initializes the NeoPixel library and checks if enough memory was available
    if (!NeoPixelMatrix.begin(&Serial)) {
        // Blink forever
        while (true) {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(500);
            digitalWrite(LED_BUILTIN, LOW);
            delay(500);
        }
    }

    /*
     * Fixed pattern.
     * The numbers behind the pattern are related to the pattern number in:
     * https://learn.adafruit.com/neopixel-matrix-snowflake-sweater/code
     */
    Serial.println(F("Displaying 9 fixed patterns"));
    NeoPixelMatrix.drawQuarterPatternOdd(0xDA68, COLOR32_WHITE_HALF, COLOR32_BLACK); // Adafruit pattern 5
    delay(TEST_DELAY_MILLIS);
    NeoPixelMatrix.drawQuarterPatternOdd(0x7AC8, COLOR32_WHITE_HALF, COLOR32_BLACK); // 6
    delay(TEST_DELAY_MILLIS);
    NeoPixelMatrix.drawQuarterPatternOdd(0x6DA4, COLOR32_WHITE_HALF, COLOR32_BLACK); // 9
    delay(TEST_DELAY_MILLIS);
    NeoPixelMatrix.drawQuarterPatternOdd(0x8641, COLOR32_WHITE_HALF, COLOR32_BLACK); // 3
    delay(TEST_DELAY_MILLIS);

    NeoPixelMatrix.drawQuarterPatternOdd(~0x7AC8, COLOR32_WHITE_HALF, COLOR32_BLACK); // 11
    delay(TEST_DELAY_MILLIS);
    NeoPixelMatrix.drawQuarterPatternOdd(~0xDA68, COLOR32_WHITE_HALF, COLOR32_BLACK); // own pattern
    delay(TEST_DELAY_MILLIS);

    NeoPixelMatrix.drawQuarterPatternOdd(0x936D, COLOR32_WHITE_HALF, COLOR32_BLACK); // 8
    delay(TEST_DELAY_MILLIS);
    NeoPixelMatrix.drawQuarterPatternOdd(0x79AC, COLOR32_WHITE_HALF, COLOR32_BLACK); // 1
    delay(TEST_DELAY_MILLIS);
    NeoPixelMatrix.drawQuarterPatternOdd(0x79AD, COLOR32_WHITE_HALF, COLOR32_BLACK); // 1
    delay(TEST_DELAY_MILLIS);
    delay(4000);
    Serial.println(F("Now creating random patterns"));
}

void loop() {
    // show random pattern in random color
    uint16_t tPatternValue = random(250, 64000);
    uint8_t tColorValue = random(255);
    NeoPixelMatrix.drawQuarterPatternOdd(tPatternValue, NeoPixelMatrix.Wheel(tColorValue), COLOR32_BLACK); // 1
    Serial.print(F("Pattern value=0x"));
    Serial.print(tPatternValue,HEX);
    Serial.print(F(" color="));
    Serial.println(tColorValue);
    delay(TEST_DELAY_MILLIS);
}
