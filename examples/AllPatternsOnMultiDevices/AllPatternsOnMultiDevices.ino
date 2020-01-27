/*
 *  AllPatternsOnMultiDevices.cpp
 *
 *  Shows all patterns for strips rings and matrixes included in the NeoPattern MatrixNeoPattern and Snake library.
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

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/gpl.html>.
 *
 */

// Version 1.2
// - Added low battery voltage shutdown
#define VERSION_EXAMPLE "2.0"

#include <Arduino.h>

#include <MatrixSnake.h>
#if defined(__AVR__)
#include "ADCUtils.h"
#include <avr/power.h>
#include <avr/pgmspace.h>

//#define ALL_PATTERN_ON_ONE_STRIP

#define VCC_STOP_THRESHOLD_MILLIVOLT 3400   // We have voltage drop at the connectors, so the battery voltage is assumed higher, than the Arduino VCC.
#define VCC_STOP_MIN_MILLIVOLT 3200         // We have voltage drop at the connectors, so the battery voltage is assumed higher, than the Arduino VCC.
#define VCC_CHECK_PERIOD_MILLIS 2000         // Period of VCC checks
#define VCC_STOP_PERIOD_REPETITIONS 9       // Shutdown after 9 times (18 seconds) VCC below VCC_STOP_THRESHOLD_MILLIVOLT or 1 time below VCC_STOP_MIN_MILLIVOLT
#define FALLING_STAR_DURATION 12
#endif // (__AVR__)

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN_NEOPIXEL_BAR_24     2
#define PIN_NEOPIXEL_BAR_16     3

#define PIN_NEOPIXEL_RING_16    4
#define PIN_NEOPIXEL_RING_24    5
#define PIN_NEOPIXEL_RING_12    6

#define PIN_NEOPIXEL_MATRIX     8

// onComplete callback functions
void TestPatterns(NeoPatterns * aLedsPtr);
#ifdef ALL_PATTERN_ON_ONE_STRIP
#define PIN_NEOPIXEL_ALL        2
NeoPatterns allPixel = NeoPatterns(104, PIN_NEOPIXEL_ALL, NEO_GRB + NEO_KHZ800, &allPatternsRandomHandler);
NeoPatterns bar16 = NeoPatterns(&allPixel, 0, 16, true, &allPatternsRandomHandler);
NeoPatterns bar24 = NeoPatterns(&allPixel, 19, 24, true, &TestPatterns);
NeoPatterns ring12 = NeoPatterns(&allPixel, 46, 12, true, &allPatternsRandomHandler);
NeoPatterns ring16 = NeoPatterns(&allPixel, 61, 16, true, &allPatternsRandomHandler);
NeoPatterns ring24 = NeoPatterns(&allPixel, 80, 24, true, &allPatternsRandomHandler);
#else
// construct the NeoPatterns instances
NeoPatterns bar16 = NeoPatterns(16, PIN_NEOPIXEL_BAR_16, NEO_GRB + NEO_KHZ800, &allPatternsRandomHandler);
NeoPatterns bar24 = NeoPatterns(24, PIN_NEOPIXEL_BAR_24, NEO_GRB + NEO_KHZ800, &TestPatterns);
NeoPatterns ring12 = NeoPatterns(12, PIN_NEOPIXEL_RING_12, NEO_GRB + NEO_KHZ800, &allPatternsRandomHandler);
NeoPatterns ring16 = NeoPatterns(16, PIN_NEOPIXEL_RING_16, NEO_GRB + NEO_KHZ800, &allPatternsRandomHandler);
NeoPatterns ring24 = NeoPatterns(24, PIN_NEOPIXEL_RING_24, NEO_GRB + NEO_KHZ800, &allPatternsRandomHandler);
#endif

/*
 * Specify your matrix geometry as 4th parameter.
 * ....BOTTOM ....RIGHT specify the position of the zeroth pixel.
 * See MatrixNeoPatterns.h for further explanation.
 */
