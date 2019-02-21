/*
 * NeoPixel.h
 *
 *  Created on: 21.02.2019
 *      Author: Armin
 */

#ifndef SRC_LIB_NEOPATTERNS_NEOPIXEL_H_
#define SRC_LIB_NEOPATTERNS_NEOPIXEL_H_

#include "Adafruit_NeoPixel.h"
#include "Colors.h"

#if defined(__AVR_ATmega32U4__)
#define HardwareSerial Serial_
#endif

uint8_t Red(color32_t color);
uint8_t Green(color32_t color);
uint8_t Blue(color32_t color);

class NeoPixel: public Adafruit_NeoPixel {
public:
    NeoPixel(uint16_t aNumberOfPixels, uint8_t aPin, uint8_t aTypeOfPixel);

    // To enable more than one pattern on the same strip
    void setPixelBuffer(uint8_t * aNewPixelBufferPointer);

    /*
     * Extensions to Adafruit_NeoPixel functions
     */
    void begin();
    // Version with error message
    bool begin(HardwareSerial * aSerial);

    void ColorSet(color32_t color);
    uint32_t DimColor(color32_t color);
    void resetBrightnessValue(); // resets internal brightness control value to full to support restoring of patterns while brightening
    uint8_t getBytesPerPixel();
    uint16_t getPixelBufferSize();
    void storePixelBuffer(uint8_t * aPixelBufferPointerDestination);
    void restorePixelBuffer(uint8_t * aPixelBufferPointerSource, bool aResetBrightness = true);

    color32_t addPixelColor(uint16_t aPixelIndex, uint8_t aRed, uint8_t aGreen, uint8_t aBlue);
    // Static functions
    static color32_t Wheel(uint8_t WheelPos);
    static uint8_t gamma5(uint8_t aLinearBrightnessValue);
    static uint8_t gamma5WithSpecialZero(uint8_t aLinearBrightnessValue);
    static color32_t gamma5FromColor(color32_t aLinearBrightnessColor);

    void TestWS2812Resolution();

    uint8_t BytesPerPixel;  // can be 3 or 4

};

#endif /* SRC_LIB_NEOPATTERNS_NEOPIXEL_H_ */
