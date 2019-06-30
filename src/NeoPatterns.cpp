/**
 * NeoPatterns.cpp
 *
 * This file includes the original code of Adafruit as well as code of Mark Kriegsman
 *
 *  SUMMARY
 *  This is an extended version version of the NeoPattern Example by Adafruit
 *  https://learn.adafruit.com/multi-tasking-the-arduino-part-3?view=all
 *  You need to install "Adafruit NeoPixel" library under "Tools -> Manage Libraries..." or "Ctrl+Shift+I" -> use "neoPixel" as filter string.
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

//#define TRACE
//#define DEBUG
//#define INFO
//#define WARN
//#define ERROR

#include "NeoPatterns.h"

char VERSION_NEOPATTERNS[] = "1.2";

/**********************************************************************************
 * Code inspired by https://learn.adafruit.com/multi-tasking-the-arduino-part-3?view=all
 * Changed and extended for added functionality
 **********************************************************************************/
/*
 * Constructor - get and call Adafruit_NeoPixel constructor to initialize strip
 * NeoPatterns id not derived from Adafruit_NeoPixel to enable running more than one pattern on the same NeoPixel object.
 * In this case it avoids doubling the big pixel buffer.
 */

NeoPatterns::NeoPatterns(uint16_t aNumberOfPixels, uint8_t aPin, uint8_t aTypeOfPixel, // @suppress("Class members should be properly initialized")
        void (*aPatternCompletionCallback)(NeoPatterns*)) :
        NeoPixel(aNumberOfPixels, aPin, aTypeOfPixel) {

    OnPatternComplete = aPatternCompletionCallback;
}

void NeoPatterns::setCallback(void (*callback)(NeoPatterns*)) {
    OnPatternComplete = callback;
}

bool NeoPatterns::CheckForUpdate() {
    if ((millis() - lastUpdate) > Interval) {
        return true;
    }
    return false;
}

// Update the pattern returns true if update has happened in order to give the caller a chance to manually change parameters (like color etc.)
bool NeoPatterns::Update(bool doShow) {
    if ((millis() - lastUpdate) > Interval) {

        switch (ActivePattern) {
        case PATTERN_RAINBOW_CYCLE:
            RainbowCycleUpdate();
            break;
        case PATTERN_COLOR_WIPE:
            ColorWipeUpdate();
            break;
        case PATTERN_FADE:
            FadeUpdate();
            break;
        case PATTERN_PROCESS_SELECTIVE:
            ProcessSelectiveColorUpdate();
            break;
        case PATTERN_FIRE:
            FireUpdate();
            break;
        case PATTERN_DELAY:
            DelayUpdate();
            break;
        case PATTERN_SCANNER_EXTENDED:
            ScannerExtendedUpdate();
            break;
        case PATTERN_STRIPES:
            StripesUpdate();
            break;
        case PATTERN_USER_PATTERN1:
            Pattern1Update();
            break;
        case PATTERN_USER_PATTERN2:
            Pattern2Update();
            break;
        default:
            break;
        }

        if (doShow) {
            show();
        }
        // remember last time of update
        lastUpdate = millis();
        return true;
    }
    return false;
}

// Update the pattern returns true if update has happened in order to give the caller a chance to manually change parameters (like color etc.)
bool NeoPatterns::UpdateOrRedraw() {

    bool tDoUpdate = (millis() - lastUpdate) > Interval;

    switch (ActivePattern) {
    case PATTERN_RAINBOW_CYCLE:
        RainbowCycleUpdate(tDoUpdate);
        break;
    case PATTERN_COLOR_WIPE:
        ColorWipeUpdate(tDoUpdate);
        break;
    case PATTERN_FADE:
        FadeUpdate(tDoUpdate);
        break;
    case PATTERN_PROCESS_SELECTIVE:
        ProcessSelectiveColorUpdate(tDoUpdate);
        break;
    case PATTERN_FIRE:
        FireUpdate(tDoUpdate);
        break;
    case PATTERN_DELAY:
        DelayUpdate(tDoUpdate);
        break;
    case PATTERN_SCANNER_EXTENDED:
        ScannerExtendedUpdate(tDoUpdate);
        break;
    case PATTERN_STRIPES:
        StripesUpdate(tDoUpdate);
        break;
    case PATTERN_USER_PATTERN1:
        Pattern1Update(tDoUpdate);
        break;
    case PATTERN_USER_PATTERN2:
        Pattern2Update(tDoUpdate);
        break;
    default:
        break;
    }

    if (tDoUpdate) {
        // remember last time of update
        lastUpdate = millis();
    }
    return tDoUpdate;
}

