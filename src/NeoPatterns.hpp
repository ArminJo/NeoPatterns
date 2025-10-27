/**
 * NeoPatterns.hpp
 *
 * This file includes the original code of Adafruit https://learn.adafruit.com/multi-tasking-the-arduino-part-3?view=all
 * as well as code of Fastled by Mark Kriegsman https://github.com/FastLED/FastLED/tree/master/examples/Fire2012
 *
 *  SUMMARY
 *  This is an extended version version of the NeoPattern Example by Adafruit
 *  https://learn.adafruit.com/multi-tasking-the-arduino-part-3?view=all
 *  You need to install "Adafruit NeoPixel" library under "Tools -> Manage Libraries..." or "Ctrl+Shift+I" -> use "neoPixel" as filter string.
 *  Extension are made to include more patterns and combined patterns and patterns for 8x8 NeoPixel matrix.
 *
 *  TIMING
 *  Calling the pattern function sets all parameters ready for generating the pattern, and immediately does a refresh to show the first pattern step.
 *  If the refresh should not happen if called asynchronously i.e. should only happen at update time (which in turn
 *  can call a pattern function by calling the callback function) you can set the parameter aShowOnlyAtUpdate to true at the constructor.
 *  The parameters were changed by the update before drawing the next pattern step. This allows arbitrary refresh (doUpdate = false) for the specified period.
 *  The very last step only does termination and calls the callback function, which will show the next pattern, but does not call show by itself,
 *  thus allowing to see the last pattern step for the specified period.
 *  Therefore a pattern with 4 different pattern steps needs 4 updates, the last updates only switches to next pattern or sets ActivePattern to PATTERN_NONE.
 *
 *  Copyright (C) 2018-2025  Armin Joachimsmeyer
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
#ifndef _NEOPATTERNS_HPP
#define _NEOPATTERNS_HPP

#include <Arduino.h>

#include "NeoPatterns.h"
#include "LongUnion.h" // for faster random()

// include sources
#include "NeoPixel.hpp"

// This block must be located after the includes of other *.hpp files
//#define LOCAL_INFO  // This enables info output only for this file
//#define LOCAL_DEBUG // This enables debug output only for this file - only for development
//#define LOCAL_TRACE // This enables trace output only for this file - only for development
#include "LocalDebugLevelStart.h"

const char PatternNone[] PROGMEM ="None";
const char PatternRainbowCycle[] PROGMEM ="Rainbow cycle";
const char PatternColorWipe[] PROGMEM ="Color wipe";
const char PatternFade[] PROGMEM ="Fade";
const char PatternDelay[] PROGMEM ="Delay";
const char PatternScannerExtended[] PROGMEM ="Scanner extended";
const char PatternStripes[] PROGMEM ="Stripes";
const char PatternFlash[] PROGMEM ="Flash";
const char PatternProcessSelectiveColor[] PROGMEM ="Process selective color";
const char PatternHeartbeat[] PROGMEM ="Heartbeat";
const char PatternFire[] PROGMEM ="Fire";
const char PatternTwinkle[] PROGMEM ="Twinkle";
const char PatternBouncingBall[] PROGMEM ="Bouncing ball";
const char PatternUserPattern1[] PROGMEM ="User pattern 1";
const char PatternUserPattern2[] PROGMEM ="User pattern 2";

/*
 * Include known matrix patterns
 */
const char MatrixPatternTicker[] PROGMEM ="Ticker";
const char MatrixPatternMove[] PROGMEM ="Move";
const char MatrixPatternMovingPicture[] PROGMEM ="Moving picture";
const char MatrixPatternSnow[] PROGMEM ="Snow";
const char MatrixExtraPatternSnake[] PROGMEM ="Snake";
const char PatternUnknown[] PROGMEM ="Unknown";

// ActivePattern can be used as index of PatternNamesArray
const char *const PatternNamesArray[] PROGMEM = { PatternNone, PatternRainbowCycle, PatternColorWipe, PatternFade, PatternDelay,
        PatternScannerExtended, PatternStripes, PatternFlash, PatternProcessSelectiveColor, PatternHeartbeat, PatternFire,
        PatternTwinkle, PatternBouncingBall, PatternUserPattern1, PatternUserPattern2, MatrixPatternTicker, MatrixPatternMove,
        MatrixPatternMovingPicture, PatternFire, MatrixPatternSnow, MatrixExtraPatternSnake, PatternUnknown };

// array of update function pointer, not used since it needs 150 bytes more :-(
//bool (NeoPatterns::*sUpdateFunctionPointerArray[])(
//        bool) = {&NeoPatterns::RainbowCycleUpdate, &NeoPatterns::ColorWipeUpdate, &NeoPatterns::FadeUpdate, &NeoPatterns::DelayUpdate,
//            &NeoPatterns::ScannerExtendedUpdate, &NeoPatterns::StripesUpdate, &NeoPatterns::ProcessSelectiveColorUpdate, &NeoPatterns::FireUpdate,
//            &NeoPatterns::HeartbeatUpdate, &NeoPatterns::Pattern1Update, &NeoPatterns::Pattern2Update};
// call it with: bool tPatternEnded = (this->*sUpdateFunctionPointerArray[ActivePattern])(true);

/*
 * Start of static list of all NeoPatterns object
 */
NeoPatterns *NeoPatterns::FirstNeoPatternsObject = nullptr;
/**********************************************************************************
 * Code inspired by https://learn.adafruit.com/multi-tasking-the-arduino-part-3?view=all
 * Changed and extended for added functionality
 **********************************************************************************/
/*
 * Constructor - get and call Adafruit_NeoPixel constructor to initialize strip
 * NeoPatterns is not derived from Adafruit_NeoPixel to enable running more than one pattern
 * on the same NeoPixel object by using NeoPixel::setPixelBuffer().
 * In this case it avoids doubling the big pixel buffer.
 */

NeoPatterns::NeoPatterns() : // @suppress("Class members should be properly initialized")
        NeoPixel() {
    init();
}

/*
 * Insert "this" in the NextNeoPatternsObject list
 * Do not forget to manage list in destructor (not yet implemented)
 */
void NeoPatterns::_insertIntoNeopatternsList() {
    /*
     * Insert "this" in the NextNeoPatternsObject list
     */
    NextNeoPatternsObject = nullptr;
    NeoPatterns *tNextObjectPointer = FirstNeoPatternsObject;
    if (tNextObjectPointer == nullptr) {
        FirstNeoPatternsObject = this;
    } else {
        while (tNextObjectPointer->NextNeoPatternsObject != nullptr) {
            tNextObjectPointer = tNextObjectPointer->NextNeoPatternsObject;
        }
        tNextObjectPointer->NextNeoPatternsObject = this;
    }
}

void NeoPatterns::init() {
    OnPatternComplete = nullptr;
    ActivePattern = PATTERN_NONE;
    LongValue1.PixelHeatArrayPtr = nullptr;
    _insertIntoNeopatternsList();
}

NeoPatterns::NeoPatterns(uint16_t aNumberOfPixels, uint8_t aPin, neoPixelType aTypeOfPixel, // @suppress("Class members should be properly initialized")
        void (*aPatternCompletionCallback)(NeoPatterns*), bool aShowOnlyAtUpdate) :
        NeoPixel(aNumberOfPixels, aPin, aTypeOfPixel) {

    init();

    if (aShowOnlyAtUpdate) {
        PixelFlags |= PIXEL_FLAG_SHOW_ONLY_AT_UPDATE;
    }
    OnPatternComplete = aPatternCompletionCallback;
}

/*
 * @return false if no memory available
 */
bool NeoPatterns::init(uint16_t aNumberOfPixels, uint8_t aPin, neoPixelType aTypeOfPixel,
        void (*aPatternCompletionCallback)(NeoPatterns*), bool aShowOnlyAtUpdate) {

    bool tRetvalue = NeoPixel::init(aNumberOfPixels, aPin, aTypeOfPixel);
    init();

    if (aShowOnlyAtUpdate) {
        PixelFlags |= PIXEL_FLAG_SHOW_ONLY_AT_UPDATE;
    }
    OnPatternComplete = aPatternCompletionCallback;
    return tRetvalue;
}

/*
 * Used to create a NeoPattern, which runs on a segment of the existing NeoPattern object.
 * This creates a new NeoPixel object and replaces the new pixel buffer with the underlying existing one.
 * @param aShowAllPixel - true = calls show function of the existing NeoPixel object (compatibility mode)
 *                        false = suppress calling the show function, since it makes no sense
 * ATTENTION. To save a lot of CPU time, set aShowAllPixel to false and use only UnderlyingNeoPixelObject->show().
 * if(PartialNeoPixelBar1.update() || PartialNeoPixelBar2.update()){
 *   UnderlyingNeoPixelObject->show();
 * }
 */
NeoPatterns::NeoPatterns(NeoPixel *aUnderlyingNeoPixelObject, uint16_t aPixelOffset, uint16_t aNumberOfPixels, // @suppress("Class members should be properly initialized")
        bool aEnableShowOfUnderlyingPixel, void (*aPatternCompletionCallback)(NeoPatterns*), bool aShowOnlyAtUpdate) :
        NeoPixel(aUnderlyingNeoPixelObject, aPixelOffset, aNumberOfPixels, aEnableShowOfUnderlyingPixel) {

    init();

    OnPatternComplete = aPatternCompletionCallback;
    if (aShowOnlyAtUpdate) {
        PixelFlags |= PIXEL_FLAG_SHOW_ONLY_AT_UPDATE;
    }
}

void NeoPatterns::init(NeoPixel *aUnderlyingNeoPixelObject, uint16_t aPixelOffset, uint16_t aNumberOfPixels,
        bool aEnableShowOfUnderlyingPixel, void (*aPatternCompletionCallback)(NeoPatterns*), bool aShowOnlyAtUpdate) {

    NeoPixel::init(aUnderlyingNeoPixelObject, aPixelOffset, aNumberOfPixels, aEnableShowOfUnderlyingPixel);
    init();

    OnPatternComplete = aPatternCompletionCallback;
    if (aShowOnlyAtUpdate) {
        PixelFlags |= PIXEL_FLAG_SHOW_ONLY_AT_UPDATE;
    }
}

void NeoPatterns::setCallback(void (*callback)(NeoPatterns*)) {
    OnPatternComplete = callback;
}

bool NeoPatterns::checkForUpdate() {
    if ((millis() - lastUpdate) > Interval) {
        return true;
    }
    return false;
}

bool NeoPatterns::isActive() {
    return (ActivePattern != PATTERN_NONE);
}

void NeoPatterns::updateShowAndWaitForPatternToStop(uint8_t aBrightness) {
#if defined(SUPPORT_BRIGHTNESS)
    Brightness = aBrightness;
#else
    (void) aBrightness;
#endif
    updateShowAndWaitForPatternToStop();
}

void NeoPatterns::updateShowAndWaitForPatternToStop() {
    void (*tOnPatternCompleteBackup)(NeoPatterns*) = OnPatternComplete;
    OnPatternComplete = nullptr;
    while (ActivePattern != PATTERN_NONE) {
        update();
        yield();
    }
    OnPatternComplete = tOnPatternCompleteBackup;
}

