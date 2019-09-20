/*
 * SnakeAutorun.cpp
 *
 *  It runs the snake game using your AI code in the findBestSnakeDirection() function.
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
#define GAME_REFRESH_INTERVAL   200

#define PIN_NEOPIXEL_MATRIX_SNAKE 8

#define RIGHT_BUTTON_PIN     2
#define LEFT_BUTTON_PIN      3
/*
 * if connected, use up or down button first after reset to enable 4 button direct direction input
 */
#define UP_BUTTON_PIN        4
#define DOWN_BUTTON_PIN      5

/*
 * Specify your matrix geometry as 4th parameter.
 * ....BOTTOM ....RIGHT specify the position of the zeroth pixel.
 * See MatrixNeoPatterns.h for further explanation.
 */
MatrixSnake NeoPixelMatrixSnake = MatrixSnake(8, 8, PIN_NEOPIXEL_MATRIX_SNAKE,
NEO_MATRIX_BOTTOM | NEO_MATRIX_RIGHT | NEO_MATRIX_ROWS | NEO_MATRIX_PROGRESSIVE, NEO_GRB + NEO_KHZ800);

/********************************************
 * Put your Snake solver code here
 * This function is called before every snake move and must return the new direction for the snake.
 *
 * TopLeft coordinates are 0,0
 * BottomRight are e.g 8,8
 *
 * aSnakeBodyArray[0] is head position of snake
 * aSnakeBodyArray[aSnakeLength-1] is tail position of snake
 *
 * Apple.x, Apple.y is position of the apple
 *
 * Useful functions are: aSnake->checkDirection(uint8_t aDirectionToCheck)
 *                       computeDirection(position aStartPosition, position aEndPosition)
 *                       computeNewHeadPosition(uint8_t aDirection, position * aSnakeNewHeadPosition)
 *
 * More functions can be found in MatrixSnake.h / .cpp
 *
 ********************************************/
uint8_t getNextSnakeDirection(MatrixSnake * aSnake) {

    /*
     * Call internal solver
     * Comment this to enable your own code
     */
    return aSnake->getNextSnakeDir();

    /*
     * Debug output
     */
    Serial.print(F("getSnakeDirection CurrentDirection="));
    Serial.print(DirectionToString(aSnake->Direction));
    Serial.print(F(" head=("));
    Serial.print(aSnake->SnakePixelList[0].x);
    Serial.print(',');
    Serial.print(aSnake->SnakePixelList[0].y);
    Serial.println(')');

    int8_t tDeltaX = aSnake->Apple.x - aSnake->SnakePixelList[0].x;
    int8_t tDeltaY = aSnake->Apple.y - aSnake->SnakePixelList[0].y;

    Serial.print(F("DeltaX="));
    Serial.print(tDeltaX);
    Serial.print(F(" DeltaY="));
    Serial.println(tDeltaY);

    uint8_t tNewDirection = aSnake->Direction;

    /*
     *  Simple example, go towards the apple.
     */

    /*
     * Avoid going to opposite direction, because this is invalid.
     * Eg. if actual direction is UP, we must not change to DOWN.
     */
    if (tDeltaX > 0 && aSnake->Direction != DIRECTION_LEFT) {
        tNewDirection = DIRECTION_RIGHT;
    } else if (tDeltaX < 0 && aSnake->Direction != DIRECTION_RIGHT) {
        tNewDirection = DIRECTION_LEFT;
    }
    if (tDeltaY > 0 && aSnake->Direction != DIRECTION_UP) {
        tNewDirection = DIRECTION_DOWN;
    } else if (tDeltaY < 0 && aSnake->Direction != DIRECTION_DOWN) {
        tNewDirection = DIRECTION_UP;
    }

    // check new direction...
    if (aSnake->checkDirection(tNewDirection) != 0) {
        /*
         * check was not successful just check all available directions
         */
        for (tNewDirection = 0; tNewDirection < NUMBER_OF_DIRECTIONS; ++tNewDirection) {
            if (aSnake->checkDirection(tNewDirection) == 0) {
                break;
            }
        }
    }

// End of dummy example
    Serial.print(F("NewDirection="));
    Serial.println(DirectionToString(tNewDirection));

    return tNewDirection;
}

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

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
    initSnakeAutorun(&NeoPixelMatrixSnake, GAME_REFRESH_INTERVAL, COLOR32_BLUE);
}

void loop() {
    NeoPixelMatrixSnake.Update();
}

