/*
 * VUBar.hpp
 *
 * Handles the VU bar generation
 * Supports different dampening values, manual or auto scaling and linear/logarithmic display.
 * Peak push and drop handling
 *
 *  SUMMARY
 *  You need to install "Adafruit NeoPixel" library under "Tools -> Manage Libraries..." or "Ctrl+Shift+I" -> use "neoPixel" as filter string.
 *
 *  Copyright (C) 2019-2026  Armin Joachimsmeyer
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
 *  along with this program. If not, see <https://www.gnu.org/licenses/gpl-3.0.html>.
 */

#ifndef _VU_BAR_HPP
#define _VU_BAR_HPP

#include "VUBar.h"
#include <math.h>

// This block must be located after the includes of other *.hpp files
//#define LOCAL_DEBUG // This enables debug output only for this file - only for development
//#define LOCAL_TRACE // This enables trace output only for this file - only for development
#include "LocalDebugLevelStart.h"

VUBar::VUBar() { // @suppress("Class members should be properly initialized")
    BarLength = 0;
    DampingFactorForValue = 0; // no damping
    Flags = FLAG_VU_DEFAULT;
    ScalingForFullBar.DampenedMaxValueShift8 = 0; // assume auto scaling
    DampenedValue = 0;
    PeakValue = 0;
    CallsPerPeakDrop = CALLS_PER_PEAK_DROP;
}

/**
 * @param aDampingFactorForValue The shift amount of the underlying EMA low pass filter.
 *                               if 0 then use value direct and not dampened
 */
VUBar::VUBar(uint8_t aBarLength, uint8_t aDampingFactorForValue, uint8_t aFlags) { // @suppress("Class members should be properly initialized")
    init(aBarLength, aDampingFactorForValue, aFlags);
}

/*
 * For FLAG_VU_USE_VALUE_LOG aValueForFullBar it is the input value which gives a full bar.
 * E.g we have max input value (0 to 2047) which would give only max 12 pixel on a 16 pixel bar for log display.
 * To get the full bar, use 2047 as aValueForFullBar resulting in a factor of 16/12.
 */
VUBar::VUBar(uint8_t aBarLength, uint8_t aDampingFactorForValue, uint8_t aFlags, uint16_t aValueForFullBar) { // @suppress("Class members should be properly initialized")
    init(aBarLength, aDampingFactorForValue, aFlags, aValueForFullBar);
}

void VUBar::init(uint8_t aBarLength, uint8_t aDampingFactorForValue, uint8_t aFlags) {
    BarLength = aBarLength;
    DampingFactorForValue = aDampingFactorForValue;
    Flags = aFlags;
    if (Flags & FLAG_VU_USE_VALUE_LOG) {
        ScalingForFullBar.FactorForFullBar = 1.0;
    } else {
        ScalingForFullBar.DampenedMaxValueShift8 = 0; // assume auto scaling
    }
    CallsPerPeakDrop = CALLS_PER_PEAK_DROP;
    DampenedValue = 0;
    PeakValue = 0;
//    PeakCallCounter = 0; // can be uninitialized, because the first call always pushes the peak
}

void VUBar::init(uint8_t aBarLength, uint8_t aDampingFactorForValue, uint8_t aFlags, uint16_t aValueForFullBar) {
    BarLength = aBarLength;
    Flags = aFlags;
    if (Flags & FLAG_VU_USE_VALUE_LOG) {
        ScalingForFullBar.FactorForFullBar = (float) BarLength / (log(aValueForFullBar) * FACTOR_FOR_LOG + 1);
    } else {
        ScalingForFullBar.ValueForFullBar = aValueForFullBar;
        Flags = aFlags | FLAG_VU_USE_FIXED_SCALING;
    }
    DampingFactorForValue = aDampingFactorForValue;
    CallsPerPeakDrop = CALLS_PER_PEAK_DROP;
    DampenedValue = 0;
    PeakValue = 0;
//    PeakCallCounter = 0; // can be uninitialized, because the first call always pushes the peak
}

VUBar::~VUBar() {
}

void VUBar::setValueForFullBarForFixedScaling(uint16_t aValueForFullBar) {
    if (Flags & FLAG_VU_USE_VALUE_LOG) {
        ScalingForFullBar.FactorForFullBar = (float) BarLength / (log(aValueForFullBar) * FACTOR_FOR_LOG + 1);
    } else {
        ScalingForFullBar.ValueForFullBar = aValueForFullBar;
        Flags |= FLAG_VU_USE_FIXED_SCALING;
    }
}

void VUBar::setCallsPerPeakDrop(uint8_t aCallsPerPeakDrop) {
    CallsPerPeakDrop = aCallsPerPeakDrop;
}

