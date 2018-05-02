/**
 * NeoPatterns.cpp
 *
 * This file includes the original code of Adafruit as well as code of Mark Kriegsman
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
#include <Arduino.h>

#include "NeoPatterns.h"

// used for Ticker
#include "fonts.h"

char VERSION_NEOPATTERNS[] = "1.1";

/**********************************************************************************
 * Code from https://learn.adafruit.com/multi-tasking-the-arduino-part-3?view=all
 * Extended for added functionality
 **********************************************************************************/
// Constructor - calls base-class constructor to initialize strip
NeoPatterns::NeoPatterns(uint16_t pixels, uint8_t aPin, uint8_t aTypeOfPixel, void (*aPatternCompletionCallback)(NeoPatterns*)) : // @suppress("Class members should be properly initialized")
        Adafruit_NeoPixel(pixels, aPin, aTypeOfPixel) {
    uint8_t twOffset = (aTypeOfPixel >> 6) & 0b11; // See notes in header file Adafruit_NeoPixel.h regarding R/G/B/W offsets
    uint8_t trOffset = (aTypeOfPixel >> 4) & 0b11;
    PixelColorStorageSize = ((twOffset == trOffset) ? 3 : 4);
    OnPatternComplete = aPatternCompletionCallback;
}

void NeoPatterns::setCallback(void (*callback)(NeoPatterns*)) {
    OnPatternComplete = callback;
}

// Update the pattern returns true if update has happened in order to give the caller a chance to manually change parameters (like color etc.)
bool NeoPatterns::Update() {
    if ((millis() - lastUpdate) > Interval) {

        switch (ActivePattern) {
        case RAINBOW_CYCLE:
            RainbowCycleUpdate();
            break;
        case THEATER_CHASE:
            TheaterChaseUpdate();
            break;
        case COLOR_WIPE:
            ColorWipeUpdate();
            break;
        case SCANNER:
            ScannerUpdate();
            break;
        case FADE:
            FadeUpdate();
            break;
        case FADE_SELECTIVE:
            FadeSelectiveUpdate();
            break;
        case PROCESS_SELECTIVE:
            ProcessSelectiveColorUpdate();
            break;
        case CYLON:
            CylonUpdate();
            break;
        case FIRE:
            FireUpdate();
            break;
        case DELAY:
            DelayUpdate();
            break;
        case PATTERN1:
            Pattern1Update();
            break;
        case PATTERN2:
            Pattern2Update();
            break;
        default:
            break;
        }
        // time to update
        lastUpdate = millis();
        return true;
    }
    return false;
}

// Increment the Index and reset at the end
void NeoPatterns::Increment() {
    if (Direction == DIRECTION_UP) {
        Index++;
        if (Index >= TotalSteps) {
            Index = 0;
            if (OnPatternComplete != NULL) {
                OnPatternComplete(this); // call the completion callback
            }
        }
    } else // Direction == DOWN
    {
        --Index;
        if (Index == 0xFFFF) {
            Index = TotalSteps - 1;
            if (OnPatternComplete != NULL) {
                OnPatternComplete(this); // call the completion callback
            }
        }
    }
}

// Reverse pattern direction
void NeoPatterns::Reverse() {
    if (Direction == DIRECTION_UP) {
        Direction = DIRECTION_DOWN;
        Index = TotalSteps - 1;
    } else {
        Direction = DIRECTION_UP;
        Index = 0;
    }
}

// Helper to set index accordingly to direction
void NeoPatterns::setDirectionAndTotalStepsAndIndex(uint8_t aDirection, uint16_t totalSteps) {
    Direction = aDirection;
    TotalSteps = totalSteps;
    if (Direction == DIRECTION_UP) {
        Index = 0;
    } else {
        Index = totalSteps - 1;
    }
}

// Initialize for a RainbowCycle
void NeoPatterns::RainbowCycle(uint8_t interval, uint8_t aDirection) {
    ActivePattern = RAINBOW_CYCLE;
    Interval = interval;
    setDirectionAndTotalStepsAndIndex(aDirection, 255);
}

// Update the Rainbow Cycle Pattern
void NeoPatterns::RainbowCycleUpdate() {
    for (uint16_t i = 0; i < numPixels(); i++) {
        setPixelColor(i, Wheel(((i * 256 / numPixels()) + Index) & 255));
    }
    show();
    Increment();
}

