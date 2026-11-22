/*
 * EasyButtonAtInt01.hpp
 *
 * This file can be directly configured and included in one source.
 * Include EasyButtonAtInt01.h file if you need the declarations in a second source file.
 *
 *  Arduino library for handling push buttons connected between ground and INT0 and / or INT1 pin.
 *  INT0 and INT1 are connected to Pin 2 / 3 on most Arduinos (ATmega328), to PB6 / PA3 on ATtiny167 and on ATtinyX5 we have only INT0 at PB2.
 *  The library is totally based on interrupt.
 *
 *  How debouncing works:
 *  The first level change after a quiet period is accepted immediately.
 *  Every further level change within BUTTON_DEBOUNCING_MILLIS (default 30 ms) is ignored as bouncing, and restarts the debouncing time.
 *  So the button must be quiet for 30 ms before the next change is accepted.
 *  Because the debouncing time is retriggered, a small value is sufficient, even for buttons with long bouncing.
 *  A very short press, whose release falls into the debouncing time, is not completely detected. Call updateButtonState() in your loop to catch up with such a missed release.
 *  To determine the bouncing of your button, use the DebounceTest example. It enables ANALYZE_MAX_BOUNCING_PERIOD, which prints the maximum time between 2 bouncing interrupts.
 *
 *  The first button must always be connected to the INT0 pin!
 *  If 2 buttons are requested, and the second is not connected to the INT1 pin it will then be handled by Pin Change Interrupts.
 *
 *  Interrupts are forced enabled after the call of many functions!
 *
 *  Usage:
 *  #define USE_BUTTON_0
 *  #include "EasyButtonAtInt01.hpp"
 *  EasyButton Button0AtPin2(true);
 *
 *  Copyright (C) 2018-2026  Armin Joachimsmeyer
 *
 *  This file is part of EasyButtonAtInt01 https://github.com/ArminJo/EasyButtonAtInt01.
 *
 *  EasyButtonAtInt01 is free software: you can redistribute it and/or modify
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

/*
 * This library can be configured at compile time by the following options / macros:
 * For more details see: https://github.com/ArminJo/EasyButtonAtInt01#compile-options--macros-for-this-library
 *
 * - USE_BUTTON_0                   Enables code for button at INT0 (pin2 on 328P, PB6 on ATtiny167, PB2 on ATtinyX5).
 * - USE_BUTTON_1                   Enables code for button at INT1 (pin3 on 328P, PA3 on ATtiny167, PCINT0 / PCx for ATtinyX5).
 * - INT1_PIN                       It overrides the usage of pin at the processors INT1 pin. Thus, it is the pin number of the pin for button 1 to use with Pin Change Interrupts.
 * - NO_INITIALIZE_IN_CONSTRUCTOR   Disables the auto initializing in all constructors not having the aIsButtonAtINT0 parameter.
 * - BUTTON_IS_ACTIVE_HIGH          Enable this if your buttons are active high.
 * - USE_ATTACH_INTERRUPT           This forces use of the arduino function attachInterrupt(). It is required if you get the error "multiple definition of __vector_1".
 * - NO_BUTTON_RELEASE_CALLBACK     Disables the code for release callback. This saves 2 bytes RAM and 64 bytes program memory.
 * - BUTTON_DEBOUNCING_MILLIS       With this you can adapt to the characteristic of your button. Default is 30.
 * - ANALYZE_MAX_BOUNCING_PERIOD    Analyze the buttons actual debounce value.
 * - BUTTON_LED_FEEDBACK            This activates LED_BUILTIN as long as button is pressed.
 * - BUTTON_LED_FEEDBACK_PIN        The pin to use for button LED feedback.
 *
 * The macros INT0_PIN and INT1_PIN are set after the include.
 */

#ifndef _EASY_BUTTON_AT_INT01_HPP
#define _EASY_BUTTON_AT_INT01_HPP

#if defined(__AVR__)
#include <Arduino.h>
#include "EasyButtonAtInt01.h"
#include <avr/cpufunc.h> // for _NOP()

/*
 * Usage:
 * #define USE_BUTTON_0  // Enables code for button at INT0 (pin2 on 328P, PB6 on ATtiny167, PB2 on ATtinyX5)
 * #define USE_BUTTON_1  // Enables code for button at INT1 (pin3 on 328P, PA3 on ATtiny167, PCINT0 / PCx for ATtinyX5)
 * #include "EasyButtonAtInt01.hpp"
 * EasyButton Button0AtPin2(true);  // true  -> Button is connected to INT0
 * EasyButton Button0AtPin3(false, &Button3CallbackHandler); // false -> button is not connected to INT0 => connected to INT1
 * ...
 * digitalWrite(LED_BUILTIN, Button0AtPin2.ButtonToggleState); // The value at the first call after first press is true
 * ...
 */

