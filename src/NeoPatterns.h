/*
 * NeoPatterns.h
 *
 *  SUMMARY
 *  This is an extended version version of the NeoPattern Example by Adafruit
 *  https://learn.adafruit.com/multi-tasking-the-arduino-part-3?view=all
 *  You need to install "Adafruit NeoPixel" library under Sketch -> Include Library -> Manage Librarys... -> use "neoPixel" as filter string
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

#include "Adafruit_NeoPixel.h"
#include "Colors.h"

extern char VERSION_NEOPATTERNS[4];

// Pattern types supported:
#define PATTERN_NONE                0
#define PATTERN_RAINBOW_CYCLE       1
#define PATTERN_COLOR_WIPE          2
#define PATTERN_FADE                3
#define PATTERN_DELAY               4

#define PATTERN_SCANNER_EXTENDED    6
#define PATTERN_STRIPES             7 // includes the old THEATER_CHASE
#define PATTERN_PROCESS_SELECTIVE   9
#define PATTERN_FIRE               10

#define PATTERN_USER_PATTERN1      16
#define PATTERN_USER_PATTERN2      17

/*
 * Values for Direction
 */
#define DIRECTION_UP 0
#define DIRECTION_LEFT 1
#define DIRECTION_DOWN 2
#define DIRECTION_RIGHT 3
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

uint8_t Red(color32_t color);
uint8_t Green(color32_t color);
uint8_t Blue(color32_t color);

// NeoPattern Class - derived from the Adafruit_NeoPixel class
class NeoPatterns: public Adafruit_NeoPixel {
public:
    NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t aTypeOfPixel, void (*aPatternCompletionCallback)(NeoPatterns*)=NULL);

    // To enable more than one pattern on the same strip
    void setPixelBuffer(uint8_t * aNewPixelBufferPointer);

    color32_t addPixelColor(uint16_t aPixelIndex, uint8_t aRed, uint8_t aGreen, uint8_t aBlue);

#ifdef ERROR
    // Version with error message
    bool begin(void);
#endif

    void setCallback(void (*callback)(NeoPatterns*));
    /*
     * Extensions to Adafruit_NeoPixel functions
     */
    void resetBrightnessValue(); // resets internal brightness control value to full to support restoring of patterns while brightening
    uint8_t getBytesPerPixel();
    uint16_t getPixelBufferSize();
    void storePixelBuffer(uint8_t * aPixelBufferPointerDestination);
    void restorePixelBuffer(uint8_t * aPixelBufferPointerSource, bool aResetBrightness = true);

    //
    bool CheckForUpdate();
    bool UpdateOrRedraw();
    bool Update(bool doShow = true);
    void DecrementTotalStepCounter();
    void NextIndexAndDecrementTotalStepCounter();
    void setDirectionAndTotalStepsAndIndex(uint8_t aDirection, uint16_t totalSteps);
    uint32_t DimColor(color32_t color);
    void ColorSet(color32_t color);

    /*
     * PATTERNS
     */
    void RainbowCycle(uint8_t interval, uint8_t aDirection = DIRECTION_UP);
    void ColorWipe(color32_t color, uint8_t interval, uint8_t aMode = 0, uint8_t aDirection = DIRECTION_UP);
    void Fade(color32_t color1, color32_t color2, uint16_t steps, uint8_t interval);

    /*
     * PATTERN extensions
     */
    void ScannerExtended(color32_t aColor1, uint8_t aLength, uint16_t aInterval, uint16_t aNumberOfBouncings = 0, uint8_t aMode = 0,
            uint8_t aDirection = DIRECTION_UP);
    void Fire(uint16_t interval, uint16_t repetitions = 100);
    void Delay(uint16_t aMillis);
    void ProcessSelectiveColor(color32_t aColorForSelection, color32_t (*aSingleLEDProcessingFunction)(NeoPatterns*),
            uint16_t steps, uint16_t interval);
    void Stripes(color32_t aColor1, uint8_t aLength1, color32_t aColor2, uint8_t aLength2, uint8_t aInterval,
            uint16_t aNumberOfSteps, uint8_t aMode = 0, uint8_t aDirection = DIRECTION_UP);

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

    void Debug(bool aFullInfo = true);
    void TestWS2812Resolution();

    // Static functions
    static color32_t Wheel(uint8_t WheelPos);
    static color32_t HeatColor(uint8_t aTemperature);
    static uint8_t gamma5(uint8_t aLinearBrightnessValue);
    static uint8_t gamma5Special(uint8_t aLinearBrightnessValue);
    static color32_t gamma5FromColor(color32_t aLinearBrightnessColor);
    /*
     * Variables for almost each pattern
     */
    uint16_t TotalStepCounter; // total number of steps in the pattern including repetitions.
    uint16_t Index; // or Position. Counter for basic pattern. Current step within the pattern. Counter for basic patterns. Step counter of snake.
    color32_t Color1;       // Main pattern color

    /*
     * Variables use by individual patterns
     */
    color32_t Color2;       // second pattern color | Number of bounces for scanner
    int8_t Direction;       // direction to run the pattern
    uint8_t PatternLength;  // the length of a (scanner) pattern

    /*
     * Temporary color for dim and lightenColor() and for FadeSelectiveColor, ProcessSelectiveColor.
     * Delta for each step for extended scanner and rainbow cycle
     */
    color32_t ColorTmp;

    uint8_t BytesPerPixel;  // can be 3 or 4

    /*
     * Internal control variables
     */
    uint8_t ActivePattern;  // which pattern is running
    unsigned long Interval;   // milliseconds between updates
    unsigned long lastUpdate; // millis of last update of pattern

    void (*OnPatternComplete)(NeoPatterns*);  // Callback on completion of pattern

    /*
     * Extensions
     */
#define GEOMETRY_BAR 1
    uint8_t PatternsGeometry; // fire pattern makes no sense on circles

    // For scanner_extended
    // Flags 0 -> one pass scanner (rocket or falling star)
    // Flags +1 -> cylon -> mirror pattern (effective length is 2*length -1)
    // Flags +2 -> start and end scanner vanishing complete e.g. first and last pattern are empty
    // Flags +0x10 -> use add-color instead off set-color
#define FLAG_SCANNER_EXT_ROCKET             0x00
#define FLAG_SCANNER_EXT_CYLON              0x01
#define FLAG_SCANNER_EXT_VANISH_COMPLETE    0x02
#define FLAG_SCANNER_EXT_START_AT_BOTH_ENDS 0x04

#define FLAG_DO_NOT_CLEAR                   0x10 // do not write black pixels - for colorWipe

    uint8_t Flags;  // special behavior of the pattern
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
void initMultipleFallingStars(NeoPatterns * aLedsPtr, color32_t aColor, uint8_t aDuration, uint8_t aRepetitions,
        void (*aNextOnCompleteHandler)(NeoPatterns*));
void multipleFallingStarsCompleteHandler(NeoPatterns * aLedsPtr);

void allPatternsRandomExample(NeoPatterns * aLedsPtr);

#endif // NEOPATTERNS_H
