/*
 *  AllPatternsOnMultiDevices.cpp
 *
 *  Shows all patterns for strips rings and matrixes included in the NeoPattern MatrixNeoPattern and Snake library.
 *
 *  You need to install "Adafruit NeoPixel" library under "Tools -> Manage Libraries..." or "Ctrl+Shift+I" -> use "neoPixel" as filter string
 *
 *  Copyright (C) 2018-2022  Armin Joachimsmeyer
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

#include <MatrixSnake.hpp>

#if defined(__AVR__)
//#define DEBUG
#  if defined(DEBUG)
#include "AvrTracing.hpp"
#include "AVRUtils.h"
#  endif
#include "ADCUtils.hpp"

//#define ALL_PATTERN_ON_ONE_STRIP // shows all patterns on one consecutive device / multiple chained devices

#define VCC_STOP_THRESHOLD_MILLIVOLT 3400   // We have voltage drop at the connectors, so the battery voltage is assumed higher, than the Arduino VCC.
#define VCC_STOP_MIN_MILLIVOLT 3200         // We have voltage drop at the connectors, so the battery voltage is assumed higher, than the Arduino VCC.
#define VCC_CHECK_PERIOD_MILLIS 10000       // Period of VCC checks
#define VCC_STOP_PERIOD_REPETITIONS 9       // Shutdown after 9 times (18 seconds) VCC below VCC_STOP_THRESHOLD_MILLIVOLT or 1 time below VCC_STOP_MIN_MILLIVOLT
#define FALLING_STAR_DURATION 12
char sStringBufferForVCC[7] = "xxxxmV";
#endif // (__AVR__)

#define BRIGHTNESS_INPUT_PIN    A0

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN_NEOPIXEL_BAR_24     3
#define PIN_NEOPIXEL_BAR_16     4

#define PIN_NEOPIXEL_RING_16    5
#define PIN_NEOPIXEL_RING_24    6
#define PIN_NEOPIXEL_RING_12    7

#define PIN_NEOPIXEL_MATRIX      8

#define MATRIX_NUMBER_OF_COLUMNS 8
#define MATRIX_NUMBER_OF_ROWS    8

// onComplete callback functions
void TestPatterns(NeoPatterns *aLedsPtr);
#if defined(ALL_PATTERN_ON_ONE_STRIP)
#define PIN_NEOPIXEL_ALL        5
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
MatrixSnake NeoPixelMatrix = MatrixSnake(MATRIX_NUMBER_OF_COLUMNS, MATRIX_NUMBER_OF_ROWS, PIN_NEOPIXEL_MATRIX,
NEO_MATRIX_BOTTOM | NEO_MATRIX_RIGHT | NEO_MATRIX_ROWS | NEO_MATRIX_PROGRESSIVE, NEO_GRB + NEO_KHZ800,
        &MatrixAndSnakePatternsDemoHandler);

uint8_t readBrightness();
void checkAndHandleVCCTooLow();

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) /*stm32duino*/|| defined(USBCON) /*STM32_stm32*/|| defined(SERIALUSB_PID) || defined(ARDUINO_attiny3217)
    delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_NEOPATTERNS));
    ring12.PixelFlags |= PIXEL_FLAG_GEOMETRY_CIRCLE;
    ring16.PixelFlags |= PIXEL_FLAG_GEOMETRY_CIRCLE;
    ring24.PixelFlags |= PIXEL_FLAG_GEOMETRY_CIRCLE;

#if defined(ALL_PATTERN_ON_ONE_STRIP)
    allPixel.printConnectionInfo(&Serial);
#endif
    bar16.printConnectionInfo(&Serial);
    bar24.printConnectionInfo(&Serial);
    ring12.printConnectionInfo(&Serial);
    ring16.printConnectionInfo(&Serial);
    ring24.printConnectionInfo(&Serial);
    NeoPixelMatrix.printConnectionInfo(&Serial);

#if defined(SUPPORT_BRIGHTNESS)
    uint8_t tBrightness = readBrightness();
    randomSeed(tBrightness);
#else
    uint8_t tBrightness = 0; // value is ignored :-)
#endif

    // This initializes the NeoPixel library and checks if enough memory was available
    // check the last object defined
    if (!NeoPixelMatrix.begin(&Serial, tBrightness, true)) {
        // Blink forever
        while (true) {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(500);
            digitalWrite(LED_BUILTIN, LOW);
            delay(500);
        }
    }

#if defined(__AVR__)
#  if defined(DEBUG)
    initStackFreeMeasurement();
    initTrace();
    printFreeRam(&Serial);
#  endif
#endif

    bar16.begin(tBrightness, true); // This initializes the NeoPixel library. true for EnableBrightnessNonZeroMode
    bar24.begin(tBrightness, true); // This initializes the NeoPixel library.
    ring12.begin(tBrightness, true); // This initializes the NeoPixel library.
    ring16.begin(tBrightness, true); // This initializes the NeoPixel library.
    ring24.begin(tBrightness, true); // This initializes the NeoPixel library.

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