// For external measurement of code timing
//#define MEASURE_EASY_BUTTON_INTERRUPT_TIMING
#if defined(MEASURE_EASY_BUTTON_INTERRUPT_TIMING) || defined(BUTTON_LED_FEEDBACK)
#include "digitalWriteFast.h"
#endif

#if defined(USE_BUTTON_0)
EasyButton *EasyButton::sPointerToButton0ForISR;
#endif
#if defined(USE_BUTTON_1)
EasyButton *EasyButton::sPointerToButton1ForISR;
#endif

// After all includes
#if defined(TRACE)
#define LOCAL_TRACE
#else
//#define LOCAL_TRACE // This enables trace output only for this file
#endif

// The eclipse formatter has problems with // comments in undefined code blocks
// !!! Must be without trailing comment and closed by @formatter:on
// @formatter:off

/*
 * These constructors are deterministic if only one button is enabled
 * If both buttons are enabled, it initializes the 1. button (USE_BUTTON_0),
 * so the second button must be enabled manually or by using a constructor with the parameter aIsButtonAtINT0.
 */
EasyButton::EasyButton() { // @suppress("Class members should be properly initialized")
#if !defined(NO_INITIALIZE_IN_CONSTRUCTOR)
#  if defined(USE_BUTTON_0)
    init(BUTTON_AT_INT0);  // 1. button
#  else
    init(BUTTON_AT_INT1_OR_PCINT); // 2. button
#  endif
#endif
}
/*
 * The same with aButtonPressCallback
 * If both buttons are enabled, it initializes the 1. button (USE_BUTTON_0)
 */
EasyButton::EasyButton(void (*aButtonPressCallback)(bool aButtonToggleState)) { // @suppress("Class members should be properly initialized")
    ButtonPressCallback = aButtonPressCallback;
#if !defined(NO_INITIALIZE_IN_CONSTRUCTOR)
#  if defined(USE_BUTTON_0)
    init(BUTTON_AT_INT0);  // 1. button
#  else
    init(BUTTON_AT_INT1_OR_PCINT); // 2. button
#  endif
#endif
}

#if !defined(NO_BUTTON_RELEASE_CALLBACK)
EasyButton::EasyButton(void (*aButtonPressCallback)(bool aButtonToggleState),
        void (*aButtonReleaseCallback)(bool aButtonToggleState, uint16_t aButtonPressDurationMillis)) { // @suppress("Class members should be properly initialized")
    ButtonPressCallback = aButtonPressCallback;
    ButtonReleaseCallback = aButtonReleaseCallback;
#  if !defined(NO_INITIALIZE_IN_CONSTRUCTOR)
#    if defined(USE_BUTTON_0)
    init(BUTTON_AT_INT0); // 1. button
#    else
    init(BUTTON_AT_INT1_OR_PCINT); // 2. button
#    endif
#  endif
}
#endif // NO_BUTTON_RELEASE_CALLBACK

/**
 * @param aIsButtonAtINT0   true if this button is connected to INT0 i.e. is button 0
 */
EasyButton::EasyButton(bool aIsButtonAtINT0) {
#if !(defined(USE_BUTTON_0) && defined(USE_BUTTON_1))
    (void) aIsButtonAtINT0; // to suppress compiler warnings
#endif
#if defined(USE_BUTTON_0) && not defined(USE_BUTTON_1)
    init(BUTTON_AT_INT0); // 1. button
#elif defined(USE_BUTTON_1) && not defined(USE_BUTTON_0)
    init(BUTTON_AT_INT1_OR_PCINT); // 2. button
#else
    init(aIsButtonAtINT0);
#endif
}

/**
 * @param aIsButtonAtINT0   true if this button is connected to INT0 i.e. is button 0
 */
EasyButton::EasyButton(bool aIsButtonAtINT0, void (*aButtonPressCallback)(bool aButtonToggleState)) {
#if !(defined(USE_BUTTON_0) && defined(USE_BUTTON_1))
    (void) aIsButtonAtINT0; // to suppress compiler warnings
#endif
    ButtonPressCallback = aButtonPressCallback;
#if defined(USE_BUTTON_0) && not defined(USE_BUTTON_1)
    init(BUTTON_AT_INT0); // 1. button
#elif defined(USE_BUTTON_1) && not defined(USE_BUTTON_0)
    init(BUTTON_AT_INT1_OR_PCINT); // 2. button
#else
    init(aIsButtonAtINT0);
#endif
}

