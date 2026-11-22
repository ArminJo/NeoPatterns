/*
 *  ThreeVUBars.ino
 *
 *  Reads value from analog input A2 and shows it or generate an increasing and decreasing value
 *  the dampening factor increases on every loop from 0 to 4
 *  Bar 1 uses an GreenYellowRed array for bar color.
 *  Bar 2 has a blue bar and peak pixel with the GreenYellowRed color schema
 *  Bar 3 has a green bar.
 *
 *  Copyright (C) 2021-2026  Armin Joachimsmeyer
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

#include <Arduino.h>

//#define TRACE
//#define DEBUG
//#define PEAK_PIXEL_COLOR      COLOR32_RED
#define PEAK_PIXEL_COLOR        COLOR32_PEAK_RYG // if color is COLOR32_PEAK_RYG, then use Red Yellow Green color schema for peak pixel
#define CALLS_PER_PEAK_DROP     2                // Fast Peak drop
#include "NeoPixelVU.hpp"

#define VERSION_EXAMPLE "3.0"


#define NUMBER_OF_PIXELS_ON_BAR  16  // Number of neopixels on strip
#define VU_BAR_1_PIN    3
#define VU_BAR_2_PIN    4
#define VU_BAR_3_PIN    5
#define VU_BAR_POTENTIOMETER_INPUT_PIN     A2
#define VU_BAR_POTENTIOMETER_ENABLE_PIN    12 // if low use value at A2 otherwise generate demo values

/*
 * The central NeoPixel object used for every bar, since there is no persistence needed for the bar pixel content
 */
NeoPixel LEDBar = NeoPixel(NUMBER_OF_PIXELS_ON_BAR, VU_BAR_1_PIN, NEO_GRB + NEO_KHZ800);

color32_t BarColorArray[NUMBER_OF_PIXELS_ON_BAR];
// Use the same NeoPixel object for all bars, since there is no persistence needed for the bar pixel content
NeoPixelVU VUBar1(VU_BAR_1_PIN, &LEDBar, FACTOR_FOR_NO_DAMPENING, FLAG_VU_USE_NO_PEAK | FLAG_VU_USE_HIGH_RES_DAMPENING,
        BarColorArray); // high resolution, because of small input values
NeoPixelVU VUBar2(VU_BAR_2_PIN, &LEDBar, FACTOR_FOR_NO_DAMPENING, FLAG_VU_SCALED_LINEAR_AUTOMATIC, COLOR32_BLUE_QUARTER);
NeoPixelVU VUBar3(VU_BAR_3_PIN, &LEDBar, FACTOR_FOR_NO_DAMPENING, FLAG_VU_USE_NO_PEAK | FLAG_VU_USE_VALUE_LOG,
COLOR32_GREEN_QUARTER);

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(VU_BAR_POTENTIOMETER_ENABLE_PIN, INPUT_PULLUP);

//    initStackFreeMeasurement();

    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_USB) || defined(SERIAL_PORT_USBVIRTUAL)
    delay(2000); // To be able to connect Serial monitor after reset and before first printout
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ "\r\nVersion " VERSION_EXAMPLE " from " __DATE__));

    // initialize the central NeoPixel object
    LEDBar.begin();

//    printRAMAndStackInfo(&Serial);

//// Rainbow, in order to see all colors
//    LEDBar.fillWithRainbow(0);
//    LEDBar.show();
//    delay(2000);

    /*
     * initialize bar colors
     * Use only one of the bars, since they all share the same BarColorArray
     */
//    VUBar1.initBarColorWheel();
//    VUBar1.initBarGreenToRed();
    VUBar1.initBarColorRYG();

    delay(300); // to avoid partial patterns at power up
}

void loop() {

    if (digitalRead(VU_BAR_POTENTIOMETER_ENABLE_PIN) == LOW) {
        uint16_t tAnalogValue = analogRead(VU_BAR_POTENTIOMETER_INPUT_PIN);
        if (tAnalogValue <= 20) {
            tAnalogValue = 0;
        } else {
            tAnalogValue -= 20;
        }
        tAnalogValue = tAnalogValue * 1023L / 700; // scale it for 3.3V max and 20 LSB offset
        VUBar1.setBarAndPeakAndShow(tAnalogValue / (1023 / 16));
        VUBar2.setBarAndPeakAndShow(tAnalogValue);
        VUBar3.setBarAndPeakAndShow(tAnalogValue);
        Serial.print(F("Analog value="));
        Serial.println(tAnalogValue);
        delay(100);

    } else {
        /*
         * Bar 1 increase and decrease
         * Bar 2 1. loop full bar while increasing, because of auto scaling
         */
        uint32_t tMask = 1;
        for (int i = 0; i <= NUMBER_OF_PIXELS_ON_BAR; ++i) {
            VUBar1.setBarAndPeakAndShow(i);
            VUBar2.setBarAndPeakAndShow(i * 16);
            VUBar3.setBarAndPeakAndShow(tMask/2);
#if defined(DEBUG)
            Serial.println();
#endif
            tMask <<= 1;
            delay(300);
        }
        delay(1000);
        tMask = 0x8000;
        for (int i = NUMBER_OF_PIXELS_ON_BAR; i >= 0; --i) {
            VUBar1.setBarAndPeakAndShow(i);
            VUBar2.setBarAndPeakAndShow(i * 16);
            VUBar3.setBarAndPeakAndShow(tMask);
#if defined(DEBUG)
            Serial.println();
#endif
            tMask >>= 1;
            delay(300);
        }
//    VUBar3.setBarAndPeakAndShow(0);

        /*
         * Increase dampening factor
         */
        VUBar1.DampingFactorForValue++;
        VUBar2.DampingFactorForValue++;
        if (VUBar1.DampingFactorForValue == 5) {
            Serial.println(F("Clear bars and wait for peak to decrease"));
            for (int i = 0; i <= 20; ++i) {
                VUBar1.setBarAndPeakAndShow(0);
                VUBar2.setBarAndPeakAndShow(0);
                delay(100);
            }
            VUBar1.DampingFactorForValue = 0;
            VUBar2.DampingFactorForValue = 0;
        }
        Serial.print(F("Set dampening factor to "));
        Serial.println(VUBar1.DampingFactorForValue);
        delay(1000);
    }
    Serial.println();
}