#if defined(__AVR__)
    /*
     * Print voltage once on matrix
     */
    uint16_t tVCC = getVCCVoltageMillivolt();
    if (tVCC < 4300) {
        itoa(tVCC, sStringBufferForVCC, 10);
        NeoPixelMatrix.Ticker(sStringBufferForVCC, NeoPatterns::Wheel(0), COLOR32_BLACK, 80, DIRECTION_LEFT);
    } else {
        NeoPixelMatrix.Delay(7000); // start later
    }
#else
    NeoPixelMatrix.Delay(7000); // start later
#endif // defined(__AVR__)

    Serial.println("started");
}

uint8_t sWheelPosition = 0; // hold the color index for the changing ticker colors

class PrintIfChanged {
public:
    const char *PGMTextPtr;
    uint8_t LastValuePrinted;
    PrintIfChanged(const char *aPGMText) { // @suppress("Class members should be properly initialized")
        PGMTextPtr = aPGMText;
    }
    void printIfChanged(uint8_t tValueToPrint) {
        if (LastValuePrinted != tValueToPrint) {
            LastValuePrinted = tValueToPrint;
            Serial.print(reinterpret_cast<const __FlashStringHelper*>(PGMTextPtr));
            Serial.println(tValueToPrint);
        }
    }
};

const char BrightnessPGM[] PROGMEM = "Brightness=";
PrintIfChanged sBrightnessPrint(BrightnessPGM);

void loop() {
#if defined(__AVR__)
    checkAndHandleVCCTooLow();
#endif // defined(__AVR__)

#if defined(SUPPORT_BRIGHTNESS)
    uint8_t tBrightness = readBrightness();
#else
    uint8_t tBrightness = 0; // value is ignored :-)
#endif
    //    sBrightnessPrint.printIfChanged(tBrightness);

    bar16.update(tBrightness);
    bar24.update(tBrightness);
    ring12.update(tBrightness);
    ring16.update(tBrightness);
    ring24.update(tBrightness);
    if (NeoPixelMatrix.update(tBrightness)) {
        if (NeoPixelMatrix.ActivePattern == MATRIX_PATTERN_TICKER) {
            // change color of ticker after each update
            NeoPixelMatrix.Color1 = NeoPatterns::Wheel(sWheelPosition);
            sWheelPosition += 4;
        } else if (NeoPixelMatrix.ActivePattern == SPECIAL_PATTERN_SNAKE) {
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
void TestPatterns(NeoPatterns *aLedsPtr) {
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

uint8_t readBrightness() {
    return NeoPixel::gamma8(analogRead(BRIGHTNESS_INPUT_PIN) >> 2);
}

#if defined(__AVR__)
/*
 * If voltage too low for VCC_STOP_PERIOD_REPETITIONS times clear all pattern and activate only 2 MultipleFallingStars pattern on the 2 bars
 */
void checkAndHandleVCCTooLow() {
    /*
     * Check VCC every 10 seconds
     */
    static long sLastMillisOfVoltageCheck;
    static bool sVoltageTooLowDetectedOnce = false; // one time flag
    static uint8_t sVoltageTooLowCounter;

    if (millis() - sLastMillisOfVoltageCheck >= VCC_CHECK_PERIOD_MILLIS) {
        sLastMillisOfVoltageCheck = millis();
        uint16_t tVCC = printVCCVoltageMillivolt(&Serial);

        if (!sVoltageTooLowDetectedOnce) {

            if (tVCC < VCC_STOP_THRESHOLD_MILLIVOLT) {
                /*
                 * Voltage too low, wait VCC_STOP_PERIOD_REPETITIONS (9) times and then shut down.
                 */
                if (tVCC < VCC_STOP_MIN_MILLIVOLT) {
                    // emergency shutdown
                    sVoltageTooLowCounter = VCC_STOP_PERIOD_REPETITIONS;
                    Serial.println(F("Voltage < 3.2 volt detected -> emergency shutdown"));
                } else {
                    sVoltageTooLowCounter++;
                    Serial.println(F("Voltage < 3.4 volt detected"));
                }
                if (sVoltageTooLowCounter == VCC_STOP_PERIOD_REPETITIONS) {
                    Serial.println(F("Shut down"));
                    sVoltageTooLowDetectedOnce = true;
                }
            } else {
                sVoltageTooLowCounter = 0;
            }

            if (sVoltageTooLowDetectedOnce) {
                initMultipleFallingStars(&bar16, COLOR32_WHITE_HALF, 7, FALLING_STAR_DURATION, 1, ENDLESS_HANDLER_POINTER);
                initMultipleFallingStars(&bar24, COLOR32_WHITE_HALF, 9, FALLING_STAR_DURATION, 1, ENDLESS_HANDLER_POINTER);
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
    if (sVoltageTooLowDetectedOnce) {
        bar16.update();
        bar24.update();
        delay(FALLING_STAR_DURATION);
        return;
    }

}
#endif // defined(__AVR__)