#if !defined(NO_BUTTON_RELEASE_CALLBACK)
/**
 * Constructor with callback
 * @param aIsButtonAtINT0   true if this button is connected to INT0 i.e. is button 0
 */
EasyButton::EasyButton(bool aIsButtonAtINT0, void (*aButtonPressCallback)(bool aButtonToggleState),
        void (*aButtonReleaseCallback)(bool aButtonToggleState, uint16_t aButtonPressDurationMillis)) {
#  if !(defined(USE_BUTTON_0) && defined(USE_BUTTON_1))
    (void) aIsButtonAtINT0; // to suppress compiler warnings
#  endif
    ButtonPressCallback = aButtonPressCallback;
    ButtonReleaseCallback = aButtonReleaseCallback;
#  if defined(USE_BUTTON_0) && not defined(USE_BUTTON_1)
    init(BUTTON_AT_INT0); // 1. button
#  elif defined(USE_BUTTON_1) && not defined(USE_BUTTON_0)
    init(BUTTON_AT_INT1_OR_PCINT); // 2. button
#  else
    init(aIsButtonAtINT0);
#  endif
}
#endif // NO_BUTTON_RELEASE_CALLBACK

#if !defined(CHANGE)
#define CHANGE 1 // For those crappy cores which did not define it. Is used as 3. Parameter for attachInterrupt() below.
#endif
/*
 * Sets pin mode to INPUT_PULLUP if not defined(BUTTON_IS_ACTIVE_HIGH) and enables INT0 Interrupt on any logical change.
 * @param aIsButtonAtINT0   true if this button is connected to INT0 i.e. is button 0
 */
