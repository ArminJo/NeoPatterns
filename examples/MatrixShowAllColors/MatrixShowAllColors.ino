/*
 * MatrixShowAllColors.cpp
 *
 *  This Example uses the drawAllColors() functions to show all colors.
 *  To dim the colors, storePixelBuffer() / restorePixelBuffer() together with setBrightness() and gamma8() are used.
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

    sPixelBuffer = (uint8_t *) malloc(NeoPixelMatrix.getPixelBufferSize());
}


void loop() {

    NeoPixelMatrix.drawAllColors();
    delay(4000);
    NeoPixelMatrix.storePixelBuffer(sPixelBuffer);

    // dim pattern
    for (int i = 0; i < 256; ++i) {
        NeoPixelMatrix.restorePixelBuffer(sPixelBuffer);
        NeoPixelMatrix.setBrightness(NeoPixelMatrix.gamma8(255 - i));
        NeoPixelMatrix.show();
        delay(10);
    }

    // brighten pattern
    for (int i = 0; i < 256; ++i) {
        // restore pixels
        NeoPixelMatrix.restorePixelBuffer(sPixelBuffer);
        NeoPixelMatrix.setBrightness(NeoPixelMatrix.gamma8(i));
        NeoPixelMatrix.show();
        delay(10);
    }

    delay(2000);

    NeoPixelMatrix.drawAllColors2();
    delay(4000);

    NeoPixelMatrix.storePixelBuffer(sPixelBuffer);

    // dim pattern
    for (int i = 0; i < 256; ++i) {
        NeoPixelMatrix.restorePixelBuffer(sPixelBuffer);
        NeoPixelMatrix.setBrightness(NeoPixelMatrix.gamma8(255 - i));
        NeoPixelMatrix.show();
        delay(10);
    }

    // brighten pattern
    for (int i = 0; i < 256; ++i) {
        // restore pixels
        NeoPixelMatrix.restorePixelBuffer(sPixelBuffer);
        NeoPixelMatrix.setBrightness(NeoPixelMatrix.gamma8(i));
        NeoPixelMatrix.show();
        delay(10);
    }
    delay(4000);
}