/*
 * Computes the length of the bar to show and clears / sets the colors of the physical NeoPixel bar.
 * The higher the dampening factor the less agile is the bar.
 * Auto gain / auto scaling is done by using the very high dampened max value as reference for scaling.
 * The scaled value can be used directly or by its logarithms.
 * Scaled output range is from 0 to BarLength
 */
uint8_t VUBar::computeNewSignalHeight(uint16_t aInputValue) {
    uint16_t tNewBarHeight;
    uint16_t tBarLength = BarLength;
    uint16_t tInputValue = aInputValue;

    /*
     * Compute dampened input value
     */
    if (Flags & FLAG_VU_USE_HIGH_RES_DAMPENING) {
        // Since we have low values here, use fixed point to increase resolution
        if (aInputValue > 127) {
            aInputValue = 127;
        }
        tInputValue = aInputValue << 8;
    }
    if (DampingFactorForValue != 0) {
        // rounding costs 26 bytes
//        DampenedValue += (((int16_t) (tInputValue - DampenedValue)) + (1 << (DampingFactorForValue - 1))) >> DampingFactorForValue;
        DampenedValue += ((int16_t) (tInputValue - DampenedValue)) >> DampingFactorForValue;
    } else {
        // take value direct
        DampenedValue = tInputValue;
    }
    if (Flags & FLAG_VU_USE_HIGH_RES_DAMPENING) {
        tNewBarHeight = DampenedValue >> 8;
    } else {
        tNewBarHeight = DampenedValue;
    }

    uint16_t tValueForFullBar;
    if (Flags & FLAG_VU_USE_FIXED_SCALING) {
        tValueForFullBar = ScalingForFullBar.ValueForFullBar;
    } else if (!(Flags & FLAG_VU_USE_VALUE_LOG)) { // Automatic scaling is disabled for logarithmic scaling
        /*
         * Automatic scaling
         * Compute maximum value for automatic gain control. It is dampened by value 8 / factor 256 only while decreasing.
         */
        uint32_t tDampenedMaxValueShift8 = ScalingForFullBar.DampenedMaxValueShift8; // local variable saves space :-)
        uint32_t tInputValueShift8 = (uint32_t) aInputValue << 8;
        if (tDampenedMaxValueShift8 < tInputValueShift8) {
            // attack -> get new maximum directly
            tDampenedMaxValueShift8 = tInputValueShift8;
        } else {
            // decay -> compute new dampened maximum value with high resolution
            // We have unsigned arithmetic here and know that tDampenedMaxValueShift8 - tInputValueShift8 > 0
            tDampenedMaxValueShift8 -= (tDampenedMaxValueShift8 - tInputValueShift8) >> 8; // max is greater than input and last 8 is damping factor not resolution shift
        }

        // Lower clipping at BarLength ((tDampenedMaxValueShift8 >> 8) >= BarLength). This avoids undesired amplification of small values.
        if ((uint16_t) (tDampenedMaxValueShift8 >> 8) < BarLength) {
            tDampenedMaxValueShift8 = (uint32_t) BarLength << 8;
        }
        tValueForFullBar = tDampenedMaxValueShift8 >> 8;
        ScalingForFullBar.DampenedMaxValueShift8 = tDampenedMaxValueShift8;
    }

    /*
     * Scale and calculate bar height based on scale value
     */
    if (tNewBarHeight != 0) {
        if (!(Flags & FLAG_VU_USE_VALUE_LOG)) {
            /*
             * Linear scaling. Results in 0 to BarLength for 0 to tValueForFullBar
             */
            if (tNewBarHeight >= tValueForFullBar) {
                tNewBarHeight = tBarLength; // full bar, also covers tValueForFullBar == 0
            } else {
                tNewBarHeight = ((uint32_t) tBarLength * tNewBarHeight) / tValueForFullBar;
            }
        } else {
            /*
             * Logarithmic scaling.
             * float tSignalValue = (float) tNewBarHeight / tValueForFullBar;
             * For 8 pixel it results in 0, 1 to BarLength for tValueForFullBar/256 (2^8), tValueForFullBar/128 (2^7) to tValueForFullBar/1 (2^0)
             * For 16 pixel it results in 1 to BarLength for tValueForFullBar/32,768 (2^15) to tValueForFullBar
             * We want to have a 1 for value 1 and therefore choose tValueForFullBar as (1 << (barLength - 1)) for log2;
             */
            float tSignalValue = (float) tNewBarHeight / SCALE_VALUE_FOR_LOG_BAR(BarLength); // 0 < tSignalValue <= 1.00

#if defined(LOCAL_TRACE)
            Serial.print(F("In="));
            Serial.print(tNewBarHeight);
            Serial.print(F(" Scaled="));
            Serial.print(tSignalValue, 5);
            Serial.print(F(" log2="));
#endif
            tSignalValue = log(tSignalValue) * FACTOR_FOR_LOG; // tSignalValue = log2(M_LOG2E) <= 0
            // shift up and round with + 0.5 since value is > 0
            int tSignalHeightTemp = ((tSignalValue + tBarLength) * ScalingForFullBar.FactorForFullBar) + 0.5; // tSignalHeightTemp <= tBarLength
            // Clip value
            if (tSignalHeightTemp < 0) {
                tSignalHeightTemp = 0;
            }
            // convert to unsigned
            tNewBarHeight = tSignalHeightTemp;
#if defined(LOCAL_TRACE)
            Serial.print(tSignalValue);
            Serial.print(F(" | "));
            Serial.println(tNewBarHeight);
#endif

        }
    }

// Clip value. 0 is no pixel, tBarLength is all pixels
    if (tNewBarHeight > tBarLength) {
        tNewBarHeight = tBarLength;
    }
#if defined(LOCAL_DEBUG)
    Serial.print(F("InputValue="));
    Serial.print(aInputValue);
    Serial.print(F(" BarHeight="));
    Serial.print(tNewBarHeight);

    if (Flags & FLAG_VU_USE_HIGH_RES_DAMPENING) {
        Serial.print(F(" DampenedValue>>8="));
        Serial.print(DampenedValue >> 8);
    } else {
        Serial.print(F(" DampenedValue="));
        Serial.print(DampenedValue);
    }

    Serial.print(F(" ScaleValue="));
    if ((Flags & FLAG_VU_USE_VALUE_LOG)) {
        Serial.print(ScalingForFullBar.FactorForFullBar);
    } else if ((Flags & FLAG_VU_USE_FIXED_SCALING)) {
        Serial.print(ScalingForFullBar.ValueForFullBar);
    } else {
        Serial.print(ScalingForFullBar.DampenedMaxValueShift8 >> 8);
    }
    Serial.println();
#endif
    return tNewBarHeight;
}

