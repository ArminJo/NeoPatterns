/*
 * ArduinoUtils.h
 *
 *  Created on: 08.11.2018
 *  Copyright (C) 2018  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
 */

#include <stdint.h>

#ifndef ARDUINO_UTILS_H_
#define ARDUINO_UTILS_H_

//void speedTestWith1kCalls(void (*aFunctionUnderTest)(void));

int8_t checkAndTruncateParamValue(int8_t aParam, int8_t aParamMax, int8_t aParamMin);

bool wasButtonJustPressed(uint8_t aButtonPin);
bool wasButtonJustToggled(uint8_t aButtonPin);

void blinkLed(uint8_t aLedPin, uint8_t aNumberOfBlinks, uint16_t aBlinkDelay);

void initINT0();

#endif // ARDUINO_UTILS_H_