/*
 * This function checks all patterns of an underlying NeoPixel for update and calls show() of the underlying NeoPixel if needed.
 * @return true, if AtLeastOnePatternIsActive
 */
bool NeoPatterns::updateAndShowAlsoAllPartialPatterns(uint8_t aBrightness) {
#if defined(SUPPORT_BRIGHTNESS)
    Brightness = aBrightness;
#else
    (void) aBrightness;
#endif
    return updateAndShowAlsoAllPartialPatterns();
}
bool NeoPatterns::updateAndShowAlsoAllPartialPatterns() {

    bool tNeedShow = false;
    bool tAtLeastOnePatternIsActive = false;
    /*
     * Traverse through complete NeoPattern list and process all UnderlyingNeoPixelObjects including the object itself!
     */
    for (NeoPatterns *tNextObjectPointer = NeoPatterns::FirstNeoPatternsObject; tNextObjectPointer != nullptr; tNextObjectPointer =
            tNextObjectPointer->NextNeoPatternsObject) {
        /*
         * Check for same underlying object (including the underlying object itself) and update if pattern is active
         */
        if (tNextObjectPointer->ActivePattern != PATTERN_NONE
                && tNextObjectPointer->UnderlyingNeoPixelObject == UnderlyingNeoPixelObject) {
            tAtLeastOnePatternIsActive = true;
            tNeedShow |= tNextObjectPointer->updateOrRedraw(DO_NO_REDRAW_IF_NO_UPDATE); // this avoids show of any patterns at this call
        }
#if defined(LOCAL_TRACE)
        printPin(&Serial);
        Serial.print(F("&Pattern=0x"));
        Serial.print((uintptr_t) tNextObjectPointer, HEX);
        Serial.print(F(" &nextPattern=0x"));
        Serial.print((uintptr_t) tNextObjectPointer->NextNeoPatternsObject, HEX);
        Serial.print(F(" ActivePattern="));
        Serial.print(tNextObjectPointer->ActivePattern);
        Serial.print(F(" &UnderlyingNeoPixel=0x"));
        Serial.print((uintptr_t) tNextObjectPointer->UnderlyingNeoPixelObject, HEX);
        Serial.print(F(" tNeedShow="));
        Serial.println(tNeedShow);
#endif
    }
    if (tNeedShow) {
        UnderlyingNeoPixelObject->show();
    }
    return tAtLeastOnePatternIsActive;
}

void NeoPatterns::updateAndShowAlsoAllPartialPatternsAndWaitForPatternsToStop(uint8_t aBrightness) {
#if defined(SUPPORT_BRIGHTNESS)
    Brightness = aBrightness;
#else
    (void) aBrightness;
#endif
    updateAndShowAlsoAllPartialPatternsAndWaitForPatternsToStop();
}
void NeoPatterns::updateAndShowAlsoAllPartialPatternsAndWaitForPatternsToStop() {
    void (*tOnPatternCompleteBackup)(NeoPatterns*) = OnPatternComplete;
    OnPatternComplete = nullptr;
    while (updateAndShowAlsoAllPartialPatterns()) {
        yield();
    }
    OnPatternComplete = tOnPatternCompleteBackup;
}

/*
 * DEPRECATED
 */
bool NeoPatterns::updateAllPartialPatterns(uint8_t aBrightness) {
    return updateAndShowAlsoAllPartialPatterns(aBrightness);
}
bool NeoPatterns::updateAllPartialPatterns() {
    return updateAndShowAlsoAllPartialPatterns();
}
void NeoPatterns::updateAndWaitForPatternToStop(uint8_t aBrightness) {
    updateShowAndWaitForPatternToStop(aBrightness);
}
void NeoPatterns::updateAndWaitForPatternToStop() {
    updateShowAndWaitForPatternToStop();
}
void NeoPatterns::updateAllPartialPatternsAndWaitForPatternsToStop(uint8_t aBrightness) {
    updateAndShowAlsoAllPartialPatternsAndWaitForPatternsToStop(aBrightness);
}
void NeoPatterns::updateAllPartialPatternsAndWaitForPatternsToStop() {
    updateAndShowAlsoAllPartialPatternsAndWaitForPatternsToStop();
}

/*
 * Checks for asynchronously calling and sets the timestamp of lastUpdate
 * The asynchronous call is detected by checking if the current pattern is not PATTERN_NONE
 */
void NeoPatterns::showPatternInitially() {
    if ((ActivePattern == PATTERN_NONE) || (PixelFlags & PIXEL_FLAG_SHOW_ONLY_AT_UPDATE) == 0) {
        show();
        lastUpdate = millis(); // to schedule the next update
#if defined(LOCAL_TRACE)
        printPin(&Serial);
        Serial.print(F("Init lastUpdate to "));
        Serial.println(lastUpdate);
#endif
    }
#if defined(LOCAL_DEBUG)
    else {
        printPin(&Serial);
        Serial.println(F("Called asynchronously -> do not show"));
    }
#endif
}

void NeoPatterns::stop() {
    ActivePattern = PATTERN_NONE;
}

void stopAllPatterns() {
    //Walk through the NextNeoPatternsObject list
    for (NeoPatterns *tNextObjectPointer = NeoPatterns::FirstNeoPatternsObject; tNextObjectPointer != nullptr; tNextObjectPointer =
            tNextObjectPointer->NextNeoPatternsObject) {
        tNextObjectPointer->ActivePattern = PATTERN_NONE;
    }
}

/*
 * @brief   Updates the pattern if update interval has expired.
 * @return  Returns true if update has happened in order to give the caller a chance to manually change parameters (like color etc.)
 */
bool NeoPatterns::update(uint8_t aBrightness) {
#if defined(SUPPORT_BRIGHTNESS)
    Brightness = aBrightness;
#else
    (void) aBrightness;
#endif
    return update();
}

/*
 * Does not call show()
 */
void NeoPatterns::forceUpdate(uint8_t aBrightness) {
#if defined(SUPPORT_BRIGHTNESS)
    Brightness = aBrightness;
#else
    (void) aBrightness;
#endif
    _update(true);
    lastUpdate = millis(); // remember last time of update
}

bool NeoPatterns::update() {
    if (ActivePattern == PATTERN_NONE) {
        return false;
    }
    if ((millis() - lastUpdate) > Interval) {
        if (!_update(true)) {
            show();
        }
        lastUpdate = millis(); // remember last time of update
        return true;
    }
    return false;
}

/*
 * updateOrRedraw() serves to draw pattern of partial neopixels (PIXEL_FLAG_IS_PARTIAL_NEOPIXEL) without showing the result
 * or to combine a background pattern with foreground patterns which are not synchronized.
 *
 * !!! updateOrRedraw() does NOT show the pattern, this must be done manually after all redraws. !!!
 *
 * Updates (and redraws) the pattern if update interval has expired. Otherwise only redraws it if aDoRedrawIfNoUpdate is true.
 *
 * @param aDoRedrawIfNoUpdate Only if true, do an redraw, if no update was scheduled.
 * @return true if update has happened to signal that a show() is required after all partial neopixels are updated.
 *              It also gives the caller a chance to manually change parameters (like color etc.).
 */
bool NeoPatterns::updateOrRedraw(bool aDoRedrawIfNoUpdate, uint8_t aBrightness) {
#if defined(SUPPORT_BRIGHTNESS)
    Brightness = aBrightness;
#else
    (void) aBrightness;
#endif
    return updateOrRedraw(aDoRedrawIfNoUpdate);
}

bool NeoPatterns::updateOrRedraw(bool aDoRedrawIfNoUpdate) {
    bool tDoUpdate = (millis() - lastUpdate) > Interval;
    if (tDoUpdate || aDoRedrawIfNoUpdate) {
        /*
         * If tDoUpdate is true, update pattern, otherwise only redraw the pattern
         */
        _update(tDoUpdate);
    }
    if (tDoUpdate) {
        lastUpdate = millis(); // remember last time of update
    }
    return tDoUpdate;
}

/*
 * @return - true if pattern has ended, false if pattern has NOT ended
 */
bool NeoPatterns::_update(bool aDoUpdate) {

    bool tPatternEnded = true; // to suppress show for ended pattern
    switch (ActivePattern) {
    case PATTERN_DELAY:
        tPatternEnded = DelayUpdate(aDoUpdate);
        break;
#if defined(ENABLE_PATTERN_RAINBOW_CYCLE)
    case PATTERN_RAINBOW_CYCLE:
        tPatternEnded = RainbowCycleUpdate(aDoUpdate);
        break;
#endif
#if defined(ENABLE_PATTERN_COLOR_WIPE)
    case PATTERN_COLOR_WIPE:
        tPatternEnded = ColorWipeUpdate(aDoUpdate);
        break;
#endif
#if defined(ENABLE_PATTERN_FADE)
    case PATTERN_FADE:
        tPatternEnded = FadeUpdate(aDoUpdate);
        break;
#endif
#if defined(ENABLE_PATTERN_PROCESS_SELECTIVE)
    case PATTERN_PROCESS_SELECTIVE:
        tPatternEnded = ProcessSelectiveColorUpdate(aDoUpdate);
        break;
#endif
#if defined(ENABLE_PATTERN_FIRE)
    case PATTERN_FIRE:
        tPatternEnded = FireUpdate(aDoUpdate);
        break;
#endif
#if defined(ENABLE_PATTERN_SCANNER_EXTENDED)
    case PATTERN_SCANNER_EXTENDED:
        tPatternEnded = ScannerExtendedUpdate(aDoUpdate);
        break;
#endif
#if defined(ENABLE_PATTERN_STRIPES)
    case PATTERN_STRIPES:
        tPatternEnded = StripesUpdate(aDoUpdate);
        break;
#endif
#if defined(ENABLE_PATTERN_FLASH)
    case PATTERN_FLASH:
        tPatternEnded = FlashUpdate(aDoUpdate);
        break;
#endif
#if defined(ENABLE_PATTERN_TWINKLE)
    case PATTERN_TWINKLE:
        tPatternEnded = TwinkleUpdate(aDoUpdate);
        break;
#endif
#if defined(ENABLE_PATTERN_HEARTBEAT)
    case PATTERN_HEARTBEAT:
        tPatternEnded = HeartbeatUpdate(aDoUpdate);
        break;
#endif
#if defined(ENABLE_PATTERN_USER_PATTERN1)
    case PATTERN_USER_PATTERN1:
        tPatternEnded = Pattern1Update(aDoUpdate);
        break;
#endif
#if defined(ENABLE_PATTERN_USER_PATTERN2)
    case PATTERN_USER_PATTERN2:
        tPatternEnded = Pattern2Update(aDoUpdate);
        break;
#endif
#if defined(ENABLE_PATTERN_BOUNCING_BALL)
    case PATTERN_BOUNCING_BALL:
        tPatternEnded = BouncingBallUpdate(aDoUpdate);
        break;
#endif
    default:
        break;
    }
    return tPatternEnded;
}

/*
 * Decrement TotalSteps and call callback
 * @return - true if pattern has ended, false if pattern has NOT ended
 */