/*
 * Decrement TotalSteps and call callback
 */
void NeoPatterns::DecrementTotalStepCounter() {
    TotalStepCounter--;
    if (TotalStepCounter == 0) {
        if (OnPatternComplete != NULL) {
            OnPatternComplete(this); // call the completion callback
        }
    }
}
/*
 * Decrement TotalSteps and call callback
 */
void NeoPatterns::NextIndexAndDecrementTotalStepCounter() {
    if (Direction == DIRECTION_UP) {
        Index++;
    } else {
        Index--;
    }
    DecrementTotalStepCounter();
}

// Helper to set index accordingly to direction
void NeoPatterns::setDirectionAndTotalStepsAndIndex(uint8_t aDirection, uint16_t totalSteps) {
    Direction = aDirection;
    TotalStepCounter = totalSteps;
    if (Direction == DIRECTION_UP) {
        Index = 0;
    } else {
        Index = totalSteps - 1;
    }
}

// Initialize for a RainbowCycle
void NeoPatterns::RainbowCycle(uint8_t interval, uint8_t aDirection) {
    ActivePattern = PATTERN_RAINBOW_CYCLE;
    Interval = interval;
    ColorTmp = 0x10000 / numLEDs;
    setDirectionAndTotalStepsAndIndex(aDirection, 255);
}

// Update the Rainbow Cycle Pattern starting with a color that is next position in wheel
void NeoPatterns::RainbowCycleUpdate(bool aDoUpdate) {
    uint16_t tWheelIndexHighResolution = 0; // = 0x10000->max. upper byte is the integer part used for setBrightness, lower byte is the fractional part
    uint16_t tWheelIndexHighDelta = ColorTmp;

    for (uint16_t i = 0; i < numLEDs; i++) {
        setPixelColor(i, Wheel(Index + (tWheelIndexHighResolution >> 8)));
        tWheelIndexHighResolution += tWheelIndexHighDelta;
    }
    if (aDoUpdate) {
        NextIndexAndDecrementTotalStepCounter();
    }
}

// Initialize for a ColorWipe.
void NeoPatterns::ColorWipe(color32_t color, uint8_t interval, uint8_t aMode, uint8_t aDirection) {
    ActivePattern = PATTERN_COLOR_WIPE;
    Interval = interval;
    Color1 = color;
    Flags = aMode;
    setDirectionAndTotalStepsAndIndex(aDirection, numLEDs);
}

// Update the Color Wipe Pattern. Fill with color.
void NeoPatterns::ColorWipeUpdate(bool aDoUpdate) {
    for (uint16_t i = 0; i < numLEDs; i++) {
        if ((Direction == DIRECTION_UP && i <= Index) || (Direction == DIRECTION_DOWN && i >= Index)) {
            setPixelColor(i, Color1);
        } else if (!(Flags & FLAG_DO_NOT_CLEAR)) {
            setPixelColor(i, COLOR32_BLACK);
        }
    }
    if (aDoUpdate) {
        NextIndexAndDecrementTotalStepCounter();
    }
}

// Initialize for a Fade from color1 to color2
void NeoPatterns::Fade(color32_t color1, color32_t color2, uint16_t steps, uint8_t interval) {
    ActivePattern = PATTERN_FADE;
    Interval = interval;
    TotalStepCounter = steps;
    PatternLength = steps;
    Color1 = color1;
    Color2 = color2;
    Index = 0;
    Direction = DIRECTION_UP;
}

// Update the Fade pattern
void NeoPatterns::FadeUpdate(bool aDoUpdate) {
// Calculate linear interpolation between Color1 and Color2
// Optimize order of operations to minimize truncation error
    uint8_t red = ((Red(Color1) * (PatternLength - Index)) + (Red(Color2) * Index)) / PatternLength;
    uint8_t green = ((Green(Color1) * (PatternLength - Index)) + (Green(Color2) * Index)) / PatternLength;
    uint8_t blue = ((Blue(Color1) * (PatternLength - Index)) + (Blue(Color2) * Index)) / PatternLength;

    ColorSet(Color(red, green, blue));
    if (aDoUpdate) {
        NextIndexAndDecrementTotalStepCounter();
    }
}

/****************************************************************************
 * START OF EXTENSIONS
 ****************************************************************************/

/*
 * Code for scanner default is: pattern completely visible at start and end
 * Starts at position 0 with DIRECTION_UP
 */
