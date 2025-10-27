/*
 * MatrixPatternsTest.cpp
 *
 *  For testing the MatrixNeoPatterns Fire, Snow, AllColors, and Snake
 *  One analog input controls brightness, the other selects the fire cooling.
 *  Fire runs always with maximum brightness.
 *  Patterns can be switched with right button connected at pin 2
 *  or switched automatically for demo if pin 10 is connected to GND.
 *
 *  You need to install "Adafruit NeoPixel" library under "Tools -> Manage Libraries..." or "Ctrl+Shift+I" -> use "neoPixel" as filter string
 *
 *  Copyright (C) 2021-2024  Armin Joachimsmeyer
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
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/gpl.html>.
 *
 */

#include <Arduino.h>

#if defined(__AVR__)
#include "AVRUtils.h" // for printRAMInfo()
//#include "AvrTracing.hpp"
#endif
//#include "HexDump.hpp"

#define INFO

#define DO_NOT_SUPPORT_RGBW // saves up to 428 bytes additional program memory for the AllPatternsOnMultiDevices() example.
//#define DO_NOT_SUPPORT_BRIGHTNESS // saves up to 428 bytes additional program memory for the AllPatternsOnMultiDevices() example.
//#define DO_NOT_SUPPORT_NO_ZERO_BRIGHTNESS // If activated, disables writing of zero only if brightness or color is zero. Saves up to 144 bytes ...

#define ENABLE_MATRIX_PATTERN_FIRE
#define ENABLE_MATRIX_PATTERN_SNOW
#define ENABLE_PATTERN_TWINKLE
#define ENABLE_PATTERNS_FOR_SNAKE_AUTORUN
#include <MatrixSnake.hpp>

#define BRIGHTNESS_INPUT_PIN       A0
#define FIRE_COOLING_INPUT_PIN     A1

// onComplete callback handler for all patterns
void DemoModeHandler(NeoPatterns *aLedsPtr);
//#define USE_16_X_16_MATRIX          // else 8x8 matrix

#define PIN_NEOPIXEL_MATRIX         8
#if defined(USE_16_X_16_MATRIX)
#define MATRIX_NUMBER_OF_COLUMNS   16
#define MATRIX_NUMBER_OF_ROWS      16
#define MATRIX_GEOMETRY             (NEO_MATRIX_BOTTOM | NEO_MATRIX_LEFT | NEO_MATRIX_ROWS | NEO_MATRIX_ZIGZAG)
#else
#define MATRIX_NUMBER_OF_COLUMNS    8
#define MATRIX_NUMBER_OF_ROWS       8
#define MATRIX_GEOMETRY             (NEO_MATRIX_BOTTOM | NEO_MATRIX_RIGHT | NEO_MATRIX_ROWS | NEO_MATRIX_PROGRESSIVE)
#endif
/*
 * Specify your matrix geometry as 4th parameter.
 * ....BOTTOM ....RIGHT specify the position of the zeroth pixel.
 * See MatrixNeoPatterns.h for further explanation.
 */
MatrixSnake NeoPixelMatrix = MatrixSnake(MATRIX_NUMBER_OF_COLUMNS, MATRIX_NUMBER_OF_ROWS, PIN_NEOPIXEL_MATRIX, MATRIX_GEOMETRY,
NEO_GRB + NEO_KHZ800, &DemoModeHandler);

typedef void (MatrixSnake::*MatrixNeoPixelMemberFunctionPointer)(void);
/*
 * Initial mode select. Fire, Snow, AllColors, and Snake
 */
#define DEMO_MODE_PIN  10   // Run all tests automatically - demo of all test patterns
#define MODE_PIN_0     11
#define MODE_PIN_1     12
uint8_t readModePins();
void printModePinInfo();
void printlnMode(uint8_t aMode);
void switchMode();
bool startPattern(uint8_t aNewMode);
bool dimOrBrightenPattern(bool aDoBrighten, uint16_t aMillisecondsDelay, MatrixNeoPixelMemberFunctionPointer aPatternFunction);
bool delayAndCheckMode(uint16_t aMilliseconds);

#define MILLIS_BETWEEN_DIMMINGS 2000

/*
 * The supported modes
 */
