/*
 * MatrixDemo.cpp
 *
 *
 *  Simply runs the snake game.
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

#define GAME_REFRESH_INTERVAL   400

#define PIN_NEO_PIXEL_MATRIX   8

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
MatrixSnake NeoPixelMatrix = MatrixSnake(8, 8, PIN_NEO_PIXEL_MATRIX,
NEO_MATRIX_BOTTOM | NEO_MATRIX_RIGHT | NEO_MATRIX_ROWS | NEO_MATRIX_PROGRESSIVE,
NEO_GRB + NEO_KHZ800, &MatrixAndSnakePatternsDemo);

void setup() {
    Serial.begin(115200);
    Serial.print((const __FlashStringHelper *) PSTR("START\r\nVersion "));
    Serial.print(VERSION_EXAMPLE);
    Serial.print((const __FlashStringHelper *) PSTR(" from  "));
    Serial.println(__DATE__);

    NeoPixelMatrix.begin(); // This initializes the NeoPixel library.

//        myLoadTest(&NeoPixelMatrix);
    MatrixAndSnakePatternsDemo(&NeoPixelMatrix);

    randomSeed(12345);

}

uint8_t sWheelPosition = 0; // hold the color index for the changing ticker colors

void loop() {
    if (NeoPixelMatrix.Update()) {
        if (NeoPixelMatrix.ActivePattern == TICKER) {
            // change color of ticker after each update
            NeoPixelMatrix.Color1 = NeoPatterns::Wheel(sWheelPosition);
            sWheelPosition += 4;
        }
    }
}
