/*
 * NeoPatterns.h
 *
 *  SUMMARY
 *  This is an extended version version of the NeoPattern Example by Adafruit
 *  https://learn.adafruit.com/multi-tasking-the-arduino-part-3?view=all
 *  You need to install "Adafruit NeoPixel" library under "Tools -> Manage Libraries..." or "Ctrl+Shift+I" -> use "neoPixel" as filter string
 *  Extension are made to include more patterns and combined patterns and patterns for 8x8 NeoPixel matrix.
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
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/gpl.html>.
 *
 */

/* Class inheritance diagram
 *                                     ,o--> MatrixNeoPixel \
 * MatrixSnake --> MatrixNeoPatterns  <                      o--> NeoPixel --> Adafruit_NeoPixel
 *                                     `o--> NeoPatterns    /
 */

#ifndef NEOPATTERNS_H
#define NEOPATTERNS_H

#if !defined (DO_NOT_USE_MATH_PATTERNS)
// Comment this out if you do NOT need the BOUNCING_BALL pattern
// This pattern needs additional 640 to 1140 bytes FLASH, depending if floating point and sqrt() are already used otherwise.
//#define DO_NOT_USE_MATH_PATTERNS
#endif

#include "NeoPixel.h"

#if !defined(__AVR__) && ! defined(PROGMEM)
#define PROGMEM
#endif

#define VERSION_NEOPATTERNS "2.2.0"
#define VERSION_NEOPATTERNS_NUMERICAL 220

/*
 * Version 2.2.0 - 4/2020
 * - Added support for RGBW patterns. Requires additional 200 bytes for AllPatternsOnMultiDevices example.
 *   Not defining SUPPORT_RGBW saves 400 bytes FLASH for AllPatternsOnMultiDevices example.
 * - Use type `Print *` instead of `Stream *`.
 * - Changed function `addPixelColor()`.
 * - Added function `NeoPixel::printInfo(aSerial)`.
 * - Added *D functions, which take the duration of the whole pattern as argument.
 * - Added OpenLedRace example.
 * - Added empty constructor and init() functions.
 * - Added function `updateAllPartialPatterns()`.
 *
 * Version 2.1.0 - 12/2019
 * - Ported to ESP8266 and ESP32.
 * - Changed signature of NeoPatterns(NeoPixel * aUnderlyingNeoPixelObject). Swapped 4. and 5. parameter to make it consistent to the NeoPixel signature.
 * - Function `setPixelOffsetForPartialNeoPixel()` in NeoPixel.cpp added.
 *
 * Version 2.0.0 - 11/2019
 * - Function `drawBar()` in NeoPixel.cpp added.
 * - Swapped parameter aNumberOfSteps and aIntervalMillis of `Stripes()`.
 * - Pattern `HEARTBEAT` and `BOUNCING_BALL` added.
 * - Added parameter aDirection to `Fire()`.
 * - Removed helper function `setDirectionAndTotalStepsAndIndex()`.
 */

extern const char * const PatternNamesArray[] PROGMEM;

// Pattern types supported:
#define PATTERN_NONE                0
#define PATTERN_RAINBOW_CYCLE       1
#define PATTERN_COLOR_WIPE          2
#define PATTERN_FADE                3
#define PATTERN_DELAY               4

#define PATTERN_SCANNER_EXTENDED    5
#define PATTERN_STRIPES             6 // includes the old THEATER_CHASE
#define PATTERN_PROCESS_SELECTIVE   7
#define PATTERN_FIRE                8
#define PATTERN_HEARTBEAT           9

#define PATTERN_USER_PATTERN1      10
#define PATTERN_USER_PATTERN2      11

#if !defined (DO_NOT_USE_MATH_PATTERNS)
#define PATTERN_BOUNCING_BALL      12
#endif

/*
 * Values for Direction
 */