MatrixSnake NeoPixelMatrix = MatrixSnake(8, 8, PIN_NEOPIXEL_MATRIX,
NEO_MATRIX_BOTTOM | NEO_MATRIX_RIGHT | NEO_MATRIX_ROWS | NEO_MATRIX_PROGRESSIVE, NEO_GRB + NEO_KHZ800, &MatrixAndSnakePatternsDemoHandler);

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__)
    while (!Serial); //delay for Leonardo, but this loops forever for Maple Serial
#endif
#if defined(SERIAL_USB)
    delay(2000); // To be able to connect Serial monitor after reset and before first printout
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ "\r\nVersion " VERSION_EXAMPLE " from " __DATE__));

    // This initializes the NeoPixel library and checks if enough memory was available
    // check the last object defined
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
    // setup ADC reference and channel
    getVCCVoltageMillivoltSimple();

    extern void *__brkval;
    Serial.print(F("Free Ram/Stack[bytes]="));
    Serial.println(SP - (uint16_t) __brkval);
#endif

    bar16.begin(); // This initializes the NeoPixel library.
    bar24.begin(); // This initializes the NeoPixel library.
    ring12.begin(); // This initializes the NeoPixel library.
    ring16.begin(); // This initializes the NeoPixel library.
    ring24.begin(); // This initializes the NeoPixel library.

    ring12.PixelFlags |= PIXEL_FLAG_GEOMETRY_CIRCLE;
    ring16.PixelFlags |= PIXEL_FLAG_GEOMETRY_CIRCLE;
    ring24.PixelFlags |= PIXEL_FLAG_GEOMETRY_CIRCLE;


    delay(300); // to avoid partial patterns at power up

    ring12.ColorWipe(COLOR32_PURPLE, 50);
    ring16.ColorWipe(COLOR32_RED, 50, 0, DIRECTION_DOWN);
    ring24.ColorWipe(COLOR32_GREEN, 50);
    bar16.ColorWipe(COLOR32_BLUE, 50, 0, DIRECTION_DOWN);
    bar24.Stripes(COLOR32_BLUE, 5, COLOR32_RED, 3, 48, 50);
//    bar24.ScannerExtended(COLOR32_BLUE, 5, 50, 1,
//            FLAG_SCANNER_EXT_ROCKET | FLAG_SCANNER_EXT_VANISH_COMPLETE | FLAG_SCANNER_EXT_START_AT_BOTH_ENDS);
    NeoPixelMatrix.clear(); // Clear matrix
    NeoPixelMatrix.show();
    NeoPixelMatrix.Delay(7000); // start later
    setMatrixAndSnakePatternsDemoHandlerTickerText(F("I love NeoPixel"));

#if defined(__AVR__)
    /*
     * Print voltage once on matrix
     */
    char sStringBufferForVCC[7] = "xxxxmV";
    uint16_t tVCC = getVCCVoltageMillivoltSimple();
    if (tVCC < 4300) {
        itoa(tVCC, sStringBufferForVCC, 10);
        NeoPixelMatrix.Ticker(sStringBufferForVCC, NeoPatterns::Wheel(0), COLOR32_BLACK, 80, DIRECTION_LEFT);
    }
#endif // defined(__AVR__)

    Serial.println("started");
}

uint8_t sWheelPosition = 0; // hold the color index for the changing ticker colors

#if defined(__AVR__)
/*
 * Returns true if shutdown
 */
bool checkVCC(uint16_t aVCC) {
    static uint8_t sVoltageTooLowCounter;

    if (aVCC < VCC_STOP_THRESHOLD_MILLIVOLT) {
        /*
         * Voltage too low, wait VCC_STOP_PERIOD_REPETITIONS (9) times and then shut down.
         */
        if (aVCC < VCC_STOP_MIN_MILLIVOLT) {
            // emergency shutdown
            sVoltageTooLowCounter = VCC_STOP_PERIOD_REPETITIONS;
            Serial.println(F("Voltage < 3.2 volt detected"));
        } else {
            sVoltageTooLowCounter++;
            Serial.println(F("Voltage < 3.4 volt detected"));
        }
        if (sVoltageTooLowCounter == VCC_STOP_PERIOD_REPETITIONS) {
            Serial.println(F("Shut down"));
            return true;
        }
    } else {
        sVoltageTooLowCounter = 0;
    }
    return false;
}
#endif // defined(__AVR__)

