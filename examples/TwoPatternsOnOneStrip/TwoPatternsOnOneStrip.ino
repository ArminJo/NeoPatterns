/*
 *  TwoPatternsOnOneStrip.cpp
 *
 *  Runs 2 patterns simultaneously on a 144 NeoPixel strip. One is the main background pattern
 *  and the other is the fast moves pattern intended to be more random and quite seldom.
 *  First the background pattern is completely generated
 *  Then the fast moves pattern overwrites the background. Therefore we can only use small patterns here which do not draw black pixels
 *
 *  The delay between patterns is controlled by a potentiometer at pin A0.
 *  The pattern stops / resumes if the button at pin 2 is pressed.
 *
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

#define ENABLE_PATTERN_STRIPES
#define ENABLE_PATTERN_COLOR_WIPE
#define ENABLE_PATTERN_SCANNER_EXTENDED
#define ENABLE_PATTERN_RAINBOW_CYCLE
#define DO_NOT_SUPPORT_RGBW // saves up to 428 bytes additional program memory for the AllPatternsOnMultiDevices() example.
//#define DO_NOT_SUPPORT_BRIGHTNESS // saves up to 428 bytes additional program memory for the AllPatternsOnMultiDevices() example.
//#define DO_NOT_SUPPORT_NO_ZERO_BRIGHTNESS // If activated, disables writing of zero only if brightness or color is zero. Saves up to 144 bytes ...
#include <NeoPatterns.hpp>

#define USE_BUTTON_1
#include "EasyButtonAtInt01.hpp"
EasyButton Button0AtPin3;

#define PIN_TIMING_DEBUG_BUTTON   6
#define PIN_DELAY_POTI     A0
// Which pin on the Arduino is connected to the NeoPixels?
#define PIN_NEOPIXEL_STRIP  5

//#define NEOPIXEL_STRIP_LENGTH  144
//#define NEOPIXEL_STRIP_LENGTH  256
#define NEOPIXEL_STRIP_LENGTH  300

#define INTERVAL_BACKGROUND_MIN 10
#define INTERVAL_FAST_MOVES_MIN 2 // measured 4.3 ms for 144 pixel, but the 1 ms clock interrupt is disabled while sending so 2-3 interrupts/ms-ticks are lost.

#define DELAY_MILLIS_BACKGROUND_MIN 500
#define DELAY_MILLIS_FAST_MOVES_MIN 4000
uint8_t sDelay; // from 1 to 28 in exponential scale

// onComplete callback functions
void PatternsBackground(NeoPatterns *aLedsPtr);
void PatternsFastMoves(NeoPatterns *aLedsPtr);

#if __has_include("ADCUtils.h")
#include "ADCUtils.hpp" // for getVCCVoltageMillivoltSimple()
#define TEST_PATTERN_LENGTH 16  // 6 is required by my Li-Ion battery
#endif
uint16_t getActualNeopixelLenghtSimple(NeoPatterns *aLedsPtr);

// construct the NeoPatterns instances
NeoPatterns NeoPatternsBackground = NeoPatterns(NEOPIXEL_STRIP_LENGTH, PIN_NEOPIXEL_STRIP, NEO_GRB + NEO_KHZ800,
        &PatternsBackground);
// Second pattern, which uses the same pixel memory
NeoPatterns NeoPatternsFastMoves;

bool sRunning = true;

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(10);
    digitalWrite(LED_BUILTIN, LOW);

    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) /*stm32duino*/|| defined(USBCON) /*STM32_stm32*/|| defined(SERIALUSB_PID) || defined(ARDUINO_attiny3217)
    delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_NEOPATTERNS));
    NeoPatternsBackground.printConnectionInfo(&Serial);

    // This initializes the NeoPixel library and checks if enough memory was available
    if (!NeoPatternsBackground.begin(&Serial)) {
        // Blink forever
        while (true) {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(500);
            digitalWrite(LED_BUILTIN, LOW);
            delay(500);
        }
    }

    /*
     * Get the actual length of the strip (which only can be lower than the current one)
     * and adjust pixel buffer to the new length.
     * After this initialize the pattern with the underlying object to also get the new length here
     */
    uint16_t tActualNeopixelLength = getActualNeopixelLenghtSimple(&NeoPatternsBackground);
    Serial.print(F("Actual neopixel length="));
    Serial.println(tActualNeopixelLength);
    // update length to next even number
    tActualNeopixelLength = (tActualNeopixelLength + 1) & ~0x01;
    if (tActualNeopixelLength != 0) {
        NeoPatternsBackground.updateLength(tActualNeopixelLength);
    }
    // must init after length update, in order to copy the right length
    NeoPatternsFastMoves.init(&NeoPatternsBackground, 0, tActualNeopixelLength, true, &PatternsFastMoves);
    NeoPatternsFastMoves.begin();
    NeoPatternsFastMoves.printConnectionInfo(&Serial);

    extern void *__brkval;
    Serial.print(F("Free Ram/Stack[bytes]="));
    Serial.println(SP - (uint16_t) __brkval);

    pinMode(PIN_TIMING_DEBUG_BUTTON, OUTPUT);

    NeoPatternsBackground.ColorWipe(COLOR32_GREEN_HALF, 8); // start the pattern