bool NeoPatterns::decrementTotalStepCounter() {
    TotalStepCounter--;
    if (TotalStepCounter < 0) {
        // Safety net. The pattern has ended, but the callback has not set a new pattern
        ActivePattern = PATTERN_NONE;
#if defined(LOCAL_INFO)
        printPin(&Serial);
        Serial.println(F("Reset pattern to NONE"));
#endif
        return true;
    }
    if (TotalStepCounter == 0) {
        if (OnPatternComplete != nullptr) {
            /*
             * Do not set activePattern to PATTERN_NONE, to enable the callback to see the finished one.
             */
#if defined(LOCAL_DEBUG)
            printPin(&Serial);
            printPattern();
            Serial.print(F(": Call completion callback 0x"));
            Serial.println((__SIZE_TYPE__) (OnPatternComplete) << 1, HEX);
            Serial.flush();
#endif
            OnPatternComplete(this); // call the completion callback
#if defined(LOCAL_DEBUG)
            printPin(&Serial);
            Serial.print(F("New "));
            printPattern();
            Serial.println();
            Serial.flush();
#endif
        } else {
#if defined(LOCAL_DEBUG)
            printPin(&Serial);
            printPattern();
            Serial.println(F(": No completion callback, ActivePattern = PATTERN_NONE"));
#endif
            ActivePattern = PATTERN_NONE; // reset ActivePattern to enable polling for end of pattern.
        }
        return true;
    }
#if defined(LOCAL_TRACE)
    Serial.print(F(" TotalStepCounter="));
    Serial.print(TotalStepCounter);
    Serial.print(F(" lastUpdate="));
    Serial.println(lastUpdate);
#endif
    return false;
}

/*
 * At 800 kHz we have 1,25 us per bit and 30 us per pixel of 24 bit
 * this is 12 ms for 400 pixel
 * This update time must be subtracted from all aIntervalMillis parameters.
 */
void NeoPatterns::setCompensatedInterval(uint16_t aIntervalToCompensate) {
    uint8_t tCompensationForShowTime = UnderlyingNeoPixelObject->getNumberOfPixels() / 33;
    if (aIntervalToCompensate > tCompensationForShowTime) {
        Interval = aIntervalToCompensate - tCompensationForShowTime;
    } else {
        Interval = 0;
    }
}
/*
 * Decrement TotalSteps and call callback
 */
void NeoPatterns::setNextIndex() {
    if (Direction == DIRECTION_UP) {
        Index++;
    } else {
        Index--;
    }
#if defined(LOCAL_DEBUG)
    printPin(&Serial);
    printPattern();
    Serial.print(F("TotalSteps="));
    Serial.print(TotalStepCounter);
    Serial.print(F(" Index="));
    Serial.println((int16_t) Index);

#endif
}

/*
 * Decrement TotalSteps and call callback
 * @return - true if pattern has ended, false if pattern has NOT ended
 */
bool NeoPatterns::decrementTotalStepCounterAndSetNextIndex() {
    if (decrementTotalStepCounter()) {
        return true;
    }
    setNextIndex();
    return false;
}

#if defined(ENABLE_PATTERN_RAINBOW_CYCLE)
/*
 * RainbowCycleDuration: Parameter aCompleteDurationMillis is complete duration and not step interval.
 * Initialize for a RainbowCycle
 * First of 256 steps is last pixel = Wheel(0) = RED going up over yellow and green to BLUE or backwards if DIRECTION_DOWN
 */
void NeoPatterns::RainbowCycleDuration(uint8_t aCompleteDurationMillis, uint8_t aDirection, uint8_t aRepetitions) {
    RainbowCycle(aCompleteDurationMillis, aDirection, aRepetitions);
    setCompensatedInterval(aCompleteDurationMillis / 256);
}

/*
 * @param aDirection where the rainbow starts and therfor also in which direction it moves
 */
void NeoPatterns::RainbowCycle(uint8_t aIntervalMillis, uint8_t aDirection, uint8_t aRepetitions) {
    // Must move index in opposite direction
    Direction = OppositeDirection(aDirection & DIRECTION_UP_DOWN_MASK);
    TotalStepCounter = 256 * aRepetitions;
    Index = 0;
    setCompensatedInterval(aIntervalMillis);

    RainbowCycleUpdate(false);
    showPatternInitially();
// must be after showPatternInitially(), since it requires the old value do detect asynchronous calling
    ActivePattern = PATTERN_RAINBOW_CYCLE;
#if defined(LOCAL_TRACE)
    printInfo(&Serial, true);
#endif
}

/*
 * Update the Rainbow Cycle Pattern starting with a color that is next position in wheel
 * @return - true if pattern has ended, false if pattern has NOT ended
 */
bool NeoPatterns::RainbowCycleUpdate(bool aDoUpdate) {
    if (aDoUpdate) {
        if (decrementTotalStepCounter()) {
            return true;
        }
        Index++; // choose next color, direction is determined at fillWithRainbow() below
    }
    /*
     * Refresh pattern, Index is start position of color wheel
     */
    fillWithRainbow(Index, Direction);
    return false;
}
#endif

#if defined(ENABLE_PATTERN_COLOR_WIPE)
/**
 * ColorWipeDuration: Parameter aCompleteDurationMillis is complete duration and not step interval.
 * @brief   Initialize for a ColorWipe. First step is one pixel set, last step is all pixel set.
 * @param  aMode can be 0 / FLAG_DO_CLEAR (default) or FLAG_DO_NOT_CLEAR(_BlackPixel)
 * @param  aDirection can be DIRECTION_UP (default) or DIRECTION_DOWN
 *           if PARAMETER_IS_DURATION bit is set, the interval is not between steps, but for the complete pattern.
 *
 * Idea: length of head and random fade of wipe tail gives meteor rain
 */
void NeoPatterns::ColorWipeDuration(color32_t aColor, uint16_t aCompleteDurationMillis, bool aDoNotClearBefore, uint8_t aDirection) {
    ColorWipe(aColor, aCompleteDurationMillis, aDoNotClearBefore, aDirection );
    setCompensatedInterval(aCompleteDurationMillis / TotalStepCounter);
}
void NeoPatterns::ColorWipe(color32_t aColor, uint16_t aIntervalMillis, bool aDoNotClearBefore, uint8_t aDirection) {
    Color1 = aColor;
    Direction = aDirection & DIRECTION_UP_DOWN_MASK;
    TotalStepCounter = numLEDs;

    if (Direction == DIRECTION_UP) {
        Index = 0;
    } else {
        Index = numLEDs - 1;
    }

    setCompensatedInterval(aIntervalMillis);

    if( !aDoNotClearBefore){
        clear();
    }
    ColorWipeUpdate(false);
    showPatternInitially();
// must be after showPatternInitially(), since it requires the old value do detect asynchronous calling
    ActivePattern = PATTERN_COLOR_WIPE;
#if defined(LOCAL_TRACE)
    printInfo(&Serial, true);
#endif
}

/*
 * Update the Color Wipe Pattern. Fill with color.
 * @param aDoUpdate - false just redraw current pattern
 * @return - true if pattern has ended, false if pattern has NOT ended
 */
bool NeoPatterns::ColorWipeUpdate(bool aDoUpdate) {
    if (aDoUpdate) {
        if (decrementTotalStepCounterAndSetNextIndex()) {
            return true;
        }
    }
    /*
     * Refresh pattern
     */
    for (int_fast16_t i = 0; i < (int16_t)numLEDs; i++) {
        if ((Direction == DIRECTION_UP && i <= Index) || (Direction == DIRECTION_DOWN && i >= Index)) {
            setPixelColor(i, Color1);
        }
    }
    return false;
}
#endif

#if defined(ENABLE_PATTERN_TWINKLE)
/*
 * @param aColorSpecial - If aColorSpecial == COLOR32_SPECIAL use random color
 */
void NeoPatterns::Twinkle(color32_t aColorSpecial, uint8_t aAverageNumberOfActivePixel, uint16_t aIntervalMillis,
        uint16_t aRepetitions, bool aDoNotClearBefore) {
    Color1 = aColorSpecial;
    Interval = aIntervalMillis;
    TotalStepCounter = 2 * aRepetitions;
    Index = 0;

    if (aAverageNumberOfActivePixel == 0) {
        aAverageNumberOfActivePixel = 1;
    }
    ByteValue1.AverageNumberOfActivePixel = aAverageNumberOfActivePixel;
    setCompensatedInterval(aIntervalMillis);
    if( !aDoNotClearBefore){
        clear();
    }
    TwinkleUpdate(false);
    showPatternInitially();
// must be after showPatternInitially(), since it requires the old value do detect asynchronous calling
    ActivePattern = PATTERN_TWINKLE;
#if defined(LOCAL_TRACE)
    printInfo(&Serial, true);
#endif
}

bool NeoPatterns::TwinkleUpdate(bool aDoUpdate) {
    if (aDoUpdate) {
        if (decrementTotalStepCounterAndSetNextIndex()) {
            return true;
        }
    }

    /*
     * Refresh pattern
     * Remove every nth pixel, so we have n loops to remove all old pixel
     */
    for (uint16_t i = random8(ByteValue1.AverageNumberOfActivePixel); i < numLEDs - 1; i += ByteValue1.AverageNumberOfActivePixel) {
        clearPixel(i);
    }
    /*
     * Set only one pixel
     */
    color32_t tColor = Color1;
    if (tColor == COLOR32_SPECIAL) {
        tColor = NeoPatterns::Wheel(random8(MAX_WHEEL_POSITION));
    }
    setPixelColor(random(numLEDs), tColor);

    return false;
}
#endif

#if defined(ENABLE_PATTERN_FADE)
/*
 * Initialize for a Fade from aColorStart to aColorEnd
 * @ param aNumberOfSteps first step is aColorStart, last step is aColorEnd
 */
void NeoPatterns::FadeDuration(color32_t aColorStart, color32_t aColorEnd, uint16_t aNumberOfSteps, uint16_t aCompleteDurationMillis) {
    Fade( aColorStart,  aColorEnd,  aNumberOfSteps,  aCompleteDurationMillis);
    setCompensatedInterval(aCompleteDurationMillis/TotalStepCounter);
}
void NeoPatterns::Fade(color32_t aColorStart, color32_t aColorEnd, uint16_t aNumberOfSteps, uint16_t aIntervalMillis) {
    setCompensatedInterval(aIntervalMillis);
    TotalStepCounter = aNumberOfSteps + 1; // + 1 for the last step to show BackgroundColor
    ByteValue1.NumberOfSteps = aNumberOfSteps; // original start value of aNumberOfSteps
    Color1 = aColorStart;
    LongValue1.Color2 = aColorEnd;
    Index = 0;
    Direction = DIRECTION_UP;

    FadeUpdate(false);
    showPatternInitially();
// must be after showPatternInitially(), since it requires the old value do detect asynchronous calling
    ActivePattern = PATTERN_FADE;
#if defined(LOCAL_TRACE)
    printInfo(&Serial, true);
#endif
}