void EasyButton::init(bool aIsButtonAtINT0) {
    isButtonAtINT0 = aIsButtonAtINT0;
    // Initialize state before enabling the interrupt, otherwise an early interrupt may be overwritten
    ButtonStateIsActive = false; // true means button pin is LOW
    ButtonToggleState = false;
#if defined(BUTTON_IS_ACTIVE_HIGH)
    LastButtonLevel = false; // Hardware level for fast filtering pin changes - initial state is inactive
#else
    LastButtonLevel = true; // Hardware level for fast filtering pin changes - initial state is inactive
#endif
#if defined(MEASURE_EASY_BUTTON_INTERRUPT_TIMING)
    pinModeFast(INTERRUPT_TIMING_OUTPUT_PIN, OUTPUT);
#endif

#if defined(BUTTON_LED_FEEDBACK)
    pinModeFast(BUTTON_LED_FEEDBACK_PIN, OUTPUT);
#endif

#if defined(USE_BUTTON_0) && not defined(USE_BUTTON_1)
    /*
     * Only button 0 requested
     */
    INT0_DDR_PORT &= ~(_BV(INT0_BIT)); // pinModeFast(2, INPUT_PULLUP);
#  if !defined(BUTTON_IS_ACTIVE_HIGH)
    INT0_OUT_PORT |= _BV(INT0_BIT); // enable pullup
#  endif
    sPointerToButton0ForISR = this;
    /*************************************************
     * Enable interrupt for first button USE_BUTTON_0
     *************************************************/
#  if defined(USE_ATTACH_INTERRUPT)
    attachInterrupt(digitalPinToInterrupt(INT0_PIN), &handleINT0Interrupt, CHANGE);

#  elif defined(USE_INT2_FOR_BUTTON_0) // is set e.g. for Atmega644P
    MY_EICRA |= _BV(ISC20);  // interrupt on any logical change
    MY_EIFR = _BV(INTF2);    // clear interrupt bit. Writing 1 clears the flag
    MY_EIMSK |= _BV(INT2);   // enable interrupt on next change

#  else
    // USE_INT0_FOR_BUTTON_0
    MY_EICRA |= _BV(ISC00);  // interrupt on any logical change, if ISC01 was set we get interrupt on rising edge, what we will notice
    MY_EIFR = _BV(INTF0);    // clear interrupt bit. Writing 1 clears the flag
    MY_EIMSK |= _BV(INT0);   // enable interrupt on next change
#  endif //USE_ATTACH_INTERRUPT

#elif defined(USE_BUTTON_1) && not defined(USE_BUTTON_0)
    /*
     * Only button 1 requested
     */
    INT1_DDR_PORT &= ~(_BV(INT1_BIT));
#  if !defined(BUTTON_IS_ACTIVE_HIGH)
    INT1_OUT_PORT |= _BV(INT1_BIT); // enable pullup
#  endif
    sPointerToButton1ForISR = this;

    /**********************************************************
     * Enable regular interrupt for second button USE_BUTTON_1
     **********************************************************/
#  if (!defined(ISC10)) || ((defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)) && INT1_PIN != 3)
#    if defined(PCICR)
    PCICR |= _BV(PCIE0); // Enable pin change interrupt for port PA0 to PA7
    PCMSK0 = _BV(INT1_BIT);
#    else
    // ATtinyX5 no ISC10 flag existent
    GIMSK |= _BV(PCIE); //PCINT enable, we have only one
    PCMSK = _BV(INT1_BIT);
#    endif
#  elif (INT1_PIN != 3)
    /*
     * ATmega328 (Uno, Nano ) etc. Enable pin change interrupt for port PD4 to PD7 (Arduino pin 4 to 7)
     */
    PCICR |= _BV(PCIE2);
    PCMSK2 = _BV(INT1_BIT);
#  else
#    if defined(USE_ATTACH_INTERRUPT)
    attachInterrupt(digitalPinToInterrupt(INT1_PIN), &handleINT1Interrupt, CHANGE);
#    else
    MY_EICRA |= _BV(ISC10);  // interrupt on any logical change
    MY_EIFR = _BV(INTF1);    // clear interrupt bit
    MY_EIMSK |= _BV(INT1);     // enable interrupt on next change
#    endif //USE_ATTACH_INTERRUPT
#  endif // !defined(ISC10)

#elif defined(USE_BUTTON_0) && defined(USE_BUTTON_1)
    /*
     * Both buttons 0 + 1 requested
     */
    if (isButtonAtINT0) {
        /*************************************************
         * Enable interrupt for first button USE_BUTTON_0
         *************************************************/
        INT0_DDR_PORT &= ~(_BV(INT0_BIT)); // pinModeFast(2, INPUT);
#  if !defined(BUTTON_IS_ACTIVE_HIGH)
        INT0_OUT_PORT |= _BV(INT0_BIT); // enable pullup
#  endif
        sPointerToButton0ForISR = this;
#  if defined(USE_ATTACH_INTERRUPT)
        attachInterrupt(digitalPinToInterrupt(INT0_PIN), &handleINT0Interrupt, CHANGE);
#  else
        MY_EICRA |= _BV(ISC00);    // interrupt on any logical change
        MY_EIFR = _BV(INTF0);      // clear interrupt bit
        MY_EIMSK |= _BV(INT0);     // enable interrupt on next change
#  endif //USE_ATTACH_INTERRUPT
    } else {
        /*
         * Button 1
         * Allow PinChangeInterrupt
         */
        INT1_DDR_PORT &= ~(_BV(INT1_BIT));  // pinModeFast(INT1_BIT, INPUT_PULLUP);
#  if !defined(BUTTON_IS_ACTIVE_HIGH)
        INT1_OUT_PORT |= _BV(INT1_BIT); // enable pullup
#  endif
        sPointerToButton1ForISR = this;

        /************************************************************************
         * Enable Pin Change or regular interrupt for second button USE_BUTTON_1
         ***********************************************************************/
#  if (!defined(ISC10)) || ((defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)) && INT1_PIN != 3)
#    if defined(PCICR)
        /*
         * ATtiny167 + 87. Enable pin change interrupt for port PA0 to PA7
         */
        PCICR |= _BV(PCIE0);
        PCMSK0 = _BV(INT1_BIT);
#    else
        /*
         *ATtinyX5. Enable pin change interrupt for port PB0 to PB5
         */
        GIMSK |= _BV(PCIE); // PCINT enable, we have only one
        PCMSK = _BV(INT1_BIT);
#    endif
#  elif INT1_PIN == 4 || INT1_PIN == 5 || INT1_PIN == 6 || INT1_PIN == 7
    //ATmega328 (Uno, Nano ) etc. Enable pin change interrupt for port PD4 to PD7 (Arduino pin 4 to 7)
        PCICR |= _BV(PCIE2);
        PCMSK2 = _BV(INT1_BIT);
#  else
#    if defined(USE_ATTACH_INTERRUPT)
        attachInterrupt(digitalPinToInterrupt(INT1_PIN), &handleINT1Interrupt, CHANGE);
#    else
        // ATmega328 here
        MY_EICRA |= _BV(ISC10);  // interrupt on any logical change
        MY_EIFR = _BV(INTF1);    // clear interrupt bit
        MY_EIMSK |= _BV(INT1);   // enable interrupt on next change
#    endif //USE_ATTACH_INTERRUPT
#  endif // !defined(ISC10)
    }
#endif
}

/*
 * if NOT defined BUTTON_IS_ACTIVE_HIGH we have negative logic for readButtonState()!
 * In this case BUTTON_IS_ACTIVE (true) means button pin is LOW
 * @return BUTTON_IS_ACTIVE (true) or BUTTON_IS_INACTIVE (false)
 */
