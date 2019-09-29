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

// Propagate debug level
#ifdef TRACE
#define DEBUG
#endif
#ifdef DEBUG
#define INFO
#endif
#ifdef INFO
#define WARN
#endif
#ifdef WARN
#define ERROR
#endif

#include "NeoPixel.h"

#if !defined(__AVR__) && ! defined(PROGMEM)
#define PROGMEM void
#endif

extern char VERSION_NEOPATTERNS[4];

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

#define PATTERN_USER_PATTERN1       9
#define PATTERN_USER_PATTERN2      10

/*
 * Values for Direction
 */
#define DIRECTION_UP 0
#define DIRECTION_LEFT 1
#define DIRECTION_DOWN 2
#define DIRECTION_RIGHT 3
#define DIRECTION_MASK  0x03
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
    NeoPatterns(uint16_t aNumberOfPixels, uint8_t aPin, uint8_t aTypeOfPixel,
            void (*aPatternCompletionCallback)(NeoPatterns*)=NULL);
    NeoPatterns(NeoPixel * aUnderlyingNeoPixelObject, uint16_t aPixelOffset, uint16_t aNumberOfPixels,
            void (*aPatternCompletionCallback)(NeoPatterns*) = NULL, bool aEnableShowOfUnderlyingPixel = true);

    void setCallback(void (*callback)(NeoPatterns*));

    bool CheckForUpdate();
    bool UpdateOrRedraw();
    bool Update(bool doShow = true);
    bool update(bool doShow = true);
    bool DecrementTotalStepCounter();
    void NextIndex();
    void NextIndexAndDecrementTotalStepCounter();
    void setDirectionAndTotalStepsAndIndex(uint8_t aDirection, uint16_t aTotalSteps);

    void updateAndWaitForPatternToStop();

    /*
     * PATTERNS
     */
    void RainbowCycle(uint8_t aIntervalMillis, uint8_t aDirection = DIRECTION_UP);
    void ColorWipe(color32_t aColor, uint16_t aIntervalMillis, uint8_t aMode = 0, uint8_t aDirection = DIRECTION_UP);
    void Fade(color32_t aColor1, color32_t aColor2, uint16_t aNumberOfSteps, uint16_t aIntervalMillis);

    /*
     * PATTERN extensions
     */
    void ScannerExtended(color32_t aColor1, uint8_t aLength, uint16_t aIntervalMillis, uint16_t aNumberOfBouncings = 0, uint8_t aMode = 0,
            uint8_t aDirection = DIRECTION_UP);
    void Fire(uint16_t interval, uint16_t repetitions = 100);
    color32_t HeatColor(uint8_t aTemperature);

    void Delay(uint16_t aMillis);
    void ProcessSelectiveColor(color32_t aColorForSelection, color32_t (*aSingleLEDProcessingFunction)(NeoPatterns*),
            uint16_t aNumberOfSteps, uint16_t aIntervalMillis);
    void Stripes(color32_t aColor1, uint8_t aLength1, color32_t aColor2, uint8_t aLength2, uint16_t aIntervalMillis,
            uint16_t aNumberOfSteps, uint8_t aDirection = DIRECTION_UP);

    /*
     * UPDATE functions
     */
    void RainbowCycleUpdate(bool aDoUpdate = true);
    void ColorWipeUpdate(bool aDoUpdate = true);
    void FadeUpdate(bool aDoUpdate = true);

    /*
     * Extensions
     */
    void ScannerExtendedUpdate(bool aDoUpdate = true);
    void StripesUpdate(bool aDoUpdate = true);
    void FireUpdate(bool aDoUpdate = true);
    void DelayUpdate(bool aDoUpdate = true);
    void ProcessSelectiveColorUpdate(bool aDoUpdate = true);
    void ProcessSelectiveColorForAllPixel();
    void Pattern1Update(bool aDoUpdate = true);
    void Pattern2Update(bool aDoUpdate = true);

