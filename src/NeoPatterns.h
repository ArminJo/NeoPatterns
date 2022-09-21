/*
 * NeoPatterns.h
 *
 *  SUMMARY
 *  This is an extended version version of the NeoPattern Example by Adafruit
 *  https://learn.adafruit.com/multi-tasking-the-arduino-part-3?view=all
 *  You need to install "Adafruit NeoPixel" library under "Tools -> Manage Libraries..." or "Ctrl+Shift+I" -> use "neoPixel" as filter string
 *  Extension are made to include more patterns and combined patterns and patterns for 8x8 NeoPixel matrix.
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

/* Class inheritance diagram
 *                                     ,o--> MatrixNeoPixel \
 * MatrixSnake --> MatrixNeoPatterns  <                      o--> NeoPixel --> Adafruit_NeoPixel
 *                                     `o--> NeoPatterns    /
 */

#ifndef _NEOPATTERNS_H
#define _NEOPATTERNS_H

#if !defined(DO_NOT_USE_MATH_PATTERNS)
// This pattern needs additional 640 to 1140 bytes program memory, depending if floating point and sqrt() are already used otherwise.
// Activate the next line if you do NOT need the BOUNCING_BALL pattern.
//#define DO_NOT_USE_MATH_PATTERNS
#endif

#include "NeoPixel.h"

#if !defined(__AVR__) && !defined(PROGMEM)
#define PROGMEM
#endif

#define VERSION_NEOPATTERNS "3.1.1"
#define VERSION_NEOPATTERNS_MAJOR 3
#define VERSION_NEOPATTERNS_MINOR 1
#define VERSION_NEOPATTERNS_PATCH 1
// The change log is at the bottom of the file

/*
 * Macro to convert 3 version parts into an integer
 * To be used in preprocessor comparisons, such as #if VERSION_NEOPATTERNS_HEX >= VERSION_HEX_VALUE(3, 7, 0)
 */
#define VERSION_HEX_VALUE(major, minor, patch) ((major << 16) | (minor << 8) | (patch))
#define VERSION_NEOPATTERNS_HEX  VERSION_HEX_VALUE(VERSION_NEOPATTERNS_MAJOR, VERSION_NEOPATTERNS_MINOR, VERSION_NEOPATTERNS_PATCH)

extern const char *const PatternNamesArray[] PROGMEM;

#if (!(defined(ENABLE_PATTERN_RAINBOW_CYCLE) || defined(ENABLE_PATTERN_COLOR_WIPE) || defined(ENABLE_PATTERN_FADE) \
|| defined(ENABLE_PATTERN_SCANNER_EXTENDED) || defined(ENABLE_PATTERN_STRIPES) || defined(ENABLE_PATTERN_FLASH) || defined(ENABLE_PATTERN_PROCESS_SELECTIVE) \
|| defined(ENABLE_PATTERN_HEARTBEAT) || defined(ENABLE_PATTERN_FIRE) \
|| defined(ENABLE_PATTERN_USER_PATTERN1) || defined(ENABLE_PATTERN_USER_PATTERN2) || defined(ENABLE_PATTERN_BOUNCING_BALL) \
|| defined(ENABLE_NO_NEO_PATTERN_BY_DEFAULT) ))
#define ENABLE_PATTERN_RAINBOW_CYCLE
#define ENABLE_PATTERN_COLOR_WIPE
#define ENABLE_PATTERN_FADE
#define ENABLE_PATTERN_SCANNER_EXTENDED
#define ENABLE_PATTERN_STRIPES
#define ENABLE_PATTERN_FLASH
#define ENABLE_PATTERN_PROCESS_SELECTIVE
#define ENABLE_PATTERN_HEARTBEAT
#define ENABLE_PATTERN_FIRE
#define ENABLE_PATTERN_USER_PATTERN1
#define ENABLE_PATTERN_USER_PATTERN2
#   if !defined(DO_NOT_USE_MATH_PATTERNS)
#define ENABLE_PATTERN_BOUNCING_BALL // Requires up to 640 to 1140 bytes program memory, depending if floating point and sqrt() are already used otherwise.
#   endif
#endif