#define DIRECTION_UP 0
#define DIRECTION_LEFT 1
#define DIRECTION_DOWN 2
#define DIRECTION_RIGHT 3
#define DIRECTION_MASK  0x03
#define PARAMETER_IS_DURATION  0x80 // if highest bit is set for direction parameter, the intervalMillis parameter is interpreted as durationMillis.
#define OppositeDirection(aDirection) (((aDirection) + 2) & DIRECTION_MASK)
#define NUMBER_OF_DIRECTIONS 4
#define DIRECTION_IMPOSSIBLE NUMBER_OF_DIRECTIONS   // No direction possible (for AI)
#define DIRECTION_NONE 5                            // No button pressed until now
const char DirectionUp[] = "up";
const char DirectionLeft[] = "left";
const char DirectionDown[] = "down";
const char DirectionRight[] = "right";
const char DirectionNo[] = "no";
const char* DirectionToString(uint8_t aDirection);

// only for Backwards compatibility
#define FORWARD DIRECTION_UP
#define REVERSE DIRECTION_DOWN

// NeoPattern Class - derived from the NeoPixel and Adafruit_NeoPixel class
// virtual to enable double inheritance of the NeoPixel functions and the NeoPatterns ones.
class NeoPatterns: public virtual NeoPixel {
public:
    NeoPatterns();
    void init();
    NeoPatterns(uint16_t aNumberOfPixels, uint8_t aPin, uint8_t aTypeOfPixel, void (*aPatternCompletionCallback)(NeoPatterns*)=NULL,
            bool aShowOnlyAtUpdate = false);
    bool init(uint16_t aNumberOfPixels, uint8_t aPin, uint8_t aTypeOfPixel, void (*aPatternCompletionCallback)(NeoPatterns*)=NULL,
                bool aShowOnlyAtUpdate = false);
    NeoPatterns(NeoPixel * aUnderlyingNeoPixelObject, uint16_t aPixelOffset, uint16_t aNumberOfPixels,
            bool aEnableShowOfUnderlyingPixel = true, void (*aPatternCompletionCallback)(NeoPatterns*) = NULL,
            bool aShowOnlyAtUpdate = false);
    void init(NeoPixel * aUnderlyingNeoPixelObject, uint16_t aPixelOffset, uint16_t aNumberOfPixels,
                bool aEnableShowOfUnderlyingPixel = true, void (*aPatternCompletionCallback)(NeoPatterns*) = NULL,
                bool aShowOnlyAtUpdate = false);

    void setCallback(void (*callback)(NeoPatterns*));

    bool checkForUpdate();
    bool updateOrRedraw();
    bool update();
    bool updateAllPartialPatterns();
    void showPatternInitially();
    bool decrementTotalStepCounter();
    void setNextIndex();
    bool decrementTotalStepCounterAndSetNextIndex();

    void updateAndWaitForPatternToStop();
    void updateAllPartialPatternsAndWaitForPatternsToStop();

    /*
     * PATTERNS
     */
    void RainbowCycle(uint8_t aIntervalMillis, uint8_t aDirection = DIRECTION_UP);
    void RainbowCycleD(uint8_t aDurationMillis, uint8_t aDirection = DIRECTION_UP);
    void ColorWipe(color32_t aColor, uint16_t aIntervalMillis, uint8_t aMode = 0, uint8_t aDirection = DIRECTION_UP);
    void ColorWipeD(color32_t aColor, uint16_t aDurationMillis, uint8_t aMode = 0, uint8_t aDirection = DIRECTION_UP);
    void Fade(color32_t aColorStart, color32_t aColorEnd, uint16_t aNumberOfSteps, uint16_t aIntervalMillis);

