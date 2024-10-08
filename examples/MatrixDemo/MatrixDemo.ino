/*
 * MatrixDemo.cpp
 *
 *  Simply runs the MatrixAndSnakePatternsDemoHandler for one 8x8 matrix at PIN_NEO_PIXEL_MATRIX.
 *      1. Runs ticker "I love Neopixel" from right to left / from bottom to top
 *      2. Moves heart in from top / bottom, show 2 heart beats, and move heart out
 *      3. Show 2 snake runs / fire. Snake shows up on the odd loops, fire on the even ones
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

#define INFO

#define DO_NOT_SUPPORT_RGBW // saves up to 428 bytes additional program memory for the AllPatternsOnMultiDevices() example.
#define DO_NOT_SUPPORT_BRIGHTNESS // saves up to 428 bytes additional program memory for the AllPatternsOnMultiDevices() example.
//#define DO_NOT_SUPPORT_NO_ZERO_BRIGHTNESS // If activated, disables writing of zero only if brightness or color is zero. Saves up to 144 bytes ...

#define ENABLE_PATTERNS_FOR_MATRIX_AND_SNAKE_DEMO_HANDLER
#include <MatrixSnake.hpp>

#if defined(__AVR__)
#  if defined(DEBUG)
#include "AvrTracing.hpp"
#include "AVRUtils.h"
#  endif
#endif

#if defined(ESP32)
#define PIN_NEOPIXEL_MATRIX        2
#else
#define PIN_NEOPIXEL_MATRIX        8
#endif
#define MATRIX_NUMBER_OF_COLUMNS   8
#define MATRIX_NUMBER_OF_ROWS      8

/*
 * Specify your matrix geometry as 4th parameter.
 * ....BOTTOM ....RIGHT specify the position of the zeroth pixel.
 * See MatrixNeoPatterns.h for further explanation.
 */
MatrixSnake NeoPixelMatrix = MatrixSnake(MATRIX_NUMBER_OF_COLUMNS, MATRIX_NUMBER_OF_ROWS, PIN_NEOPIXEL_MATRIX,
NEO_MATRIX_BOTTOM | NEO_MATRIX_RIGHT | NEO_MATRIX_ROWS | NEO_MATRIX_PROGRESSIVE, NEO_GRB + NEO_KHZ800,
        &MatrixAndSnakePatternsDemoHandler);

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) /*stm32duino*/|| defined(USBCON) /*STM32_stm32*/ \
    || defined(SERIALUSB_PID)  || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_attiny3217)
    delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_NEOPATTERNS));
    NeoPixelMatrix.printConnectionInfo(&Serial);

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

#if defined(__AVR__)
#  if defined(DEBUG)
    initStackFreeMeasurement();
    initTrace();
    printFreeRam(&Serial);
#  endif
#endif

    MatrixAndSnakePatternsDemoHandler(&NeoPixelMatrix); // start pattern
}

uint8_t sWheelPosition = 0; // hold the color index for the changing ticker colors

void loop() {
    if (NeoPixelMatrix.update()) {
        if (NeoPixelMatrix.ActivePattern == MATRIX_PATTERN_TICKER) {
            // change color of ticker after each update
            NeoPixelMatrix.Color1 = NeoPatterns::Wheel(sWheelPosition);
            sWheelPosition += 4;
        }
    }
}
