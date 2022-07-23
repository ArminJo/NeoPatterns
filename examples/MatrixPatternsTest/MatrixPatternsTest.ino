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
 *  along with this program. If not, see <http://www.gnu.org/licenses/gpl.html>.
 *
 */

#include <Arduino.h>

#define INFO

#define DO_NOT_SUPPORT_RGBW // saves up to 428 bytes additional program memory for the AllPatternsOnMultiDevices() example.
//#define DO_NOT_SUPPORT_BRIGHTNESS // saves up to 428 bytes additional program memory for the AllPatternsOnMultiDevices() example.
//#define DO_NOT_SUPPORT_NO_ZERO_BRIGHTNESS // If activated, disables writing of zero only if brightness or color is zero. Saves up to 144 bytes ...

#define ENABLE_MATRIX_PATTERN_FIRE
#define ENABLE_MATRIX_PATTERN_SNOW
#define ENABLE_PATTERNS_FOR_SNAKE_AUTORUN
#include <MatrixSnake.hpp>

#if defined(__AVR__)
#include "AVRUtils.h" // for printFreeHeap()
//#include "AvrTracing.hpp"
#endif

#define BRIGHTNESS_INPUT_PIN       A0
#define FIRE_COOLING_INPUT_PIN     A0

#define USE_16_X_16_MATRIX // else 8x8 matrix

#if defined(USE_16_X_16_MATRIX)
#define PIN_NEOPIXEL_MATRIX         8
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
/*
 * Use 8 x 8 matrix
 */
#define PIN_NEOPIXEL_MATRIX         8
#define MATRIX_NUMBER_OF_COLUMNS    8
#define MATRIX_NUMBER_OF_ROWS       8
/*
 * Specify your matrix geometry as 4th parameter.
 * ....BOTTOM ....RIGHT specify the position of the zeroth pixel.
 * See MatrixNeoPatterns.h for further explanation.
 */
MatrixSnake NeoPixelMatrix = MatrixSnake(MATRIX_NUMBER_OF_COLUMNS, MATRIX_NUMBER_OF_ROWS, PIN_NEOPIXEL_MATRIX,
NEO_MATRIX_BOTTOM | NEO_MATRIX_RIGHT | NEO_MATRIX_ROWS | NEO_MATRIX_PROGRESSIVE, NEO_GRB + NEO_KHZ800, NULL);
#endif

/*
 * Initial mode select. Fire, Snow, AllColors, and Snake
 */
#define MODE_PIN_0     11
#define MODE_PIN_1     12
uint8_t readModePins();
void printModePinInfo();
// The supported modes
#define MODE_SNOW       0
#define MODE_SNAKE      1
#define MODE_FIRE       2
#define MODE_ALL_COLORS 3
#define MODE_MAX        3
#define MODE_NO_MODE  255
volatile uint8_t sMode;
uint8_t sOldMode = MODE_NO_MODE;

#define USE_BUTTON_0  // Enable code for button 0 at INT0.
#include "EasyButtonAtInt01.hpp"
// The callback function for button 0
void handleButtonPress(bool aButtonToggleState);
EasyButton Button0AtPin2(&handleButtonPress);       // Only callback parameter -> button is connected to INT0

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
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) /*stm32duino*/|| defined(USBCON) /*STM32_stm32*/|| defined(SERIALUSB_PID) || defined(ARDUINO_attiny3217)
    delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_NEOPATTERNS));
    NeoPixelMatrix.printConnectionInfo(&Serial);

    printModePinInfo();
    sMode = readModePins();

#if defined(SUPPORT_BRIGHTNESS)
    uint8_t tBrightness = NeoPixel::gamma8(analogRead(BRIGHTNESS_INPUT_PIN) >> 2);
    randomSeed(tBrightness);
#else
    uint8_t tBrightness = 0; // value is ignored :-)