// Pattern types supported: Can be used as index of PatternNamesArray
#define PATTERN_NONE                0
#define PATTERN_RAINBOW_CYCLE       1
#define PATTERN_COLOR_WIPE          2
#define PATTERN_FADE                3
#define PATTERN_DELAY               4

#define PATTERN_SCANNER_EXTENDED    5
#define PATTERN_STRIPES             6 // includes the old THEATER_CHASE
#define PATTERN_FLASH               7
#define PATTERN_PROCESS_SELECTIVE   8
#define PATTERN_HEARTBEAT           9
#define PATTERN_FIRE               10

#define PATTERN_USER_PATTERN1      11
#define PATTERN_USER_PATTERN2      12

#define PATTERN_BOUNCING_BALL      13
#define LAST_NEO_PATTERN           PATTERN_BOUNCING_BALL

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
#define DIRECTION_NONE NUMBER_OF_DIRECTIONS   // No button pressed until now, no direction possible (for AI)
const char DirectionUp[] = "up";
const char DirectionLeft[] = "left";
const char DirectionDown[] = "down";
const char DirectionRight[] = "right";
const char DirectionNo[] = "no";
const char* DirectionToString(uint8_t aDirection);

// NeoPattern Class - derived from the NeoPixel and Adafruit_NeoPixel class
// virtual to enable double inheritance of the NeoPixel functions and the NeoPatterns ones.
class NeoPatterns: public virtual NeoPixel {
public:
    NeoPatterns();
    void init();
    NeoPatterns(uint16_t aNumberOfPixels, uint8_t aPin, neoPixelType aTypeOfPixel,
            void (*aPatternCompletionCallback)(NeoPatterns*)=NULL, bool aShowOnlyAtUpdate = false);
    bool init(uint16_t aNumberOfPixels, uint8_t aPin, neoPixelType aTypeOfPixel,
            void (*aPatternCompletionCallback)(NeoPatterns*)=NULL, bool aShowOnlyAtUpdate = false);
    NeoPatterns(NeoPixel *aUnderlyingNeoPixelObject, uint16_t aPixelOffset, uint16_t aNumberOfPixels,
            bool aEnableShowOfUnderlyingPixel = true, void (*aPatternCompletionCallback)(NeoPatterns*) = NULL,
            bool aShowOnlyAtUpdate = false);
    void init(NeoPixel *aUnderlyingNeoPixelObject, uint16_t aPixelOffset, uint16_t aNumberOfPixels,
            bool aEnableShowOfUnderlyingPixel = true, void (*aPatternCompletionCallback)(NeoPatterns*) = NULL,
            bool aShowOnlyAtUpdate = false);
    void _insertIntoNeopatternsList();

    void setCallback(void (*callback)(NeoPatterns*));

    bool isActive();
    bool checkForUpdate();
    bool update();
    bool update(uint8_t aBrightness);

#define DO_REDRAW_IF_NO_UPDATE  true
#define DO_NO_REDRAW_IF_NO_UPDATE  false
    bool updateOrRedraw(bool aDoRedrawIfNoUpdate);
    bool updateOrRedraw(bool aDoRedrawIfNoUpdate, uint8_t aBrightness);

    void stop();

    void updateShowAndWaitForPatternToStop();
    void updateShowAndWaitForPatternToStop(uint8_t aBrightness);
    bool updateAndShowAlsoAllPartialPatterns();
    bool updateAndShowAlsoAllPartialPatterns(uint8_t aBrightness);
    void updateAndShowAlsoAllPartialPatternsAndWaitForPatternsToStop();
    void updateAndShowAlsoAllPartialPatternsAndWaitForPatternsToStop(uint8_t aBrightness);

