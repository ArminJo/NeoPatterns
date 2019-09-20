/*
 * SnakeGame.cpp
 *
 *  Simply runs the Snake game. It can be controlled by 2 or 4 buttons or by serial input (WASD).
 *  The experimental Python script in the extras folder converts key presses and game controller input to appropriate serial output for the game.
 *  After 7 seconds of inactivity it runs the Snake demo with a simple AI.
 *  If pin 10 is connected to ground, the game is in 2 button mode, i.e. one button for turn left and one for turn right.
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

#define VERSION_EXAMPLE "1.0"

#include <Arduino.h>
#include <MatrixSnake.h>

// Delay between two SNAKE moves / Speed of game
#define GAME_REFRESH_INTERVAL   400

#define PIN_NEOPIXEL_MATRIX_SNAKE  8

#define RIGHT_BUTTON_PIN     2
#define LEFT_BUTTON_PIN      3
/*
 * if connected, use up or down button first after reset to enable 4 button direct direction input
 */
#define UP_BUTTON_PIN        4
#define DOWN_BUTTON_PIN      5

#define TWO_BUTTON_MODE_SELECT_PIN 10 // if pulled low, then 2 button mode is activated

/*
 * Specify your matrix geometry as 4th parameter.
 * ....BOTTOM ....RIGHT specify the position of the zeroth pixel.
 * See MatrixNeoPatterns.h for further explanation.
 */
MatrixSnake NeoPixelMatrixSnake = MatrixSnake(8, 8, PIN_NEOPIXEL_MATRIX_SNAKE,
NEO_MATRIX_BOTTOM | NEO_MATRIX_RIGHT | NEO_MATRIX_PROGRESSIVE, NEO_GRB + NEO_KHZ800);

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(TWO_BUTTON_MODE_SELECT_PIN, INPUT_PULLUP);

    Serial.begin(115200);
    while (!Serial)
        ; //delay for Leonardo
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ "\r\nVersion " VERSION_EXAMPLE " from " __DATE__));

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
    if (digitalRead(TWO_BUTTON_MODE_SELECT_PIN)) {
        // 4 button mode
        NeoPixelMatrixSnake.Snake(GAME_REFRESH_INTERVAL, COLOR32_BLUE, RIGHT_BUTTON_PIN, LEFT_BUTTON_PIN, UP_BUTTON_PIN,
        DOWN_BUTTON_PIN);
    } else {
        // 2 button mode
        NeoPixelMatrixSnake.Snake(GAME_REFRESH_INTERVAL, COLOR32_BLUE, RIGHT_BUTTON_PIN, LEFT_BUTTON_PIN, 0, 0);
    }
    pinMode(TWO_BUTTON_MODE_SELECT_PIN, INPUT);
}

void loop() {
    static bool sButtonWasPressed = false;
    // Direction is DIRECTION_NONE after each game.
    if ((!sButtonWasPressed) && NeoPixelMatrixSnake.Direction == DIRECTION_NONE) {
        /*
         * Just wait for TIME_TO_SWITCH_TO_AUTO_MODE_MILLIS and then start snake autorun mode.
         */
        long tMillis = millis();
        if (tMillis > TIME_TO_SWITCH_TO_AUTO_MODE_MILLIS) {
            // switch to demo mode after switching delay if snake has not moved
            initSnakeAutorun(&NeoPixelMatrixSnake, GAME_REFRESH_INTERVAL / 2, COLOR32_BLUE);
        }
    } else {
        sButtonWasPressed = true;
    }
    NeoPixelMatrixSnake.Update();
}
