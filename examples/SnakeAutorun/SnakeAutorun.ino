/*
 * SnakeAutorun.cpp
 *
 *
 *  It runs the snake game using your AI code in the computeSnakeDirection() function.
 *
 *  You need to install "Adafruit NeoPixel" library under Sketch -> Include Library -> Manage Librarys... -> use "neoPixel" as filter string
 *
 *  Copyright (C) 2018  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/gpl.html>.
 *
 */

#define VERSION_EXAMPLE "1.0"

#include <Arduino.h>
#include <MatrixSnake.h>

// Delay between two SNAKE moves / Speed of game
#define GAME_REFRESH_INTERVAL   200

#define PIN_AREA_SNAKE       8

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
MatrixSnake NeoPixelMatrixSnake = MatrixSnake(8, 8, PIN_AREA_SNAKE,
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
 * Useful functions are: aSnake->checkDirection(uint8_t aDirectionToCheck)
 *                  and: computeDirection(position aStartPosition, position aEndPosition)
 *
 ********************************************/
uint8_t computeSnakeDirection(MatrixSnake * aSnake, uint8_t aColumns, uint8_t aRows, position aSnakeHeadPosition,
        position aApplePosition, uint8_t aActualDirection, uint16_t aSnakeLength, position * aSnakeBodyArray) {

    Serial.print(F("computeSnakeDirection aActualDirection="));
    Serial.print(aActualDirection);
    Serial.print(F(" head=("));
    Serial.print(aSnakeHeadPosition.x);
    Serial.print(',');
    Serial.print(aSnakeHeadPosition.y);
    Serial.println(')');

    uint8_t tNewDirection = aActualDirection;

    int8_t tDeltaX = aApplePosition.x - aSnakeHeadPosition.x;
    int8_t tDeltaY = aApplePosition.y - aSnakeHeadPosition.y;

    Serial.print(F("DeltaX="));
    Serial.print(tDeltaX);
    Serial.print(F(" DeltaY="));
    Serial.println(tDeltaY);

// Simple example

    /*
     * Avoid going to opposite direction, because this is invalid.
     * Eg. if actual direction is UP, we must not change to DOWN.
     */
    if (tDeltaX > 0 && aActualDirection != DIRECTION_LEFT) {
        tNewDirection = DIRECTION_RIGHT;
    }
    /*
     * And so on...
     */

    // check new direction...
    if (aSnake->checkDirection(tNewDirection) != 0) {
        // take next direction
        tNewDirection = (tNewDirection + 1) % NUMBER_OF_DIRECTIONS;
    }

// End of dummy example
    Serial.print(F("NewDirection="));
    Serial.println(tNewDirection);

    return tNewDirection;
}

void setup() {
    Serial.begin(115200);
    Serial.println(F("START " __FILE__ "\r\nVersion " VERSION_EXAMPLE " from  " __DATE__));

    NeoPixelMatrixSnake.begin(); // This initializes the NeoPixel library.
    initSnakeAutorun(&NeoPixelMatrixSnake, GAME_REFRESH_INTERVAL, COLOR32_BLUE);

    randomSeed(12345);
}

void loop() {
    NeoPixelMatrixSnake.Update();
}