    bool updateAllPartialPatterns() __attribute__ ((deprecated ("Renamed to updateAndShowAllPartialPatterns()")));
    bool updateAllPartialPatterns(uint8_t aBrightness)
            __attribute__ ((deprecated ("Renamed to updateAndShowAllPartialPatterns()")));
    void updateAndWaitForPatternToStop() __attribute__ ((deprecated ("Renamed to updateShowAndWaitForPatternToStop()")));
    void updateAndWaitForPatternToStop(uint8_t aBrightness)
            __attribute__ ((deprecated ("Renamed to updateShowAndWaitForPatternToStop()")));
    void updateAllPartialPatternsAndWaitForPatternsToStop()
            __attribute__ ((deprecated ("Renamed to updateAndShowAllPartialPatternsAndWaitForPatternsToStop()")));
    void updateAllPartialPatternsAndWaitForPatternsToStop(uint8_t aBrightness)
            __attribute__ ((deprecated ("Renamed to updateAndShowAllPartialPatternsAndWaitForPatternsToStop()")));

    void showPatternInitially();
    bool decrementTotalStepCounter();
    void setNextIndex();
    bool decrementTotalStepCounterAndSetNextIndex();

    /*
     * PATTERNS
     * *Update() functions return true if pattern has ended, false if pattern has NOT ended
     * If aDoUpdate is true, update pattern, otherwise only redraw the pattern
     */
#if defined(ENABLE_PATTERN_RAINBOW_CYCLE)
    void RainbowCycle(uint8_t aIntervalMillis, uint8_t aDirection = DIRECTION_UP, uint8_t aRepetitions = 1);
    void RainbowCycleD(uint8_t aDurationMillis, uint8_t aDirection = DIRECTION_UP, uint8_t aRepetitions = 1);
    bool RainbowCycleUpdate(bool aDoUpdate = true);
#endif
#if defined(ENABLE_PATTERN_COLOR_WIPE)
    void ColorWipe(color32_t aColor, uint16_t aIntervalMillis, uint8_t aMode = 0, uint8_t aDirection = DIRECTION_UP);
    void ColorWipeD(color32_t aColor, uint16_t aDurationMillis, uint8_t aMode = 0, uint8_t aDirection = DIRECTION_UP);
    bool ColorWipeUpdate(bool aDoUpdate = true);
#endif
#if defined(ENABLE_PATTERN_FADE)
    void Fade(color32_t aColorStart, color32_t aColorEnd, uint16_t aNumberOfSteps, uint16_t aIntervalMillis);
    bool FadeUpdate(bool aDoUpdate = true);
#endif

    /*
     * PATTERN extensions
     */
    // Delay is always enabled, since it is used by many other patterns
    void Delay(uint16_t aMillis);
    bool DelayUpdate(bool aDoUpdate = true);

#if defined(ENABLE_PATTERN_STRIPES)
    void StripesD(color32_t aColor1, uint8_t aLength1, color32_t aColor2, uint8_t aLength2, uint16_t aNumberOfSteps,
            uint16_t aDurationMillis, uint8_t aDirection = DIRECTION_UP);
    void Stripes(color32_t aColor1, uint8_t aLength1, color32_t aColor2, uint8_t aLength2, uint16_t aNumberOfSteps,
            uint16_t aIntervalMillis, uint8_t aDirection = DIRECTION_UP);
    bool StripesUpdate(bool aDoUpdate = true);
#endif

#if defined(ENABLE_PATTERN_FLASH)
    void Flash(color32_t aColor1, uint16_t aIntervalMillisColor1, color32_t aColor2, uint16_t aIntervalMillisColor2, uint16_t aNumberOfSteps, bool doEndWithBlack = false);
    bool FlashUpdate(bool aDoUpdate = true);
#endif

#if defined(ENABLE_PATTERN_SCANNER_EXTENDED)
    void ScannerExtended(color32_t aColor, uint8_t aLength, uint16_t aIntervalMillis, uint16_t aNumberOfBouncings = 0,
            uint8_t aMode = 0, uint8_t aDirection = DIRECTION_UP);
    void ScannerExtendedD(color32_t aColor, uint8_t aLength, uint16_t aDurationMillis, uint16_t aNumberOfBouncings = 0,
            uint8_t aMode = 0, uint8_t aDirection = DIRECTION_UP);
    bool ScannerExtendedUpdate(bool aDoUpdate = true);
#endif

#if defined(ENABLE_PATTERN_HEARTBEAT)
    void Heartbeat(color32_t aColor, uint16_t aIntervalMillis, uint16_t aRepetitions, uint8_t aMode = 0);
    bool HeartbeatUpdate(bool aDoUpdate = true);
#endif

#if defined(ENABLE_PATTERN_PROCESS_SELECTIVE)
    void ProcessSelectiveColor(color32_t aColorForSelection, color32_t (*aSingleLEDProcessingFunction)(NeoPatterns*),
            uint16_t aNumberOfSteps, uint16_t aIntervalMillis);
    bool ProcessSelectiveColorUpdate(bool aDoUpdate = true);
#endif

#if defined(ENABLE_PATTERN_FIRE)
    void Fire(uint16_t aNumberOfSteps = 100, uint16_t aIntervalMillis = 30, uint8_t aDirection = DIRECTION_UP);
    bool FireUpdate(bool aDoUpdate = true);
#endif