//    NeoPatternsBackground.setColor(COLOR32_RED); // start the pattern
    NeoPatternsFastMoves.ScannerExtended(COLOR32_BLUE_HALF, 16, 8, 0, FLAG_SCANNER_EXT_ROCKET, DIRECTION_DOWN); // start the pattern
//    NeoPatternsFastMoves.Delay(2 * DELAY_MILLIS_FAST_MOVES_MIN); // start the pattern
}

void loop() {
    if (sRunning) {

        bool tMustUpdate = NeoPatternsBackground.checkForUpdate() || NeoPatternsFastMoves.checkForUpdate();
        if (tMustUpdate) {
#if defined(DEBUG)
            uint32_t tStartMillis = millis();
#endif
            /*
             * Here at least one update is pending.
             * First the background pattern is completely generated
             * Then the fast moves pattern overwrites the background. Therefore we can only use small patterns which do not draw black pixels
             */
            NeoPatternsBackground.updateOrRedraw(true);
            NeoPatternsFastMoves.updateOrRedraw(true);
#if defined(DEBUG)

            uint32_t tEndMillis = millis();
            if ((tEndMillis - tStartMillis) > 2) {
                Serial.print("millis needed=");
                Serial.println((tEndMillis - tStartMillis));
            }
#endif
//            digitalWrite(PIN_TIMING_DEBUG_BUTTON, HIGH);
            NeoPatternsBackground.show(); // 4.5 ms for 144 pixel
        }
    }

    sRunning = !Button0AtPin3.ButtonToggleState;
}

/*
 * @return  0 if length could not be determined
 * Could be improved by:
 * 1. Check at position slight below first value how may pixels are required for a voltage drop.
 * 2  Re-check first position slowly with these numbers of pixels.
 */
uint16_t getActualNeopixelLenghtSimple(NeoPatterns *aLedsPtr) {
#if __has_include("ADCUtils.h")
    /*
     * First set ADC reference and channel and clear strip
     */
    getVCCVoltageMillivoltSimple(); // to set ADC channel and reference
    aLedsPtr->clear();
    aLedsPtr->show();
    delay(50);
    uint16_t tStartMillivolt = getVCCVoltageMillivoltSimple(); // it is faster to use this
    Serial.print(F("Start VCC="));
    Serial.print(tStartMillivolt);
    Serial.println(" mV");

    /*
     * First run is fast run
     */
    int tLedIndex = aLedsPtr->getNumberOfPixels() - 1;
    uint_fast8_t tStepWidth = 4;
    for (unsigned int i = 0; i <= 4; i += 4) {
        for (; tLedIndex >= 0; tLedIndex -= tStepWidth) {
            aLedsPtr->setPixelColor(tLedIndex, COLOR32_WHITE);
            aLedsPtr->show();
            delay(i);
            uint16_t tCurrentMillivolt = getVCCVoltageMillivoltSimple(); // we have a resolution of 20 mV
            if (tCurrentMillivolt < tStartMillivolt - 40) {
                tLedIndex++;
                break;
            }
        }
        /*
         * 2. run is slow run from starting nearby
         */
        aLedsPtr->clear();
        aLedsPtr->show();
        if (i != 0) {
            return tLedIndex;
        }
        delay(20);

        tLedIndex = tLedIndex + 36; // with 36 we get 8 steps back
        if (tLedIndex > (int) aLedsPtr->getNumberOfPixels() - 1) {
            tLedIndex = aLedsPtr->getNumberOfPixels() - 1;
        }
        tStepWidth = 1;
    }
    return 0;
#endif
}

/*
 * converts value read at analog pin into exponential scale between 1 and 28
 */
void getDelay() {

    // convert linear to logarithmic scale
    float tDelayValue = analogRead(PIN_DELAY_POTI);
    Serial.print("DelayRawValue=");
    Serial.print(tDelayValue);
    tDelayValue /= 700; // 700 gives value 0.0 to 1.46
    sDelay = pow(10, tDelayValue); // gives value 1 to 28
    Serial.print(" -> ");
    Serial.println(sDelay);
}

/*
 * Callback handler for background pattern
 * since sState starts with (0++) scanner is the first pattern you see
 */
