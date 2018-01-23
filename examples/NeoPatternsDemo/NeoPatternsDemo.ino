/*
 *  NeoPatternsDemo.cpp
 *
 *  Shows all patterns included in the NeoPattern library.
 *  First you need to install "Adafruit NeoPixel" library under Sketch -> Include Library -> Manage Librarys... -> use "neo" as filter string
 *
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

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN_BAR_16          7
#define PIN_BAR_24          4
#define PIN_RING_12         8
#define PIN_RING_16         6
#define PIN_RING_24         5
#define PIN_AREA_64         9

// onComplete callback functions
void allPatternsRandom(NeoPatterns * aLedsPtr);
void DemoPatternsGraphic(NeoPatterns * aLedsPtr);
void TestPatterns1(NeoPatterns * aLedsPtr);

// construct the NeoPatterns instances
NeoPatterns bar16 = NeoPatterns(16, PIN_BAR_16, NEO_GRB + NEO_KHZ800, &allPatternsRandom);
NeoPatterns bar24 = NeoPatterns(24, PIN_BAR_24, NEO_GRB + NEO_KHZ800, &TestPatterns1);
NeoPatterns ring12 = NeoPatterns(12, PIN_RING_12, NEO_GRB + NEO_KHZ800, &allPatternsRandom);
NeoPatterns ring16 = NeoPatterns(16, PIN_RING_16, NEO_GRB + NEO_KHZ800, &allPatternsRandom);
NeoPatterns ring24 = NeoPatterns(24, PIN_RING_24, NEO_GRB + NEO_KHZ800, &allPatternsRandom);
NeoPatterns area64 = NeoPatterns(64, PIN_AREA_64, NEO_GRB + NEO_KHZ800, &DemoPatternsGraphic);

// a demo graphics
const uint8_t heart8x8[] PROGMEM = { 0x66, 0xFF, 0xFF, 0xFF, 0x7E, 0x3C, 0x18, 0x00 };

uint8_t sWheelPosition = 0; // hold the color index for the changing ticker colors

void setup() {
    Serial.begin(115200);
    Serial.println("start");

    bar16.begin(); // This initializes the NeoPixel library.
    bar24.begin(); // This initializes the NeoPixel library.
    ring12.begin(); // This initializes the NeoPixel library.
    ring16.begin(); // This initializes the NeoPixel library.
    ring24.begin(); // This initializes the NeoPixel library.
    area64.begin(); // This initializes the NeoPixel library.

    bar16.isBar = true;
    bar24.isBar = true;
    ring12.ColorWipe(COLOR(255, 0, 255), 50); // Pink
    ring16.ColorWipe(COLOR(255, 0, 0), 50, REVERSE); // Red
    ring24.ColorWipe(COLOR(0, 255, 0), 50); // Green
    bar16.ColorWipe(COLOR(0, 0, 255), 50, REVERSE); // Blue
    bar24.ColorWipe(COLOR(0, 255, 255), 50); // lightblue
    area64.ColorSet(COLOR(0, 0, 0)); // Clear matrix
    area64.Delay(10000); // start later
    randomSeed(12345);

    Serial.println("started");
}

void loop() {
    bar16.Update();
    bar24.Update();
    ring12.Update();
    ring16.Update();
    ring24.Update();
    if (area64.Update()) {
        if (area64.ActivePattern == TICKER) {
            // change color of ticker after each update
            area64.Color1 = NeoPatterns::Wheel(sWheelPosition);
            sWheelPosition += 4;
        }
    }
}

/*
 * Handler for random pattern
 */
void allPatternsRandom(NeoPatterns * aLedsPtr) {
    uint8_t tState = random(11);

    uint8_t tDuration = random(40, 81);
    uint8_t tColor = random(255);

    switch (tState) {
    case 0:
        aLedsPtr->Cylon(NeoPatterns::Wheel(tColor), tDuration, 2);
        break;
    case 1:
        aLedsPtr->Scanner(NeoPatterns::Wheel(tColor), tDuration);
        break;
    case 2:
        // Falling star
        aLedsPtr->Scanner(COLOR(127, 127, 127), tDuration / 2, 5);
        break;
    case 3:
        aLedsPtr->RainbowCycle(20);
        break;
    case 4:
        aLedsPtr->TheaterChase(COLOR(127, 127, 127), COLOR(0, 0, 0), tDuration + tDuration / 2); // White on Black
        break;
    case 5:
        aLedsPtr->TheaterChase(NeoPatterns::Wheel(tColor), NeoPatterns::Wheel(tColor + 0x80), tDuration + tDuration / 2); //
        break;
    case 6:
        aLedsPtr->Fade(COLOR(255, 0, 0), COLOR(0, 0, 255), 32, tDuration, REVERSE);
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
        initFallingStar(aLedsPtr, COLOR(127, 127, 127), 3, &allPatternsRandom);
        break;
    case 10:
        if (aLedsPtr->isBar) {
            //Fire
            aLedsPtr->Fire(tDuration / 2, 150);
        } else {
            // start at both end
            aLedsPtr->Scanner(NeoPatterns::Wheel(tColor), tDuration, 6);
        }
        break;
    default:
        Serial.println("ERROR");
        break;
    }
}