    void convertHeatToColor();
    void printHeat(Print *aSerial);

    static color32_t HeatColor(uint8_t aTemperature);
    static color32_t HeatColorGamma5(uint8_t aTemperature);

#if defined(ENABLE_PATTERN_BOUNCING_BALL)
    void BouncingBall(color32_t aColor, uint16_t aIndexOfTopPixel, uint16_t aIntervalMillis = 70, int8_t aPercentageOfLossAtBounce =
            10, uint8_t aDirection = DIRECTION_DOWN);
    bool BouncingBallUpdate(bool aDoUpdate = true);
#endif

#if defined(ENABLE_PATTERN_USER_PATTERN1)
    bool Pattern1Update(bool aDoUpdate = true);
#endif
#if defined(ENABLE_PATTERN_USER_PATTERN2)
    bool Pattern2Update(bool aDoUpdate = true);
#endif

    void ProcessSelectiveColorForAllPixel();

#if defined(__AVR__)
    void getPatternName(uint8_t aPatternNumber, char *aBuffer, uint8_t aBuffersize);
#else
    // use PatternNamesArray[aPatternNumber] on other platforms
#endif
    void printPatternName(uint8_t aPatternNumber, Print *aSerial);
    void printInfo(Print *aSerial, bool aFullInfo = true);
    void printPattern();
    void printlnPattern();

    /*
     * Internal control variables
     */
    uint8_t ActivePattern; // Number of pattern which is running. If no callback activated, set to PATTERN_NONE in decrementTotalStepCounter().
    uint16_t Interval;   // Milliseconds between updates
    unsigned long lastUpdate; // Milliseconds of last update of pattern. Set by decrementTotalStepCounter(), showPatternInitially() or XXXupdate()

    void (*OnPatternComplete)(NeoPatterns*); // Callback on completion of pattern. This should set aLedsPtr->ActivePattern = PATTERN_NONE; if no other pattern is started.

    /*
     * Variables for almost each pattern
     */
    int16_t TotalStepCounter; // Total number of steps in the pattern including all repetitions and the last delay step to show the end result
    uint16_t Index;             // or Position. Counter for basic patterns. Current step within the pattern. Step counter of snake.
    color32_t Color1;           // Main pattern color
    int8_t Direction;           // Direction to run the pattern  DIRECTION_UP, DIRECTION_LEFT, DIRECTION_DOWN or DIRECTION_RIGHT

    // For ScannerExtended()
    // PatternFlags 0 -> one pass scanner (rocket or falling star)
    // PatternFlags +1 -> cylon -> mirror pattern (effective length is 2*length -1)
    // PatternFlags +2 -> start and end scanner vanishing complete e.g. first and last pattern are empty
    // PatternFlags +4 -> start pattern at both ends i.e. direction is irrelevant
#define FLAG_SCANNER_EXT_ROCKET             0x00
#define FLAG_SCANNER_EXT_CYLON              0x01
#define FLAG_SCANNER_EXT_VANISH_COMPLETE    0x02
#define FLAG_SCANNER_EXT_START_AT_BOTH_ENDS 0x04

#define FLAG_DO_CLEAR                       0x00
    // Do not write black pixels / pixels not used by pattern. Can be used to overwrite existing patterns - for colorWipe() and ScannerExtended()
#define FLAG_DO_NOT_CLEAR                   0x10
#define FLAG_END_WITH_BLACK                 0x20    // Last color of FLASH pattern is black
// see also: #define FLAG_TICKER_DATA_IN_FLASH 0x01 // Flag if DataPtr points to RAM or FLASH.
    uint8_t PatternFlags;  // special behavior of the pattern - BouncingBall: PercentageOfLossAtBounce