void NeoPatterns::ScannerExtended(color32_t aColor1, uint8_t aLength, uint16_t aInterval, uint16_t aNumberOfBouncings,
        uint8_t aMode, uint8_t aDirection) {
    // The variables MultipleExtension, Repetitions and NextOnPatternCompleteHandler are used by MultipleFallingStars and cannot be used here
    ActivePattern = PATTERN_SCANNER_EXTENDED;
    Interval = aInterval;
    Color1 = aColor1;
    Color2 = aNumberOfBouncings; // Abuse Color2 as storage
    ColorTmp = 0x10000 / aLength; // Delta for each step. Abuse ColorTmp as storage

    PatternLength = aLength;
    Flags = aMode;
    Direction = aDirection;
    TotalStepCounter = numLEDs - aLength; // pattern is completely visible at start and end
    Index = aLength - 1; // start position pattern is completely visible at start

    uint16_t tStepsForBounce = numLEDs - 1;

    if (aMode & FLAG_SCANNER_EXT_VANISH_COMPLETE) {
        // invisible at start and end
        Index -= aLength;
        TotalStepCounter += 2 * aLength;
        if (aMode & FLAG_SCANNER_EXT_CYLON) {
            Index -= aLength - 1;
            TotalStepCounter += aLength - 1; // since the brightest led is not doubled and total length is (length + length -1)
            tStepsForBounce = numLEDs - 2 * (aLength - 1) - 1;
        }
    } else {
        if (aMode & FLAG_SCANNER_EXT_CYLON) {
            // cylon visible at start and end
            TotalStepCounter -= aLength - 1; // since the brightest led is not doubled and total length is (length + length -1)
            tStepsForBounce = numLEDs - 2 * (aLength - 1) - 1;
        }
    }

    if (aNumberOfBouncings > 0) {
        TotalStepCounter += (tStepsForBounce * aNumberOfBouncings);
    }
    if (aDirection == DIRECTION_DOWN) {
        Index = (numLEDs - 1) - Index;
    }

    TotalStepCounter++; // since first step just shows the start pattern
}