// Update the Fade pattern
bool NeoPatterns::FadeUpdate(bool aDoUpdate) {
    if (aDoUpdate) {
        if (decrementTotalStepCounterAndSetNextIndex()) {
            return true;
        }
    }
    /*
     * Refresh pattern
     */
// Calculate linear interpolation between Color1 and BackgroundColor
// Optimize order of operations to minimize truncation error
    uint8_t tRed = ((getRedPart(Color1) * (ByteValue1.NumberOfSteps - Index)) + (getRedPart(LongValue1.Color2) * Index))
            / ByteValue1.NumberOfSteps;
    uint8_t tGreen = ((getGreenPart(Color1) * (ByteValue1.NumberOfSteps - Index)) + (getGreenPart(LongValue1.Color2) * Index))
            / ByteValue1.NumberOfSteps;
    uint8_t tBlue = ((getBluePart(Color1) * (ByteValue1.NumberOfSteps - Index)) + (getBluePart(LongValue1.Color2) * Index))
            / ByteValue1.NumberOfSteps;
#if defined(_SUPPORT_RGBW)
    uint8_t tWhite = ((getWhitePart(Color1) * (ByteValue1.NumberOfSteps - Index)) + (getWhitePart(LongValue1.Color2) * Index))
            / ByteValue1.NumberOfSteps;
    setColor(Color(tRed, tGreen, tBlue, tWhite));
#else
    setColor(Color(tRed, tGreen, tBlue));
#endif

    return false;
}
#endif

#if defined(ENABLE_PATTERN_HEARTBEAT)
/*
 * Start with brightness low, then brighten and dim color, each in 16 steps.
 * Using 16 values from the gamma32 table starting with index 1 ending with index 31
 * First step is all at gamma[1], last step of pattern is all at gamma[1], last (extra) step after all repetitions is all black.
 * @param   aDoNotClearAfter - If true, last clear() step is skipped
 * @param   aRepetitions 1 => 2 times brighten and dim
 */
void NeoPatterns::Heartbeat(color32_t aColor, uint16_t aIntervalMillis, uint16_t aRepetitions, bool aDoNotClearAfter) {
    Color1 = aColor;
    setCompensatedInterval(aIntervalMillis);
    TotalStepCounter = (2 * 16 * (aRepetitions + 1)) + 1; // 2 times 16 values plus the last clear step
    if(aDoNotClearAfter) {
        PatternFlags = FLAG_DO_NOT_CLEAR;
        TotalStepCounter--;
    } else {
        PatternFlags = FLAG_DO_CLEAR;
    }
    Direction = DIRECTION_UP;
    Index = 8;

    HeartbeatUpdate(false);
    showPatternInitially();
// must be after showPatternInitially(), since it requires the old value do detect asynchronous calling
    ActivePattern = PATTERN_HEARTBEAT;
#if defined(LOCAL_TRACE)
    printInfo(&Serial, true);
#endif
}

/*
 * @return - true if pattern has ended, false if pattern has NOT ended
 */
bool NeoPatterns::HeartbeatUpdate(bool aDoUpdate) {
    color32_t tDimmedColor = 1;
    do {
        if (aDoUpdate) {
            if (decrementTotalStepCounter()) {
                return true;
            }
            /*
             * Next index
             */
            if (Direction == DIRECTION_UP) {
                if (Index >= 248) {
                    // show highest index twice
                    Direction = DIRECTION_DOWN;
                } else {
                    Index += 16; // up to 256
                }
            } else {
                if (Index <= 8) {
                    // show index 1 twice
                    Direction = DIRECTION_UP;
                } else {
                    Index -= 16;
                }
            }
        }
#if defined(LOCAL_TRACE)
        printPin(&Serial);
        Serial.print(F("Index="));
        Serial.println(Index);
#endif
        /*
         * Refresh pattern
         */
        if (TotalStepCounter == 1 && (PatternFlags & FLAG_DO_NOT_CLEAR) == 0) {
            clear(); // last pattern -> clear pattern
        } else {
            tDimmedColor = dimColorWithGamma5(Color1, Index);
            if (tDimmedColor == 0) {
                // now we have black, which should be skipped
                aDoUpdate = true; // enable next index for next loop
#if defined(LOCAL_TRACE)
                Serial.println(F("Skip black color index"));
#endif
            } else {
                setColor(tDimmedColor);
            }
        }
    } while (tDimmedColor == 0);

    return false;
}
#endif

/****************************************************************************
 * START OF EXTENSIONS
 ****************************************************************************/
#if defined(ENABLE_PATTERN_SCANNER_EXTENDED)
/*
 * ScannerExtendedDuration: Parameter aCompleteDurationMillis is complete duration and not step interval.
 * Code for scanner default is: pattern completely visible at start and end
 * If FLAG_SCANNER_EXT_VANISH_COMPLETE then first and last pattern are clear
 * @param   aNumberOfBouncings - 0 = no bouncing, one time pattern.
 * @param   aMode - see NeoPatterns.h line 240
 *                  FLAG_SCANNER_EXT_ROCKET, FLAG_SCANNER_EXT_CYLON, FLAG_SCANNER_EXT_VANISH_COMPLETE, FLAG_SCANNER_EXT_START_AT_BOTH_ENDS
 *                  FLAG_DO_NOT_CLEAR
 * @param   aDirection - DIRECTION_UP (default) or DIRECTION_DOWN
 */
void NeoPatterns::ScannerExtendedDuration(color32_t aColor, uint8_t aLength, uint16_t aCompleteDurationMillis, uint16_t aNumberOfBouncings,
        uint8_t aMode, uint8_t aDirection) {
    ScannerExtended(aColor, aLength, aCompleteDurationMillis, aNumberOfBouncings, aMode, aDirection);
    setCompensatedInterval(aCompleteDurationMillis / TotalStepCounter);

}
void NeoPatterns::ScannerExtended(color32_t aColor, uint8_t aLength, uint16_t aIntervalMillis, uint16_t aNumberOfBouncings,
        uint8_t aMode, uint8_t aDirection) {
// The variables ByteValue2, Repetitions and NextOnPatternCompleteHandler are used by MultipleFallingStars and cannot be used here
    Color1 = aColor;
    LongValue1.NumberOfBouncings = aNumberOfBouncings;
    LongValue2.DeltaBrightnessShift8 = 0x10000 / aLength; // Delta for each step

    ByteValue1.PatternLength = aLength;
    PatternFlags = aMode;
    Direction = aDirection & DIRECTION_UP_DOWN_MASK;
    TotalStepCounter = numLEDs - aLength; // pattern is completely visible at start and end
    Index = aLength - 1; // start position pattern is completely visible at start

    uint16_t tStepsForBounce = numLEDs - 1;

    if (!(aMode & FLAG_DO_NOT_CLEAR)) {
        clear();
    }

    if (aMode & FLAG_SCANNER_EXT_VANISH_COMPLETE) {
        // invisible at start and end
        Index -= aLength;
        TotalStepCounter += 2 * aLength;
        if (aMode & FLAG_SCANNER_EXT_CYLON) {
            // Cylon has just another side to hide :-)
            TotalStepCounter += 2 * (aLength - 1);
            Index -= aLength - 1;
        }
    }
    if (aMode & FLAG_SCANNER_EXT_CYLON) {
        TotalStepCounter -= aLength - 1; // subtract for pattern is completely visible at start and end
        tStepsForBounce = numLEDs - (2 * aLength - 1); // total length is (length + length - 1) since the brightest led is not doubled
    }

    if (aNumberOfBouncings > 0) {
        TotalStepCounter += (tStepsForBounce * aNumberOfBouncings);
    }

    if (Direction == DIRECTION_DOWN) {
        Index = (numLEDs - 1) - Index;
    }

    setCompensatedInterval(aIntervalMillis);

    TotalStepCounter += 1; // + 1 for last pattern

    ScannerExtendedUpdate(false);
    showPatternInitially();
// must be after showPatternInitially(), since it requires the old value do detect asynchronous calling
    ActivePattern = PATTERN_SCANNER_EXTENDED;
#if defined(LOCAL_TRACE)
    printInfo(&Serial, true);
#endif
}

/*
 * @return - true if pattern has ended, false if pattern has NOT ended
 */
bool NeoPatterns::ScannerExtendedUpdate(bool aDoUpdate) {
    if (aDoUpdate) {
        if (decrementTotalStepCounterAndSetNextIndex()) {
            return true;
        }
        /*
         * check for bouncing condition
         */
        uint16_t tNumberOfBouncings = LongValue1.NumberOfBouncings;
        if (tNumberOfBouncings > 0) {
            if (Direction == DIRECTION_UP) {
                // show pattern for index == numLEDs-1 still in old direction!
                if (Index == (int16_t)numLEDs) {
                    Index -= 2;
                    Direction = DIRECTION_DOWN;
                    tNumberOfBouncings--;
                }
                if (PatternFlags & FLAG_SCANNER_EXT_CYLON) {
                    if (Index + ByteValue1.PatternLength == (int16_t)numLEDs + 1) {
                        Index -= 2;
                        Direction = DIRECTION_DOWN;
                        tNumberOfBouncings--;
                    }
                }
            } else {
                // DIRECTION_DOWN here
                if (Index == -1) {
                    Index = 1;
                    Direction = DIRECTION_UP;
                    tNumberOfBouncings--;
                }
                if (PatternFlags & FLAG_SCANNER_EXT_CYLON) {
                    if (Index - ByteValue1.PatternLength == -2) {
                        Index += 2;
                        Direction = DIRECTION_UP;
                        tNumberOfBouncings--;
                    }
                }
            }
            LongValue1.NumberOfBouncings = tNumberOfBouncings;
        }
    }
    /*
     * Refresh pattern
     */

    /*
     * index is starting position of brightest led (middle of pattern if mode is cylon)
     */
    uint16_t tBrightnessHighResolution = 0xFFFF; // max. upper byte is the integer part used for setBrightness, lower byte is the fractional part
    uint16_t tBrightnessDelta = LongValue2.DeltaBrightnessShift8;

#if defined(LOCAL_TRACE)
        printPin(&Serial);
        Serial.print(F("ScannerExtendedUpdate: Index="));
        Serial.print(Index);
        Serial.print(F(" Direction="));
        Serial.print(Direction);
        Serial.print(F(" aDoUpdate="));
        Serial.print(aDoUpdate);
        Serial.println();
#endif

    uint8_t tPatternIndex;
    for (tPatternIndex = 0; tPatternIndex < ByteValue1.PatternLength; ++tPatternIndex) {
        uint8_t tBrightness = tBrightnessHighResolution >> 8;
        /*
         * compute new dimmed color value
         */
        color32_t tDimmedColor = dimColorWithGamma5(Color1, tBrightness);
        if (tDimmedColor == 0) {
            // break if color is black - this avoids long black tail if overwrite an existing pattern
            break;
        }

        int16_t tOffset;
        if (Direction == DIRECTION_UP) {
            tOffset = tPatternIndex;
        } else {
            tOffset = -tPatternIndex;
        }
        setPixelColor(Index - tOffset, tDimmedColor);

        if (PatternFlags & FLAG_SCANNER_EXT_START_AT_BOTH_ENDS) {
            // draw at other end too
            setPixelColor((numLEDs - 1) - (Index - tOffset), tDimmedColor);
        }
        if (PatternFlags & FLAG_SCANNER_EXT_CYLON) {
            // mirror pattern to + Offset
            setPixelColor(Index + tOffset, tDimmedColor);
            if (PatternFlags & FLAG_SCANNER_EXT_START_AT_BOTH_ENDS) {
                // draw at other end too
                setPixelColor((numLEDs - 1) - (Index + tOffset), tDimmedColor);
            }
        }

#if defined(LOCAL_TRACE)
        printPin(&Serial);
        Serial.print(F("Loop="));
        Serial.print(tPatternIndex);
        Serial.print(F(" Pixel="));
        Serial.print(int16_t(Index - tOffset));
        Serial.print(F(" Brightness="));
        Serial.print(tBrightness);
        Serial.print(F(" Dimmed color=0x"));
        Serial.print(tDimmedColor, HEX);
        Serial.print(F(" written=0x"));
        Serial.print(getPixelColor(Index - tOffset), HEX);
        Serial.println();
#endif
// compute next brightness
        tBrightnessHighResolution -= tBrightnessDelta;
    }

    if (aDoUpdate) {
#if defined(LOCAL_TRACE)
        Serial.println(F("clear tail"));
#endif
        /*
         * Clear tail. This needs only be done at update.
         * tPatternIndex is PatternLength except if dimmed color is black
         */
        if (Direction == DIRECTION_UP) {
            /*
             * DIRECTION_UP - Cleanup last tail pixel from old pattern
             */
            if (PatternFlags & FLAG_SCANNER_EXT_START_AT_BOTH_ENDS) {
                /*
                 * Check if both pattern are not at the middle of the strip i.e. they touch each other
                 * Since we have integer comparison we must use (numLEDs - 1) / 2) as limit to be valid for odd numLEDs
                 */
                if ((Index - (tPatternIndex / 2) <= (int16_t)((numLEDs - 1) / 2)) || (Index - tPatternIndex) >= (int16_t)(numLEDs / 2)) {
                    setPixelColor((numLEDs - 1) - (Index - tPatternIndex), COLOR32_BLACK);
                    setPixelColor(Index - tPatternIndex, COLOR32_BLACK);
                }
            } else {
                setPixelColor(Index - tPatternIndex, COLOR32_BLACK);
            }

        } else {
            /*
             * DIRECTION_DOWN - Cleanup last tail pixel from old pattern
             */
            if (PatternFlags & FLAG_SCANNER_EXT_START_AT_BOTH_ENDS) {
                // Since we have integer comparison we must use (numLEDs - 1) / 2) as limit to be valid for odd numLEDs
                if ((Index + (tPatternIndex / 2) >= (int16_t)(numLEDs / 2)) || (Index + tPatternIndex) <= (int16_t)((numLEDs - 1) / 2)) {
                    setPixelColor((numLEDs - 1) - (Index + tPatternIndex), COLOR32_BLACK);
                    setPixelColor(Index + tPatternIndex, COLOR32_BLACK);
                }
            } else {
                setPixelColor(Index + tPatternIndex, COLOR32_BLACK);
            }
        }
    } //  if (aDoUpdate)
    return false;
}
#endif

