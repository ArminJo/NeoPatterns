/*
 * SnakeGame.cpp
 *
 *  Simply runs the Snake game. It can be controlled by 2 or 4 buttons or by serial input (WASD).
 *  The experimental Python script in the extras folder converts key presses and game controller input to appropriate serial output for the game.
 *  After 7 seconds of inactivity it runs the Snake demo with a simple AI.
 *  The game starts in 2 button mode, i.e. one button for turn left and one for turn right.
 *  If one of the up or down button is used, 4 button mode is entered automatically.
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
#include <MatrixSnake.h>

// Delay between two SNAKE moves / Speed of game
#define GAME_REFRESH_INTERVAL   400

#define PIN_NEOPIXEL_MATRIX_SNAKE 8
#define MATRIX_NUMBER_OF_COLUMNS  8
#define MATRIX_NUMBER_OF_ROWS     8

#define RIGHT_BUTTON_PIN     2
#define LEFT_BUTTON_PIN      3
// If one of the up or down button is used, 4 button mode is entered automatically.
#define UP_BUTTON_PIN        4
#define DOWN_BUTTON_PIN      5

/*
 * Specify your matrix geometry as 4th parameter.
 * ....BOTTOM ....RIGHT specify the position of the zeroth pixel.
 * See MatrixNeoPatterns.h for further explanation.
 */
MatrixSnake NeoPixelMatrixSnake = MatrixSnake(MATRIX_NUMBER_OF_COLUMNS, MATRIX_NUMBER_OF_ROWS, PIN_NEOPIXEL_MATRIX_SNAKE,
NEO_MATRIX_BOTTOM | NEO_MATRIX_RIGHT | NEO_MATRIX_PROGRESSIVE, NEO_GRB + NEO_KHZ800);

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_USB) || defined(SERIAL_PORT_USBVIRTUAL)  || defined(ARDUINO_attiny3217)
    delay(2000); // To be able to connect Serial monitor after reset or power up and before first printout
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_NEOPATTERNS));

    // This initializes the NeoPixel library and checks if enough memory was available
    if (!NeoPixelMatrixSnake.begin(&Serial)) {
        Serial.println(F("Not enough memory for Snake matrix"));
        // Blink forever as error indicator
        while (true) {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(500);
            digitalWrite(LED_BUILTIN, LOW);
            delay(500);
        }
    }
    // Prepare for 4 button mode. If up and down button are not connected
    NeoPixelMatrixSnake.Snake(GAME_REFRESH_INTERVAL, COLOR32_BLUE, RIGHT_BUTTON_PIN, LEFT_BUTTON_PIN, UP_BUTTON_PIN,
    DOWN_BUTTON_PIN);

}

void loop() {
    static bool sButtonWasPressedOnce = false;

    if (NeoPixelMatrixSnake.Direction != DIRECTION_NONE) {
        // Direction is DIRECTION_NONE at start => direction != NONE indicates a pressed button
        sButtonWasPressedOnce = true;
    }

    if (!sButtonWasPressedOnce) {
        /*
         * Just wait for TIME_TO_SWITCH_TO_AUTO_MODE_MILLIS and then start snake autorun mode.
         */
        long tMillis = millis();
        if (tMillis > TIME_TO_SWITCH_TO_AUTO_MODE_MILLIS) {
            // switch to demo mode after switching delay if snake has not moved
            initSnakeAutorun(&NeoPixelMatrixSnake, GAME_REFRESH_INTERVAL / 2, COLOR32_BLUE);
        }
    }
    NeoPixelMatrixSnake.update();
}