// Initialize for a Theater Chase
void NeoPatterns::TheaterChase(color32_t color1, color32_t color2, uint8_t interval, uint8_t aDirection) {
    ActivePattern = THEATER_CHASE;
    Interval = interval;
    Color1 = color1;
    Color2 = color2;
    setDirectionAndTotalStepsAndIndex(aDirection, numPixels());
}

// Update the Theater Chase Pattern
void NeoPatterns::TheaterChaseUpdate() {
    for (uint16_t i = 0; i < numPixels(); i++) {
        if ((i + Index) % 3 == 0) {
            setPixelColor(i, Color1);
        } else {
            setPixelColor(i, Color2);
        }
    }
    show();
    Increment();
}

// Initialize for a ColorWipe
void NeoPatterns::ColorWipe(color32_t color, uint8_t interval, uint8_t aDirection) {
    ActivePattern = COLOR_WIPE;
    Interval = interval;
    Color1 = color;
    setDirectionAndTotalStepsAndIndex(aDirection, numPixels());
}

// Update the Color Wipe Pattern
void NeoPatterns::ColorWipeUpdate() {
    setPixelColor(Index, Color1);
    show();
    Increment();
}

// Initialize for a Fade
void NeoPatterns::Fade(color32_t color1, color32_t color2, uint16_t steps, uint8_t interval, uint8_t aDirection) {
    ActivePattern = FADE;
    Interval = interval;
    TotalSteps = steps;
    Color1 = color1;
    Color2 = color2;
    Index = 0;
    Direction = aDirection;
}

// Update the Fade pattern
void NeoPatterns::FadeUpdate() {
// Calculate linear interpolation between Color1 and Color2
// Optimize order of operations to minimize truncation error
    uint8_t red = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
    uint8_t green = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
    uint8_t blue = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;

    ColorSet(Color(red, green, blue));
    show();
    Increment();
}

// Initialize for a SCANNNER
// extended with modes:
// mode 0 -> old scanner starting at 0 - 2 passes
// mode 1 -> old scanner but starting at numPixels() -1 - one pass (falling star pattern)
// mode +2 -> starting at both ends
// mode +4 -> let scanner vanish complete (>=7 additional steps at the end)
void NeoPatterns::Scanner(color32_t color1, uint8_t interval, uint8_t mode) {
    clear();
    ActivePattern = SCANNER;
    TotalSteps = numPixels() + 42; // not really needed - for increment() it must be greater than numPixels() +1
    Interval = interval;
    Color1 = color1;
    Index = 0;
    Direction = DIRECTION_UP;
    if (mode & FLAG_SCANNER_ONE_PASS) {
        // only one pass -> falling star pattern
        Index = numPixels() - 1;
        Direction = DIRECTION_DOWN;
    }
    Flags = mode;
}

// Update the Scanner Pattern
void NeoPatterns::ScannerUpdate() {
    for (uint16_t i = 0; i < numPixels(); i++) {
        if (i == Index
                || ((Flags & FLAG_SCANNER_STARTING_AT_BOTH_ENDS) && (i + numPixels() - 1) == ((numPixels() - 1) * 2) - Index)) {
            // Scan Pixel forth (and simultaneously back, if Special == 1)
            setPixelColor(i, Color1);
        } else { // Fading tail
            setPixelColor(i, DimColor(getPixelColor(i)));
        }
    }

    if (Index >= numPixels() - 1) {
        Direction = DIRECTION_DOWN;
    }
    show();

    if ((Index == 0 || Index == 0xFFFF) && (Flags & FLAG_SCANNER_VANISH_COMPLETE) && Direction == DIRECTION_DOWN) {
        // let tail vanish
        Index = 0xFFFF;
        if (getPixelColor(0) == 0) {
            if (OnPatternComplete != NULL) {
                OnPatternComplete(this); // call the completion callback
            }
        }
    } else {
        Increment();
    }
}

// Calculate 50% dimmed version of a color (used by ScannerUpdate)
uint32_t NeoPatterns::DimColor(color32_t color) {
// Shift R, G and B components one bit to the right
    uint32_t dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
    return dimColor;
}

// Set all pixels to a color (synchronously)
void NeoPatterns::ColorSet(color32_t color) {
    for (uint16_t i = 0; i < numPixels(); i++) {
        setPixelColor(i, color);
    }
    show();
}

