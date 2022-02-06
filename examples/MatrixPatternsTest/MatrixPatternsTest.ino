/*
 * MatrixPatternsTest.cpp
 *
 *  For testing the MatrixNeoPatterns Fire, Snow, AllColors, and Snake
 *  the are selected by the two mode pins.
 *
 *  You need to install "Adafruit NeoPixel" library under "Tools -> Manage Libraries..." or "Ctrl+Shift+I" -> use "neoPixel" as filter string
 *
 *  Copyright (C) 2021-2022  Armin Joachimsmeyer
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

#define ENABLE_MATRIX_PATTERN_FIRE
#define ENABLE_MATRIX_PATTERN_SNOW
#define ENABLE_PATTERNS_FOR_SNAKE_AUTORUN
#include <MatrixSnake.hpp>

#if defined(__AVR__)
#include "AVRUtils.h" // for printFreeHeap()
//#include "AvrTracing.hpp"
#endif

#define USE_16_X_16_MATRIX
#if defined(USE_16_X_16_MATRIX)
#define PIN_NEOPIXEL_MATRIX         5
#define MATRIX_NUMBER_OF_COLUMNS   16
#define MATRIX_NUMBER_OF_ROWS      16
/*
 * Specify your matrix geometry as 4th parameter.
 * ....BOTTOM ....RIGHT specify the position of the zeroth pixel.
 * See MatrixNeoPatterns.h for further explanation.
 */
MatrixSnake NeoPixelMatrix = MatrixSnake(MATRIX_NUMBER_OF_COLUMNS, MATRIX_NUMBER_OF_ROWS, PIN_NEOPIXEL_MATRIX,
NEO_MATRIX_BOTTOM | NEO_MATRIX_LEFT | NEO_MATRIX_ROWS | NEO_MATRIX_ZIGZAG, NEO_GRB + NEO_KHZ800, NULL);
#else
#define PIN_NEOPIXEL_MATRIX         8
#define MATRIX_NUMBER_OF_COLUMNS    8
#define MATRIX_NUMBER_OF_ROWS       8
/*
 * Specify your matrix geometry as 4th parameter.
 * ....BOTTOM ....RIGHT specify the position of the zeroth pixel.
 * See MatrixNeoPatterns.h for further explanation.
 */
MatrixNeoPatterns NeoPixelMatrix = MatrixNeoPatterns(MATRIX_NUMBER_OF_COLUMNS, MATRIX_NUMBER_OF_ROWS, PIN_NEOPIXEL_MATRIX,
NEO_MATRIX_BOTTOM | NEO_MATRIX_RIGHT | NEO_MATRIX_ROWS | NEO_MATRIX_PROGRESSIVE, NEO_GRB + NEO_KHZ800, NULL);
#endif

/*
 * Mode select. Fire, Snow, AllColors, and Snake
 */
#define MODE_PIN_0     11
#define MODE_PIN_1     12
uint8_t readModePins();
void printModePinInfo();
// The supported modes
#define MODE_FIRE       0
#define MODE_SNOW       1
#define MODE_ALL_COLORS 2
#define MODE_SNAKE      3
#define MODE_NO_MODE  255

/*
 * Helper macro for getting a macro definition as string
 */
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

void setup() {
#if defined(__AVR__)
    initStackFreeMeasurement();
#endif
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(MODE_PIN_0, INPUT_PULLUP);
    pinMode(MODE_PIN_1, INPUT_PULLUP);

    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) || defined(SERIALUSB_PID) || defined(ARDUINO_attiny3217)
    delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_NEOPATTERNS));

    printModePinInfo();

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
    NeoPixelMatrix.clear();

//    for (int i = 0; i < 256; ++i) {
//        NeoPixelMatrix.setPixelColor(i, COLOR32_WHITE_32TH);
//        NeoPixelMatrix.show();
//        delay(10);
//    }
//    NeoPixelMatrix.clear();
//    for (int i = 0; i < 16; ++i) {
//        for (int j = 0; j < 16; ++j) {
//            NeoPixelMatrix.setMatrixPixelColor(j, i, COLOR32_WHITE_32TH);
//            NeoPixelMatrix.show();
//            delay(100);
//        }
//    }
//    NeoPixelMatrix.clear();

#if defined(__AVR__)
    printFreeHeap(&Serial);
    printStackUnusedAndUsedBytes(&Serial);
#endif
//    initTrace();
}

uint8_t sOldMode = MODE_NO_MODE;