bool EasyButton::readButtonState() {
#if defined(USE_BUTTON_0) && not defined(USE_BUTTON_1)
#  if defined(BUTTON_IS_ACTIVE_HIGH)
    return (INT0_IN_PORT & _BV(INT0_BIT));  //  = digitalReadFast(2);
#  else
    return !(INT0_IN_PORT & _BV(INT0_BIT));  //  = digitalReadFast(2);
#  endif

#elif defined(USE_BUTTON_1) && not defined(USE_BUTTON_0)
#  if defined(BUTTON_IS_ACTIVE_HIGH)
    return (INT1_IN_PORT & _BV(INT1_BIT));  //  = digitalReadFast(3);
#  else
    return !(INT1_IN_PORT & _BV(INT1_BIT));  //  = digitalReadFast(3);
#  endif

#elif defined(USE_BUTTON_0) && defined(USE_BUTTON_1)
#  if defined(BUTTON_IS_ACTIVE_HIGH)
    if (isButtonAtINT0) {
        return (INT0_IN_PORT & _BV(INT0_BIT));  //  = digitalReadFast(2);
    } else {
        return (INT1_IN_PORT & _BV(INT1_BIT));  //  = digitalReadFast(3);
    }
#  else
    if (isButtonAtINT0) {
        return !(INT0_IN_PORT & _BV(INT0_BIT));  //  = digitalReadFast(2);
    } else {
        return !(INT1_IN_PORT & _BV(INT1_BIT));  //  = digitalReadFast(3);
    }
#  endif
#else
    return false;
#endif
}

// @formatter:on

bool EasyButton::getStoredButtonState() {
    return ButtonStateIsActive;
}

/*
 * Returns stored state if in debouncing period, otherwise current state of button
 * @return BUTTON_IS_ACTIVE (true) or BUTTON_IS_INACTIVE (false)
 */
bool EasyButton::readDebouncedButtonState() {
    // Check if we are in bouncing period
    noInterrupts();
    bool tInBouncingPeriod = (millis() - ButtonLastInterruptMillis <= BUTTON_DEBOUNCING_MILLIS);
    interrupts();
    if (tInBouncingPeriod) {
        return ButtonStateIsActive;
    }
    return readButtonState();
}
/*
 * Not guarded by noInterrupt() - for internal use
 */
bool EasyButton::_readDebouncedButtonState() {
    // Check if we are in bouncing period
    bool tInBouncingPeriod = (millis() - ButtonLastInterruptMillis <= BUTTON_DEBOUNCING_MILLIS);
    if (tInBouncingPeriod) {
        return ButtonStateIsActive;
    }
    return readButtonState();
}

/*
 * Update button state.
 * can be used to compensate for state change which that was not captured by the ISR
 */
void EasyButton::updateButtonState() {
    noInterrupts();
    bool tButtonStateIsActive = readButtonState();
    if (millis() - ButtonLastInterruptMillis > BUTTON_DEBOUNCING_MILLIS && tButtonStateIsActive != ButtonStateIsActive) {
        /*
         * Here we have no debouncing any more and button state was not stored in ButtonStateIsActive
         * -> force handleINT01Interrupts() to evaluate the current level
         */
#if defined(BUTTON_IS_ACTIVE_HIGH)
        LastButtonLevel = !tButtonStateIsActive; // force handleINT01Interrupts() to evaluate the current level
#else
        LastButtonLevel = tButtonStateIsActive;
#endif
        handleINT01Interrupts();
    }
    interrupts();
}

/*
 * Updates the ButtonPressDurationMillis by polling, since this cannot be done by interrupt.
 */
uint16_t EasyButton::updateButtonPressDuration() {
// noInterrupts() is required, since otherwise we may get wrong results if interrupted during load of long value by button ISR
    noInterrupts();
    if (_readDebouncedButtonState()) {
        interrupts();
        _NOP(); // let a pending button ISR run
        noInterrupts();
        // Button still active -> update ButtonPressDurationMillis
        ButtonPressDurationMillis = millis() - ButtonLastChangeMillis;
    }
    uint16_t tButtonPressDurationMillis = ButtonPressDurationMillis;
    interrupts();
    return tButtonPressDurationMillis;
}

/*
 * Used for long button press recognition, while button is still pressed!
 * !!! Consider to use button release callback handler and check the ButtonPressDurationMillis
 * @param aLongPressThresholdMillis - Default is EASY_BUTTON_LONG_PRESS_DEFAULT_MILLIS which is 400
 * returns EASY_BUTTON_LONG_PRESS_DETECTED, EASY_BUTTON_LONG_PRESS_STILL_POSSIBLE and EASY_BUTTON_LONG_PRESS_ABORT
 */