#if defined(ENABLE_PATTERN_STRIPES)
/**
 * StripesD Parameter is complete duration and not step interval
 * Start with aLength1 pixel of aLength1 followed by aLength2 pixel of aColor2 and then starting with aColor1 again until strip ends.
 * If DIRECTION_DOWN the same pattern starts from the other end.
 * If aNumberOfSteps == (aLength1 + aLength2) the last pattern is equal first pattern
 *
 * Idea. increment colors with color increment to have rainbow colors on the strip and optional change starting colors with another increment
 */
void NeoPatterns::StripesDuration(color32_t aColor1, uint8_t aLength1, color32_t aColor2, uint8_t aLength2, uint16_t aNumberOfSteps,
        uint16_t aCompleteDurationMillis, uint8_t aDirection) {
    Stripes(aColor1, aLength1, aColor2, aLength2, aNumberOfSteps, aCompleteDurationMillis, aDirection);
    setCompensatedInterval(aCompleteDurationMillis / TotalStepCounter);
}
void NeoPatterns::Stripes(color32_t aColor1, uint8_t aLength1, color32_t aColor2, uint8_t aLength2, uint16_t aNumberOfSteps,
        uint16_t aIntervalMillis, uint8_t aDirection) {
    Color1 = aColor1;
    ByteValue1.PatternLength = aLength1;
    ByteValue2.PatternLength = aLength2;
    LongValue1.Color2 = aColor2;
    setCompensatedInterval(aIntervalMillis);

    TotalStepCounter = aNumberOfSteps + 1;     // + 1 step for the last pattern to show
// The direction of index has the opposite direction of pattern
    Direction = OppositeDirection(aDirection & DIRECTION_UP_DOWN_MASK);
// start index (in pattern) running from 0 to (aLength1 + aLength2)
    if ((aDirection & DIRECTION_UP_DOWN_MASK) == DIRECTION_UP) {
        Index = 0;
    } else {
        uint16_t tNumPixels = numLEDs;
        // Subtract as many full patterns as possible
        tNumPixels = tNumPixels % (aLength1 + aLength2);

        if (aLength1 >= tNumPixels) {
            Index = aLength1 - tNumPixels;
        } else {
//            Index = (aLength1 - tNumPixels) + (aLength1 + aLength2);
            // (aLength1 - tNumPixels) is negative, so add length of pattern (aLength1 + aLength2)
            Index = ((aLength1 + (aLength1 + aLength2)) - tNumPixels);
        }
    }
#if defined(LOCAL_DEBUG)
    printPin(&Serial);
    Serial.print(F("Index="));
    Serial.print(Index);
    Serial.print(F(" Length1="));
    Serial.print(ByteValue1.PatternLength);
    Serial.print(F(" Length2="));
    Serial.print(ByteValue2.PatternLength);
    Serial.print(F(" Direction="));
    Serial.print(Direction);
    Serial.println();
#endif

    StripesUpdate(false);
    showPatternInitially();
// must be after showPatternInitially(), since it requires the old value do detect asynchronous calling
    ActivePattern = PATTERN_STRIPES;
#if defined(LOCAL_TRACE)
    printInfo(&Serial, true);
#endif
}

/*
 * @return - true if pattern has ended, false if pattern has NOT ended
 */
bool NeoPatterns::StripesUpdate(bool aDoUpdate) {
    if (aDoUpdate) {
        if (decrementTotalStepCounter()) {
            return true;
        }
        /*
         * Next index
         */
        if (Direction == DIRECTION_UP) {
            Index++;
            if (Index >= ByteValue1.PatternLength + ByteValue2.PatternLength) {
                Index = 0;
            }
        } else {
            Index--;
            // check for wrap around
            if (Index < 0) {
                Index = ByteValue1.PatternLength + ByteValue2.PatternLength - 1;
            }
        }
    }

    /*
     * Refresh pattern
     */
    uint8_t tRunningIndex = Index;
    for (uint_fast16_t i = 0; i < numLEDs; i++) {
        if (tRunningIndex < ByteValue1.PatternLength) {
            // first color
            setPixelColor(i, Color1);
        } else {
            // second color
            setPixelColor(i, LongValue1.Color2);
        }
        tRunningIndex++;
        // check for end of pattern
        if (tRunningIndex >= ByteValue1.PatternLength + ByteValue2.PatternLength) {
            tRunningIndex = 0;
        }
    }
    return false;
}
#endif

#if defined(ENABLE_PATTERN_FLASH)
/*
 * Start with aColor1 for aIntervalMillisColor1.
 * @param   doEndWithBlack if true aColor2 is replaced with black for last step of flash pattern
 */
void NeoPatterns::Flash(color32_t aColor1, uint16_t aIntervalMillisColor1, color32_t aColor2, uint16_t aIntervalMillisColor2,
        uint16_t aRepetitions, bool doEndWithBlack) {
    Color1 = aColor1;
    LongValue2.Intervals.Interval1 = aIntervalMillisColor1;
    LongValue1.Color2 = aColor2;
    LongValue2.Intervals.Interval2 = aIntervalMillisColor2;
    TotalStepCounter = aRepetitions * 2;
    Index = TotalStepCounter; // index is even
    if (doEndWithBlack) {
        PatternFlags = FLAG_END_WITH_BLACK;
    }

    FlashUpdate(false); // This sets Interval
    showPatternInitially();
// must be after showPatternInitially(), since it requires the old value do detect asynchronous calling
    ActivePattern = PATTERN_FLASH;
#if defined(LOCAL_TRACE)
    printInfo(&Serial, true);
#endif
}

/*
 * @return - true if pattern has ended, false if pattern has NOT ended
 */
bool NeoPatterns::FlashUpdate(bool aDoUpdate) {
    if (aDoUpdate) {
        if (decrementTotalStepCounter()) {
            return true;
        }
        Index--;
    }

    /*
     * Refresh pattern
     */
    if (Index & 0x01) {
        // second color
        if (Index == 1 && (PatternFlags & FLAG_END_WITH_BLACK)) {
            fillRegion(COLOR32_BLACK, 0, numLEDs);
        } else {
            fillRegion(LongValue1.Color2, 0, numLEDs);
        }
        setCompensatedInterval(LongValue2.Intervals.Interval2);
    } else {
        // first color
        fillRegion(Color1, 0, numLEDs);
        setCompensatedInterval(LongValue2.Intervals.Interval1);
    }
    return false;
}
#endif

#if defined(ENABLE_PATTERN_BOUNCING_BALL)
#include <math.h>

/*
 * Let a pixel of aColor move down and bounce up like a falling ball which bounces
 * Using the formula s = 0,5 a * t^2 => t = sqrt( 2*s / a).
 *
 * @param aIntervalMillis - interval for down motion from start pixel to next one, i.e. s=1.
 * @param aIndexOfTopPixel - Index of top pixel for first movement.
 * @param aPercentageOfLossAtBounce - loss of energy at bounce. if > 0 then the top pixel will not be reached at next iteration!
 * A ball needs for the first meter of free fall in real life 0,45 seconds using
 * And for 10 cm 0,143 seconds
 *
 * Requires up to 640 to 1140 bytes program memory, depending if floating point and sqrt() are already used otherwise.
 */
void NeoPatterns::BouncingBall(color32_t aColor, uint16_t aIndexOfTopPixel, uint16_t aIntervalMillis,
        int8_t aPercentageOfLossAtBounce, uint8_t aDirection) {
    Color1 = aColor;
    Direction = aDirection;
    LongValue1.StartIntervalMillis = aIntervalMillis; // Interval for first step - the t of the formula for s=1.
    LongValue2.TopPixelIndex = aIndexOfTopPixel;
    ByteValue1.IndexOfTopPixel = aIndexOfTopPixel;
    TotalStepCounter = 2;
    if (aDirection == DIRECTION_UP) {
        Index = 0;
        setCompensatedInterval((sqrt(aIndexOfTopPixel) - sqrt(aIndexOfTopPixel - 1)) * aIntervalMillis);
    } else {
        Index = aIndexOfTopPixel;
        setCompensatedInterval(aIntervalMillis);
    }
    PatternFlags = aPercentageOfLossAtBounce;
//    LongValue2.ColorTmp = millis(); // store start millis() to avoid accumulating errors for one move

    setPixelColor(Index, Color1); // start pattern
    showPatternInitially();
// must be after showPatternInitially(), since it requires the old value do detect asynchronous calling
    ActivePattern = PATTERN_BOUNCING_BALL;
}

/*
 * This implementation requires only the sqrt() function and no static float variables
 */