void loop() {
    /*
     * Get new mode
     */
    uint8_t tMode = readModePins();
    if (sOldMode != tMode) {
        /*
         * Stop old pattern
         */
        if (sOldMode == MODE_FIRE) {
            NeoPixelMatrix.FireMatrixStop();

        } else if (sOldMode == MODE_SNOW) {
            NeoPixelMatrix.SnowStop();

        } else if (sOldMode == MODE_SNAKE) {
            NeoPixelMatrix.SnakeStop();
        }

        /*
         * Start new pattern
         */
        Serial.print(F("Set mode from "));
        Serial.print(sOldMode);
        Serial.print(F(" to "));
        if (tMode == MODE_FIRE) {
            Serial.print(F("Fire"));
            if (!NeoPixelMatrix.Fire(200, 30)) {
                Serial.print(F(": not enough heap available"));
                tMode = MODE_NO_MODE;
            }

        } else if (tMode == MODE_SNOW) {
            Serial.print(F("Snow"));
            if (!NeoPixelMatrix.Snow(500, 20)) {
                Serial.print(F(": not enough heap available"));
                tMode = MODE_NO_MODE;
            }

        } else if (tMode == MODE_ALL_COLORS) {
            Serial.print(F("All colors"));

        } else if (tMode == MODE_SNAKE) {
            Serial.print(F("Snake"));
            if (!initSnakeAutorun(&NeoPixelMatrix, 200, COLOR32_BLUE, 1)) {
                Serial.print(F(": not enough heap available"));
                tMode = MODE_NO_MODE;
            }

        } else {
            Serial.print(tMode);
        }
        Serial.println();
        sOldMode = tMode;
#if defined(__AVR__)
        printFreeHeap(&Serial);
#endif
        delay(100); // debounce

    } else if ((millis() - NeoPixelMatrix.lastUpdate) > NeoPixelMatrix.Interval) {
        /*
         * call update
         */
#if defined(__AVR__)
        printStackUnusedAndUsedBytesIfChanged(&Serial);
#endif
        if (tMode == MODE_FIRE) {
            NeoPixelMatrix.FireMatrixUpdate();
            NeoPixelMatrix.show();
            NeoPixelMatrix.TotalStepCounter = 42; // set to any value > 1

            /*
             * Can set cooling and sparking parameters by potentiometers
             */
            // set cooling. 10 to 25 are sensible with optimum around 14 to 20
            uint8_t tOldCooling = NeoPixelMatrix.PatternLength;
            NeoPixelMatrix.PatternLength = map(analogRead(A0), 0, 1023, 2, 40);
            if (tOldCooling != NeoPixelMatrix.PatternLength) {
                Serial.print(F("Cooling="));
                Serial.println(NeoPixelMatrix.PatternLength);
            }

            // Not yet implemented
//          NeoPixelMatrix.PatternFlags = map(analogRead(A1), 0, 1023, 30, 200);
//          Serial.print(F(" Sparking="));
//          Serial.println(NeoPixelMatrix.PatternFlags);

        } else if (tMode == MODE_SNOW) {
            NeoPixelMatrix.SnowUpdate();
            NeoPixelMatrix.show();
            NeoPixelMatrix.TotalStepCounter = 42; // set to any value > 1

        } else if (tMode == MODE_SNAKE) {
            NeoPixelMatrix.SnakeUpdate();
            NeoPixelMatrix.show();

        } else if (tMode == MODE_ALL_COLORS) {
            NeoPixelMatrix.drawAllColors();  // 1.2 ampere for 16x16 at 5 volt
            NeoPixelMatrix.show();
            delay(4000);

            // dim pattern
            Serial.println(F("Dim pattern"));
            for (int i = 0; i < 256; ++i) {
                NeoPixelMatrix.drawAllColors();
                NeoPixelMatrix.setAdafruitBrightnessValue(0xFF); // we reconstructed all colors to full brightness with drawAllColors()
                NeoPixelMatrix.setBrightness(NeoPixelMatrix.gamma8(255 - i));
                NeoPixelMatrix.show();
                delay(10);
            }

            // brighten pattern
            Serial.println(F("Brighten pattern"));
            for (int i = 0; i < 256; ++i) {
                // restore pixels
                NeoPixelMatrix.drawAllColors();
                NeoPixelMatrix.setAdafruitBrightnessValue(0xFF); // we reconstructed all colors to full brightness with drawAllColors()
                NeoPixelMatrix.setBrightness(NeoPixelMatrix.gamma8(i));
                NeoPixelMatrix.show();
                delay(10);
            }

            delay(4000);

            Serial.println(F("Draw all colors 2"));
            NeoPixelMatrix.drawAllColors2(); // 1.5 ampere for 16x16 at 5 volt
            NeoPixelMatrix.show();
            delay(4000);

            // dim pattern
            Serial.println(F("Dim pattern"));
            for (int i = 0; i < 256; ++i) {
                NeoPixelMatrix.drawAllColors2();
                NeoPixelMatrix.setAdafruitBrightnessValue(0xFF); // we reconstructed all colors to full brightness with drawAllColors()
                NeoPixelMatrix.setBrightness(NeoPixelMatrix.gamma8(255 - i));
                NeoPixelMatrix.show();
                delay(10);
            }

            // brighten pattern
            Serial.println(F("Brighten pattern"));
            for (int i = 0; i < 256; ++i) {
                // restore pixels
                NeoPixelMatrix.drawAllColors2();
                NeoPixelMatrix.setAdafruitBrightnessValue(0xFF); // we reconstructed all colors to full brightness with drawAllColors()
                NeoPixelMatrix.setBrightness(NeoPixelMatrix.gamma8(i));
                NeoPixelMatrix.show();
                delay(10);
            }
            delay(4000);
        }
    }
    delay(5);
}

#define MODE_PIN_0     11
#define MODE_PIN_1     12
uint8_t readModePins();
// The supported modes
#define MODE_FIRE       0
#define MODE_SNOW       1
#define MODE_ALL_COLORS 2
#define MODE_SNAKE      3

void printModePinInfo() {
    Serial.println(F("Mode pins are pin " STR(MODE_PIN_0) " (LSB) and " STR(MODE_PIN_1) " (MSB)"));
    Serial.println(F("Open -> 0, connected to GND -> 1"));
    Serial.println(F("Modes are: 0 = FIRE, 1 = SNOW, 2 = ALL_COLORS and 3 = SNAKE"));
}

uint8_t readModePins() {
    return (digitalRead(MODE_PIN_0) | (digitalRead(MODE_PIN_1) << 1)) ^ 0x03;
}