// Input a value 0 to 255 to get a color value.
// The colors are a transition r - g - b - back to r.
uint32_t NeoPatterns::Wheel(byte WheelPos) {
    WheelPos = 255 - WheelPos;
    if (WheelPos < 85) {
        return Color(255 - WheelPos * 3, 0, WheelPos * 3);
    } else if (WheelPos < 170) {
        WheelPos -= 85;
        return Color(0, WheelPos * 3, 255 - WheelPos * 3);
    } else {
        WheelPos -= 170;
        return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    }
}
/****************************************************************************
 * START OF EXTENSIONS
 ****************************************************************************/

/********************************************************
 * The original Fire code is from: Fire2012 by Mark Kriegsman, July 2012
 * https://github.com/FastLED/FastLED/tree/master/examples/Fire2012
 * Adapted to NeoPatterns Class
 ********************************************************/

// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 55, suggested range 20-100
#define COOLING  20

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 120

// initialize for fire -> set all to zero
void NeoPatterns::Fire(uint16_t aIntervalMillis, uint16_t aRepetitions) {
    ActivePattern = FIRE;
    Interval = aIntervalMillis;
    Direction = DIRECTION_UP;
    Index = 0;
    Repetitions = aRepetitions;
    // reset colors
    ColorSet(0);
}

