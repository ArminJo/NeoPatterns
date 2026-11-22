/*
 * Colors.h
 *
 *
 *  Copyright (C) 2018-2026  Armin Joachimsmeyer
 *
 *  This file is part of Arduino-BlueDisplay https://github.com/ArminJo/Arduino-BlueDisplay.
 *  This file is part of android-blue-display https://github.com/ArminJo/android-blue-display.
 *
 *  BlueDisplay is free software: you can redistribute it and/or modify
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
 *
 */

#ifndef _COLORS_H
#define _COLORS_H

#include <stdint.h>

/*
 * Basic 16 Bit Colors
 * RGB to 16 bit 565 schema - 5 red | 6 green | 5 blue
 */
typedef uint16_t color16_t;

#define COLOR16_BLUEMASK 0x1F
#define COLOR16_GET_RED(rgb)    (((rgb) & 0xF800) >> 8)
#define COLOR16_GET_GREEN(rgb)  (((rgb) & 0x07E0) >> 3)
#define COLOR16_GET_BLUE(rgb)   (((rgb) & 0x001F) << 3)
#define COLOR16(r,g,b) ((color16_t)((((r)&0xF8)<<8)|(((g)&0xFC)<<3)|(((b)&0xF8)>>3))) // 5 red | 6 green | 5 blue

#define COLOR16_WHITE     ((color16_t)0xFFFF)
#define COLOR16_ALMOST_WHITE ((color16_t)0xF7DE) // Every color is 1 bit less than maximum value, but we can distinguish it from white

// If used as background color for char or text, the background will not filled
#define COLOR16_NO_BACKGROUND ((color16_t)COLOR16_WHITE - 1) // 0xFFFE No background drawing for text and button (Only-text button)

#define COLOR16_LIGHT_GREY ((color16_t)0x7BEF)
#define COLOR16_GREY      ((color16_t)0x39E7)
#define COLOR16_DARK_GREY ((color16_t)0x18E3)

#define COLOR16_NO_DELETE ((color16_t)0X0001) // 01 is used as flag (e.g. for not delete old chart)
#define COLOR16_BLACK     ((color16_t)0X0000)
#define COLOR16_RED       ((color16_t)0xF800)
#define COLOR16_GREEN     ((color16_t)0X07E0)
#define COLOR16_BLUE      ((color16_t)0x001F)
#define COLOR16_DARK_BLUE ((color16_t)0x0014)
#define COLOR16_YELLOW    ((color16_t)0XFFE0)
#define COLOR16_ORANGE    ((color16_t)0XFE00)
#define COLOR16_PURPLE    ((color16_t)0xF81F)
#define COLOR16_CYAN      ((color16_t)0x07FF)

/*
 * 32 Bit Color values
 */
typedef uint32_t color32_t;

#define COLOR32_GET_WHITE(color) (((color) >> 24) & 0xFF)
#define COLOR32_GET_RED(color)   (((color) >> 16) & 0xFF)
#define COLOR32_GET_GREEN(color) (((color) >> 8) & 0xFF)
#define COLOR32_GET_BLUE(color)  ((color) & 0xFF)
/*
 * Converts a 32 bit (W)RGB color to 16 bit RGB 565. The white part is ignored.
 */
inline color16_t Color32ToColor16(color32_t aColor) {
    return (color16_t) (((aColor >> 8) & 0xF800)    // red:   bits 23..19 -> 15..11
    | ((aColor >> 5) & 0x07E0)    // green: bits 15..10 -> 10..5
            | ((aColor >> 3) & 0x001F));  // blue:  bits  7..3  ->  4..0
}

// Eases constant color declarations but should only be used for constant colors. Otherwise better use Adafruit_NeoPixel::Color() to save program memory
#define COLOR32(r,g,b)  ((color32_t)(((uint32_t)(r)<<16)|((uint16_t)(g)<<8)|(b))) // return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
#define COLOR32_W(r,g,b,w)   ((color32_t)(((uint32_t)(w)<<24)|((uint32_t)(r)<<16)|((uint16_t)(g)<<8)|(b))) // return ((uint32_t)w << 24) |(uint32_t)r << 16) | ((uint32_t)g <<  8) | b;

/*
 * Converts a 16 bit RGB 565 color to 32 bit RGB. White part is 0.
 * The 5 or 6 bits are expanded to 8 bits by copying their upper bits into the lower ones,
 * so COLOR16_WHITE becomes 0x00FFFFFF and COLOR16_BLACK becomes 0.
 */
inline color32_t Color16ToColor32(color16_t aColor) {
    uint8_t tRed = (aColor >> 8) & 0xF8;    // 5 bit
    tRed |= tRed >> 5;
    uint8_t tGreen = (aColor >> 3) & 0xFC;  // 6 bit
    tGreen |= tGreen >> 6;
    uint8_t tBlue = (aColor << 3) & 0xF8;   // 5 bit
    tBlue |= tBlue >> 5;
    return COLOR32(tRed, tGreen, tBlue);
}

#define COLOR32_BLACK          COLOR32(0,0,0)

#define COLOR32_WHITE          COLOR32(255,255,255)
#define COLOR32_SPECIAL        (COLOR32(255,255,255) - 1) // To signal special color treatment e.g. use random color
#define COLOR32_WHITE_HALF     COLOR32(128,128,128) // to reduce power consumption
#define COLOR32_WHITE_QUARTER  COLOR32(64,64,64)    // to reduce power consumption
#define COLOR32_WHITE_EIGHTH   COLOR32(32,32,32)    // to reduce power consumption
#define COLOR32_WHITE_16TH     COLOR32(16,16,16)
#define COLOR32_WHITE_32TH     COLOR32(8,8,8)
#define COLOR32_WHITE_64TH     COLOR32(4,4,4)
#define COLOR32_WHITE_128TH    COLOR32(2,2,2)
#define COLOR32_RED            COLOR32(255,0,0)
#define COLOR32_RED_HALF       COLOR32(128,0,0)
#define COLOR32_RED_QUARTER    COLOR32(64,0,0)
#define COLOR32_GREEN          COLOR32(0,255,0)
#define COLOR32_GREEN_HALF     COLOR32(0,128,0)
#define COLOR32_GREEN_QUARTER  COLOR32(0,64,0)
#define COLOR32_BLUE           COLOR32(0,0,255)
#define COLOR32_BLUE_HALF      COLOR32(0,0,128)
#define COLOR32_BLUE_QUARTER   COLOR32(0,0,64)
#define COLOR32_YELLOW         COLOR32(255,200,0) // value was visually determined
#define COLOR32_YELLOW_HALF    COLOR32(128,100,0)
#define COLOR32_YELLOW_QUARTER COLOR32(64,50,0)
#define COLOR32_ORANGE         COLOR32(255,64,0)  // value was visually determined
#define COLOR32_ORANGE_HALF    COLOR32(128,32,0)
#define COLOR32_ORANGE_QUARTER COLOR32(64,16,0)
#define COLOR32_PURPLE         COLOR32(255,0,255)
#define COLOR32_PURPLE_HALF    COLOR32(128,0,128)
#define COLOR32_PURPLE_QUARTER COLOR32(64,0,64)
#define COLOR32_CYAN           COLOR32(0,255,255)
#define COLOR32_CYAN_HALF      COLOR32(0,128,128)
#define COLOR32_CYAN_QUARTER   COLOR32(0,64,64)

#endif // _COLORS_H