void NeoPatterns::ScannerExtendedUpdate(bool aDoUpdate) {
    /*
     * index is starting position of brightest led (middle of pattern if mode is bubble)
     */
    uint16_t tBrightnessHighResolution = 0; // = 0x10000->max. upper byte is the integer part used for setBrightness, lower byte is the fractional part
    uint16_t tBrightnessDelta = ColorTmp;
#ifdef TRACE
    Serial.print("aStartPosition=");
    Serial.print(aStartPosition);
    Serial.print(" tLength=");
    Serial.print(tLength);
#endif

    /*
     * get color components for later dimming
     */
    uint8_t tRedDimmed, tGreenDimmed, tBlueDimmed;
    uint8_t tRed = tRedDimmed = (uint8_t) (Color1 >> 16);
    uint8_t tGreen = tGreenDimmed = (uint8_t) (Color1 >> 8);
    uint8_t tBlue = tBlueDimmed = (uint8_t) Color1;

    uint8_t tPatternIndex;
    for (tPatternIndex = 0; tPatternIndex < PatternLength; ++tPatternIndex) {
        uint8_t tBrightness = tBrightnessHighResolution >> 8;
        /*
         * compute new dimmed color value
         */
        tBrightness = gamma5(tBrightness);
        if (tBrightness) {
            // (tRed + 1) since (255 *1) >> 8 gives 0 (and not 1)
            tRedDimmed = ((tRed + 1) * tBrightness) >> 8;
            tGreenDimmed = ((tGreen + 1) * tBrightness) >> 8;
            tBlueDimmed = ((tBlue + 1) * tBrightness) >> 8;
        }

        if (tRedDimmed == 0 && tGreenDimmed == 0 && tBlueDimmed == 0) {
            // break if color is black
            break;
        }

        uint16_t tOffset;
        if (Direction == DIRECTION_UP) {
            tOffset = tPatternIndex;
        } else {
            tOffset = -tPatternIndex; // deliberate use unsigned here!
        }
        setPixelColor(Index - tOffset, tRedDimmed, tGreenDimmed, tBlueDimmed);
        if (Flags & FLAG_SCANNER_EXT_START_AT_BOTH_ENDS) {
            // draw at other end too
            setPixelColor((numLEDs - 1) - (Index - tOffset), tRedDimmed, tGreenDimmed, tBlueDimmed);
        }
        if (Flags & FLAG_SCANNER_EXT_CYLON) {
            // mirror pattern
            setPixelColor(Index + tOffset, tRedDimmed, tGreenDimmed, tBlueDimmed);
            if (Flags & FLAG_SCANNER_EXT_START_AT_BOTH_ENDS) {
                // draw at other end too
                setPixelColor((numLEDs - 1) - (Index + tOffset), tRedDimmed, tGreenDimmed, tBlueDimmed);

            }
        }

#ifdef DEBUG
        Serial.print("i=");
        Serial.print(tPatternIndex);
        Serial.print(" Position=");
        Serial.print(int16_t(Index - tOffset));
        if (aDoUpdate) {
            Serial.print(" Brightness=");
            Serial.print(tBrightness);
            Serial.print(" r=");
            Serial.print(tRedDimmed);
            Serial.print(" g=");
            Serial.print(tGreenDimmed);
            Serial.print(" b=");
            Serial.print(tBlueDimmed);
        }
        Serial.println();
#endif
        // compute next brightness
        tBrightnessHighResolution -= tBrightnessDelta;
    }

    if (aDoUpdate) {
        uint16_t tNumberOfBouncings = Color2;
        if (Direction == DIRECTION_UP) {
            /*
             * Cleanup last tail pixel from old pattern
             */
            setPixelColor(Index - tPatternIndex, COLOR32_BLACK);
            if (Flags & FLAG_SCANNER_EXT_START_AT_BOTH_ENDS) {
                setPixelColor((numLEDs - 1) - (Index - tPatternIndex), COLOR32_BLACK);
            }

            /*
             * check for bouncing condition
             */
            if (tNumberOfBouncings > 0) {
                if (Index == numLEDs - 1) {
                    Direction = DIRECTION_DOWN;
                    tNumberOfBouncings--;
                }
                if (Flags & FLAG_SCANNER_EXT_CYLON) {
                    if (Index + PatternLength == numLEDs) {
                        Direction = DIRECTION_DOWN;
                        tNumberOfBouncings--;
                    }
                }
            }
        } else {
            /*
             * DIRECTION_DOWN - Cleanup last tail pixel from old pattern
             */
            setPixelColor(Index + tPatternIndex, COLOR32_BLACK);
            if (Flags & FLAG_SCANNER_EXT_START_AT_BOTH_ENDS) {
                setPixelColor((numLEDs - 1) - (Index + tPatternIndex), COLOR32_BLACK);
            }

            /*
             * check for bouncing condition
             */
            if (tNumberOfBouncings > 0) {
                if (Index == 0) {
                    Direction = DIRECTION_UP;
                    tNumberOfBouncings--;
                }
                if (Flags & FLAG_SCANNER_EXT_CYLON) {
                    if (Index - (PatternLength - 1) == 0) {
                        Direction = DIRECTION_UP;
                        tNumberOfBouncings--;
                    }
                }
            }
        }
        Color2 = tNumberOfBouncings;
        NextIndexAndDecrementTotalStepCounter();
    } //  if (aDoUpdate)
}

void NeoPatterns::Stripes(color32_t aColor1, uint8_t aLength1, color32_t aColor2, uint8_t aLength2, uint8_t aInterval,
        uint16_t aNumberOfSteps, uint8_t aMode, uint8_t aDirection) {
    ActivePattern = PATTERN_STRIPES;
    Color1 = aColor1;
    PatternLength = aLength1;
    Color2 = aColor2;
    MultipleExtension = aLength2;
    Interval = aInterval;
    TotalStepCounter = aNumberOfSteps;
    Flags = aMode;
    // the direction of index has the opposite direction of pattern
    if (aDirection == DIRECTION_UP) {
        Direction = DIRECTION_DOWN;
    } else {
        Direction = DIRECTION_UP;
    }
    Index = 0; // start index (in pattern) running from 0 to (aLength1 + aLength2)
}