#endif

    // This initializes the NeoPixel library and checks if enough memory was available. true for EnableBrightnessNonZeroMode
    if (!NeoPixelMatrix.begin(&Serial, tBrightness, true)) {
        // Blink forever
        while (true) {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(500);
            digitalWrite(LED_BUILTIN, LOW);
            delay(500);
        }
    }
    NeoPixelMatrix.clear();

//    NeoPixelMatrix.setBrightnessValue(MAX_BRIGHTNESS);
//    for (int i = 0; i < 256; ++i) {
//        NeoPixelMatrix.setPixelColor(i, COLOR32(i,i,i));
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

void switchMode() {
    /*
     * Stop old pattern
     */
#if defined(__AVR__)
    printFreeHeap(&Serial);
    Serial.println(F("Stop old mode"));
#endif
    if (sOldMode == MODE_FIRE) {
        NeoPixelMatrix.FireMatrixStop();
    } else if (sOldMode == MODE_SNOW) {
        NeoPixelMatrix.SnowStop();
    } else if (sOldMode == MODE_SNAKE) {
        NeoPixelMatrix.SnakeStop();
    }
#if defined(__AVR__)
    printFreeHeap(&Serial);
    Serial.println(F("Old mode stopped, start new mode"));
#endif
    /*
     * Start new pattern
     */
    Serial.print(F("Set mode from "));
    Serial.print(sOldMode);
    Serial.print(F(" to "));
    if (sMode == MODE_FIRE) {
        Serial.println(F("Fire"));
        if (!NeoPixelMatrix.Fire(200, 30)) {
            Serial.println(F("Not enough heap available"));
            sMode = MODE_NO_MODE;
        }
    } else if (sMode == MODE_SNOW) {
        Serial.println(F("Snow"));
        if (!NeoPixelMatrix.Snow(500, 20)) {
            Serial.println(F("Not enough heap available"));
            sMode = MODE_NO_MODE;
        }
    } else if (sMode == MODE_ALL_COLORS) {
        Serial.print(F("All colors"));
    } else if (sMode == MODE_SNAKE) {
        Serial.println(F("Snake"));
        if (!initSnakeAutorun(&NeoPixelMatrix, 200, COLOR32_BLUE, 1)) {
            Serial.println(F("Not enough heap available"));
            sMode = MODE_NO_MODE;
        }
    } else {
        Serial.println(sMode);
    }

    sOldMode = sMode;
#if defined(__AVR__)
    printFreeHeap(&Serial);
    Serial.println(F("New mode started"));

#endif
    delay(100); // debounce
}