bool NeoPatterns::BouncingBallUpdate(bool aDoUpdate) {
    uint8_t tIndexToDraw = Index;
    if (aDoUpdate) {
        // clear old pixel
        setPixelColor(Index, COLOR32_BLACK);
        if (decrementTotalStepCounterAndSetNextIndex()) {
            return true;
        }
        tIndexToDraw = Index;
        /*
         * Index is the index of the current pixel to draw
         * Compute delay to NEXT index
         */
        TotalStepCounter = 2;
        float tDistanceToTopPixel = LongValue2.TopPixelIndex - Index; // from 0 to IndexOfTopPixel - the s of the formula
        float tSqrtOfDistanceToTopPixel = sqrt(tDistanceToTopPixel);
        float tDifferenceOfSquareRoots;
        if (Direction == DIRECTION_UP) {
            if (Index == ByteValue1.IndexOfTopPixel) {
                /*
                 * Reached top pixel -> change direction but use this pixel twice (by Index++).
                 * Compute time for vertex of parabola from the top integer index to the top float index and back.
                 * Use this as time until next draw of same top integer index (then we are at direction down)
                 */
                Direction = DIRECTION_DOWN;
                Index++;
                tDifferenceOfSquareRoots = 2 * sqrt(LongValue2.TopPixelIndex - ByteValue1.IndexOfTopPixel);
            } else {
                // for the next step up we decelerate, e.g. we increase the time to next position
                tDifferenceOfSquareRoots = (tSqrtOfDistanceToTopPixel - sqrt(tDistanceToTopPixel - 1));
            }

        } else {
            /*
             * Down - for the next step down we accelerate, i.e. we decrease the time to next index
             */
            if (Index != 0) {
                tDifferenceOfSquareRoots = (sqrt(tDistanceToTopPixel + 1) - tSqrtOfDistanceToTopPixel);
            } else {
                // do not use lowest pixel twice
                Direction = DIRECTION_UP;
                // set value for the next up and down cycle
                /*
                 * Compute loss from length of pattern, not from index
                 */
                LongValue2.TopPixelIndex = ((LongValue2.TopPixelIndex + 1.0) * (100 - PatternFlags) /*aPercentageOfLossAtBounce*/
                / 100.0) - 1.0;
                ByteValue1.IndexOfTopPixel = LongValue2.TopPixelIndex; // integer value of TopPixelIndex
                if (ByteValue1.IndexOfTopPixel < 1) {
                    TotalStepCounter = 1; // last step
                }
                tDifferenceOfSquareRoots = (sqrt(LongValue2.TopPixelIndex) - sqrt(LongValue2.TopPixelIndex - 1));
            }
        }
        setCompensatedInterval(tDifferenceOfSquareRoots * LongValue1.StartIntervalMillis);
    }
    /*
     * Refresh pattern
     */
    setPixelColor(tIndexToDraw, Color1);
#if defined(LOCAL_TRACE)
    printPin(&Serial);
    Serial.print(F("Ball: Index="));
    Serial.print(Index);
    Serial.print(F(" tIndexToDraw="));
    Serial.print(tIndexToDraw);
    Serial.print(F(" TopPixelIndex="));
    Serial.print(ByteValue1.IndexOfTopPixel);
    Serial.print(F(" IntervalToNext="));
    Serial.print(Interval);
    Serial.println();
#endif
    return false;
}
#endif // defined(ENABLE_PATTERN_BOUNCING_BALL)

#if defined(ENABLE_PATTERN_FIRE)
/********************************************************
 * The original Fire code is from: Fire2012 by Mark Kriegsman, July 2012
 * https://github.com/FastLED/FastLED/tree/master/examples/Fire2012
 * Adapted to NeoPatterns Class
 ********************************************************/

// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 40, suggested range 30-60
#define COOLING  40

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 60-200.
#define SPARKING 120

// initialize for fire -> set all to zero
/*
 * @param aDirection if not DIRECTION_UP show pattern bottom up
 */
void NeoPatterns::Fire(uint16_t aNumberOfSteps, uint16_t aIntervalMillis, uint8_t aDirection) {
    Interval = aIntervalMillis;
    Direction = aDirection;
    Index = 0;
    TotalStepCounter = aNumberOfSteps + 1;  // + 1 step for the last pattern to show
    clear();

    LongValue1.PixelHeatArrayPtr = (uint8_t*) calloc(numLEDs, 1);

    FireUpdate(false);
    showPatternInitially();
// must be after showPatternInitially(), since it requires the old value do detect asynchronous calling
    ActivePattern = PATTERN_FIRE;
#if defined(LOCAL_TRACE)
    printInfo(&Serial, true);
#endif
}

// Update the Fire Pattern
bool NeoPatterns::FireUpdate(bool aDoUpdate) {

// map heap space to heat array
    uint8_t *heat = LongValue1.PixelHeatArrayPtr;

    if (aDoUpdate) {
        if (TotalStepCounter == 1) {
            // we must free the memory before decrementTotalStepCounter(), because the pointer may be overwritten by the next pattern
            if (LongValue1.PixelHeatArrayPtr != nullptr) {
                free(LongValue1.PixelHeatArrayPtr);
            }
        }
        if (decrementTotalStepCounter()) {
            return true;
        }

        /*
         * Next index
         */
// Step 1.  Cool down every cell a little
        for (uint_fast16_t i = 0; i < numLEDs; i++) {
            uint8_t tChill = random8(((COOLING * 20) / numLEDs) + 2);
            if (tChill >= heat[i]) {
                heat[i] = 0;
            } else {
                heat[i] = heat[i] - tChill;
            }
        }

// Step 2.  Heat from each cell drifts 'up' and diffuses a little
        for (uint_fast16_t k = numLEDs - 1; k >= 2; k--) {
            heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
        }
        LongUnion tRandom; // usage of Long union saves 20 bytes and is way faster
        tRandom.Long = random();

// Step 3.  Randomly ignite one new 'spark' of heat near the bottom
        if (tRandom.UBytes[0] < SPARKING) {
            uint8_t y = tRandom.UBytes[1] % (numLEDs / 4);
            uint8_t tNewHeat = ((tRandom.UBytes[2] * (256 - 160)) >> 8) + 160; // random(160, 256); fastest version
//            uint8_t tNewHeat = (tRandom.UBytes[2] % ((uint8_t)(255-160))) + 160; // random(160, 255); uint8_t is required here :-(
//            uint8_t tNewHeat = (tRandom.UBytes[2] % 95) + 160; // random(160, 255);
            if (heat[y] + tNewHeat < heat[y]) {
                heat[y] = UINT8_MAX;
            } else {
                heat[y] += tNewHeat;
            }
        }
    }
    /*
     * Refresh pattern
     */
// Step 4.  Map from heat cells to LED colors
    for (uint_fast16_t j = 0; j < numLEDs; j++) {
        if (Direction == DIRECTION_UP) {
            setPixelColor(j, HeatColor(heat[j]));
        } else {
            setPixelColor((numLEDs - 1) - j, HeatColor(heat[j]));
        }
    }
    return false;
}
#endif // #if defined(ENABLE_PATTERN_FIRE)

uint32_t NeoPatterns::HeatColorSimple(uint8_t aTemperature) {
    uint8_t tGreen = 0;
    if (aTemperature >= 0x80) {
        tGreen = gamma8((aTemperature & ~0x80) << 1); // Mask upper bit (or - 128 without overflow)
    }
    uint8_t tBlue = 0;
    if (aTemperature >= 0xE0) {
        tBlue = gamma8((aTemperature & ~0xE0) << 3); // Mask upper 2 bits (or - 192 without overflow)
    }

    return Color(gamma8(aTemperature), tGreen, tBlue);
}

uint32_t NeoPatterns::HeatColor(uint8_t aTemperature) {

// Scale 'heat' down from 0-255 to 0-191 (0xBF),
// which can then be easily divided into three
// equal 'thirds' of 64 units each.
    uint8_t tTemperature0To191 = (aTemperature == 0) ? 0 : ((aTemperature * (uint16_t) (192)) >> 8);

// calculate a value that ramps up from
// zero to 255 in each 'third' of the scale.
    uint8_t heatramp = tTemperature0To191 & 0x3F;  // 0..63
    heatramp <<= 2;  // scale up to 0..252 (0xFC)
    heatramp = gamma8(heatramp);

// now figure out which third of the spectrum we're in:
    if (tTemperature0To191 & 0x80) {
// we're in the hottest third, ramp from yellow to white
//        return Color(255, 255, heatramp);
        return Color(255, 190, heatramp);
    } else if (tTemperature0To191 & 0x40) {
// we're in the middle third, ramp from red to yellow
//        return Color(255, heatramp, 0);
        return Color(255, heatramp - heatramp / 4, 0);
    } else {
// we're in the coolest third, ramp from black to red
        return Color(heatramp, 0, 0);
    }
}

/*
 * Same as HeatColor, but using a smaller / coarser gamma table of 32
 * In the original HeatColor() function we use only 64 entries of the gamma table because of heatramp <<= 2
 */
uint32_t NeoPatterns::HeatColorGamma5(uint8_t aTemperature) {

// Scale 'heat' down from 0-255 to 0-191 (0xBF),
// which can then be easily divided into three
// equal 'thirds' of 64 units each.
    uint8_t tTemperature0To191 = (aTemperature == 0) ? 0 : ((aTemperature * (uint16_t) (192)) >> 8);

// calculate a value that ramps up from
// zero to 255 in each 'third' of the scale.
    uint8_t heatramp = tTemperature0To191 & 0x3F;  // 0..63
//    heatramp <<= 2;  // scale up to 0..252 (0xFC)
//    heatramp = gamma5(heatramp);
    heatramp = pgm_read_byte(&GammaTable32[(heatramp / 2)]); // replacement for << 2 and gamma5()

// now figure out which third of the spectrum we're in:
    if (tTemperature0To191 & 0x80) {
// we're in the hottest third, ramp from yellow to white
        return Color(255, 255, heatramp);
    } else if (tTemperature0To191 & 0x40) {
// we're in the middle third, ramp from red to yellow
        return Color(255, heatramp, 0);
    } else {
// we're in the coolest third, ramp from black to red
        return Color(heatramp, 0, 0);
    }
}

/********************************************************
 * End of code from: Fire2012 by Mark Kriegsman, July 2012
 ********************************************************/
/*
 * Convert an uint8_t array of numLEDs heat entries to color values and store them in the NeoPattern object
 */
void NeoPatterns::convertHeatToColorSimple() {
    uint8_t *tHeatGraphPtr = LongValue1.PixelHeatArrayPtr;

    for (uint_fast8_t i = 0; i < numLEDs; i++) {
        // version with brightness
        Adafruit_NeoPixel::setPixelColor(i, NeoPatterns::HeatColorSimple(*tHeatGraphPtr++));
    }
}

void NeoPatterns::convertHeatToColor() {
    uint8_t *tHeatGraphPtr = LongValue1.PixelHeatArrayPtr;

    for (uint_fast8_t i = 0; i < numLEDs; i++) {
        // version with brightness
        Adafruit_NeoPixel::setPixelColor(i, NeoPatterns::HeatColor(*tHeatGraphPtr++));
    }
}
/*
 * Display all heat values from 1 to 0xFF on a strip
 */