uint8_t EasyButton::checkForLongPress(uint16_t aLongPressThresholdMillis) {
    uint8_t tReturnValue = EASY_BUTTON_LONG_PRESS_ABORT;
// noInterrupts() is required, since otherwise we may get wrong results if interrupted during processing by button ISR
    noInterrupts();
    if (_readDebouncedButtonState()) {
        interrupts();
        _NOP(); // let a pending button ISR run
        noInterrupts();
        // Button still active -> update current ButtonPressDurationMillis
        ButtonPressDurationMillis = millis() - ButtonLastChangeMillis;
#if defined(LOCAL_TRACE)
        Serial.print(F("LastButtonLevel="));
        Serial.print(LastButtonLevel);
        Serial.print(F(" Duration "));
        Serial.print(ButtonPressDurationMillis);
        Serial.print('=');
        Serial.print(millis());
        Serial.print('-');
        Serial.println(ButtonLastChangeMillis);
#endif
        tReturnValue = EASY_BUTTON_LONG_PRESS_STILL_POSSIBLE; // if not detected yet, you may have a chance at next try

        if (ButtonPressDurationMillis >= aLongPressThresholdMillis) {
            tReturnValue = EASY_BUTTON_LONG_PRESS_DETECTED;
        }
    }
    interrupts();
    return tReturnValue;
}

/*
 * Checks for long press of button
 * Blocks until long press threshold is reached or button was released.
 * !!! Consider to use button release callback handler and check the ButtonPressDurationMillis
 * @param aLongPressThresholdMillis - Default is EASY_BUTTON_LONG_PRESS_DEFAULT_MILLIS which is 400
 * @return true if long press was detected. If button is not pressed, it returns true, if the last press was a long press (since 3.0.0)!
 */
bool EasyButton::checkForLongPressBlocking(uint16_t aLongPressThresholdMillis) {
    /*
     * wait as long as button is pressed shorter than threshold millis.
     */
    while (checkForLongPress(aLongPressThresholdMillis) == EASY_BUTTON_LONG_PRESS_STILL_POSSIBLE) {
        delay(1);
    }
    /*
     * Here button was not pressed or time was greater than threshold.
     * ButtonPressDurationMillis was updated by call to checkForLongPress before
     */
    return (ButtonPressDurationMillis >= aLongPressThresholdMillis);
}

/*
 * Double press detection by computing difference between current (active) timestamp ButtonLastChangeMillis
 * and last release timestamp ButtonReleaseMillis.
 * !!!Works only reliable if called early in ButtonPress callback function!!!
 * !!!Do not call it in ButtonRelease callback function, it makes no sense!!!
 * @param aDoublePressCheckDistanceMillis - default is EASY_BUTTON_DOUBLE_PRESS_DEFAULT_MILLIS which is 400
 * @return true if double press detected.
 */
bool EasyButton::checkForDoublePress(uint16_t aDoublePressCheckDistanceMillis) {
    /*
     * Check if ButtonReleaseMillis is not in initialized state
     * otherwise a single press before aDoublePressCheckDistanceMillis after boot is mistakenly detected as double press
     * Check costs 26 bytes program memory :-(
     */
    if (ButtonReleaseMillis != 0) {
        // because ButtonReleaseMillis is initialized with 0 milliseconds, which is interpreted as the first press happened at the beginning of boot.
        unsigned long tReleaseToPressTimeMillis = ButtonLastChangeMillis - ButtonReleaseMillis;

        // tReleaseToPressTimeMillis != 0 adds 4 bytes, but avoids wrong double press detection if calling this function after release of button
        bool tReturnValue = (tReleaseToPressTimeMillis != 0 && tReleaseToPressTimeMillis <= aDoublePressCheckDistanceMillis);
#if defined(LOCAL_TRACE)
        if (tReturnValue) {
            Serial.print(F("DoublePressCheckMillis="));
            Serial.print(aDoublePressCheckDistanceMillis);
            Serial.print(F(", ReleaseToPressTimeMillis="));
            Serial.println(tReleaseToPressTimeMillis);
        }
#endif
        return tReturnValue;
    }
    return false;
}

/*
 * Checks if button was not pressed in the last aTimeoutMillis
 * Can be used to recognize timeout for user button actions
 * @return true if timeout reached: false if last button release was before aTimeoutMillis
 */
bool EasyButton::checkForButtonNotPressedTime(uint16_t aTimeoutMillis) {
// noInterrupts() is required, since otherwise we may get wrong results if interrupted during load of long value by button ISR
    noInterrupts();
    unsigned long tButtonReleaseMillis = ButtonReleaseMillis;
    interrupts();
    return (millis() - tButtonReleaseMillis >= aTimeoutMillis);
}