void loop() {

#if defined(SUPPORT_BRIGHTNESS)
    static uint8_t sLastBrightness;

    uint8_t tBrightness = MAX_BRIGHTNESS;
    if (sMode != MODE_FIRE) { // Fire runs with MAX_BRIGHTNESS
        tBrightness = NeoPixel::gamma8(analogRead(BRIGHTNESS_INPUT_PIN) >> 2);
        if ((tBrightness < 10 && sLastBrightness != tBrightness) || abs(sLastBrightness - tBrightness) > 3) {
            sLastBrightness = tBrightness;
            Serial.print(F("Brightness="));
            Serial.println(tBrightness);
        }
    }
#else
    uint8_t tBrightness = 0; // value is ignored :-)
#endif

    /*
     * Check if mode switched
     */
    if (sOldMode != sMode) {
        switchMode();
    }

    /*
     * call update
     */
#if defined(__AVR__)
    printStackUnusedAndUsedBytesIfChanged(&Serial);
#endif
    if (sMode == MODE_FIRE) {
        NeoPixelMatrix.TotalStepCounter = 42; // set to any value > 1
        /*
         * Can set cooling and sparking parameters by potentiometers
         */
        // set cooling. 10 to 25 are sensible with optimum around 14 to 20
        uint8_t tOldCooling = NeoPixelMatrix.ByteValue1.Cooling;
        NeoPixelMatrix.ByteValue1.Cooling = map(analogRead(FIRE_COOLING_INPUT_PIN), 0, 1023, 2, 40);
        if (tOldCooling != NeoPixelMatrix.ByteValue1.Cooling) {
            Serial.print(F("Cooling="));
            Serial.println(NeoPixelMatrix.ByteValue1.Cooling);
        }

        NeoPixelMatrix.update(tBrightness); // tBrightness is MAX_BRIGHTNESS here

        // Not yet implemented
//          NeoPixelMatrix.PatternFlags = map(analogRead(A1), 0, 1023, 30, 200);
//          Serial.print(F(" Sparking="));
//          Serial.println(NeoPixelMatrix.PatternFlags);

    } else {
        if (sMode == MODE_SNOW) {
            NeoPixelMatrix.update(tBrightness);
            NeoPixelMatrix.TotalStepCounter = 42; // set to any value > 1

        } else if (sMode == MODE_SNAKE) {
            NeoPixelMatrix.update(tBrightness);

        } else if (sMode == MODE_ALL_COLORS) {
            NeoPixelMatrix.drawAllColors();  // 1.2 ampere for 16x16 at 5 volt
            NeoPixelMatrix.show();
            delay(4000);
            if (sMode != MODE_ALL_COLORS) {
                return;
            }

            // dim pattern
            Serial.println(F("Dim pattern"));
            for (int i = 0; i < 256; ++i) {
                NeoPixelMatrix.setBrightnessValue(NeoPixelMatrix.gamma8(255 - i));
                NeoPixelMatrix.drawAllColors();
                NeoPixelMatrix.show();
                delay(10);
            }
            if (sMode != MODE_ALL_COLORS) {
                return;
            }

            // brighten pattern
            Serial.println(F("Brighten pattern"));
            for (int i = 0; i < 256; ++i) {
                // restore pixels
                NeoPixelMatrix.setBrightnessValue(NeoPixelMatrix.gamma8(i));
                NeoPixelMatrix.drawAllColors();
                NeoPixelMatrix.show();
                delay(10);
            }
            if (sMode != MODE_ALL_COLORS) {
                return;
            }

            delay(4000);
            if (sMode != MODE_ALL_COLORS) {
                return;
            }

            Serial.println(F("Draw all colors 2"));
            NeoPixelMatrix.drawAllColors2(); // 1.5 ampere for 16x16 at 5 volt
            NeoPixelMatrix.show();
            delay(4000);
            if (sMode != MODE_ALL_COLORS) {
                return;
            }

            // dim pattern
            Serial.println(F("Dim pattern"));
            for (int i = 0; i < 256; ++i) {
                NeoPixelMatrix.setBrightnessValue(NeoPixelMatrix.gamma8(255 - i));
                NeoPixelMatrix.drawAllColors2();
                NeoPixelMatrix.show();
                delay(10);
            }
            if (sMode != MODE_ALL_COLORS) {
                return;
            }

            // brighten pattern
            Serial.println(F("Brighten pattern"));
            for (int i = 0; i < 256; ++i) {
                // restore pixels
                NeoPixelMatrix.setBrightnessValue(NeoPixelMatrix.gamma8(i));
                NeoPixelMatrix.drawAllColors2();
                NeoPixelMatrix.show();
                delay(10);
            }
            if (sMode != MODE_ALL_COLORS) {
                return;
            }
            delay(4000);
        }
    }
    delay(5);
}

void printModePinInfo() {
    Serial.println(F("Mode pins for initial mode are pin " STR(MODE_PIN_0) " (LSB) and " STR(MODE_PIN_1) " (MSB)"));
    Serial.println(F("Open -> 0, connected to GND -> 1"));
    Serial.println(F("Modes are: 0 = SNOW, 1 = SNAKE, 2 = ALL_COLORS and 3 = FIRE"));
    Serial.println(F("Mode can be switched by pressing button at INT0"));

}

uint8_t readModePins() {
    return (digitalRead(MODE_PIN_0) | (digitalRead(MODE_PIN_1) << 1)) ^ 0x03;
}

void handleButtonPress(bool aButtonToggleState) {
    sMode++;
    if (sMode > MODE_MAX) {
        sMode = 0;
    }
}
