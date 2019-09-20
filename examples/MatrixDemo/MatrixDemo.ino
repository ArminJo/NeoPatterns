/*
 * MatrixDemo.cpp
 *
 *  Simply runs the MatrixAndSnakePatternsDemo for one 8x8 matrix at PIN_NEO_PIXEL_MATRIX.
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

#define VERSION_EXAMPLE "1.1"

#include <Arduino.h>
#include <MatrixSnake.h>

#define PIN_NEOPIXEL_MATRIX   8

#define NEOPIXEL_MATRIX_NUM_ROWS   		8
#define NEOPIXEL_MATRIX_NUM_COLUMNS   	8
#define NEOPIXEL_MATRIX_NUM_PIXELS   	(NEOPIXEL_MATRIX_NUM_COLUMNS * NEOPIXEL_MATRIX_NUM_ROWS)

/*
 * Specify your matrix geometry as 4th parameter.
 * ....BOTTOM ....RIGHT specify the position of the zeroth pixel.
 * See MatrixNeoPatterns.h for further explanation.
 */
MatrixSnake NeoPixelMatrix = MatrixSnake(NEOPIXEL_MATRIX_NUM_COLUMNS, NEOPIXEL_MATRIX_NUM_ROWS, PIN_NEOPIXEL_MATRIX,
NEO_MATRIX_BOTTOM | NEO_MATRIX_RIGHT | NEO_MATRIX_ROWS | NEO_MATRIX_PROGRESSIVE, NEO_GRB + NEO_KHZ800, &MatrixAndSnakePatternsDemo);

void setup() {
	pinMode(LED_BUILTIN, OUTPUT);

	Serial.begin(115200);
	while (!Serial)
		; //delay for Leonardo
	// Just to know which program is running on my Arduino
	Serial.println(F("START " __FILE__ "\r\nVersion " VERSION_EXAMPLE " from " __DATE__));

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

	extern void *__brkval;
	Serial.print(F("Free Ram/Stack[bytes]="));
	Serial.println(SP - (uint16_t) __brkval);

	MatrixAndSnakePatternsDemo(&NeoPixelMatrix);
}

uint8_t sWheelPosition = 0; // hold the color index for the changing ticker colors

void loop() {
	if (NeoPixelMatrix.Update()) {
		if (NeoPixelMatrix.ActivePattern == PATTERN_TICKER) {
			// change color of ticker after each update
			NeoPixelMatrix.Color1 = NeoPatterns::Wheel(sWheelPosition);
			sWheelPosition += 256 / NEOPIXEL_MATRIX_NUM_PIXELS;
		}
	}
}