/*
 * Computes new PeakValue
 * Every CallsPerPeakDrop call, make the PeakValue pixels drop by 1. Except when peak was just pushed, then wait for 2 * CallsPerPeakDrop.
 * Do not use millis() instead, because millis() needs more resources (RAM)
 * and in most environments where the VU Meter is used, the millis interrupt / Timer0 is disabled, since it disturbs the analog reading
 */
void VUBar::computeNewPeakValue(uint8_t aSignalHeight) {
    if (!(Flags & FLAG_VU_USE_NO_PEAK)) {
        uint8_t tPeakCallCounter = PeakCallCounter; // local variable saves space :-)
        tPeakCallCounter++;

        if (aSignalHeight >= PeakValue) {
            PeakValue = aSignalHeight; // Keep 'peak' dot at top
            Flags |= FLAG_VU_PEAK_WAS_PUSHED;
            tPeakCallCounter = 0;
        } else {
            /*
             * Every CallsPerPeakDrop call, make the PeakValue pixels drop by 1, except when peak was just pushed, then wait for 2 * CallsPerPeakDrop.
             */
            if (Flags & FLAG_VU_PEAK_WAS_PUSHED) {
                if (tPeakCallCounter >= (CallsPerPeakDrop * 2)) {
                    Flags &= ~FLAG_VU_PEAK_WAS_PUSHED;
                    tPeakCallCounter = 0;
                    PeakValue--;
                }
            } else {
                if (tPeakCallCounter >= CallsPerPeakDrop) {
                    tPeakCallCounter = 0;
                    if (PeakValue > 0) {
                        PeakValue--;
                    }
                }
            }
        }
        PeakCallCounter = tPeakCallCounter;
#if defined(LOCAL_DEBUG)
        Serial.print(F("PeakValue="));
        Serial.print(PeakValue);
        Serial.println();
#endif
    }
}

/*
 * Print values for Arduino Serial Plotter
 * Do not print newline in order to enable concatenation of print calls
 * Use pointer to HardwareSerial in order to avoid linking of the Serial class if this function is not used
 */
void VUBar::print(Print *aSerial) {
    if (Flags & FLAG_VU_USE_HIGH_RES_DAMPENING) {
        aSerial->print(DampenedValue >> 8);
    } else {
        aSerial->print(DampenedValue);
    }
    aSerial->print(' ');
    if (Flags & FLAG_VU_USE_VALUE_LOG) {
        aSerial->print(ScalingForFullBar.FactorForFullBar);
    } else if (Flags & FLAG_VU_USE_FIXED_SCALING) {
        aSerial->print(ScalingForFullBar.ValueForFullBar);
    } else {
        aSerial->print(ScalingForFullBar.DampenedMaxValueShift8 >> 8);
    }
}

#include "LocalDebugLevelEnd.h"
#endif // _VU_BAR_HPP
