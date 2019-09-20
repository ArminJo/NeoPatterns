/*
 * Colors.h
 *
 *  SUMMARY
 *  Blue Display is an Open Source Android remote Display for Arduino etc.
 *  It receives basic draw requests from Arduino etc. over Bluetooth and renders it.
 *  It also implements basic GUI elements as buttons and sliders.
 *  GUI callback, touch and sensor events are sent back to Arduino.
 *
 *  Copyright (C) 2018  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
 *
 *  This file is part of BlueDisplay https://github.com/ArminJo/android-blue-display.
 *
 *  BlueDisplay is free software: you can redistribute it and/or modify
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

#ifndef COLORS_H_
#define COLORS_H_

#include <stdint.h>

// deprecated
typedef uint16_t Color_t;
// new
typedef uint16_t color16_t;

/*
 * Basic 16 Bit Colors
 */
// RGB to 16 bit 565 schema - 5 red | 6 green | 5 blue
#define COLOR_WHITE     ((color16_t)0xFFFF)
// 01 because 0 is used as flag (e.g. in touch button for default color)
#define COLOR_BLACK     ((color16_t)0X0001)
#define COLOR_RED       ((color16_t)0xF800)
#define COLOR_GREEN     ((color16_t)0X07E0)
#define COLOR_BLUE      ((color16_t)0x001F)
#define COLOR_DARK_BLUE ((color16_t)0x0014)
#define COLOR_YELLOW    ((color16_t)0XFFE0)
#define COLOR_ORANGE    ((color16_t)0XFE00)
#define COLOR_PURPLE   ((color16_t)0xF81F)
#define COLOR_CYAN      ((color16_t)0x07FF)

// If used as background color for char or text, the background will not filled
#define COLOR_NO_BACKGROUND   ((color16_t)0XFFFE)

#define BLUEMASK 0x1F
#define GET_RED(rgb) ((rgb & 0xF800) >> 8)
#define GET_GREEN(rgb) ((rgb & 0x07E0) >> 3)
#define GET_BLUE(rgb) ((rgb & 0x001F) << 3)
#define RGB(r,g,b)   ((color16_t)(((r&0xF8)<<8)|((g&0xFC)<<3)|((b&0xF8)>>3))) //5 red | 6 green | 5 blue
#define COLOR16(r,g,b)   ((color16_t)(((r&0xF8)<<8)|((g&0xFC)<<3)|((b&0xF8)>>3))) //5 red | 6 green | 5 blue


/*
 * 32 Bit Color values
 */
typedef uint32_t color32_t;

// eases constant color declarations
#define COLOR32(r,g,b)   ((color32_t)(((uint32_t)r<<16)|((uint16_t)g<<8)|b)) // return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;

#define RED(color)   ((color >> 16) & 0xFF)
#define GREEN(color) ((color >> 8) & 0xFF)
#define BLUE(color)  (color 0xFF)

#define COLOR32_WHITE          COLOR32(255,255,255)
#define COLOR32_WHITE_HALF     COLOR32(128,128,128) // to reduce power consumption
#define COLOR32_WHITE_QUARTER  COLOR32(64,64,64)    // to reduce power consumption
#define COLOR32_BLACK          COLOR32(0,0,0)
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
#define COLOR32_ORANGE         COLOR32(255,64,0)  // value was visually determined
#define COLOR32_PURPLE         COLOR32(255,0,255)
#define COLOR32_CYAN           COLOR32(0,255,255)

#endif /* COLORS_H_ */

#pragma once