void PatternsBackground(NeoPatterns *aLedsPtr) {
    static int8_t sState = 0;
    static bool sNoDelay = false;

    /*
     * implement a random delay between each case
     */
    getDelay();
    long tRandomDelay = random(DELAY_MILLIS_BACKGROUND_MIN * sDelay, DELAY_MILLIS_BACKGROUND_MIN * sDelay * 4);
    uint16_t tInterval = random(INTERVAL_BACKGROUND_MIN, INTERVAL_BACKGROUND_MIN * 2);

    if ((sState & 1) == 1) {
        /*
         * Insert a random delay if sState is odd
         * Can be disabled by patterns using "sNoDelay"
         */
        sState++;
        if (!sNoDelay) {
            aLedsPtr->Delay(tRandomDelay); // to separate each pattern
            return;
        }
    }

    int8_t tState = sState / 2;
    switch (tState) {
    case 0:
        // STRIPES
        aLedsPtr->Stripes(COLOR32_GREEN_HALF, 15, COLOR32_BLACK, 25, tInterval, 400);
        sNoDelay = true;
        break;
    case 1:
        // clear pattern
        aLedsPtr->ColorWipe(COLOR32_BLACK, INTERVAL_FAST_MOVES_MIN, FLAG_DO_NOT_CLEAR, DIRECTION_DOWN);
        sNoDelay = false;
        break;
    case 2:
        aLedsPtr->RainbowCycle(tInterval);
        break;
    case 3:
        aLedsPtr->clear();
        aLedsPtr->ScannerExtended(COLOR32_GREEN, 32, tInterval, 1, FLAG_SCANNER_EXT_CYLON | FLAG_SCANNER_EXT_VANISH_COMPLETE);
        break;
    case 4:
        // old TheaterChase
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverflow" // of 2 * aLedsPtr->numPixels()
        aLedsPtr->Stripes(COLOR32_RED_HALF, 2, COLOR32_GREEN_HALF, 4, tInterval * 3, aLedsPtr->numPixels() * 2);
        sNoDelay = true;
        break;
#pragma GCC diagnostic pop
    case 5:
        aLedsPtr->ColorWipe(COLOR32_BLACK, INTERVAL_FAST_MOVES_MIN, FLAG_DO_NOT_CLEAR);
        sNoDelay = false;
        sState = -2; // Start from beginning
        break;
    default:
        Serial.println("ERROR");
        break;
    }

    Serial.print("Pin=");
    Serial.print(aLedsPtr->getPin());
    Serial.print(" Length=");
    Serial.print(aLedsPtr->numPixels());
    Serial.print(" ActivePattern=");
    aLedsPtr->printPatternName(aLedsPtr->ActivePattern, &Serial);
    Serial.print("|");
    Serial.print(aLedsPtr->ActivePattern);
    Serial.print(" Delay=");
    Serial.print(sDelay);
    Serial.print(" Interval=");
    Serial.print(tInterval);
    Serial.print(" sNoDelay=");
    Serial.print(sNoDelay);
    Serial.print(" StateBack=");
    Serial.println(tState);

    sState++;
}

/*
 * Callback handler for fast and seldom patterns
 * since sState starts with (0++) scanner is the first pattern you see
 */
void PatternsFastMoves(NeoPatterns *aLedsPtr) {
    static int8_t sState = 0;

    /*
     * implement a random delay between each case
     */
    getDelay();
    long tRandomDelay = random(DELAY_MILLIS_FAST_MOVES_MIN * sDelay, DELAY_MILLIS_FAST_MOVES_MIN * sDelay * 4);
    uint16_t tInterval = random(INTERVAL_FAST_MOVES_MIN, INTERVAL_FAST_MOVES_MIN * 2);

    int8_t tState = sState / 2;
    if ((sState & 1) == 1) {
        /*
         * Insert a random delay if sState is odd
         */
        aLedsPtr->Delay(tRandomDelay); // to separate each pattern
        sState++;
    } else {

        switch (tState) {
        case 0:
            // falling star
            aLedsPtr->ScannerExtended(COLOR32_WHITE_HALF, 10, tInterval, 0, FLAG_SCANNER_EXT_VANISH_COMPLETE, DIRECTION_DOWN);
            break;
        case 1:
            // old scanner 2 times
            aLedsPtr->ScannerExtended(COLOR32_BLUE, 10, tInterval, 1,
            FLAG_SCANNER_EXT_VANISH_COMPLETE | FLAG_SCANNER_EXT_START_AT_BOTH_ENDS);
            break;
        case 2:
            aLedsPtr->ScannerExtended(COLOR32_CYAN, 10, INTERVAL_FAST_MOVES_MIN, 0, FLAG_SCANNER_EXT_VANISH_COMPLETE);
            break;
        case 3:
            aLedsPtr->ScannerExtended(COLOR32_GREEN, 8, tInterval, 0,
            FLAG_SCANNER_EXT_CYLON | FLAG_SCANNER_EXT_VANISH_COMPLETE);
            break;
        case 4:
            aLedsPtr->ScannerExtended(COLOR32_GREEN, 6, tInterval, 0,
            FLAG_SCANNER_EXT_CYLON | FLAG_SCANNER_EXT_VANISH_COMPLETE | FLAG_SCANNER_EXT_START_AT_BOTH_ENDS);
            sState = -2; // Start from beginning
            break;
        default:
            Serial.println("ERROR");
            break;
        }
    }
    Serial.print("Length=");
    Serial.print(aLedsPtr->numPixels());
    Serial.print(" ActivePattern=");
    aLedsPtr->printPatternName(aLedsPtr->ActivePattern, &Serial);
    Serial.print("|");
    Serial.print(aLedsPtr->ActivePattern);
    Serial.print(" Delay=");
    Serial.print(sDelay);
    Serial.print(" Interval=");
    Serial.print(tInterval);
    Serial.print(" StateFast=");
    Serial.println(tState);

    sState++;
}
