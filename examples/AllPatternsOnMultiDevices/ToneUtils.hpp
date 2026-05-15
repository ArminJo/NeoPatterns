/*
 *  ToneUtils.hpp
 *
 *  Direct hardware output without interrupts of tone on pin 3 or 11 for Uno, Nano etc..
 *  Restricted to prescaler 32 to 256, thus supports only frequencies from 244 Hz to 500 kHz.
 *  Uses 80 bytes instead if 1500 when using Arduino tone().
 *
 *  Copyright (C) 2022-2026  Armin Joachimsmeyer
 *  Email: armin.joachimsmeyer@gmail.com
 *
 *  This file is part of Arduino-Utils https://github.com/ArminJo/Arduino-Utils.
 *
 *  Arduino-Utils is free software: you can redistribute it and/or modify
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

#ifndef _TONE_UTILS_HPP
#define _TONE_UTILS_HPP

#if defined(__AVR__) && defined(OCR2A) && defined(TIMSK2) && defined(TCCR2A) && defined(TCCR2B)

#include <Arduino.h>
#include "digitalWriteFast.h"

void setTimer2PrescalerAndCTCMode(unsigned int aFrequencyHerz){
    /*
     * Linear range only from  03 = %32 to 06 =  %256. 02 is %8 and 07 is %1024
     */
    uint8_t tPrescaler = 0x03; // F_CPU / 32 = 500 kHz
    uint16_t tOCR = (F_CPU / 32) / aFrequencyHerz;
    while (tOCR > 0x100 && tPrescaler <= 0x06) {
        tPrescaler++;
        tOCR >>= 1;
    }
    TCCR2B = tPrescaler;
    OCR2A = tOCR - 1;       // The frequency of the PWM will be (F_CPU / prescaler) frequency divided by (OCR1C value + 1).
}
void toneSimpleAtPin3(unsigned int aFrequencyHerz) {
    pinModeFast(3, OUTPUT);
    TCCR2A |= _BV(COM2B0)| _BV(WGM21);  // Enable direct output at pin 3 and CTC with OCRA as top
    setTimer2PrescalerAndCTCMode(aFrequencyHerz); // must be after TCCR2A = ...
    TIMSK2 = 0;             // Disable interrupts for Arduino tone() function
    OCR2B = OCR2A;          // Must use channel b for output at pin 3
}

/*
 * Small code: 80 Bytes for first call
 * Supports only frequencies from 244 Hz to 500 kHz
 */
void toneSimpleAtPin11(unsigned int aFrequencyHerz) {
    pinModeFast(11, OUTPUT);
    TCCR2A = _BV(COM2A0) | _BV(WGM21);  // Enable direct output at pin 11 and CTC with OCRA as top
    setTimer2PrescalerAndCTCMode(aFrequencyHerz); // must be after TCCR2A = ...
    TIMSK2 = 0;             // Disable interrupts for Arduino tone() function
}


void noToneSimpleAtPin3Or11(){
    TCCR2A = 0; // Disconnect pins and stop timer
}


#endif // defined(__AVR__)
#endif // _TONE_UTILS_HPP
