/*
 * MatrixSnake.h
 *
 *  SUMMARY
 *  You need to install "Adafruit NeoPixel" library under "Tools -> Manage Libraries..." or "Ctrl+Shift+I" -> use "neoPixel" as filter string
 *  Extension are made to include more patterns and combined patterns and patterns for 8x8 NeoPixel matrix.
 *
 *  Copyright (C) 2018  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
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
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/gpl.html>.
 *
 */

/* Class inheritance diagram
 *                                     ,o--> MatrixNeoPixel \
 * MatrixSnake --> MatrixNeoPatterns  <                      o--> NeoPixel --> Adafruit_NeoPixel
 *                                     `o--> NeoPatterns    /
 */

#ifndef MATRIXSNAKE_H_
#define MATRIXSNAKE_H_

#include "MatrixNeoPatterns.h"

//#define USE_SERIAL_CONTROL // Accepts snake direction commands over the serial line

#define PATTERN_SNAKE 64

#define APPLE_COLOR COLOR32_RED

#define MILLIS_FOR_BUTTON_DEBOUNCING 100

#define FLAG_USE_4_BUTTONS 0x01             // will be set if PinOfUpButton != 0 and up or down button was at least pressed once
#define FLAG_SNAKE_AUTORUN 0x02             // autorun mode - will be set if no left buttons is defined
#define FLAG_SNAKE_SHOW_LENGTH_SCORE 0x20   // signal number shown -> enables delay
#define FLAG_SNAKE_SHOW_END 0x40            // signal end of snake shown -> enables delay
#define SHOW_END_INTERVAL_MILLIS 3000
#define SHOW_NUMBER_INTERVAL_MILLIS 2000

#define TIME_TO_SWITCH_TO_AUTO_MODE_MILLIS 7000
//#define TIME_TO_SWITCH_TO_AUTO_MODE_MILLIS 2000

// stores a 2d position TopLeft coordinates are 0,0
struct position {
    uint8_t x;
    uint8_t y;
};

// stores the initial shape of the snake. SnakeInitialPixels[0] is head of snake
position const SnakeInitialPixels[] = { { 4, 4 }, { 5, 4 }, { 6, 4 }, { 6, 5 } };

// extension of NeoPattern Class approximately 85 Byte / object
class MatrixSnake: public MatrixNeoPatterns {
public:
    MatrixSnake(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry, uint8_t aTypeOfPixel,
            void (*aPatternCompletionCallback)(NeoPatterns*)=NULL);

    void Snake(uint16_t aIntervalMillis, color32_t aColor, uint8_t aPinOfRightButton = 0, uint8_t aPinOfLeftButton = 0,
            uint8_t aPinOfUpButton = 0, uint8_t aPinOfDownButton = 0);

    bool Update(bool doShow = true);

    void SnakeUpdate();
    uint8_t SnakeInputHandler();
    void SnakeEndHandler();
    void showScore();

    void newApple();
    void drawApple();
    void drawSnake();
    void clearResetAndShowSnakeAndNewApple();
    void rotateRight();
    void rotateLeft();
    //
    bool isPositionInArea(position aPositionToCheck);
    uint16_t getIndexOfPositionInSnake(uint8_t aPositionToCheckX, uint8_t aPositionToCheckY);
    uint16_t getIndexOfPositionInSnake(position aPositionToCheck);
    uint16_t getIndexOfPositionInSnakeTail(position aPositionToCheck);
    uint16_t checkDirection(uint8_t aDirectionToCheck);
    //
    bool computeNewHeadPosition(uint8_t aActualDirection, position * aSnakeNewHeadPosition);
    bool moveSnakeAndCheckApple(position tSnakeNewHeadPosition);

    /*
     * internal auto solver functions
     */
    uint8_t findNextDir();
    uint8_t runAndCheckIfAppleCanBeReached();
    uint8_t getNextSnakeDir();

    // The pixel positions of the Snake. Only the positions up until snake_length - 1 are displayed
    // SnakePixelList[0] is head of snake
    position * SnakePixelList;
    uint16_t SnakeLength;

#define AUTOSOLVER_CLOCKWISE_FLAG 0x01 // if set choose directions from 3 to 0
    uint8_t SnakeAutoSolverMode; // The mode for findNextDir()

    // the apple the snake chases after
    position Apple;

    uint16_t HighScore;

    /*
     * Snake input stuff
     */
    uint8_t PinOfRightButton; // if not set (==0), snake runs in autorun mode
    uint8_t PinOfLeftButton;
    uint8_t PinOfUpButton;
    uint8_t PinOfDownButton;
    uint8_t DirectionOfLastButtonPressed;  // for debouncing
    uint32_t MillisOfLastButtonChange; // for debouncing

private:
};

uint8_t computeDirection(position aStartPosition, position aEndPosition);

extern const char sDefaultTickerText[] PROGMEM; // = "I love Neopixel"
extern const char * sTickerTextPtr; // = sDefaultTickerText;
void setMatrixAndSnakePatternsDemoTickerText(const __FlashStringHelper * aTextForTicker);
void MatrixAndSnakePatternsDemo(NeoPatterns * aLedsPtr);

void initSnakeAutorun(MatrixSnake * aLedsPtr, uint16_t aIntervalMillis, color32_t aColor, uint16_t aRepetitions = 1);
uint8_t getNextSnakeDirection(MatrixSnake * aSnake);
void SnakeAutorunCompleteHandler(NeoPatterns * aLedsPtr);

#endif /* MATRIXSNAKE_H_ */

#pragma once