    union {
        uint8_t PatternLength;      // Length of a (scanner, stripes) pattern
        uint8_t NumberOfSteps;      // For Fade
        uint8_t IndexOfTopPixel;    // BouncingBall: Current integer IndexOfTopPixel
        uint8_t Cooling;            // Fire: Cooling
        uint8_t NumberOfFlakes;     // Snow: Number of flakes
    } ByteValue1;

    /*
     * 3 Extra variables used by some patterns
     */
    union {
        uint8_t PatternLength;          // 2. length of a (stripes) pattern
        uint8_t SnakeAutorunStep;
        uint8_t ScannerIntervalMillis;  // for delay of multiple falling stars
    } ByteValue2;

    union {
        color32_t BackgroundColor;
        color32_t Color2;               // second pattern color
        uint8_t *heatOfPixelArrayPtr;   // Allocated array for current heat values for Fire pattern
        uint16_t StartIntervalMillis;   // BouncingBall: interval for first step
        uint16_t NumberOfBouncings;     // ScannerExtended: Number of bounces
    } LongValue1;

    union {
        color32_t ColorTmp;             // Temporary color for dim and lightenColor() and for FadeSelectiveColor, ProcessSelectiveColor.
        float TopPixelIndex;            // BouncingBall: float index of TopPixel
        uint16_t DeltaBrightnessShift8; // ScannerExtended: Delta for each step for
        union {
            uint16_t Interval1;             // Flash: interval for color1
            uint16_t Interval2;             // Flash: interval for color2
        } Intervals;
    } LongValue2;

    union {
        // for ProcessSelectiveColor could not be member of the first 2 values, since these are used by the processing functions like fadeColor()
        uint32_t (*SingleLEDProcessingFunction)(NeoPatterns*);
    } Value3; // can be 16 bit for AVR and 32 bit for other platforms

    /*
     * for multiple pattern extensions
     */
    uint16_t Repetitions;               // counter for multipleHandler
    void (*NextOnPatternCompleteHandler)(NeoPatterns*);  // Next callback after completion of multiple pattern
    /*
     * List of all NeoPatterns
     */
    NeoPatterns *NextNeoPatternsObject;
    static NeoPatterns *FirstNeoPatternsObject;
};

void stopAllPatterns();

#define ENDLESS_HANDLER_POINTER ((void (*)(NeoPatterns*)) 1) // currently for initMultipleFallingStars()

//  Sample processing functions for ProcessSelectiveColor()
color32_t FadeColor(NeoPatterns *aNeoPatternsPtr);
color32_t DimColor(NeoPatterns *aNeoPatternsPtr);
color32_t BrightenColor(NeoPatterns *aNeoPatternsPtr);

// multiple pattern example
#if defined(ENABLE_PATTERN_SCANNER_EXTENDED)
void initMultipleFallingStars(NeoPatterns *aLedsPtr, color32_t aColor, uint8_t aLength, uint8_t aHalfDelayBetweenStarsMillis,
        uint8_t aRepetitions, void (*aNextOnCompleteHandler)(NeoPatterns*), uint8_t aDirection = DIRECTION_DOWN);
void multipleFallingStarsCompleteHandler(NeoPatterns *aLedsPtr);
#endif

#if defined(ENABLE_PATTERN_SCANNER_EXTENDED) && defined(ENABLE_PATTERN_RAINBOW_CYCLE) && defined(ENABLE_PATTERN_STRIPES) \
    && defined(ENABLE_PATTERN_FADE) && defined(ENABLE_PATTERN_COLOR_WIPE) && defined(ENABLE_PATTERN_HEARTBEAT)
void allPatternsRandomHandler(NeoPatterns *aLedsPtr);
#endif