void NeoPatterns::StripesUpdate(bool aDoUpdate) {
    uint8_t tRunningIndex = Index;
    for (uint16_t i = 0; i < numLEDs; i++) {
        /*
         * draw 1 pattern
         */
        if (tRunningIndex < PatternLength) {
            // first part
            setPixelColor(i, Color1);
        } else {
            // second part
            setPixelColor(i, Color2);
        }
        tRunningIndex++;
        // check for end of pattern
        if (tRunningIndex >= PatternLength + MultipleExtension) {
            tRunningIndex = 0;
        }
    }

    if (aDoUpdate) {
        if (Direction == DIRECTION_UP) {
            Index++;
            if (Index >= PatternLength + MultipleExtension) {
                Index = 0;
            }
        } else {
            Index--;
            if (Index == 0xFFFF) {
                Index = PatternLength + MultipleExtension - 1;
            }
        }
        // must be last action before return
        DecrementTotalStepCounter();
    }
}

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
void NeoPatterns::Fire(uint16_t aIntervalMillis, uint16_t aRepetitions) {
    ActivePattern = PATTERN_FIRE;
    Interval = aIntervalMillis;
    Direction = DIRECTION_UP;
    Index = 0;
    Repetitions = aRepetitions;
    // reset colors
    ColorSet(0);
}

// Update the Fire Pattern
void NeoPatterns::FireUpdate(bool aDoUpdate) {
    static byte heat[24];

    if (aDoUpdate) {
        // Step 1.  Cool down every cell a little
        for (uint16_t i = 0; i < numLEDs; i++) {
            uint8_t tChill = random(((COOLING * 20) / numLEDs) + 2);
            if (tChill >= heat[i]) {
                heat[i] = 0;
            } else {
                heat[i] = heat[i] - tChill;
            }
        }

        // Step 2.  Heat from each cell drifts 'up' and diffuses a little
        for (uint16_t k = numLEDs - 1; k >= 2; k--) {
            heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
        }

        // Step 3.  Randomly ignite one new 'spark' of heat near the bottom
        if (random(255) < SPARKING) {
            int y = random(numLEDs / 4);
            uint8_t tNewHeat = random(160, 255);
            if (heat[y] + tNewHeat < heat[y]) {
                heat[y] = 0xFF;
            } else {
                heat[y] += tNewHeat;
            }
        }
    }
    // Step 4.  Map from heat cells to LED colors
    for (uint16_t j = 0; j < numLEDs; j++) {
        setPixelColor(j, HeatColor(heat[j]));
    }

    if (aDoUpdate) {
        Repetitions--;
        if (Repetitions == 0) {
            if (OnPatternComplete != NULL) {
                OnPatternComplete(this); // call the completion callback
            }
        }
    }
}