#define MODE_SNOW       0
#define MODE_SNAKE      1
#define MODE_FIRE       2
#define MODE_TWINKLE    3
#define MODE_ALL_COLORS 4
#define MODE_MAX        4
#define MODE_NO_MODE  255 // in case of error
uint8_t sCurrentMode;
volatile bool sButtonJustPressed = false;
bool sDemoModeEnabled;

#define USE_BUTTON_0  // Enable code for button 0 at INT0.
#define NO_BUTTON_RELEASE_CALLBACK
#define BUTTON_DEBOUNCING_MILLIS 80 // With this you can adapt to the characteristic of your button. Default is 50.
#include "EasyButtonAtInt01.hpp"
// The callback function for button 0
void handleButtonPress(bool aButtonToggleState);
EasyButton Button0AtPin2(&handleButtonPress); // Only one callback parameter => button is connected to INT0

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
    pinMode(DEMO_MODE_PIN, INPUT_PULLUP);

    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) /*stm32duino*/|| defined(USBCON) /*STM32_stm32*/ \
    || defined(SERIALUSB_PID)  || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_attiny3217)
    delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_NEOPATTERNS));
    Serial.println(
            F(
                    "Button pin=" STR(INT0_PIN) ", Brightness pin=" STR(BRIGHTNESS_INPUT_PIN) ", Fire cooling pin=" STR(FIRE_COOLING_INPUT_PIN)));

    NeoPixelMatrix.printConnectionInfo(&Serial);

    printModePinInfo();
    sCurrentMode = readModePins(); // Read initial mode setting
    Serial.print(F("Start with mode "));
    printlnMode(sCurrentMode);

    sDemoModeEnabled = !digitalRead(DEMO_MODE_PIN);
    Serial.print(F("Demo mode "));
    if (sDemoModeEnabled) {
        Serial.print(F("en"));
    } else {
        Serial.print(F("dis"));
    }
    Serial.println(F("abled"));

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
    NeoPixelMatrix.testMapping(100); // Draw a triangle with its right angle at upper left

//    NeoPixelMatrix.setBrightness(MAX_BRIGHTNESS);
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
    printRAMInfo(&Serial);
#endif
//    initTrace();
    delay(1000);
    startPattern(sCurrentMode);
#if defined(__AVR__)
    printRAMInfo(&Serial);
#endif
}