/*
 * 1. Read button pin level and invert logic level since we have negative logic because of using pullups.
 * 2. Check for bouncing - state change during debounce period. We need millis() to be enabled to run in the background.
 * 3. Check for spikes - interrupts but no level change.
 * 4. Process valid button state change. If callback requested, call callback routine, get button pin level again and handle if button was released in the meantime.
 */
void EasyButton::handleINT01Interrupts() {
// Read button value
    bool tCurrentButtonStateIsActive;

    /*
     * 1. Read button state
     * This is faster than readButtonState();
     */
#if defined(USE_BUTTON_0) && not defined(USE_BUTTON_1)
    tCurrentButtonStateIsActive = INT0_IN_PORT & _BV(INT0_BIT);  //  = digitalReadFast(2);

#elif defined(USE_BUTTON_1) && not defined(USE_BUTTON_0)
    tCurrentButtonStateIsActive = INT1_IN_PORT & _BV(INT1_BIT);  //  = digitalReadFast(3);

#elif defined(USE_BUTTON_0) && defined(USE_BUTTON_1)
    if (isButtonAtINT0) {
        tCurrentButtonStateIsActive = INT0_IN_PORT & _BV(INT0_BIT);  //  = digitalReadFast(2);
    } else {
        tCurrentButtonStateIsActive = INT1_IN_PORT & _BV(INT1_BIT);  //  = digitalReadFast(3);
    }
#endif

    if (LastButtonLevel == tCurrentButtonStateIsActive) {
        /*
         * Fast return, if we detect a missing interrupt / state has not changed, which may happen at really bad buttons.
         * This also allow us to call this program at any time from main or after callback
         */
        return;
    }
    /*
     * Here we know, that pin level has changed
     */
    LastButtonLevel = tCurrentButtonStateIsActive;

    // handle active low / high button
#if !defined(BUTTON_IS_ACTIVE_HIGH)
    tCurrentButtonStateIsActive = !tCurrentButtonStateIsActive; // negative logic for tCurrentButtonStateIsActive! true means button pin is LOW
#endif

#if defined(LOCAL_TRACE)
// This prints show a long line of adjacent ones or zeros, which means, that we miss a lot of the bouncing interrupts at least for LOCAL_TRACE
    Serial.print(tCurrentButtonStateIsActive);
#endif

    unsigned long tMillis = millis();
    unsigned long tDeltaMillis = tMillis - ButtonLastInterruptMillis;
    ButtonLastInterruptMillis = tMillis; // rearm dead time
    /*
     * 2. Check for bouncing - state change during debounce period
     */
    if (tDeltaMillis <= BUTTON_DEBOUNCING_MILLIS) {
        /*
         * Here we have an interrupt during dead time of BUTTON_DEBOUNCING_MILLIS
         * Button is bouncing, ignore and wait for next interrupt
         * Therefore the release of a very short press is not detected.
         */
#if defined(ANALYZE_MAX_BOUNCING_PERIOD)
        if (MaxBouncingPeriodMillis < tDeltaMillis) {
            MaxBouncingPeriodMillis = tDeltaMillis;
            Serial.print(F("MaxBouncingPeriod="));
            Serial.println(MaxBouncingPeriodMillis);
        }
#endif

    } else if (tCurrentButtonStateIsActive != ButtonStateIsActive
            || (tCurrentButtonStateIsActive && (tMillis - ButtonLastChangeMillis) > (4 * BUTTON_DEBOUNCING_MILLIS))) {
        /*
         * Here we are at first interrupt or after debouncing period
         * or we had a very short press whose release was not detected, but now enough time is gone to accept a new first press
         * We always ignore very short releases!
         */
#if defined(LOCAL_TRACE)
        if (tCurrentButtonStateIsActive == ButtonStateIsActive) {
            Serial.print(F("Last missing release simulated"));
        }
#endif
        uint16_t tPressDurationMillis = tMillis - ButtonLastChangeMillis; // before ButtonLastChangeMillis is overwritten
        ButtonLastChangeMillis = tMillis;
        ButtonStateIsActive = tCurrentButtonStateIsActive;
        ButtonStateHasJustChanged = true;
#if defined(LOCAL_TRACE)
        Serial.print(F("\r\n->")); // because here we are the first after a break
        Serial.print(tCurrentButtonStateIsActive);
        Serial.print(' ');
#endif
        if (tCurrentButtonStateIsActive) {
            /*
             * Button pressed
             */
            ButtonToggleState = !ButtonToggleState;
#if defined(BUTTON_LED_FEEDBACK)
            digitalWriteFast(BUTTON_LED_FEEDBACK_PIN, HIGH);
#endif
            if (ButtonPressCallback != nullptr) {
                /*
                 * Call callback function.
                 * interrupts() is required if callback function needs more time to allow millis() to proceed.
                 * Otherwise we may see bouncing instead of button release followed by spike instead of button press
                 */
                interrupts();
                ButtonPressCallback(ButtonToggleState);
                /*
                 * Check button again since it may have changed state while processing callback function
                 */
                noInterrupts();
                handleINT01Interrupts(); // call to maintain state
            }
        } else {
            /*
             * Button release
             */
            ButtonPressDurationMillis = tPressDurationMillis;
            ButtonReleaseMillis = tMillis;
#if defined(BUTTON_LED_FEEDBACK)
            digitalWriteFast(BUTTON_LED_FEEDBACK_PIN, LOW);
#endif
#if !defined(NO_BUTTON_RELEASE_CALLBACK)
            if (ButtonReleaseCallback != nullptr) {
                /*
                 * Call callback function.
                 * interrupts() is required if callback function needs more time to allow millis() to proceed.
                 */
                interrupts();
                ButtonReleaseCallback(ButtonToggleState, ButtonPressDurationMillis);
                /*
                 * Check button state again since it may be changed while processing callback function
                 */
                noInterrupts();
                handleINT01Interrupts(); // call to maintain state
            }
#endif
        }

    } // after debouncing period
}