void NeoPatterns::showAllHeatSimple() {
    for (uint_fast8_t i = 0; i < numLEDs; i++) {
        uint8_t tHeatValue = (i + 1) * (0x100 / numLEDs);
//        Serial.println(tHeatValue);
        setPixelColor(i, NeoPatterns::HeatColorSimple(tHeatValue));
    }
    show();
}

void NeoPatterns::showAllHeat() {
    for (uint_fast8_t i = 0; i < numLEDs; i++) {
        uint8_t tHeatValue = (i + 1) * (0x100 / numLEDs);
//        Serial.println(tHeatValue);
        setPixelColor(i, NeoPatterns::HeatColor(tHeatValue));
    }
    show();
}

/*
 * Print all numLEDs heat values contained in the PixelHeatArrayPtr
 * Used for debugging
 */
void NeoPatterns::printHeat(Print *aSerial) {
    uint8_t *tHeatArrayPtr = LongValue1.PixelHeatArrayPtr;

    for (uint_fast8_t i = 0; i < numLEDs; i++) {
        aSerial->print(*tHeatArrayPtr);
        aSerial->print(',');
        tHeatArrayPtr++;
    }
    aSerial->println();
}

/*
 * Print all numLEDs heat values contained in a PixelHeatArrayPtr
 * Used for debugging
 */
void NeoPatterns::printHeat(Print *aSerial, uint8_t *aHeatArrayPtr) {
    for (uint_fast8_t i = 0; i < numLEDs; i++) {
        aSerial->print(*aHeatArrayPtr);
        aSerial->print(',');
        aHeatArrayPtr++;
    }
    aSerial->println();
}

// Initialize for a delay -> just keep the old pattern displayed
void NeoPatterns::Delay(uint16_t aMillis) {
    ActivePattern = PATTERN_DELAY;
    setCompensatedInterval(aMillis);
    lastUpdate = millis(); // to schedule the end of the delay
    TotalStepCounter = 1;
}

// Update / End the Delay pattern
bool NeoPatterns::DelayUpdate(bool aDoUpdate) {
    if (aDoUpdate) {
        decrementTotalStepCounter();
    }
    return true; // do not call show() anyway
}

#if defined(ENABLE_PATTERN_PROCESS_SELECTIVE)
/*
 * call provided processing routine only for pixel which have color equal to ColorForSelection
 * the last resulting color is found in BackgroundColor
 */
void NeoPatterns::ProcessSelectiveColor(color32_t aColorForSelection, color32_t (*aSingleLEDProcessingFunction)(NeoPatterns*),
        uint16_t aNumberOfSteps, uint16_t aIntervalMillis) {
    Interval = aIntervalMillis;
    TotalStepCounter = aNumberOfSteps;
    Color1 = aColorForSelection;
    Index = 0; // for FadeColor
// initialize temporary color
    LongValue2.ColorTmp = aColorForSelection;
    Pointer1.SingleLEDProcessingFunction = aSingleLEDProcessingFunction;

// call this direct, since it is called only at update
    ProcessSelectiveColorForAllPixel();
    showPatternInitially();
// must be after showPatternInitially(), since it requires the old value do detect asynchronous calling
    ActivePattern = PATTERN_PROCESS_SELECTIVE;
#if defined(LOCAL_TRACE)
    printInfo(&Serial, true);
#endif
}

bool NeoPatterns::ProcessSelectiveColorUpdate(bool aDoUpdate) {
    if (aDoUpdate) {
        if (decrementTotalStepCounter()) {
            // store last color
            LongValue1.Color2 = LongValue2.ColorTmp;
            return true;
        }
        ProcessSelectiveColorForAllPixel();
    }
// no pattern refresh possible
    return false;
}
#endif

/*
 * Process only pixels with ColorForSelection
 */
void NeoPatterns::ProcessSelectiveColorForAllPixel() {

    color32_t tNewColor = Pointer1.SingleLEDProcessingFunction(this);
    for (uint_fast16_t i = 0; i < numLEDs; i++) {
        color32_t tOldColor = getPixelColor(i);
        if (tOldColor == LongValue2.ColorTmp) {
            setPixelColor(i, tNewColor);
        }
    }
    LongValue2.ColorTmp = tNewColor;
}

/***********************************************************
 * Sample processing functions for ProcessSelectiveColor()
 ***********************************************************/
color32_t FadeColor(NeoPatterns *aNeoPatternsPtr) {
    aNeoPatternsPtr->Index++;
    uint16_t tIndex = aNeoPatternsPtr->Index;
    uint16_t tTotalSteps = aNeoPatternsPtr->TotalStepCounter;
    color32_t tColor1 = aNeoPatternsPtr->Color1;
    color32_t tColor2 = aNeoPatternsPtr->LongValue1.Color2;
    uint8_t tRed = ((getRedPart(tColor1) * (tTotalSteps - tIndex)) + (getRedPart(tColor2) * tIndex)) / tTotalSteps;
    uint8_t tGreen = ((getGreenPart(tColor1) * (tTotalSteps - tIndex)) + (getGreenPart(tColor2) * tIndex)) / tTotalSteps;
    uint8_t tBlue = ((getBluePart(tColor1) * (tTotalSteps - tIndex)) + (getBluePart(tColor2) * tIndex)) / tTotalSteps;
#if defined(_SUPPORT_RGBW)
    uint8_t tWhite = ((getWhitePart(tColor1) * (tTotalSteps - tIndex)) + (getWhitePart(tColor2) * tIndex)) / tTotalSteps;
    return Adafruit_NeoPixel::Color(tRed, tGreen, tBlue, tWhite);
#else
    return Adafruit_NeoPixel::Color(tRed, tGreen, tBlue);
#endif
}

/*
 * works on LongValue2.ColorTmp
 */
color32_t DimColor(NeoPatterns *aNeoPatternsPtr) {
    color32_t tColor = aNeoPatternsPtr->LongValue2.ColorTmp;
    uint8_t tRed = getRedPart(tColor) >> 1;
    uint8_t tGreen = getGreenPart(tColor) >> 1;
    uint8_t tBlue = getBluePart(tColor) >> 1;
#if defined(_SUPPORT_RGBW)
    uint8_t tWhite = getWhitePart(tColor) >> 1;
    return Adafruit_NeoPixel::Color(tRed, tGreen, tBlue, tWhite);
#else
// call to function saves 6 byte program memory
    return Adafruit_NeoPixel::Color(tRed, tGreen, tBlue);
#endif

//    return COLOR32(red, green, blue);
}

/*
 * works on LongValue2.ColorTmp
 */
color32_t BrightenColor(NeoPatterns *aNeoPatternsPtr) {
    color32_t tColor = aNeoPatternsPtr->LongValue2.ColorTmp;
    uint8_t tRed = getRedPart(tColor) << 1;
    uint8_t tGreen = getGreenPart(tColor) << 1;
    uint8_t tBlue = getBluePart(tColor) << 1;
#if defined(_SUPPORT_RGBW)
    uint8_t tWhite = getWhitePart(tColor) << 1;
    return Adafruit_NeoPixel::Color(tRed, tGreen, tBlue, tWhite);
#else
// call to function saves 22 byte program memory
    return Adafruit_NeoPixel::Color(tRed, tGreen, tBlue); //    return COLOR32(red, green, blue);
#endif
}

#if defined(__AVR__)
/*
 * Not required for non AVR platforms, it is then just PatternNamesArray[aPatternNumber]
 */
void NeoPatterns::getPatternName(uint8_t aPatternNumber, char *aBuffer, uint8_t aBuffersize) {
    const char *aNameArrayPointerPGM = (char*) pgm_read_word(&PatternNamesArray[aPatternNumber]);
    char tPGMChar;
    do {
        tPGMChar = pgm_read_byte(aNameArrayPointerPGM++);
        *aBuffer++ = tPGMChar;
        aBuffersize--;
    } while (tPGMChar != '\0' && aBuffersize > 1);
// Guarantee, that last element is \0
    *aBuffer = '\0';
}
#endif

/*
 * Prints name of the pattern e.g. "Rainbow cycle"
 * call it e.g. printPatternName(&Serial);
 */
void NeoPatterns::printPatternName(uint8_t aPatternNumber, Print *aSerial) {
#if defined(__AVR__)
    const char *aNameArrayPointerPGM = (char*) pgm_read_word(&PatternNamesArray[aPatternNumber]);
#if defined(LOCAL_TRACE)
    Serial.print(F("aNameArrayPointerPGM=0x"));
    Serial.println((unsigned int) aNameArrayPointerPGM, HEX);
#endif
    aSerial->print((const __FlashStringHelper*) aNameArrayPointerPGM);
#else
    aSerial->print(PatternNamesArray[aPatternNumber]);
#endif
}

/*
 * For debugging purposes
 */
void NeoPatterns::printPattern() {
    Serial.print(F("Pattern="));
    printPatternName(ActivePattern, &Serial);
    Serial.print(' ');
}

void NeoPatterns::printlnPattern() {
    Serial.print(F("Pattern="));
    printPatternName(ActivePattern, &Serial);
    Serial.println();
}

void NeoPatterns::printInfo(Print *aSerial, bool aFullInfo) {
    static int16_t sLastTotalStepCounter;

    NeoPixel::printInfo(aSerial);

    /*
     * only print if TotalStepCounter changed, or full info
     */
    if (TotalStepCounter != sLastTotalStepCounter || aFullInfo) {
        sLastTotalStepCounter = TotalStepCounter;

        aSerial->print(F(" TotalSteps="));
        aSerial->print(TotalStepCounter);
        aSerial->print(F(" Index="));
        aSerial->print((int16_t) Index);
        aSerial->print(F(" Direction="));
        aSerial->print(Direction);
        aSerial->print(F(" Repetitions="));
        aSerial->print(Repetitions);
        aSerial->print(F(" lastUpdate="));
        aSerial->print(lastUpdate);
        aSerial->print(F(" ms brightness="));
        aSerial->println(uint8_t(Brightness));
    }
    if (aFullInfo) {
        aSerial->print(F("ActivePattern="));
        printPatternName(ActivePattern, aSerial);
        aSerial->print('|');
        aSerial->print(ActivePattern);
        aSerial->print(F(" CompensatedInterval="));
        aSerial->print(Interval);
        aSerial->print(F(" Color1=0x"));
        aSerial->print(Color1, HEX);
        aSerial->print(F(" LongValue1=0x"));
        aSerial->print(LongValue1.LongValue, HEX);
        aSerial->print('|');
        aSerial->print(LongValue1.LongValue);
        aSerial->print(F(" LongValue2=0x"));
        aSerial->print(LongValue2.LongValue2, HEX);
        aSerial->print('|');
        aSerial->print(LongValue2.LongValue2);
        aSerial->print(F(" PatternFlags=0x"));
        aSerial->print(PatternFlags, HEX);
        aSerial->print(F(" &NeoPatterns=0x"));
        aSerial->println((uintptr_t) this, HEX);
    }
}

/********************************************
 * Code for user provided pattern extensions
 ********************************************/