    /*
     * PATTERN extensions
     */
    void StripesD(color32_t aColor1, uint8_t aLength1, color32_t aColor2, uint8_t aLength2, uint16_t aNumberOfSteps,
            uint16_t aDurationMillis, uint8_t aDirection = DIRECTION_UP);
    void Stripes(color32_t aColor1, uint8_t aLength1, color32_t aColor2, uint8_t aLength2, uint16_t aNumberOfSteps,
            uint16_t aIntervalMillis, uint8_t aDirection = DIRECTION_UP);
    void Heartbeat(color32_t aColor, uint16_t aIntervalMillis, uint16_t aRepetitions, uint8_t aMode = 0);
    void ScannerExtended(color32_t aColor, uint8_t aLength, uint16_t aIntervalMillis, uint16_t aNumberOfBouncings = 0,
            uint8_t aMode = 0, uint8_t aDirection = DIRECTION_UP);
    void ScannerExtendedD(color32_t aColor, uint8_t aLength, uint16_t aDurationMillis, uint16_t aNumberOfBouncings = 0,
            uint8_t aMode = 0, uint8_t aDirection = DIRECTION_UP);
    void Fire(uint16_t aNumberOfSteps = 100, uint16_t aIntervalMillis = 30, uint8_t aDirection = DIRECTION_UP);
    void Delay(uint16_t aMillis);

    void ProcessSelectiveColor(color32_t aColorForSelection, color32_t (*aSingleLEDProcessingFunction)(NeoPatterns*),
            uint16_t aNumberOfSteps, uint16_t aIntervalMillis);

    color32_t HeatColor(uint8_t aTemperature);

#if !defined (DO_NOT_USE_MATH_PATTERNS)
    void BouncingBall(color32_t aColor, uint16_t aIndexOfTopPixel, uint16_t aIntervalMillis = 70, int8_t aPercentageOfLossAtBounce =
            10, uint8_t aDirection = DIRECTION_DOWN);
    bool BouncingBallUpdate(bool aDoUpdate = true);
#endif

    /*
     * UPDATE functions
     * return true if pattern has ended, false if pattern has NOT ended
     */
    bool RainbowCycleUpdate(bool aDoUpdate = true);
    bool ColorWipeUpdate(bool aDoUpdate = true);
    bool FadeUpdate(bool aDoUpdate = true);

    /*
     * Extensions
     */
    bool ScannerExtendedUpdate(bool aDoUpdate = true);
    bool StripesUpdate(bool aDoUpdate = true);
    bool HeartbeatUpdate(bool aDoUpdate = true);
    bool FireUpdate(bool aDoUpdate = true);
    bool DelayUpdate(bool aDoUpdate = true);
    bool ProcessSelectiveColorUpdate(bool aDoUpdate = true);
    bool Pattern1Update(bool aDoUpdate = true);
    bool Pattern2Update(bool aDoUpdate = true);

    void ProcessSelectiveColorForAllPixel();

#if defined(__AVR__)
    void getPatternName(uint8_t aPatternNumber, char * aBuffer, uint8_t aBuffersize);
#else
    // use PatternNamesArray[aPatternNumber] on other platforms
#endif
    void printPatternName(uint8_t aPatternNumber, Print * aSerial);
    void printInfo(Print * aSerial, bool aFullInfo = true);

    /*
     * Variables for almost each pattern
     */
    uint16_t TotalStepCounter; // Total number of steps in the pattern including all repetitions and the last delay step to show the end result
    uint16_t Index;         // or Position. Counter for basic patterns. Current step within the pattern. Step counter of snake.
    color32_t Color1;       // Main pattern color
    int8_t Direction;       // Direction to run the pattern
    uint8_t PatternLength;  // Length of a (scanner) pattern - BouncingBall: Current integer IndexOfTopPixel