void loop() {

#if defined(SUPPORT_BRIGHTNESS)
    static uint8_t sLastBrightness;

    uint8_t tBrightness;
    if (sCurrentMode == MODE_FIRE) {
        // Fire runs with MAX_BRIGHTNESS
        tBrightness = MAX_BRIGHTNESS;
    } else {
        tBrightness = NeoPixel::gamma8(analogRead(BRIGHTNESS_INPUT_PIN) >> 2);
        if (sCurrentMode == MODE_SNOW && tBrightness < 32) {
            tBrightness = 32; // Below 32, COLOR32_WHITE_64TH / COLOR32(4,4,4) changes to blue
        }
        if (abs(sLastBrightness - tBrightness) > (tBrightness / 8)) {
            sLastBrightness = tBrightness;
            Serial.print(F("Brightness="));
            Serial.println(tBrightness);
        }
    }
#else
    uint8_t tBrightness = 0; // value is ignored :-)
#endif

    sDemoModeEnabled = !digitalRead(DEMO_MODE_PIN);

    /*
     * Check if sCurrentMode switched by button ISR
     */
    if (sButtonJustPressed) {
        sButtonJustPressed = false;
        switchMode();
    }

    /*
     * call update
     */
#if defined(__AVR__)
    if (printStackMaxUsedAndUnusedSizesIfChanged(&Serial)) {
//        Serial.println(F("Dump stack / end of RAM"));
//        printMemoryHexDump((uint8_t*) (RAMEND - 512) + 1, 512);
    }
#endif
    if (sCurrentMode == MODE_FIRE) {
        if (!sDemoModeEnabled) {
            NeoPixelMatrix.TotalStepCounter = 3; // to enable fast ending, if demo mode is enabled again
        }
        /*
         * Can set cooling (and sparking- not yet) parameters by potentiometers
         */
        // set cooling. 10 to 25 are sensible with optimum around 14 to 20
        uint8_t tOldCooling = NeoPixelMatrix.ByteValue1.Cooling;
        NeoPixelMatrix.ByteValue1.Cooling = map(analogRead(FIRE_COOLING_INPUT_PIN), 0, 1023, 2, 40);
        if (tOldCooling != NeoPixelMatrix.ByteValue1.Cooling) {
            Serial.print(F("Cooling="));
            Serial.println(NeoPixelMatrix.ByteValue1.Cooling);
        }

        NeoPixelMatrix.update(MAX_BRIGHTNESS);

        // Not yet implemented
//          NeoPixelMatrix.PatternFlags = map(analogRead(BRIGHTNESS_INPUT_PIN), 0, 1023, 30, 200);
//          Serial.print(F(" Sparking="));
//          Serial.println(NeoPixelMatrix.PatternFlags);

    } else {
        if (sCurrentMode == MODE_SNOW) {
            NeoPixelMatrix.update(tBrightness);
            if (!sDemoModeEnabled) {
                // set to constant value to allow endless running
                NeoPixelMatrix.TotalStepCounter = 3; // 3 to enable fast ending, if demo mode is enabled again
            }

        } else if (sCurrentMode == MODE_TWINKLE) {
            NeoPixelMatrix.update(tBrightness);
            if (!sDemoModeEnabled) {
                NeoPixelMatrix.TotalStepCounter = 3; // 3 to enable fast ending, if demo mode is enabled again
            }

        } else if (sCurrentMode == MODE_SNAKE) {
            NeoPixelMatrix.update(tBrightness);
            if (!sDemoModeEnabled) {
                NeoPixelMatrix.Repetitions = 1; // 1 to enable fast ending, if demo mode is enabled again
            }

        } else if (sCurrentMode == MODE_ALL_COLORS) {
            /*
             * first color map 1 / drawAllColors, then next map / drawAllColors2
             */
            MatrixNeoPixelMemberFunctionPointer tDrawAllColorsFunctionPointer = &MatrixSnake::drawAllColors;
            for (int i = 0; i < 2; ++i) {
                Serial.print(F("Draw color map "));
                Serial.println(i);
                NeoPixelMatrix.drawAllColors();  // 1.2 ampere for 16x16 at 5 volt
                NeoPixelMatrix.show();

                // delay 4 seconds
                if (delayAndCheckMode(MILLIS_BETWEEN_DIMMINGS)) {
                    return;
                }
                // dim pattern
                dimOrBrightenPattern(false, 10, tDrawAllColorsFunctionPointer);
                // brighten pattern
                dimOrBrightenPattern(true, 10, tDrawAllColorsFunctionPointer);
                // delay 4 seconds
                if (delayAndCheckMode(MILLIS_BETWEEN_DIMMINGS)) {
                    return;
                }
                tDrawAllColorsFunctionPointer = &MatrixSnake::drawAllColors2; // Color map 2
            }

            if (sDemoModeEnabled) {
                startPattern(0); // start with first pattern
            }
        }
    }
    delay(5);
}

/*
 * @return true, if sCurrentMode changed
 */
bool delayAndCheckMode(uint16_t aMilliseconds) {
    for (uint16_t i = 0; i < aMilliseconds / 10; ++i) {
        delay(10);
        if (sButtonJustPressed) {
            return true;
        }
    }
    return false;
}
bool dimOrBrightenPattern(bool aDoBrighten, uint16_t aMillisecondsDelay, MatrixNeoPixelMemberFunctionPointer aPatternFunction) {
    if (aDoBrighten) {
        Serial.print(F("Brighten"));
    } else {
        Serial.print(F("Dim"));
    }
    Serial.println(F(" pattern"));
    for (int i = 0; i < 256; ++i) {
        if (aDoBrighten) {
            NeoPixelMatrix.setBrightness(NeoPixelMatrix.gamma8(i));
        } else {
            NeoPixelMatrix.setBrightness(NeoPixelMatrix.gamma8(255 - i));
        }
        (NeoPixelMatrix.*aPatternFunction)(); // Call member function, first parenthesis is required
        NeoPixelMatrix.show();
        delay(aMillisecondsDelay);
        if (sButtonJustPressed) {
            return true;
        }
    }
    return false;
}