uint32_t NeoPatterns::HeatColor(uint8_t aTemperature) {

// Scale 'heat' down from 0-255 to 0-191,
// which can then be easily divided into three
// equal 'thirds' of 64 units each.
    uint8_t t192 = (aTemperature == 0) ? 0 : ((aTemperature * (uint16_t) (192)) >> 8) + 1;

// calculate a value that ramps up from
// zero to 255 in each 'third' of the scale.
    uint8_t heatramp = t192 & 0x3F; // 0..63
    heatramp <<= 2; // scale up to 0..252
    heatramp = gamma8(heatramp);

    // now figure out which third of the spectrum we're in:
    if (t192 & 0x80) {
        // we're in the hottest third, ramp from yellow to white
        return Color(255, 255, heatramp);
    } else if (t192 & 0x40) {
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

// Initialize for a delay -> just keep the old pattern displayed
void NeoPatterns::Delay(uint16_t aMillis) {
    ActivePattern = PATTERN_DELAY;
    Interval = aMillis;
    lastUpdate = millis();
    TotalStepCounter = 1;
}

// Update / End the Delay pattern
void NeoPatterns::DelayUpdate(bool aDoUpdate) {
    if (aDoUpdate) {
        DecrementTotalStepCounter();
    }
}

/*
 * call provided processing routine only for pixel which have color equal to ColorForSelection
 * the last resulting color is found in Color2
 */
void NeoPatterns::ProcessSelectiveColor(color32_t aColorForSelection, color32_t (*aSingleLEDProcessingFunction)(NeoPatterns*),
        uint16_t steps, uint16_t interval) {
    ActivePattern = PATTERN_PROCESS_SELECTIVE;
    Interval = interval;
    TotalStepCounter = steps;
    Index = 0; // needed for FadeColor
    Color1 = aColorForSelection;
    // initialize temporary color
    ColorTmp = aColorForSelection;
    SingleLEDProcessingFunction = aSingleLEDProcessingFunction;
}

/*
 * Process only pixels with ColorForSelection
 */
void NeoPatterns::ProcessSelectiveColorForAllPixel() {

    color32_t tNewColor = SingleLEDProcessingFunction(this);
    for (uint16_t i = 0; i < numLEDs; i++) {
        color32_t tOldColor = getPixelColor(i);
        if (tOldColor == ColorTmp) {
            setPixelColor(i, tNewColor);
        }
    }
    ColorTmp = tNewColor;
}

void NeoPatterns::ProcessSelectiveColorUpdate(bool aDoUpdate) {
    if (aDoUpdate) {
        ProcessSelectiveColorForAllPixel();
        DecrementTotalStepCounter();
    }
}

/***********************************************************
 * Sample processing functions for ProcessSelectiveColor()
 ***********************************************************/
color32_t FadeColor(NeoPatterns * aLedPtr) {
    aLedPtr->Index++;
    uint16_t tIndex = aLedPtr->Index;
    uint16_t tTotalSteps = aLedPtr->TotalStepCounter;
    color32_t tColor1 = aLedPtr->Color1;
    color32_t tColor2 = aLedPtr->Color2;
    uint8_t red = ((Red(tColor1) * (tTotalSteps - tIndex)) + (Red(tColor2) * tIndex)) / tTotalSteps;
    uint8_t green = ((Green(tColor1) * (tTotalSteps - tIndex)) + (Green(tColor2) * tIndex)) / tTotalSteps;
    uint8_t blue = ((Blue(tColor1) * (tTotalSteps - tIndex)) + (Blue(tColor2) * tIndex)) / tTotalSteps;
    return Adafruit_NeoPixel::Color(red, green, blue);
}

/*
 * works on ColorTmp
 */
color32_t DimColor(NeoPatterns * aLedPtr) {
    color32_t tColor = aLedPtr->ColorTmp;
    uint8_t red = Red(tColor) >> 1;
    uint8_t green = Green(tColor) >> 1;
    uint8_t blue = Blue(tColor) >> 1;
// call to function saves 76 byte program space
    return Adafruit_NeoPixel::Color(red, green, blue);
//    return COLOR(red, green, blue);
}

/*
 * works on ColorTmp
 */
color32_t BrightenColor(NeoPatterns * aLedPtr) {
    color32_t tColor = aLedPtr->ColorTmp;
    uint8_t red = Red(tColor) << 1;
    uint8_t green = Green(tColor) << 1;
    uint8_t blue = Blue(tColor) << 1;
// call to function saves 44 byte program space
    return Adafruit_NeoPixel::Color(red, green, blue);
//    return COLOR(red, green, blue);
}

void NeoPatterns::Debug(Stream * aSerial, bool aFullInfo) {
    static uint16_t sLastSteps;
    if (aFullInfo) {
        sLastSteps = 0x9000;
        aSerial->print("ActivePattern=");
        aSerial->print(ActivePattern);
        aSerial->print(" Interval=");
        aSerial->print(Interval);
        aSerial->print(" Color1=0x");
        aSerial->print(Color1, HEX);
        aSerial->print(" Color2=0x");
        aSerial->print(Color2, HEX);
        aSerial->print("|");
        aSerial->print(Color2);
        aSerial->print(" ColorTmp=0x");
        aSerial->print(ColorTmp, HEX);
        aSerial->print("|");
        aSerial->print(ColorTmp);
        aSerial->print(" Flags=0x");
        aSerial->print(Flags, HEX);
        aSerial->print(' ');
    }
    /*
     * only print if TotalSteps changed
     */
    if (Index != sLastSteps) {
        sLastSteps = TotalStepCounter;
        aSerial->print("Pin=");
        aSerial->print(getPin());

        aSerial->print(" TotalSteps=");
        aSerial->print(TotalStepCounter);
        aSerial->print(" Index=");
        aSerial->print((int16_t) Index);
        aSerial->print(" Direction=");
        aSerial->print(Direction);
        aSerial->print(" Repetitions=");
        aSerial->print(Repetitions);
        aSerial->println();
    }
}

/******************************
 * Code for pattern extensions
 *****************************/

// Initialize for Pattern1
// set all pixel to aColor1 and let a pixel of color2 move through
void __attribute__((weak)) UserPattern1(NeoPatterns * aNeoPatterns, color32_t aColor1, color32_t aColor2, uint8_t aInterval,
        uint8_t aDirection) {
    aNeoPatterns->ActivePattern = PATTERN_USER_PATTERN1;
    aNeoPatterns->Interval = aInterval;
    aNeoPatterns->Color1 = aColor1;
    aNeoPatterns->Color2 = aColor2;
    aNeoPatterns->Direction = aDirection;
    aNeoPatterns->TotalStepCounter = aNeoPatterns->numPixels();
}

void NeoPatterns::Pattern1Update(bool aDoUpdate) {
    /*
     * Sample implementation
     */
    for (uint16_t i = 0; i < numLEDs; i++) {
        if (i == Index) {
            setPixelColor(i, Color2);
        } else {
            setPixelColor(i, Color1);
        }
    }
    if (aDoUpdate) {
        NextIndexAndDecrementTotalStepCounter();
    }
}

// clear all pixel and let a pixel of color2 move up and down
void __attribute__((weak)) UserPattern2(NeoPatterns * aNeoPatterns, color32_t aColor1, color32_t aColor2, uint8_t aInterval,
        uint8_t aDirection) {
    /*
     * Sample implementation
     */
    aNeoPatterns->ActivePattern = PATTERN_USER_PATTERN2;
    aNeoPatterns->Interval = aInterval;
    aNeoPatterns->Color1 = aColor1;
    aNeoPatterns->Color2 = aColor2;
    aNeoPatterns->Direction = aDirection;
    aNeoPatterns->Index = 0;
    aNeoPatterns->TotalStepCounter = (2 * aNeoPatterns->numPixels()) - 1; // up and down but do nor use upper pixel twice
}

/*
 * The user may specify its own implementation
 */
void __attribute__((weak)) UserPattern2Update(NeoPatterns * aNeoPatterns, bool aDoUpdate) {
    /*
     * Sample implementation
     */
    for (uint16_t i = 0; i < aNeoPatterns->numPixels(); i++) {
        if (i == aNeoPatterns->Index) {
            aNeoPatterns->setPixelColor(i, aNeoPatterns->Color2);
        } else {
            aNeoPatterns->setPixelColor(i, COLOR32_BLACK);
        }
    }

    if (aDoUpdate) {
        aNeoPatterns->NextIndexAndDecrementTotalStepCounter();
        if (aNeoPatterns->Index == aNeoPatterns->numPixels()) {
            // change direction
            aNeoPatterns->Direction = DIRECTION_DOWN;
            // do nor use upper pixel twice
            aNeoPatterns->Index--;
        }
    }
}

void NeoPatterns::Pattern2Update(bool aDoUpdate) {
    UserPattern2Update(this, aDoUpdate);
}

/*****************************************************************
 * COMBINED PATTERN EXAMPLE
 * overwrites the OnComplete Handler pointer and sets it
 * to aNextOnComplete after completion of combined patterns
 * If aNextOnCompleteHandler == NULL, run falling star forever with random delay
 *****************************************************************/
// initialize for falling star (scanner with delay after pattern)
void initMultipleFallingStars(NeoPatterns * aLedsPtr, color32_t aColor, uint8_t aDuration, uint8_t aRepetitions,
        void (*aNextOnCompleteHandler)(NeoPatterns*)) {

    aLedsPtr->MultipleExtension = aDuration;
    if (aRepetitions < (0xFF / 2)) {
        aLedsPtr->Repetitions = (aRepetitions * 2) - 1; // get an odd number
    } else {
        aLedsPtr->Repetitions = 0xFF;
    }
    aLedsPtr->OnPatternComplete = &multipleFallingStarsCompleteHandler;
    aLedsPtr->NextOnPatternCompleteHandler = aNextOnCompleteHandler;

    /*
     * Start with one scanner
     */
    aLedsPtr->clear();
    aLedsPtr->ScannerExtended(aColor, 7, aDuration, 0, FLAG_SCANNER_EXT_VANISH_COMPLETE, DIRECTION_DOWN);
}

/*
 * start delay pattern and then a new falling star
 * if all falling stars are completed switch back to NextOnComplete
 * if NextOnPatternCompleteHandler == NULL, run falling star forever with random delay
 */
void multipleFallingStarsCompleteHandler(NeoPatterns * aLedsPtr) {
    uint8_t tDuration = aLedsPtr->MultipleExtension;
    uint16_t tRepetitions = aLedsPtr->Repetitions;
    if (tRepetitions == 1) {
        // perform delay and then switch back to NextOnComplete
        if (aLedsPtr->NextOnPatternCompleteHandler != NULL) {
            aLedsPtr->OnPatternComplete = aLedsPtr->NextOnPatternCompleteHandler;
            aLedsPtr->Delay(tDuration * 2);
        } else {
            // do it forever but with random delay
            aLedsPtr->Repetitions = 2; // set to even;
            aLedsPtr->Delay(random(tDuration * 10, tDuration * 1000));
        }
    } else {
        /*
         * Next falling star
         */
        tRepetitions = tRepetitions & 0x01; // = tRepetitions % 2;
        if (tRepetitions == 1) {
            // for odd Repetitions 3,5,7, etc. -> do delay
            aLedsPtr->Delay(tDuration * 2);
        } else {
            // for even Repetitions 2,4,6, etc. -> do scanner
            aLedsPtr->ScannerExtended(aLedsPtr->Color1, 7, tDuration, 0, FLAG_SCANNER_EXT_VANISH_COMPLETE, DIRECTION_DOWN);
        }
        aLedsPtr->Repetitions--;
    }
}

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
 * Sample handler for random pattern
 */
void allPatternsRandomExample(NeoPatterns * aLedsPtr) {
    uint8_t tState = random(11);

    uint8_t tDuration = random(40, 81);
    uint8_t tColor = random(255);

    switch (tState) {
    case 0:
        //aLedsPtr->Cylon(NeoPatterns::Wheel(tColor), tDuration, 2);
        aLedsPtr->clear();
        aLedsPtr->ScannerExtended(NeoPatterns::Wheel(tColor), 5, tDuration, 3,
        FLAG_SCANNER_EXT_CYLON | FLAG_SCANNER_EXT_VANISH_COMPLETE, (tDuration & DIRECTION_DOWN));
        break;
    case 1:
        // rocket 2 times bouncing
        aLedsPtr->ScannerExtended(NeoPatterns::Wheel(tColor), 7, tDuration, 2,
        FLAG_SCANNER_EXT_ROCKET | FLAG_SCANNER_EXT_VANISH_COMPLETE, (tDuration & DIRECTION_DOWN));
        break;
    case 2:
        // 1 times rocket or falling star
        aLedsPtr->clear();
        aLedsPtr->ScannerExtended(COLOR32_WHITE_HALF, 7, tDuration / 2, 0, FLAG_SCANNER_EXT_VANISH_COMPLETE,
                (tDuration & DIRECTION_DOWN));
        break;
    case 3:
        aLedsPtr->RainbowCycle(20);
        break;
    case 4:
        aLedsPtr->Stripes(COLOR32_WHITE_HALF, 5, NeoPatterns::Wheel(tColor), 3, tDuration, 2 * aLedsPtr->numPixels(), 0,
                (tDuration & DIRECTION_DOWN));
        break;
    case 5:
        // old TheaterChase
        aLedsPtr->Stripes(NeoPatterns::Wheel(tColor), 1, NeoPatterns::Wheel(tColor + 0x80), 2, tDuration * 2,
                2 * aLedsPtr->numPixels(), 0, (tDuration & DIRECTION_DOWN));
        break;
    case 6:
        aLedsPtr->Fade(COLOR32_RED, COLOR32_BLUE, 32, tDuration);
        break;
    case 7:
        aLedsPtr->ColorWipe(NeoPatterns::Wheel(tColor), tDuration, FLAG_DO_NOT_CLEAR, (tDuration & DIRECTION_DOWN)); // switch direction
        break;
    case 8:
        // rocket start at both end
        aLedsPtr->ScannerExtended(NeoPatterns::Wheel(tColor), 7, tDuration / 2, 3,
        FLAG_SCANNER_EXT_ROCKET | FLAG_SCANNER_EXT_VANISH_COMPLETE | FLAG_SCANNER_EXT_START_AT_BOTH_ENDS);
        break;
    case 9:
        // Multiple falling star
        initMultipleFallingStars(aLedsPtr, COLOR32_WHITE_HALF, tDuration, 3, &allPatternsRandomExample);
        break;
    case 10:
        if (aLedsPtr->PatternsGeometry == GEOMETRY_BAR) {
            //Fire
            aLedsPtr->Fire(tDuration / 2, 100);
        } else {
            // start at both end
            aLedsPtr->ScannerExtended(NeoPatterns::Wheel(tColor), 5, tDuration, 0,
            FLAG_SCANNER_EXT_START_AT_BOTH_ENDS | FLAG_SCANNER_EXT_VANISH_COMPLETE);
        }
        break;
    default:
        break;
    }

#ifdef INFO
    Serial.print("Pin=");
    Serial.print(aLedsPtr->getPin());
    Serial.print(" Length=");
    Serial.print(aLedsPtr->numPixels());
    Serial.print(" ActivePattern=");
    Serial.print(aLedsPtr->ActivePattern);
    Serial.print(" State=");
    Serial.println(tState);
#endif
}