    // For ScannerExtended()
    // PatternFlags 0 -> one pass scanner (rocket or falling star)
    // PatternFlags +1 -> cylon -> mirror pattern (effective length is 2*length -1)
    // PatternFlags +2 -> start and end scanner vanishing complete e.g. first and last pattern are empty
    // PatternFlags +4 -> start pattern at both ends i.e. direction is irrelevant
#define FLAG_SCANNER_EXT_ROCKET             0x00
#define FLAG_SCANNER_EXT_CYLON              0x01
#define FLAG_SCANNER_EXT_VANISH_COMPLETE    0x02
#define FLAG_SCANNER_EXT_START_AT_BOTH_ENDS 0x04

#define FLAG_DO_CLEAR                   	0x00
    // Do not write black pixels / pixels not used by pattern. Can be used to overwrite existing patterns - for colorWipe() and ScannerExtended()
#define FLAG_DO_NOT_CLEAR                   0x10
    uint8_t PatternFlags;  // special behavior of the pattern - BouncingBall: PercentageOfLossAtBounce

    /*
     * Internal control variables
     */
    uint8_t ActivePattern;  // Number of pattern which is running
    uint16_t Interval;   // Milliseconds between updates
    unsigned long lastUpdate; // Milliseconds of last update of pattern

    void (*OnPatternComplete)(NeoPatterns*); // Callback on completion of pattern. This should set aLedsPtr->ActivePattern = PATTERN_NONE; if no other pattern is started.

    /*
     * 3 Extra variables used by some patterns
     */
    union {
        color32_t BackgroundColor;
        color32_t Color2; // second pattern color
        uint8_t * heatOfPixelArrayPtr; // Allocated array for current heat values for Fire pattern
        uint16_t StartIntervalMillis; // BouncingBall: interval for first step
        uint16_t NumberOfBouncings; // ScannerExtended: Number of bounces
    } LongValue1;

    union {
        color32_t ColorTmp; // Temporary color for dim and lightenColor() and for FadeSelectiveColor, ProcessSelectiveColor.
        float TopPixelIndex; // BouncingBall: float index of TopPixel

        uint16_t DeltaBrightnessShift8; // ScannerExtended: Delta for each step for
    } LongValue2;

    union {
        // for ProcessSelectiveColor could not be member of the first 2 values, since these are used by the processing functions like fadeColor()
        uint32_t (*SingleLEDProcessingFunction)(NeoPatterns*);
    } Value3; // can be 16 bit for AVR and 32 bit for other platforms

    /*
     * for multiple pattern extensions
     */
    uint16_t Repetitions; // counter for multipleHandler
    uint8_t MultipleExtension; // for delay of multiple falling stars and snake flags and length of stripes
    void (*NextOnPatternCompleteHandler)(NeoPatterns*);  // Next callback after completion of multiple pattern
    /*
     * List of all NeoPatterns
     */
    NeoPatterns * NextNeoPatternsObject; // For underlying NeoPixels, the first partial NeoPixel, else the next partial NeoPixel for the same underlying NeoPixel
    static NeoPatterns * FirstNeoPatternsObject;
};

//  Sample processing functions for ProcessSelectiveColor()
color32_t FadeColor(NeoPatterns* aNeoPatternsPtr);
color32_t DimColor(NeoPatterns* aNeoPatternsPtr);
color32_t BrightenColor(NeoPatterns* aNeoPatternsPtr);

// multiple pattern example
void initMultipleFallingStars(NeoPatterns * aLedsPtr, color32_t aColor, uint8_t aLength, uint8_t aDuration, uint8_t aRepetitions,
        void (*aNextOnCompleteHandler)(NeoPatterns*), uint8_t aDirection = DIRECTION_DOWN);
void multipleFallingStarsCompleteHandler(NeoPatterns * aLedsPtr);

void allPatternsRandomHandler(NeoPatterns * aLedsPtr);

void __attribute__((weak)) UserPattern1(NeoPatterns * aNeoPatterns, color32_t aPixelColor, color32_t aBackgroundColor,
        uint16_t aIntervalMillis, uint8_t aDirection = DIRECTION_UP);
void __attribute__((weak)) UserPattern2(NeoPatterns * aNeoPatterns, color32_t aColor, uint16_t aIntervalMillis,
        uint16_t aRepetitions = 0, uint8_t aDirection = DIRECTION_UP);
#endif // NEOPATTERNS_H

#pragma once