/*
 * @return false, if not enough heap available / if calloc() failed
 */
bool startPattern(uint8_t aNewMode) {
    sCurrentMode = aNewMode;

    if (aNewMode == MODE_SNOW) {
        return NeoPixelMatrix.Snow(1000, 20);

    } else if (aNewMode == MODE_TWINKLE) {
        NeoPixelMatrix.Twinkle(COLOR32_SPECIAL, NeoPixelMatrix.getNumberOfPixels() / 4, 50, 60);
        return true;

    } else if (aNewMode == MODE_SNAKE) {
        return initSnakeAutorun(&NeoPixelMatrix, 200, COLOR32_BLUE, 2);

    } else if (aNewMode == MODE_FIRE) {
        return NeoPixelMatrix.Fire(800, 30);
    }
    return true;
}

void switchMode() {
    /*
     * Stop old pattern
     */
    Serial.println();
#if defined(__AVR__)
    printRAMInfo(&Serial);
#endif
    Serial.print(F("Stop old mode "));
    printlnMode(sCurrentMode);
    if (sCurrentMode == MODE_FIRE) {
        NeoPixelMatrix.FireMatrixStop();
    } else if (sCurrentMode == MODE_SNOW) {
        NeoPixelMatrix.SnowStop();
    } else if (sCurrentMode == MODE_SNAKE) {
        NeoPixelMatrix.SnakeStop();
    } else {
        DemoModeHandler(&NeoPixelMatrix); // no stop function, which calls handler, so call handler manually
    }
    Serial.println();
    /*
     * New pattern is started by completion handler called by *Stop() function
     */

#if defined(__AVR__)
    printRAMInfo(&Serial);
#endif

    delay(100); // debounce
}

/*
 * Handler for automatically playing all patterns
 */
void DemoModeHandler(NeoPatterns *aLedsPtr) {

    Serial.println();
    Serial.print(F("Old mode="));
    printlnMode(sCurrentMode);

    sCurrentMode++;
    if (sCurrentMode > MODE_MAX) {
        sCurrentMode = 0;
    }

    /*
     * Start new pattern
     */
    if (!startPattern(sCurrentMode)) {
        Serial.print(F("Not enough heap available for new mode "));
        printlnMode(sCurrentMode);
#if defined(__AVR__)
        printRAMInfo(&Serial);
#endif
        DemoModeHandler(aLedsPtr); // Try next pattern
    } else {
        Serial.print(F("New mode="));
        printlnMode(sCurrentMode);
        Serial.println();
    }
#if defined(__AVR__)
    printRAMInfo(&Serial);
#endif
}

void printlnMode(uint8_t aMode) {
    if (aMode == MODE_SNOW) {
        Serial.println(F("Snow"));
    } else if (aMode == MODE_SNAKE) {
        Serial.println(F("Snake"));
    } else if (aMode == MODE_TWINKLE) {
        Serial.println(F("Twinkle"));
    } else if (aMode == MODE_FIRE) {
        Serial.println(F("Fire"));
    } else if (aMode == MODE_ALL_COLORS) {
        Serial.println(F("All colors"));
    } else if (aMode == MODE_NO_MODE) {
        Serial.println(F("No pattern"));
    }
}

void printModePinInfo() {
    Serial.println(F("Mode pins for initial mode are pin " STR(MODE_PIN_0) " (LSB) and " STR(MODE_PIN_1) " (MSB)"));
    Serial.println(F("Open -> 0, connected to GND -> 1"));
    Serial.println(F("Modes are: 0 = SNOW, 1 = SNAKE, 2 = ALL_COLORS and 3 = FIRE"));
    Serial.println(F("Mode can be switched by pressing button at INT0 / pin " STR(INT0_PIN)));
    Serial.println(F("Demo mode can be enabled by connecting pin " STR(DEMO_MODE_PIN) " to GND"));
}

uint8_t readModePins() {
    return (digitalRead(MODE_PIN_0) | (digitalRead(MODE_PIN_1) << 1)) ^ 0x03;
}

void handleButtonPress(bool aButtonToggleState) {
    (void) aButtonToggleState;
    sButtonJustPressed = true;
}