// Update the Fire Pattern
void NeoPatterns::FireUpdate() {
    static byte heat[24];

    // Step 1.  Cool down every cell a little
    for (uint16_t i = 0; i < numPixels(); i++) {
        uint8_t tChill = random(((COOLING * 10) / numPixels()) + 2);
        if (tChill >= heat[i]) {
            heat[i] = 0;
        } else {
            heat[i] = heat[i] - tChill;
        }
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for (uint16_t k = numPixels() - 1; k >= 2; k--) {
        heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
    }

    // Step 3.  Randomly ignite one new 'spark' of heat near the bottom
    if (random(255) < SPARKING) {
        int y = random(numPixels() / 4);
        uint8_t tNewHeat = random(160, 255);
        if (heat[y] + tNewHeat < heat[y]) {
            heat[y] = 0xFF;
        } else {
            heat[y] += tNewHeat;
        }
    }

    // Step 4.  Map from heat cells to LED colors
    for (uint16_t j = 0; j < numPixels(); j++) {
        setPixelColor(j, HeatColor(heat[j]));
    }

    show();
    Repetitions--;
    if (Repetitions == 0) {
        if (OnPatternComplete != NULL) {
            OnPatternComplete(this); // call the completion callback
        }
    }
}

uint32_t NeoPatterns::HeatColor(uint8_t aTemperature) {

// Scale 'heat' down from 0-255 to 0-191,
// which can then be easily divided into three
// equal 'thirds' of 64 units each.
    uint8_t t192 = (aTemperature == 0) ? 0 : (((int) aTemperature * (int) (192)) >> 8) + 1;

// calculate a value that ramps up from
// zero to 255 in each 'third' of the scale.
    uint8_t heatramp = t192 & 0x3F; // 0..63
    heatramp <<= 2; // scale up to 0..252

// now figure out which third of the spectrum we're in:
    if (t192 & 0x80) {
        // we're in the hottest third
        return Adafruit_NeoPixel::Color(255, 255, heatramp);
//        heatcolor.r = 255; // full red
//        heatcolor.g = 255; // full green
//        heatcolor.b = heatramp; // ramp up blue

    } else if (t192 & 0x40) {
        // we're in the middle third
        return Adafruit_NeoPixel::Color(255, heatramp, 0);
//        heatcolor.r = 255; // full red
//        heatcolor.g = heatramp; // ramp up green
//        heatcolor.b = 0; // no blue

    } else {
        // we're in the coolest third
        return Adafruit_NeoPixel::Color(heatramp, 0, 0);
//        heatcolor.r = heatramp; // ramp up red
//        heatcolor.g = 0; // no green
//        heatcolor.b = 0; // no blue
    }
}

/********************************************************
 * End of code from: Fire2012 by Mark Kriegsman, July 2012
 ********************************************************/

// Initialize for a delay -> just keep the old pattern displayed
void NeoPatterns::Delay(uint16_t aMillis) {
    ActivePattern = DELAY;
    Interval = aMillis;
//lastUpdate = millis();
    TotalSteps = 1;
    Index = 0;
    Direction = DIRECTION_UP;
}

// Update / End the Delay pattern
void NeoPatterns::DelayUpdate() {
    Increment();
}

// Initialize for a CYLON
void NeoPatterns::Cylon(color32_t color1, uint16_t interval, uint8_t repetitions) {
    clear();
    ActivePattern = CYLON;
    Interval = interval;
    Direction = DIRECTION_UP;
    Index = 0;
    Repetitions = repetitions;
    uint8_t tStartIndex = 4;

// Used as steps per direction
    TotalSteps = (numPixels() - 1) - (2 * tStartIndex);
    Color1 = color1;
    uint8_t i = tStartIndex - 1;
    uint8_t j = tStartIndex + 1;
    setPixelColor(i, color1);
    setPixelColor(tStartIndex, color1);
    setPixelColor(j, color1);
    i--;
    j++;
    color1 = DimColor(DimColor(color1));
    setPixelColor(i, color1);
    setPixelColor(j, color1);
    i--;
    j++;
    color1 = DimColor(DimColor(color1));
    setPixelColor(i, color1);
    setPixelColor(j, color1);
    i--;
    j++;
    color1 = DimColor(DimColor(color1));
    setPixelColor(i, color1);
    setPixelColor(j, color1);
    j++;
}

// Update the Cylon Pattern
void NeoPatterns::CylonUpdate() {
    if (Direction == DIRECTION_UP) {
        uint16_t i = numPixels() - 1;
        while (i > 0) {
            uint8_t t = i - 1;
            setPixelColor(i, getPixelColor(t));
            i = t;
        }
        setPixelColor(0, 0);
    } else {
        uint16_t i = 0;
        while (i < numPixels()) {
            uint8_t t = i + 1;
            setPixelColor(i, getPixelColor(t));
            i = t;
        }
        setPixelColor(i, 0);
    }
    show();
    Index++;
// toggle directions and handle repetitions
    if (Index >= TotalSteps) {
        Index = 0;
        if (Direction == DIRECTION_UP) {
            Direction = DIRECTION_DOWN;
        } else {
            Direction = DIRECTION_UP;
            Repetitions--;
            if (Repetitions == 0) {
                if (OnPatternComplete != NULL) {
                    OnPatternComplete(this); // call the completion callback
                }
            }
        }
    }
}

/*
 * call provided processing routine only for pixel which have color equal to ColorForSelection
 */
void NeoPatterns::ProcessSelectiveColor(uint32_t (*aSingleLEDProcessingFunction)(NeoPatterns*), uint16_t steps, uint16_t interval) {
    ActivePattern = PROCESS_SELECTIVE;
    Interval = interval;
    TotalSteps = steps;
    Index = 0;
    SingleLEDProcessingFunction = aSingleLEDProcessingFunction;
    Direction = DIRECTION_UP;
}

/*
 * Process only pixels with ColorForSelection
 */
void NeoPatterns::ProcessSelectiveColorForAllPixelAndShow() {

    color32_t tNewColor = SingleLEDProcessingFunction(this);
    for (uint16_t i = 0; i < numPixels(); i++) {
        color32_t tOldColor = getPixelColor(i);
        if (tOldColor == ColorForSelection) {
            setPixelColor(i, tNewColor);
        }
    }
    ColorForSelection = tNewColor;
    show();
}

void NeoPatterns::ProcessSelectiveColorUpdate() {

    ProcessSelectiveColorForAllPixelAndShow();
    Increment();
}

/*
 * fade only pixel which have color1
 */
void NeoPatterns::FadeSelectiveColor(color32_t color1, color32_t color2, uint16_t steps, uint16_t interval) {
    ActivePattern = FADE_SELECTIVE;
    Interval = interval;
    TotalSteps = steps + 1; // to include color2
    Color1 = color1;
    ColorForSelection = color1;
    Color2 = color2;
    Index = 1; // since start color is already displayed
}

// Update only pixels with the right color
void NeoPatterns::FadeSelectiveUpdate() {
// Calculate linear interpolation between Color1 and Color2
// Optimize order of operations to minimize truncation error
    uint16_t tOriginalTotalSteps = TotalSteps - 1;
    uint8_t red = ((Red(Color1) * (tOriginalTotalSteps - Index)) + (Red(Color2) * Index)) / tOriginalTotalSteps;
    uint8_t green = ((Green(Color1) * (tOriginalTotalSteps - Index)) + (Green(Color2) * Index)) / tOriginalTotalSteps;
    uint8_t blue = ((Blue(Color1) * (tOriginalTotalSteps - Index)) + (Blue(Color2) * Index)) / tOriginalTotalSteps;

    color32_t tNewColor = Color(red, green, blue);
    for (uint16_t i = 0; i < numPixels(); i++) {
        color32_t tOldColor = getPixelColor(i);
        if (tOldColor == ColorForSelection) {
            setPixelColor(i, tNewColor);
        }
    }
    show();
    ColorForSelection = tNewColor;
    Increment();
}

/*
 * Code for pattern extensions
 *
 * Fill in your own code here
 */
// Initialize for Pattern1
void NeoPatterns::Pattern1(color32_t aColor1, color32_t aColor2, uint8_t aInterval, uint8_t aDirection) {
    ActivePattern = PATTERN1;
    Interval = aInterval;
    Color1 = aColor1;
    Color2 = aColor2;
    setDirectionAndTotalStepsAndIndex(aDirection, numPixels());
}

void NeoPatterns::Pattern2(color32_t aColor1, color32_t aColor2, uint8_t aInterval, uint8_t aDirection) {
    ActivePattern = PATTERN2;
    Interval = aInterval;
    Color1 = aColor1;
    Color2 = aColor2;
    Direction = DIRECTION_UP;
    Index = 0;
    TotalSteps = numPixels() + 1; // must be greater than numPixels()
}

void NeoPatterns::Pattern1Update() {
    /*
     * Dummy implementation
     */
    for (uint16_t i = 0; i < numPixels(); i++) {
        if (i == Index) {
            setPixelColor(i, Color2);
        } else {
            setPixelColor(i, Color1);
        }
    }
    show();
    Increment();
}

void NeoPatterns::Pattern2Update() {
    /*
     * Dummy implementation
     */
    for (uint16_t i = 0; i < numPixels(); i++) {
        if (i == Index) {
            setPixelColor(i, Color2);
        } else {
            setPixelColor(i, COLOR32_BLACK);
        }
    }
    show();
    Increment();
    if (Index == numPixels()) {
        // change direction
        Direction = DIRECTION_DOWN;
        // do nor use upper pixel twice
        Index--;
    }
}

/***********************************************************
 * Sample processing functions for ProcessSelectiveColor()
 ***********************************************************/
uint32_t FadeColor(NeoPatterns * aLedPtr) {
    uint16_t tIndex = aLedPtr->Index + 1;
    uint16_t tTotalSteps = aLedPtr->TotalSteps;
    color32_t tColor1 = aLedPtr->Color1;
    color32_t tColor2 = aLedPtr->Color2;
    uint8_t red = ((Red(tColor1) * (tTotalSteps - tIndex)) + (Red(tColor2) * tIndex)) / tTotalSteps;
    uint8_t green = ((Green(tColor1) * (tTotalSteps - tIndex)) + (Green(tColor2) * tIndex)) / tTotalSteps;
    uint8_t blue = ((Blue(tColor1) * (tTotalSteps - tIndex)) + (Blue(tColor2) * tIndex)) / tTotalSteps;
    return NeoPatterns::Color(red, green, blue);
// return COLOR(red, green, blue);
}

uint32_t DimColor(NeoPatterns * aLedPtr) {
    color32_t tColor = aLedPtr->ColorForSelection;
    uint8_t red = Red(tColor) >> 1;
    uint8_t green = Green(tColor) >> 1;
    uint8_t blue = Blue(tColor) >> 1;
// call to function saves 76 byte program space
    return NeoPatterns::Color(red, green, blue);
//    return COLOR(red, green, blue);
}

uint32_t LightenColor(NeoPatterns * aLedPtr) {
    color32_t tColor = aLedPtr->ColorForSelection;
    uint8_t red = Red(tColor) << 1;
    uint8_t green = Green(tColor) << 1;
    uint8_t blue = Blue(tColor) << 1;
// call to function saves 44 byte program space
    return NeoPatterns::Color(red, green, blue);
//    return COLOR(red, green, blue);
}

/*****************************************************************
 * COMBINED PATTERN EXAMPLE
 * overwrites the OnComplete Handler pointer and sets it
 * to aNextOnComplete after completion of combined patterns
 *****************************************************************/
// initialize for falling star (scanner with delay after pattern)
void initFallingStar(NeoPatterns * aLedsPtr, color32_t aColor, uint8_t aDuration, uint8_t aRepetitions,
        void (*aNextOnCompleteHandler)(NeoPatterns*)) {

    aLedsPtr->Duration = aDuration;
    aLedsPtr->Repetitions = (aRepetitions * 2) - 1;
    aLedsPtr->OnPatternComplete = &multipleFallingStarCompleteHandler;
    aLedsPtr->NextOnPatternCompleteHandler = aNextOnCompleteHandler;

    /*
     * Start with one scanner
     */
    aLedsPtr->Scanner(aColor, aDuration, FLAG_SCANNER_FALLING_STAR);
}

/*
 * if all falling stars are completed switch back to NextOnComplete
 */
void multipleFallingStarCompleteHandler(NeoPatterns * aLedsPtr) {
    uint8_t tDuration = aLedsPtr->Duration;
    uint16_t tRepetitions = aLedsPtr->Repetitions;
    if (tRepetitions == 1) {
        // perform delay and then switch back to NextOnComplete
        aLedsPtr->Delay(tDuration * 2);
        aLedsPtr->OnPatternComplete = aLedsPtr->NextOnPatternCompleteHandler;
    } else {
        /*
         * Next falling star
         */
        tRepetitions = tRepetitions % 2;
        if (tRepetitions == 1) {
            // for odd Repetitions 3,5,7, etc. -> do delay
            aLedsPtr->Delay(tDuration * 2);
        } else {
            // for even Repetitions 2,4,6, etc. -> do scanner
            aLedsPtr->Scanner(aLedsPtr->Color2, tDuration, FLAG_SCANNER_FALLING_STAR);
        }
        aLedsPtr->Repetitions--;
    }
}

// Returns the Red component of a 32-bit color
uint8_t Red(color32_t color) {
    return (color >> 16) & 0xFF;
}

// Returns the Green component of a 32-bit color
uint8_t Green(color32_t color) {
    return (color >> 8) & 0xFF;
}

// Returns the Blue component of a 32-bit color
uint8_t Blue(color32_t color) {
    return color & 0xFF;
}

int8_t checkAndTruncateParamValue(int8_t aParam, int8_t aParamMax, int8_t aParamMin) {
    if (aParam > aParamMax) {
        aParam = aParamMax;
    } else if (aParam < aParamMin) {
        aParam = aParamMin;
    }
    return aParam;
}

/*
 * Sample handler for random pattern
 */
void allPatternsRandomExample(NeoPatterns * aLedsPtr) {
    uint8_t tState = random(11);

    Serial.print("tState=");
    Serial.println(tState);

    uint8_t tDuration = random(40, 81);
    uint8_t tColor = random(255);

    switch (tState) {
    case 0:
        aLedsPtr->Cylon(NeoPatterns::Wheel(tColor), tDuration, 2);
        break;
    case 1:
        aLedsPtr->Scanner(NeoPatterns::Wheel(tColor), tDuration);
        break;
    case 2:
        // Falling star
        aLedsPtr->Scanner(COLOR32_WHITE_HALF, tDuration / 2, FLAG_SCANNER_FALLING_STAR);
        break;
    case 3:
        aLedsPtr->RainbowCycle(20);
        break;
    case 4:
        aLedsPtr->TheaterChase(COLOR32_WHITE_HALF, COLOR32_BLACK, tDuration + tDuration / 2); // White on Black
        break;
    case 5:
        aLedsPtr->TheaterChase(NeoPatterns::Wheel(tColor), NeoPatterns::Wheel(tColor + 0x80), tDuration + tDuration / 2); //
        break;
    case 6:
        aLedsPtr->Fade(COLOR32_RED, COLOR32_BLUE, 32, tDuration, DIRECTION_DOWN);
        break;
    case 7:
        aLedsPtr->ColorWipe(NeoPatterns::Wheel(tColor), tDuration);
        break;
    case 8:
        // start at both end
        aLedsPtr->Scanner(NeoPatterns::Wheel(tColor), tDuration / 2,
        FLAG_SCANNER_STARTING_AT_BOTH_ENDS | FLAG_SCANNER_VANISH_COMPLETE);
        break;
    case 9:
        // Multiple falling star
        initFallingStar(aLedsPtr, COLOR32_WHITE_HALF, tDuration, 3, &allPatternsRandomExample);
        break;
    case 10:
        if (aLedsPtr->PatternsGeometry == GEOMETRY_BAR) {
            //Fire
            aLedsPtr->Fire(tDuration / 2, 150);
        } else {
            // start at both end
            aLedsPtr->Scanner(NeoPatterns::Wheel(tColor), tDuration,
            FLAG_SCANNER_STARTING_AT_BOTH_ENDS | FLAG_SCANNER_VANISH_COMPLETE);
        }
        break;
    default:
        break;
    }
}