#if defined(ENABLE_PATTERN_USER_PATTERN1)
void __attribute__((weak)) UserPattern1(NeoPatterns *aNeoPatterns, color32_t aPixelColor, color32_t aBackgroundColor,
        uint16_t aIntervalMillis, uint8_t aDirection = DIRECTION_UP);
#endif
#if defined(ENABLE_PATTERN_USER_PATTERN2)
void __attribute__((weak)) UserPattern2(NeoPatterns *aNeoPatterns, color32_t aColor, uint16_t aIntervalMillis,
        uint16_t aRepetitions = 0, uint8_t aDirection = DIRECTION_UP);
#endif

/*
 * Version 3.1.1 - 9/2022
 * - Added parameter aRepetitions to pattern RainbowCycle.
 * - Improved layout of character c.
 *
 * Version 3.1.0 - 8/2022
 * - Added functions printConnectionInfo(), fillRegion(), stop() and stopAllPatterns().
 * - Fixed brightness initialization bug for Neopixel with UnderlyingNeoPixelObjects.
 * - Renamed updateAll* and updateAndWait* functions.
 * - Now all NeoPattern objects are contained in NeoPatterns list.
 * - Now updateOrRedraw() does never call show().
 * - New pattern FLASH.
 * - Renamed ColorSet() to setColor().
 *
 * Version 3.0.0 - 5/2022
 * - Enabled individual selection of patterns to save program memory.
 * - Renamed NeoPatterns.cpp, MatrixNeoPatterns.cpp and MatrixSnake.cpp to NeoPatterns.hpp, MatrixNeoPatterns.hpp and MatrixSnake.hpp.
 * - Renamed matrix pattern macros from PATTERN_* to MATRIX_PATTERN_*.
 * - Changed parameter for endless repeats in initMultipleFallingStars().
 * - Improved usage of random().
 * - Added function fillRegion(), isActive() and setBrightnessValue().
 * - Added support for brightness and brightness non zero mode.
 * - Fixed aDoUpdate bug for FADE.
 * - Fixed bugs in Colors.h.
 *
 * Version 2.3.1 - 02/2021
 * - Changed type of TotalStepCounter from uint16_t to int16_t.
 * - Added `SnowMatrix` pattern.
 * - Improved debugging.
 * - Fixed random() bug for ESP32.
 * - Improved Fire cooling.
 *
 * Version 2.3.0 - 12/2020
 * - Removed restriction to 8 columns for `FireMatrix` pattern.
 * - Changed TickerUpdate() and loadPicture() and their callers to achieve that YOffset is consistent with Y direction and origin.
 *
 * Version 2.2.2 /2.2.3- 12/2020
 * - Fixed bugs if rows are not equal columns.
 * - Fixed bug in MatrixNeoPatterns constructor.
 * - Added `SUPPORT_ONLY_DEFAULT_GEOMETRY` compile option.
 * - Added loadPicture() for 16 bit pictures.
 *
 * Version 2.2.1 - 9/2020
 * - Removed blocking wait for ATmega32U4 Serial in examples.
 *
 * Version 2.2.0 - 4/2020
 * - Added support for RGBW patterns. Requires additional 200 bytes for AllPatternsOnMultiDevices example.
 *   Not defining SUPPORT_RGBW saves 400 bytes program memory for AllPatternsOnMultiDevices example.
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
 * - Changed signature of NeoPatterns(NeoPixel *aUnderlyingNeoPixelObject). Swapped 4. and 5. parameter to make it consistent to the NeoPixel signature.
 * - Function `setPixelOffsetForPartialNeoPixel()` in NeoPixel.cpp added.
 *
 * Version 2.0.0 - 11/2019
 * - Function `drawBar()` in NeoPixel.cpp added.
 * - Swapped parameter aNumberOfSteps and aIntervalMillis of `Stripes()`.
 * - Pattern `HEARTBEAT` and `BOUNCING_BALL` added.
 * - Added parameter aDirection to `Fire()`.
 * - Removed helper function `setDirectionAndTotalStepsAndIndex()`.
 */

#endif // _NEOPATTERNS_H