void DemoPatternsGraphic(NeoPatterns * aLedsPtr) {
    static uint8_t sState = 0;
    static direction sHeartDirection = DOWN;

    sState++;
    /*
     * implement a delay between each case
     */
    if (sState % 2 == 1) {
        aLedsPtr->Delay(100); // not really needed
        return;
    }

    uint8_t tState = sState / 2;
    uint8_t tYOffset;
    switch (tState) {
    case 1:
        aLedsPtr->TickerPGM(PSTR("I love Neopixel"), NeoPatterns::Wheel(sWheelPosition), COLOR(0, 0, 0), 100, FORWARD);
        break;
    case 2:
        tYOffset = 8;
        if (sHeartDirection == UP) {
            tYOffset = -8;
        }
        aLedsPtr->MovingPicturePGM(heart8x8, COLOR(128, 0, 0), COLOR(0, 0, 0), 0, tYOffset, 9, 100, sHeartDirection);
        break;

    case 3:
        // Next 4 cases show 2 heart beats
        aLedsPtr->ColorForSelection = aLedsPtr->Color1;
        aLedsPtr->ProcessSelectiveColor(&DimColor, 6, 40);
        break;
    case 4:
        aLedsPtr->ProcessSelectiveColor(&LightenColor, 6, 40);
        break;
    case 5:
        aLedsPtr->ProcessSelectiveColor(&DimColor, 6, 40);
        break;
    case 6:
        aLedsPtr->ProcessSelectiveColor(&LightenColor, 6, 40);
        break;

    case 7:
        aLedsPtr->MovingPicturePGM(heart8x8, aLedsPtr->ColorForSelection, COLOR(0, 0, 0), 0, 0, 9, 100, sHeartDirection);
        // change direction for next time
        if (sHeartDirection == DOWN) {
            sHeartDirection = UP;
        } else {
            sHeartDirection = DOWN;
        }
        break;
    case 8:
        aLedsPtr->Delay(30000);
        // do not forget sState = 0; in last sensible case
        sState = 0;
        break;
    case 9:
        break;
    case 10:
        break;
    case 11:
        break;
    case 12:
        // safety net
        sState = 0;
        break;

// EXAMPLE ACTIONS for case
//        aLedsPtr->Delay(40000);
//        aLedsPtr->TickerPGM(PSTR("I love you"), COLOR(255, 0, 0), COLOR(0, 0, 0), 50, FORWARD);
//        aLedsPtr->MovingPicturePGM(heart8x8, COLOR(255, 0, 0), COLOR(0, 0, 0), 0, -8, 9, 100, UP);
//        aLedsPtr->MovingPicturePGM(heart8x8, COLOR(255, 0, 0), COLOR(0, 0, 0), 0, 0, 9, 100, UP);
//        aLedsPtr->MovingPicturePGM(heart8x8, COLOR(255, 0, 0), COLOR(0, 0, 0), 0, 8, 9, 100, DOWN);
//        aLedsPtr->MovingPicturePGM(heart8x8, COLOR(255, 0, 0), COLOR(0, 0, 0), 0, 0, 9, 100, DOWN);
//        aLedsPtr->MovingPicturePGM(heart8x8, COLOR(255, 0, 0), COLOR(0, 0, 0), -8, 0, 9, 100, FORWARD);
//        aLedsPtr->MovingPicturePGM(heart8x8, COLOR(255, 0, 0), COLOR(0, 0, 0), 0, 0, 9, 100, FORWARD);
//        aLedsPtr->MovingPicturePGM(heart8x8, COLOR(255, 0, 0), COLOR(0, 0, 0), 8, 0, 9, 100, REVERSE);
//        aLedsPtr->MovingPicturePGM(heart8x8, COLOR(255, 0, 0), COLOR(0, 0, 0), 0, 0, 9, 100, REVERSE);
//        aLedsPtr->FadeSelective(COLOR(255, 0, 0), COLOR(0, 255, 0), 20, 40);

    default:
        aLedsPtr->Delay(1);
        Serial.print("case ");
        Serial.print(tState);
        Serial.println(" not implemented");
        break;
    }
}

/*
 * Handler for testing fire patterns
 */
void TestPatterns1(NeoPatterns * aLedsPtr) {
    static uint8_t sState = 0;

    sState++;
    switch (sState) {
// No case 0!
    case 1:
        aLedsPtr->Fire(80, 100);  // too slow
        break;
    case 2:
        aLedsPtr->ColorWipe(COLOR(0, 255, 0), 5); // Green;
        break;
    case 3:
        aLedsPtr->Delay(800);
        break;
    case 4:
        aLedsPtr->Fire(20, 400); // OK
        break;
    case 5:
        aLedsPtr->ColorWipe(COLOR(0, 255, 0), 5); // Green;
        break;
    case 6:
        aLedsPtr->Delay(800);
        break;
    case 7:
        aLedsPtr->Fire(30, 260); // OK
        break;
    case 8:
        // switches to random
        initFallingStar(aLedsPtr, COLOR(127, 127, 127), 3, &allPatternsRandom);
        sState = 0;
        break;
    default:
        Serial.println("ERROR");
        break;
    }
}