void loop() {
#if defined(__AVR__)
    /*
     * Check VCC every 2 seconds
     */
    static long sLastMillisOfVoltageCheck;
    static bool sVoltageTooLow = false; // one time flag

    if (millis() - sLastMillisOfVoltageCheck >= VCC_CHECK_PERIOD_MILLIS) {
        sLastMillisOfVoltageCheck = millis();
        uint16_t tVCC = getVCCVoltageMillivoltSimple();
        Serial.print(F("VCC="));
        Serial.print(tVCC);
        Serial.println(F("mV"));

        if (!sVoltageTooLow) {
            if (checkVCC(tVCC)) {
                sVoltageTooLow = true;

                initMultipleFallingStars(&bar16, COLOR32_WHITE_HALF, 7, FALLING_STAR_DURATION, 1, NULL);
                initMultipleFallingStars(&bar24, COLOR32_WHITE_HALF, 9, FALLING_STAR_DURATION, 1, NULL);
                ring12.clear();
                ring12.show();
                ring16.clear();
                ring16.show();
                ring24.clear();
                ring24.show();
                NeoPixelMatrix.clear();
                NeoPixelMatrix.show();
                return;
            }
        }
    }
    if (sVoltageTooLow) {
        bar16.update();
        bar24.update();
        delay(FALLING_STAR_DURATION);
        return;
    }
#endif // defined(__AVR__)

    bar16.update();
    bar24.update();
    ring12.update();
    ring16.update();
    ring24.update();
    if (NeoPixelMatrix.update()) {
        if (NeoPixelMatrix.ActivePattern == PATTERN_TICKER) {
            // change color of ticker after each update
            NeoPixelMatrix.Color1 = NeoPatterns::Wheel(sWheelPosition);
            sWheelPosition += 4;
        } else if (NeoPixelMatrix.ActivePattern == PATTERN_SNAKE) {
            if (NeoPixelMatrix.Index == 4) {
                NeoPixelMatrix.Direction = DIRECTION_LEFT;
            } else if (NeoPixelMatrix.Index == 8) {
                NeoPixelMatrix.Direction = DIRECTION_DOWN;
            }
        }
    }
}

/*
 * Handler for testing patterns
 */
void TestPatterns(NeoPatterns * aLedsPtr) {
    static int8_t sState = 0;

    switch (sState) {
    case 0:
        aLedsPtr->ColorWipe(COLOR32_RED_HALF, 50);
        break;
    case 1:
        aLedsPtr->Delay(500);
        break;
    case 2:
        aLedsPtr->Heartbeat(COLOR32_GREEN, 50, 0);
        break;
    case 3:
        aLedsPtr->Delay(500);
        break;
    case 4:
        aLedsPtr->RainbowCycle(50, DIRECTION_UP);
        break;
    case 5:
        aLedsPtr->RainbowCycle(50, DIRECTION_DOWN);
        break;
    case 6:
        aLedsPtr->Delay(400);
        break;
    case 7:
        aLedsPtr->Fire(20, 400); // OK Fire(30, 260)is also OK
        break;
    case 8:
        // switch to random
        initMultipleFallingStars(aLedsPtr, COLOR32_WHITE_HALF, 7, 30, 3, &allPatternsRandomHandler);
        sState = -1; // Start from beginning
        break;
    default:
        Serial.println("ERROR");
        break;
    }

    Serial.print("TestPatterns: Pin=");
    Serial.print(aLedsPtr->getPin());
    Serial.print(" Length=");
    Serial.print(aLedsPtr->numPixels());
    Serial.print(" ActivePattern=");
    aLedsPtr->printPatternName(aLedsPtr->ActivePattern, &Serial);
    Serial.print("|");
    Serial.print(aLedsPtr->ActivePattern);
    Serial.print(" State=");
    Serial.println(sState);

    sState++;

}
