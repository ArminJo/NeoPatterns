/*
 * MatrixDemo.cpp
 *
 *
 *  Simply runs the MatrixAndSnakePatternsDemo for one 8x8 matrix at PIN_NEO_PIXEL_MATRIX.
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

#define PIN_NEOPIXEL_MATRIX   8

/*
 * Specify your matrix geometry as 4th parameter.
 * ....BOTTOM ....RIGHT specify the position of the zeroth pixel.
 * See MatrixNeoPatterns.h for further explanation.
 */
MatrixSnake NeoPixelMatrix = MatrixSnake(8, 8, PIN_NEOPIXEL_MATRIX,
NEO_MATRIX_BOTTOM | NEO_MATRIX_RIGHT | NEO_MATRIX_ROWS | NEO_MATRIX_PROGRESSIVE,
NEO_GRB + NEO_KHZ800, &MatrixAndSnakePatternsDemo);

void setup() {
    Serial.begin(115200);
    Serial.println(F("START " __FILE__ "\r\nVersion " VERSION_EXAMPLE " from " __DATE__));

    NeoPixelMatrix.begin(); // This initializes the NeoPixel library.
    if (NeoPixelMatrix.numPixels() == 0) {
        Serial.println(F("ERROR Not enough free memory available!"));
    }
//    mySnowflakeTest(&NeoPixelMatrix);
    MatrixAndSnakePatternsDemo(&NeoPixelMatrix);
}

uint8_t sWheelPosition = 0; // hold the color index for the changing ticker colors

void loop() {
//    mySnowflakeTest(&NeoPixelMatrix);

    if (NeoPixelMatrix.Update()) {
        if (NeoPixelMatrix.ActivePattern == PATTERN_TICKER) {
            // change color of ticker after each update
            NeoPixelMatrix.Color1 = NeoPatterns::Wheel(sWheelPosition);
            sWheelPosition += 4;
        }
    }
}
