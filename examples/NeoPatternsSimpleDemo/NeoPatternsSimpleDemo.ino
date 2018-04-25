/*
 *  NeoPatterns144.cpp
 *
 *  Shows all patterns included in the NeoPixel library for NeoPixel stripes.
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

#include <Arduino.h>

#include <NeoPatterns.h>
#ifdef __AVR__
#include <avr/power.h>
#include <avr/pgmspace.h>
#endif

#define VERSION_EXAMPLE "1.0"

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN_STRIPE_16          8

// onComplete callback functions
void allPatterns(NeoPatterns * aLedsPtr);

// construct the NeoPatterns instances
NeoPatterns stripe16 = NeoPatterns(16, PIN_STRIPE_16, NEO_GRB + NEO_KHZ800, &allPatterns);

void setup() {
    Serial.begin(115200);
    Serial.println(F("START " __FILE__ "\r\nVersion " VERSION_EXAMPLE " from  " __DATE__));

    stripe16.begin(); // This initializes the NeoPixel library.
    stripe16.ColorWipe(COLOR32(0, 0, 02), 50, REVERSE); // Blue

    randomSeed(12345);

    Serial.println("started");
}

void loop() {
    stripe16.Update();
}

/*
 * Handler for all pattern
 * since sState starts with (0++) scanner is the first pattern you see
 */
void allPatterns(NeoPatterns * aLedsPtr) {
    static uint8_t sState = 0;
    sState++;

    uint8_t tDuration = random(40, 81);
    uint8_t tColor = random(255);

    switch (sState) {
    case 0:
        aLedsPtr->Cylon(NeoPatterns::Wheel(tColor), tDuration, 2);
        break;
    case 1:
        aLedsPtr->Scanner(NeoPatterns::Wheel(tColor), tDuration);
        break;
    case 2:
        // Falling star
        aLedsPtr->Scanner(COLOR32_WHITE_HALF, tDuration / 2, 5);
        break;
    case 3:
        aLedsPtr->RainbowCycle(20);
        break;
    case 4:
        aLedsPtr->TheaterChase(COLOR32_WHITE_HALF, COLOR32_BLACK, tDuration + tDuration / 2); // White on Black
        break;
    case 5:
        aLedsPtr->TheaterChase(NeoPatterns::Wheel(tColor), NeoPatterns::Wheel(tColor + 0x80), tDuration + tDuration / 2); //
        break;
    case 6:
        aLedsPtr->Fade(COLOR32_RED, COLOR32_BLUE, 32, tDuration, REVERSE);
        break;
    case 7:
        aLedsPtr->ColorWipe(NeoPatterns::Wheel(tColor), tDuration);
        break;
    case 8:
        // start at both end
        aLedsPtr->Scanner(NeoPatterns::Wheel(tColor), tDuration / 2, 6);
        break;
    case 9:
        // Multiple falling star
        initFallingStar(aLedsPtr, COLOR32_WHITE_HALF, tDuration / 2, 3, &allPatterns);
        break;
    case 10:
        if (aLedsPtr->PatternsGeometry == GEOMETRY_BAR) {
            //Fire
            aLedsPtr->Fire(tDuration / 2, 150);
        } else {
            // start at both end
            aLedsPtr->Scanner(NeoPatterns::Wheel(tColor), tDuration, 6);
        }

        sState = 0; // Start from beginning
        break;
    default:
        Serial.println("ERROR");
        break;
    }
}
