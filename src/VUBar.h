/*
 * VUBar.h
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

#ifndef _VU_BAR_H
#define _VU_BAR_H

#include <stdint.h>
#include <Print.h>

#define DEFAULT_CALLS_PER_PEAK_DROP    6          // Default value (for 50 ms call delay) to be adapted to real timing or use setCallsPerPeakDrop()

#if !defined(CALLS_PER_PEAK_DROP)
#define CALLS_PER_PEAK_DROP    DEFAULT_CALLS_PER_PEAK_DROP // Default value to be adapted to real timing or use setCallsPerPeakDrop()
#endif

#if !defined(FACTOR_FOR_LOG)
/*
 * The factor to use for the logarithmic scale enabled by flag FLAG_VU_USE_VALUE_LOG
 */
// This gives overflow for bar length > 32 or other log bases. For this case the library must be changed to use float as ValueForFullBar.
#define FACTOR_FOR_LOG                      M_LOG2E   // Results in a log2 function. Values from 1, 2, 4, 8 to 256 for 8 Pixel
#define SCALE_VALUE_FOR_LOG_BAR(barLength)  (1UL << ((barLength) - 1))
//#define FACTOR_FOR_LOG                    1         // Results in a ln (log e) function
//#define SCALE_VALUE_FOR_LOG_BAR(barLength)  (pow(M_E, (barLength) - 1))
//#define FACTOR_FOR_LOG                    M_LOG10E  // Results in a log10 function
//#define SCALE_VALUE_FOR_LOG_BAR(barLength)  (pow(10, (barLength) - 1))
#endif

#define FACTOR_FOR_NO_DAMPENING                 0 // To be used as value for parameter uint8_t aDampingFactorForValue

#define FLAG_VU_SCALED_LINEAR_AUTOMATIC      0x00 // Default behavior
#define FLAG_VU_PEAK_WAS_PUSHED              0x01 // Internal flag for initial (longer) frame delay (FRAMES_PER_PEAK_DROP_INITIALLY) of falling dot
#define FLAG_VU_USE_FIXED_SCALING            0x02 // Use scaling with fixed ValueForFullBar and not scaling with DampenedMaxValue
#define FLAG_VU_USE_HIGH_RES_DAMPENING       0x04 // Use high resolution dampening accumulator. Restricts input to 0 - 127.
#define FLAG_VU_USE_VALUE_LOG                0x10 // Generate logarithmic output
#define FLAG_VU_USE_NO_PEAK                  0x20 // No peak computation return always 0 for peak
#define FLAG_VU_NEOPIXEL_USE_BAR_16_BIT_COLOR_ARRAY 0x40 // For use of NeoPixelVU. Use 16 bit bar color array instead of a 32 bit one.
#define FLAG_VU_NEOPIXEL_USE_BAR_COLOR_ARRAY 0x80 // For use of NeoPixelVU. Use color array instead of a fixed color
#define FLAG_VU_DEFAULT                      FLAG_VU_SCALED_LINEAR_AUTOMATIC

#define FLAG_VU_NEOPIXEL_COLOR_ARRAY_MASK (FLAG_VU_NEOPIXEL_USE_BAR_COLOR_ARRAY | FLAG_VU_NEOPIXEL_USE_BAR_16_BIT_COLOR_ARRAY)

class VUBar {
public:
    VUBar();
    VUBar(uint8_t aBarLength, uint8_t aDampingFactorForValue, uint8_t aFlags);
    VUBar(uint8_t aBarLength, uint8_t aDampingFactorForValue, uint8_t aFlags, uint16_t aValueForFullBar);
    void init(uint8_t aBarLength, uint8_t aDampingFactorForValue, uint8_t aFlags);
    void init(uint8_t aBarLength, uint8_t aDampingFactorForValue, uint8_t aFlags, uint16_t aValueForFullBar);
    ~VUBar();

    void setValueForFullBarForFixedScaling(uint16_t aValueForFullBar);
    void setCallsPerPeakDrop(uint8_t aCallsPerPeakDrop);
    uint8_t computeNewSignalHeight(uint16_t aInputValue);
    void computeNewPeakValue(uint8_t aSignalHeight);
    void print(Print *aSerial);

    bool has32BitColorArray() {
        return (Flags & FLAG_VU_NEOPIXEL_COLOR_ARRAY_MASK) == FLAG_VU_NEOPIXEL_USE_BAR_COLOR_ARRAY;
    }
    bool has16BitColorArray() {
        return (Flags & FLAG_VU_NEOPIXEL_COLOR_ARRAY_MASK) == FLAG_VU_NEOPIXEL_COLOR_ARRAY_MASK;
    }

    uint8_t BarLength;

    uint8_t Flags;

    /*
     * Bar control values
     */
    uint8_t DampingFactorForValue;   // if == 0 then use value direct and not dampened (and scale it with DampenedMaxValue)

    uint16_t DampenedValue; // Low pass dampened value. If no automatic scaling, we have low values here and use value shift 8 to increase resolution
    /*
     * DampenedMaxValueShift8 is maximum input value dampened by factor 256 (shift 8). Only decay is dampened.
     * DampenedMaxValueShift8 is always > BarLength to avoid undesired amplification of small values.
     * Used for automatic scaling / gain control.
     *
     * FactorForFullBar is used only for logarithmic scaling.
     * For FACTOR_FOR_LOG set as M_LOG2E we have:
     * 1.0 is the value for 16 pixel bar to get 1, 2, 3, 4 for input 1, 2, 4, 8. With 2.0 you get 2, 4, 6, 8
     * 1.0 is the value for 8 pixel bar to get 1, 2, 3, 4, 5, 6, 7, 8 for input 1, 2, 4, 8, 16, 32, 64, 128 (i.e. position of highest bit set)
     * M_LOG10E / M_LOG2E (=0.3) is the value to have an effective log10 function, i.e to get 1, 2, 3, 4 for input 10, 100, 1000, 10000
     *
     * ValueForFullBar is used only for fixed scaling. The signal value which results in a full bar. Every value above is clipped.
     */
    union {
        float FactorForFullBar;             // Active if flag FLAG_VU_USE_VALUE_LOG is set
        uint16_t ValueForFullBar;           // Active if flag FLAG_VU_USE_FIXED_SCALING is set
        uint32_t DampenedMaxValueShift8; // Active if flags FLAG_VU_USE_FIXED_SCALING and FLAG_VU_USE_VALUE_LOG are NOT set (default)
    } ScalingForFullBar;

    /*
     * Peak dot control values
     *
     * Peak is the topmost pixel of the bar if value is not decreasing.
     *
     * Every CallsPerPeakDrop call, make the PeakValue pixels drop by 1, except when peak was just pushed, then wait for 2 * CallsPerPeakDrop.
     * Do not use millis() instead, because millis() needs more resources (RAM)
     * and in most environments where the VU Meter is used, the millis interrupt / Timer0 is disabled, since it disturbs the analog reading
     */
    uint8_t CallsPerPeakDrop;   // For delaying dot-falling speed
    uint8_t PeakCallCounter;  // Call counter for delaying dot-falling speed. If peak was changed, PeakCallCounter is known to be 0.
    uint8_t PeakValue;  // current value of falling dot. Range is from 0 to BarLength => PeakValue-1 is the index of the peak pixel.
};

#endif // _VU_BAR_H