#if defined(ENABLE_PATTERN_USER_PATTERN1)
extern bool UserPattern1Update(NeoPatterns *aNeoPatterns, bool aDoUpdate); // provided by main / user program
bool NeoPatterns::Pattern1Update(bool aDoUpdate) {
    return UserPattern1Update(this, aDoUpdate);
}
#endif

#if defined(ENABLE_PATTERN_USER_PATTERN2)
extern bool UserPattern2Update(NeoPatterns *aNeoPatterns, bool aDoUpdate); // provided by main / user program
bool NeoPatterns::Pattern2Update(bool aDoUpdate) {
    return UserPattern2Update(this, aDoUpdate);
}
#endif

/*****************************************************************
 * COMBINED PATTERN EXAMPLE
 * overwrites the OnComplete Handler pointer and sets it
 * to aNextOnComplete after completion of combined patterns
 * @param aScannerIntervalMillis The delay between two stars is 2 * ScannerIntervalMillis
 * if ENDLESS_HANDLER_POINTER is used then run falling star forever with random delay between 10 and 1000 times the aScannerIntervalMillis
 *****************************************************************/
#if defined(ENABLE_PATTERN_SCANNER_EXTENDED)
// initialize for falling star (scanner with delay after pattern)
void initMultipleFallingStars(NeoPatterns *aLedsPtr, color32_t aColor, uint8_t aLength, uint8_t aScannerIntervalMillis,
        uint8_t aRepetitions, void (*aNextOnCompleteHandler)(NeoPatterns*), uint8_t aDirection) {

    aLedsPtr->ByteValue2.ScannerIntervalMillis = aScannerIntervalMillis;
    aLedsPtr->Repetitions = (aRepetitions * 2) - 1; // get an odd number
    aLedsPtr->OnPatternComplete = &multipleFallingStarsCompleteHandler;
    aLedsPtr->NextOnPatternCompleteHandler = aNextOnCompleteHandler;
    /*
     * Start with one scanner
     */
    aLedsPtr->ScannerExtended(aColor, aLength, aScannerIntervalMillis, 0, FLAG_SCANNER_EXT_VANISH_COMPLETE, aDirection);
#if defined(LOCAL_DEBUG)
    aLedsPtr->printPin(&Serial);
    aLedsPtr->printPattern();
    Serial.print(F("Repetitions="));
    Serial.println(aRepetitions);
#endif
}

/*
 * Start delay pattern and then a new falling star
 * if all falling stars are completed switch back to NextOnComplete
 * if ENDLESS_HANDLER_POINTER is used then run falling star forever with random delay between 10 and 1000 times the aScannerIntervalMillis
 */
void multipleFallingStarsCompleteHandler(NeoPatterns *aLedsPtr) {
    uint8_t tScannerIntervalMillis = aLedsPtr->ByteValue2.ScannerIntervalMillis;
    uint16_t tRepetitions = aLedsPtr->Repetitions;
#if defined(LOCAL_TRACE)
    Serial.print(F("Repetitions="));
    Serial.println(tRepetitions);
#endif
    if (tRepetitions == 1) {
// perform delay and then switch back to NextOnComplete
        if (aLedsPtr->NextOnPatternCompleteHandler == ENDLESS_HANDLER_POINTER) {
            // do it forever but with random delay
            aLedsPtr->Repetitions = 2; // set to even;
            aLedsPtr->Delay(random(tScannerIntervalMillis * 10, tScannerIntervalMillis * 1000));
        } else {
            aLedsPtr->OnPatternComplete = aLedsPtr->NextOnPatternCompleteHandler;
            aLedsPtr->Delay(tScannerIntervalMillis * 2); // after delay NextOnPatternCompleteHandler is called
        }
    } else {
        /*
         * Next falling star
         */
        tRepetitions = tRepetitions & 0x01; // = tRepetitions % 2;
        if (tRepetitions == 1) {
            // for odd Repetitions 3,5,7, etc. -> do small delay
            aLedsPtr->Delay(tScannerIntervalMillis * 2);
        } else {
            // for even Repetitions 2,4,6, etc. -> do scanner
            aLedsPtr->ScannerExtended(aLedsPtr->Color1, aLedsPtr->ByteValue1.PatternLength, tScannerIntervalMillis, 0,
            FLAG_SCANNER_EXT_VANISH_COMPLETE | FLAG_DO_NOT_CLEAR, aLedsPtr->Direction);
        }
        aLedsPtr->Repetitions--;
    }
}
#endif

const char* DirectionToString(uint8_t aDirection) {
    switch (aDirection) {
    case DIRECTION_UP:
        return DirectionUp;
        break;
    case DIRECTION_LEFT:
        return DirectionLeft;
        break;
    case DIRECTION_DOWN:
        return DirectionDown;
        break;
    case DIRECTION_RIGHT:
        return DirectionRight;
        break;
    default:
        return DirectionNo;
        break;
    }
}

/*
 * Sample callback handler for random pattern
 */
#if defined(ENABLE_PATTERN_SCANNER_EXTENDED) && defined(ENABLE_PATTERN_RAINBOW_CYCLE) && defined(ENABLE_PATTERN_STRIPES) \
    && defined(ENABLE_PATTERN_FADE) && defined(ENABLE_PATTERN_COLOR_WIPE) && defined(ENABLE_PATTERN_HEARTBEAT)
void allPatternsRandomHandler(NeoPatterns *aLedsPtr) {
    LongUnion tRandom; // usage of Long union saves 4 bytes and is way faster
    tRandom.Long = random();

    uint8_t tDuration = ((tRandom.UBytes[2] * (81 - 40)) >> 8) + 40; // = random(40, 81); fastest version
//    uint8_t tDuration = (tRandom.UBytes[2] % 41) + 40;  // = random(40, 81);
    uint8_t tColor = tRandom.UBytes[1];                 // = random(256)

    switch ((tRandom.UBytes[0] * 15) >> 8) { // = random(15)
    case 0:
        // Cylon 3 times bouncing
        aLedsPtr->ScannerExtended(NeoPatterns::Wheel(tColor), 5, tDuration, 3,
        FLAG_SCANNER_EXT_CYLON | (tDuration & FLAG_SCANNER_EXT_VANISH_COMPLETE), (tColor & DIRECTION_DOWN));
        break;
    case 1:
        // rocket 2 times bouncing
        aLedsPtr->ScannerExtended(NeoPatterns::Wheel(tColor), 7, tDuration, 2,
        FLAG_SCANNER_EXT_ROCKET | FLAG_SCANNER_EXT_VANISH_COMPLETE | FLAG_DO_NOT_CLEAR, (tColor & DIRECTION_DOWN));
        break;
    case 2:
        // 1 times rocket or falling star
        aLedsPtr->ScannerExtended(COLOR32_WHITE_HALF, 7, tDuration / 2, 0, FLAG_SCANNER_EXT_VANISH_COMPLETE,
                (tColor & DIRECTION_DOWN));
        break;
    case 3:
        // Rainbow cycle
        aLedsPtr->RainbowCycle(tDuration / 4, (tDuration & DIRECTION_DOWN));
        break;
    case 4:
        // new Stripes
        aLedsPtr->Stripes(NeoPatterns::Wheel(tColor), 5, NeoPatterns::Wheel(tColor + 0x80), 3, 2 * aLedsPtr->numPixels(),
                tDuration * 2, (tColor & DIRECTION_DOWN));
        break;
    case 5:
        // old TheaterChase
        aLedsPtr->Stripes(NeoPatterns::Wheel(tColor), 1, NeoPatterns::Wheel(tColor + 0x80), 2, 2 * aLedsPtr->numPixels(),
                tDuration * 2, (tColor & DIRECTION_DOWN));
        break;
    case 6:
        // Fade to complement
        aLedsPtr->Fade(NeoPatterns::Wheel(tColor), NeoPatterns::Wheel(tColor + 0x80), 64, tDuration);
        break;
    case 7:
        // Color wipe DO_NOT_CLEAR
        aLedsPtr->ColorWipe(NeoPatterns::Wheel(tColor), tDuration, FLAG_DO_NOT_CLEAR, (tColor & DIRECTION_DOWN));
        break;
    case 8:
        // rocket start at both end
        aLedsPtr->ScannerExtended(NeoPatterns::Wheel(tColor), 7, tDuration / 2, 3,
        FLAG_SCANNER_EXT_ROCKET | (tDuration & FLAG_SCANNER_EXT_VANISH_COMPLETE) | FLAG_SCANNER_EXT_START_AT_BOTH_ENDS);
        break;
    case 9:
        // 3 Heartbeats
        aLedsPtr->Heartbeat(NeoPatterns::Wheel(tColor), tDuration / 2, 2);
        break;
    case 10:
        // Multiple falling star
        initMultipleFallingStars(aLedsPtr, COLOR32_WHITE_HALF, 7, tDuration, 3, &allPatternsRandomHandler);
        break;
    case 11:
#if defined(ENABLE_PATTERN_FIRE)
        if ((aLedsPtr->PixelFlags & PIXEL_FLAG_GEOMETRY_CIRCLE) == 0) {
            //Fire
            aLedsPtr->Fire(tDuration * 2, tDuration / 2);
        } else {
            // rocket start at both end
            aLedsPtr->ScannerExtended(NeoPatterns::Wheel(tColor), 5, tDuration, 0,
            FLAG_SCANNER_EXT_START_AT_BOTH_ENDS | FLAG_SCANNER_EXT_VANISH_COMPLETE | FLAG_DO_NOT_CLEAR);
        }
#else
        aLedsPtr->Delay(100);
#endif
        break;
    case 12:
#if defined(ENABLE_PATTERN_BOUNCING_BALL)
        // BouncingBall only for bars
        if ((aLedsPtr->PixelFlags & PIXEL_FLAG_GEOMETRY_CIRCLE) == 0) {
            aLedsPtr->BouncingBall(NeoPatterns::Wheel(tColor), aLedsPtr->numPixels() - 1);
        } else {
            // Fade to complement
            aLedsPtr->Fade(NeoPatterns::Wheel(tColor), NeoPatterns::Wheel(tColor + 0x80), 64, tDuration);
        }
#else
        aLedsPtr->Delay(100);
#endif
        break;
    case 13:
        // 5 Flashes
        aLedsPtr->Flash(NeoPatterns::Wheel(tColor),tDuration, COLOR32_BLACK, tDuration * 2, 5, true);
        break;
    case 14:
        // Twinkle random colors
        aLedsPtr->clear();
        aLedsPtr->Twinkle(COLOR32_SPECIAL, aLedsPtr->getNumberOfPixels() / 4, tDuration, 60);
        break;
    default:
        break;
    }

#if defined(LOCAL_INFO)
    Serial.print(F("Pin="));
    Serial.print(aLedsPtr->getPin());
    Serial.print(F(" Length="));
    Serial.print(aLedsPtr->numPixels());
    Serial.print(F(" ActivePattern="));
#if defined(LOCAL_DEBUG)
    aLedsPtr->printPatternName(aLedsPtr->ActivePattern, &Serial);
    Serial.print('|');
#endif
    Serial.print(aLedsPtr->ActivePattern);
    Serial.print(F(" State="));
    Serial.println((tRandom.UBytes[0] * 13) >> 8);
#endif
}
#endif

#include "LocalDebugLevelEnd.h"
#endif // _NEOPATTERNS_HPP
