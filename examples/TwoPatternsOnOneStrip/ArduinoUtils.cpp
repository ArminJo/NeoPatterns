/*
 *  ArduinoUtils.cpp
 *
 *  - Simple Blink LED function and a non blocking version
 *
 *  Copyright (C) 2016  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/gpl.html>.
 *
 */

#include <Arduino.h>
#include "ArduinoUtils.h"

/*
 * Function for speedTest
 * calling a function consisting of just __asm__ volatile ("nop"); gives 0 to 1 micro second
 * Use of Serial. makes it incompatible with BlueDisplay library.
 */
//void speedTestWith1kCalls(void (*aFunctionUnderTest)(void)) {
//    uint32_t tMillisStart = millis();
//    for (uint8_t i = 0; i < 100; ++i) {
//        // unroll 10 times
//        aFunctionUnderTest();
//        aFunctionUnderTest();
//        aFunctionUnderTest();
//        aFunctionUnderTest();
//        aFunctionUnderTest();
//        aFunctionUnderTest();
//        aFunctionUnderTest();
//        aFunctionUnderTest();
//        aFunctionUnderTest();
//        aFunctionUnderTest();
//    }
//    uint32_t tMillisNeeded = millis() - tMillisStart;
//    Serial.print(F("Function call takes "));
//    if (tMillisNeeded > 1000000) {
//        Serial.print(tMillisNeeded / 1000);
//        Serial.print(",");
//        Serial.print((tMillisNeeded % 1000) / 100);
//        Serial.print(F(" milli"));
//    } else {
//        Serial.print(tMillisNeeded);
//        Serial.print(F(" micro"));
//    }
//    Serial.println(F(" seconds."));
//}

/*
 * Check if button input has just changed from HIGH to LOW
 */
bool wasButtonJustPressed(uint8_t aButtonPin) {
    static uint8_t sOldButtonState = HIGH;

    uint8_t tNewButtonState = digitalRead(aButtonPin);
    if (tNewButtonState == LOW) {
        if (sOldButtonState == HIGH) {
            sOldButtonState = LOW;
            return true;
        }
    } else {
        if (sOldButtonState == LOW) {
            /*
             * changed from LOW to HIGH which is known for heavily bouncing, so debounce and read again
             */
            delay(80);
            tNewButtonState = digitalRead(aButtonPin);
            if (tNewButtonState == HIGH) {
                // still high so it seems to be a valid transition
                sOldButtonState = HIGH;
            }
        }
    }
    return false;
}

/*
 * Check if button input has just changed from HIGH to LOW or back
 */
bool wasButtonJustToggled(uint8_t aButtonPin) {
    static uint8_t sOldButtonState = HIGH;

    uint8_t tNewButtonState = digitalRead(aButtonPin);
    if (tNewButtonState == LOW) {
        if (sOldButtonState == HIGH) {
            sOldButtonState = LOW;
            return true;
        }
    } else {
        if (sOldButtonState == LOW) {
            /*
             * changed from LOW to HIGH which is known for heavily bouncing, so debounce and read again
             */
            delay(80);
            tNewButtonState = digitalRead(aButtonPin);
            if (tNewButtonState == HIGH) {
                // still high so it seems to be a valid transition
                sOldButtonState = HIGH;
                return true;
            }
        }
    }
    return false;
}

/*
 * Simple blinkLed function
 */
void blinkLed(uint8_t aLedPin, uint8_t aNumberOfBlinks, uint16_t aBlinkDelay) {
    for (int i = 0; i < aNumberOfBlinks; i++) {
        digitalWrite(aLedPin, HIGH);
        delay(aBlinkDelay);
        digitalWrite(aLedPin, LOW);
        delay(aBlinkDelay);
    }
}