// end of class definitions

/*
 * This functions are weak and can be replaced by your own code
 */
#if defined(USE_BUTTON_0)
void handleINT0Interrupt() {
    EasyButton::sPointerToButton0ForISR->handleINT01Interrupts();
}
#endif

#if defined(USE_BUTTON_1)
void handleINT1Interrupt() {
    EasyButton::sPointerToButton1ForISR->handleINT01Interrupts();
}
#endif

#if !defined(USE_ATTACH_INTERRUPT)
#  if defined(USE_INT2_FOR_BUTTON_0)
ISR(INT2_vect) {
#    if defined(MEASURE_EASY_BUTTON_INTERRUPT_TIMING)
    digitalWriteFast(INTERRUPT_TIMING_OUTPUT_PIN, HIGH);
#    endif
    handleINT0Interrupt();
#    if defined(MEASURE_EASY_BUTTON_INTERRUPT_TIMING)
    digitalWriteFast(INTERRUPT_TIMING_OUTPUT_PIN, LOW);
#    endif
}
#  elif defined(USE_BUTTON_0)
ISR(INT0_vect) {
#    if defined(MEASURE_EASY_BUTTON_INTERRUPT_TIMING)
    digitalWriteFast(INTERRUPT_TIMING_OUTPUT_PIN, HIGH);
#    endif
    handleINT0Interrupt();
#    if defined(MEASURE_EASY_BUTTON_INTERRUPT_TIMING)
    digitalWriteFast(INTERRUPT_TIMING_OUTPUT_PIN, LOW);
#    endif
}
#  endif

#  if defined(USE_BUTTON_1)
// First check, if we must use Pin Change Interrupt PCINT
#    if (!defined(ISC10)) || ((defined(__AVR_ATtiny87__) || defined(__AVR_ATtiny167__)) && INT1_PIN != 3)
// on ATtinyX5 we do not have a INT1_vect (ISC10) -> use the PCINT0_vect
// if default INT1 pin on X7 is not chosen -> use the PCINT0_vect
#      if !defined(INTENTIONALLY_USE_PCI0_FOR_BUTTON1) && !(defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)) // on X5 we MUST use PCINT0
#warning You are using PCINT0 interrupt for button 1
#      endif
ISR(PCINT0_vect)
#    elif INT1_PIN == 4 || INT1_PIN == 5 || INT1_PIN == 6 || INT1_PIN == 7
// PCINT for ATmega328 Arduino pins 4 to 7
ISR(PCINT2_vect)
#    else
ISR(INT1_vect) // No Pin Change Interrupt required by pin selection -> use default INT1 :-)
#    endif
{
#    if defined(MEASURE_EASY_BUTTON_INTERRUPT_TIMING)
    digitalWriteFast(INTERRUPT_TIMING_OUTPUT_PIN, HIGH);
#    endif
    handleINT1Interrupt();
#    if defined(MEASURE_EASY_BUTTON_INTERRUPT_TIMING)
    digitalWriteFast(INTERRUPT_TIMING_OUTPUT_PIN, LOW);
#    endif
}
#  endif
#endif // not defined(USE_ATTACH_INTERRUPT)

#if defined(LOCAL_TRACE)
#undef LOCAL_TRACE
#endif

#endif // defined(__AVR__)
#endif // _EASY_BUTTON_AT_INT01_HPP