#if defined(__AVR__)
    void getPatternName(uint8_t aPatternNumber, char * aBuffer, uint8_t aBuffersize);
#else
    // use PatternNamesArray[aPatternNumber] on other platforms
#endif
    void printPatternName(uint8_t aPatternNumber, Stream * aSerial);
    void Debug(Stream * aSerial, bool aFullInfo = true);

    /*
     * Variables for almost each pattern
     */
    uint16_t TotalStepCounter; // total number of steps in the pattern including repetitions and the last delay step to show the end result.
    uint16_t Index; // or Position. Counter for basic pattern. Current step within the pattern. Counter for basic patterns. Step counter of snake.
    color32_t Color1;       // Main pattern color

    /*
     * Variables use by individual patterns
     */
    color32_t Color2;       // second pattern color | Number of bounces for scanner
    int8_t Direction;       // direction to run the pattern
    uint8_t PatternLength;  // the length of a (scanner) pattern, cooling parameter for fire

    /*
     * Temporary color for dim and lightenColor() and for FadeSelectiveColor, ProcessSelectiveColor.
     * Delta for each step for extended scanner and rainbow cycle
     */
    color32_t ColorTmp;

    /*
     * Internal control variables
     */
    uint8_t ActivePattern;  // which pattern is running
    uint16_t Interval;   // milliseconds between updates
    unsigned long lastUpdate; // milliseconds of last update of pattern

    void (*OnPatternComplete)(NeoPatterns*);  // Callback on completion of pattern

    /*
     * Extensions
     */
#define GEOMETRY_BAR 1
    uint8_t PatternsGeometry; // fire pattern makes no sense on circles

    // For ScannerExtended()
    // PatternFlags 0 -> one pass scanner (rocket or falling star)
    // PatternFlags +1 -> cylon -> mirror pattern (effective length is 2*length -1)
    // PatternFlags +2 -> start and end scanner vanishing complete e.g. first and last pattern are empty
    // PatternFlags +4 -> start pattern at both ends i.e. direction is irrelevant
#define FLAG_SCANNER_EXT_ROCKET             0x00
#define FLAG_SCANNER_EXT_CYLON              0x01
#define FLAG_SCANNER_EXT_VANISH_COMPLETE    0x02
#define FLAG_SCANNER_EXT_START_AT_BOTH_ENDS 0x04

// Do not write black pixels / pixels not used by pattern. Can be used to overwrite existing patterns - for colorWipe() and ScannerExtended()
#define FLAG_DO_NOT_CLEAR                   0x10

    uint8_t PatternFlags;  // special behavior of the pattern
    // for Cylon, Fire, multipleHandler
    uint16_t Repetitions; // counter for multipleHandler

    uint32_t (*SingleLEDProcessingFunction)(NeoPatterns*); // for ProcessSelectiveColor
    /*
     * for multiple pattern extensions
     */
    uint8_t MultipleExtension; // for delay of multiple falling stars and snake flags and length of stripes
    void (*NextOnPatternCompleteHandler)(NeoPatterns*);  // Next callback after completion of multiple pattern
};

//  Sample processing functions for ProcessSelectiveColor()
color32_t FadeColor(NeoPatterns* aLedPtr);
color32_t DimColor(NeoPatterns* aLedPtr);
color32_t BrightenColor(NeoPatterns* aLedPtr);

// multiple pattern example
void initMultipleFallingStars(NeoPatterns * aLedsPtr, color32_t aColor, uint8_t aLength, uint8_t aDuration, uint8_t aRepetitions,
        void (*aNextOnCompleteHandler)(NeoPatterns*), uint8_t aDirection = DIRECTION_DOWN);
void multipleFallingStarsCompleteHandler(NeoPatterns * aLedsPtr);

void allPatternsRandomExample(NeoPatterns * aLedsPtr);

#endif // NEOPATTERNS_H

#pragma once

