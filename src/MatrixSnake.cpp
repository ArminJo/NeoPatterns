/*
 * MatrixSnake.cpp
 *
 * Implements a snake game on NeoPixel matrix.
 * High-Score is reset if both RightButton and Left Button are pressed at start of game by calling Snake().
 *
 *  You need to install "Adafruit NeoPixel" library under "Tools -> Manage Libraries..." or "Ctrl+Shift+I" -> use "neoPixel" as filter string.
 *
 *  SUMMARY
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

#include <Arduino.h>

// Output information on Serial must be defined before #include "MatrixSnake.h"
//#define TRACE
//#define DEBUG
//#define INFO
//#define WARN
//#define ERROR

#include "MatrixSnake.h"

#include <stdlib.h>         // for utoa() etc.

#if defined(__AVR__)
EEMEM uint16_t HighScoreEEPROM; // is reset if both right and left button are pressed at startup.
#endif

MatrixSnake::MatrixSnake() : // @suppress("Class members should be properly initialized")
        NeoPixel(), MatrixNeoPatterns() {
}

// Constructor - calls base-class constructor to initialize strip
MatrixSnake::MatrixSnake(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry, uint8_t aTypeOfPixel, // @suppress("Class members should be properly initialized")
        void (*aPatternCompletionCallback)(NeoPatterns*)) :
        NeoPixel(aColumns * aRows, aPin, aTypeOfPixel), MatrixNeoPatterns(aColumns, aRows, aPin, aMatrixGeometry, aTypeOfPixel,
                aPatternCompletionCallback) {
}

bool MatrixSnake::init(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry, uint8_t aTypeOfPixel,
        void (*aPatternCompletionCallback)(NeoPatterns*)) {
    NeoPixel::init(aColumns * aRows, aPin, aTypeOfPixel);
    return MatrixNeoPatterns::init(aColumns, aRows, aPin, aMatrixGeometry, aTypeOfPixel, aPatternCompletionCallback);
}

/*
 * Update the pattern.
 * Returns true if update has really happened in order to give the caller a chance to manually change parameters after each update (like color etc.)
 * Calls snake input handler and after each interval calls snake update routine or MatrixNeoPatterns::Update().
 */
bool MatrixSnake::update() {

    if (ActivePattern == PATTERN_SNAKE && !(PatternFlags & FLAG_SNAKE_AUTORUN)) {
        // do it on every call in order not to miss any button press
        SnakeInputHandler();
    }

    if ((millis() - lastUpdate) > Interval) {
        // time to update
        if (ActivePattern == PATTERN_SNAKE) {
            SnakeUpdate();
            show();
            // remember last time of update
            lastUpdate = millis();
        } else {
            MatrixNeoPatterns::update();
        }
        return true;
    }
    return false;
}

// moves the goal to a new position
void MatrixSnake::newApple() {
// clear old apple
    setMatrixPixelColor(Apple.x, Apple.y, COLOR32_BLACK);

    position tNewApplePosition;
// get new random position until the position is not on the snake tail and different from the current apple position
    do {
        tNewApplePosition.x = random(Columns - 1);
        tNewApplePosition.y = random(Rows - 1);
    } while ((getIndexOfPositionInSnake(tNewApplePosition) != SnakeLength)
            || (tNewApplePosition.x == Apple.x && tNewApplePosition.y == Apple.y));
    Apple = tNewApplePosition;

#ifdef INFO
    Serial.print(F("New apple("));
    Serial.print(tNewApplePosition.x);
    Serial.print(',');
    Serial.print(tNewApplePosition.y);
    Serial.println(')');
#endif
}

void MatrixSnake::rotateLeft() {
    Direction = (Direction + 1) % NUMBER_OF_DIRECTIONS;
}

void MatrixSnake::rotateRight() {
    Direction--;
    if (Direction < 0) {
        Direction = 3;
    }
}

// Resets the game area, init snake to start position and gets new apple
void MatrixSnake::resetAndClearAndShowSnakeAndNewApple() {
#ifdef DEBUG
    Serial.println(F("Reset snake"));
#endif

    SnakeLength = sizeof(SnakeInitialPixels) / sizeof(position);
    Index = 0; // start value
    Direction = DIRECTION_NONE; // signal start condition

    PatternFlags &= ~FLAG_SNAKE_SHOW_LENGTH_SCORE;

// Clear all pixels
    clear();
// sets the beginning of the snake to init_snake
    memcpy(SnakePixelList, SnakeInitialPixels, sizeof(SnakeInitialPixels));
    newApple();
    drawApple();
    drawSnake();
}

/*
 * IF PinOfUpButton is 0, then snake runs in 2 button mode.
 * if aPinOfRightButton is 0, then snake starts in autorun mode.
 */
void MatrixSnake::Snake(uint16_t aIntervalMillis, color32_t aColor, uint8_t aPinOfRightButton, uint8_t aPinOfLeftButton,
        uint8_t aPinOfUpButton, uint8_t aPinOfDownButton) {
    ActivePattern = PATTERN_SNAKE;
    Interval = aIntervalMillis;
    Color1 = aColor;
    PinOfRightButton = aPinOfRightButton;
    PinOfLeftButton = aPinOfLeftButton;
    PinOfUpButton = aPinOfUpButton;
    PinOfDownButton = aPinOfDownButton;

// just to be sure
    if (SnakePixelList != NULL) {
        free(SnakePixelList);
    }
    SnakePixelList = new position[Rows * Columns];

#if defined(__AVR__)
    /*
     * Read high score from EEPROM / reset high score
     */
    HighScore = eeprom_read_word(&HighScoreEEPROM);
    if ((digitalRead(aPinOfRightButton) == LOW && digitalRead(aPinOfLeftButton) == LOW) || HighScore == 0xFFFF) {
#ifdef INFO
        Serial.print(F("Reset High Score"));
#endif
        eeprom_write_word(&HighScoreEEPROM, 0);
        HighScore = 0;
    }
#else
    HighScore = 0;
#endif

#ifdef INFO
    Serial.print(F("Starting Snake game with refresh interval="));
    Serial.print(aIntervalMillis);
    Serial.print(F("ms. High Score="));
    Serial.println(HighScore);
#endif

    resetAndClearAndShowSnakeAndNewApple();

    if (PinOfRightButton != 0) {
        pinMode(aPinOfRightButton, INPUT_PULLUP);
        pinMode(aPinOfLeftButton, INPUT_PULLUP);
        if (PinOfUpButton != 0) {
            pinMode(aPinOfUpButton, INPUT_PULLUP);
            pinMode(aPinOfDownButton, INPUT_PULLUP);
        }
        PatternFlags = 0; // always start with 2 button mode. 4 button mode is detected if up or down button are pressed
    } else {
        PatternFlags = FLAG_SNAKE_AUTORUN; // start in autorun mode
    }
}

void MatrixSnake::drawApple() {
    setMatrixPixelColor(Apple.x, Apple.y, APPLE_COLOR);
}

void MatrixSnake::drawSnake() {

    for (uint16_t i = 0; i < SnakeLength; i++) {
        position tPosition = SnakePixelList[i];
        // linear color interpolation over the body of the snake between tail and head
        uint8_t tBrightness = ((SnakeLength - i) * 255) / SnakeLength;
        color32_t tDimmedColor = dimColorWithGamma32(Color1, tBrightness, true);
        setMatrixPixelColor(tPosition.x, tPosition.y, tDimmedColor);
    }
// set snake head color
    setMatrixPixelColor(SnakePixelList[0].x, SnakePixelList[0].y, COLOR32_GREEN);
}

/*
 * Called at every call of update() in order not to miss any button press
 * Check button presses and updates direction
 */
void MatrixSnake::SnakeInputHandler() {

#ifdef INFO
    uint8_t tOldPatternFlags = PatternFlags;
#endif
#ifdef DEBUG
    uint8_t tOldDirection = Direction;
#endif
    if (PinOfUpButton != 0) {
        /*
         * First check up and down button direct input, no debouncing needed
         */
        if (digitalRead(PinOfUpButton) == LOW) {
            Direction = DIRECTION_UP;
            PatternFlags = FLAG_USE_4_BUTTONS;
        } else if (digitalRead(PinOfDownButton) == LOW) {
            Direction = DIRECTION_DOWN;
            PatternFlags = FLAG_USE_4_BUTTONS;
        }
    }

#ifdef INFO
    if (tOldPatternFlags != PatternFlags) {
        Serial.println(F("4 Button mode detected"));
    }
#endif

    if (PatternFlags & FLAG_USE_4_BUTTONS) {
        /*
         * 4 buttons direct direction input
         */
        if (digitalRead(PinOfRightButton) == LOW) {
            Direction = DIRECTION_RIGHT;
        } else if (digitalRead(PinOfLeftButton) == LOW) {
            Direction = DIRECTION_LEFT;
        }
    } else {
        /*
         * 2 buttons turn direction input, debouncing required to avoid multiple turns
         */
        uint16_t tMillisSinceLastButtonChanged = millis() - MillisOfLastButtonChange;
        if (tMillisSinceLastButtonChanged > MILLIS_FOR_BUTTON_DEBOUNCING) {

            if (DirectionOfLastButtonPressed == DIRECTION_NONE) {
                /*
                 * Check if button pressed
                 */
                if (digitalRead(PinOfRightButton) == LOW) {
                    if (Index == 0) {
                        //First button push in new game
                        Direction = DIRECTION_RIGHT;
                    } else {
                        rotateRight();
                    }
                    DirectionOfLastButtonPressed = DIRECTION_RIGHT;
                    MillisOfLastButtonChange = millis();
                } else if (digitalRead(PinOfLeftButton) == LOW) {
                    if (Index == 0) {
                        //First button push in new game
                        Direction = DIRECTION_LEFT;
                    } else {
                        rotateLeft();
                    }
                    DirectionOfLastButtonPressed = DIRECTION_LEFT;
                    MillisOfLastButtonChange = millis();
                }
            } else if (digitalRead(PinOfRightButton) == HIGH && digitalRead(PinOfLeftButton) == HIGH) {
                /*
                 * Button released
                 */
#ifdef TRACE
                Serial.println(F("Button released"));
#endif
                DirectionOfLastButtonPressed = DIRECTION_NONE;
                MillisOfLastButtonChange = millis();
            }
        }
    }
#ifdef USE_SERIAL_CONTROL
    /*
     * Serial input (e.g from python controller mapper)
     */
    if (Serial.available()) {
        char tReceivedChar = Serial.read();
        if (char(tReceivedChar) == 'd') {
            tNewDirection = DIRECTION_RIGHT;
        } else if (char(tReceivedChar) == 'a') {
            tNewDirection = DIRECTION_LEFT;
        } else if (char(tReceivedChar) == 's') {
            tNewDirection = DIRECTION_DOWN;
        } else if (char(tReceivedChar) == 'w') {
            tNewDirection = DIRECTION_UP;
        }
#ifdef DEBUG
        Serial.println(tReceivedChar);
#endif
    }
#endif
#ifdef DEBUG
    if (tOldDirection != Direction) {
        Serial.print(F("Button pressed. NewDirection="));
        Serial.println(DirectionToString(Direction));
    }
#endif
}

/*
 * Check if new direction will lead to an invalid move
 * returns 0 if direction is valid
 * returns index value (bigger than 0 and less than (SnakeLength - 1)) if position is found in snake
 * returns SnakeLength if position is out of area
 */
uint16_t MatrixSnake::checkDirection(uint8_t aDirectionToCheck) {
// get new head position
    position tNewHeadPosition;
    if (!computeNewHeadPosition(aDirectionToCheck, &tNewHeadPosition)) {
        return SnakeLength;
    }
    return getIndexOfPositionInSnakeTail(tNewHeadPosition);
}

bool MatrixSnake::isPositionInArea(position aPositionToCheck) {
    /*
     * since position is unsigned, we do not need to check for negative value, since 0-1 is 0xFF (255)
     */
    if (aPositionToCheck.x >= Columns || aPositionToCheck.y >= Rows) {
        return false;
    }
    return true;
}

/*
 * Do not check last tail element since it will move away in the next step
 * returns index value (bigger than 0 and less than (SnakeLength - 1)) if position is found in snake
 * returns 0 if position is NOT in snake tail
 */
uint16_t MatrixSnake::getIndexOfPositionInSnakeTail(position aPositionToCheck) {
    for (uint16_t i = 1; i < (SnakeLength - 1); ++i) {
        if (aPositionToCheck.x == SnakePixelList[i].x && aPositionToCheck.y == SnakePixelList[i].y) {
            return i;
        }
    }
    return 0;
}

/*
 * Check every snake element
 * returns index value if position is found in snake
 * returns !!! SnakeLength !!! if position is NOT in snake
 */
uint16_t MatrixSnake::getIndexOfPositionInSnake(uint8_t aPositionToCheckX, uint8_t aPositionToCheckY) {
    for (uint16_t i = 0; i < SnakeLength; ++i) {
        if (aPositionToCheckX == SnakePixelList[i].x && aPositionToCheckY == SnakePixelList[i].y) {
            return i;
        }
    }
    return SnakeLength;
}

/*
 * Check every snake element
 * returns index value if position is found in snake
 * returns !!! SnakeLength !!! if position is NOT in snake
 */
uint16_t MatrixSnake::getIndexOfPositionInSnake(position aPositionToCheck) {
    for (uint16_t i = 0; i < SnakeLength; ++i) {
        if (aPositionToCheck.x == SnakePixelList[i].x && aPositionToCheck.y == SnakePixelList[i].y) {
            return i;
        }
    }
    return SnakeLength;
}

/*
 * returns true if direction is valid and new position is in area
 */
bool MatrixSnake::computeNewHeadPosition(uint8_t aDirection, position *aSnakeNewHeadPosition) {
    *aSnakeNewHeadPosition = SnakePixelList[0];
    switch (aDirection) {
    case DIRECTION_UP:
        aSnakeNewHeadPosition->y = SnakePixelList[0].y - 1;
        break;
    case DIRECTION_DOWN:
        aSnakeNewHeadPosition->y = SnakePixelList[0].y + 1;
        break;
    case DIRECTION_RIGHT:
        aSnakeNewHeadPosition->x = SnakePixelList[0].x + 1;
        break;
    case DIRECTION_LEFT:
        aSnakeNewHeadPosition->x = SnakePixelList[0].x - 1;
        break;
    default:
        return false;
        break;
    }
// check if position is in area
    if (aSnakeNewHeadPosition->x >= Columns || aSnakeNewHeadPosition->y >= Rows) {
        return false;
    }
    return true;
}

/*
 * returns true if snake eats apple
 */
bool MatrixSnake::moveSnakeAndCheckApple(position tSnakeNewHeadPosition) {
    /*
     * move snake body except head back in array and set new head
     */

// Clear tail end before moving snake
    position tLastTailEnd = SnakePixelList[SnakeLength - 1];
    setMatrixPixelColor(tLastTailEnd.x, tLastTailEnd.y, COLOR32_BLACK);

    memmove(&SnakePixelList[1], &SnakePixelList[0], SnakeLength * 2);
    SnakePixelList[0] = tSnakeNewHeadPosition;
    /*
     * check apple
     */
    if (tSnakeNewHeadPosition.x == Apple.x && tSnakeNewHeadPosition.y == Apple.y) {
        return true;
    }
    return false;
}

/*
 * This is called at a fixed interval and updates the snake "pattern".
 * The button input is acquired before by the global update routine.
 */
void MatrixSnake::SnakeUpdate() {

    if (PatternFlags & FLAG_SNAKE_AUTORUN) {
        // this calls the AI to simulate button presses which leads to a direction
        Direction = getNextSnakeDirection(this);
    } else {
        /*
         * Handling of showing end and score for plain snake pattern.
         * For Autorun, this is done in the SnakeAutorunCompleteHandler()
         */
        if (PatternFlags & FLAG_SNAKE_SHOW_END) {
            // show last state until index gets 0, then show score
            Index--;
            if (Index == 0) {
                // this activates show length
                showScore();
            }
            return;
        } else if (PatternFlags & FLAG_SNAKE_SHOW_LENGTH_SCORE) {
            // show score until index gets 0, then reset snake
            Index--;
            if (Index == 0) {
                resetAndClearAndShowSnakeAndNewApple();
            }
            return;
        }
    }

    if (Direction != DIRECTION_NONE) {
        /*
         * Try to move the snake. If direction is not valid, set it to DIRECTION_NONE.
         */
        position tSnakeNewHeadPosition;
        bool tHeadPositionIsInAreaAndDirectionIsValid = computeNewHeadPosition(Direction, &tSnakeNewHeadPosition);
#ifdef INFO
        Serial.print(F("new head=("));
        Serial.print(tSnakeNewHeadPosition.x);
        Serial.print(',');
        Serial.print(tSnakeNewHeadPosition.y);
        Serial.println(')');
#endif

        uint16_t tIndexOfHeadPositionInSnake = 0;
        if (!tHeadPositionIsInAreaAndDirectionIsValid) {
#ifdef DEBUG
            Serial.print(F("New head position is out of area or direction invalid. dir="));
            Serial.print(Direction);
            Serial.print(F(" length="));
            Serial.println(SnakeLength);
#endif
        } else {
            // check for self intersection of new head with tail
            tIndexOfHeadPositionInSnake = getIndexOfPositionInSnakeTail(tSnakeNewHeadPosition);
        }
        // check for invalid head position
        if ((tIndexOfHeadPositionInSnake > 0) || !tHeadPositionIsInAreaAndDirectionIsValid) {
#ifdef INFO
            if (tIndexOfHeadPositionInSnake > 0) {
                Serial.print(F("Intersection at index="));
                Serial.print(tIndexOfHeadPositionInSnake);
                Serial.print(F(" dir="));
                Serial.print(Direction);
                Serial.print(F(" length="));
                Serial.println(SnakeLength);
            }
#endif
            Direction = DIRECTION_NONE; // Current direction is not valid
        } else {
            /*
             * move snake body -except head- back in array and set new head
             */
            if (moveSnakeAndCheckApple(tSnakeNewHeadPosition)) {
                // eat the apple -> elongate snake
                SnakeLength++;
                newApple();
                drawApple();
            }
            drawSnake();
            Index++;
        }
    }

    /*
     * Not at start of game and current direction was not valid or no valid direction found for autorun
     */
    if (Index != 0 && Direction == DIRECTION_NONE) {
        if (OnPatternComplete == NULL) {
            // set flags to show end and score and reset snake
            SnakeEndHandler();
        } else {
            free(SnakePixelList);
            SnakePixelList = NULL;
            ActivePattern = PATTERN_NONE; // reset ActivePattern to enable polling for end of pattern.
            OnPatternComplete(this); // call the completion callback
        }
    }
}

/*
 * This handler is called when game ended with snake head collision with border or snake body.
 * It handles high score and sets flags to show end and score (which in turn resets snake)
 */
void MatrixSnake::SnakeEndHandler() {

    uint16_t tSnakeLength = SnakeLength;
#ifdef INFO
    Serial.print(F("Game Over. Snake length="));
    Serial.print(tSnakeLength);
#endif

    /*
     * handle High Score
     */
    if (tSnakeLength > HighScore) {
#ifdef INFO
        Serial.print(F(" Congratulation! New"));
#endif
        HighScore = tSnakeLength;
#if defined(__AVR__)
        eeprom_write_word(&HighScoreEEPROM, tSnakeLength);
#endif
    }
#ifdef INFO
    Serial.print(F(" High Score="));
    Serial.println(HighScore);
#endif

    Index = SHOW_END_INTERVAL_MILLIS / Interval;
    PatternFlags |= FLAG_SNAKE_SHOW_END; // signal end shown -> enables delay
    PatternFlags &= ~FLAG_SNAKE_SHOW_LENGTH_SCORE;
}

/*
 * Show score and in turn enables show length of snake after a delay of SHOW_NUMBER_INTERVAL_MILLIS
 */
void MatrixSnake::showScore() {
    color32_t tNumberColor = COLOR32_GREEN_HALF;

    if (SnakeLength > HighScore) {
        tNumberColor = COLOR32_RED;
    }
    showNumberOnMatrix(SnakeLength, tNumberColor);
    /*
     * set delay for number display
     */
    Index = SHOW_NUMBER_INTERVAL_MILLIS / Interval;
    PatternFlags |= FLAG_SNAKE_SHOW_LENGTH_SCORE; // signal number shown -> enables delay
    PatternFlags &= ~FLAG_SNAKE_SHOW_END;
}

uint8_t computeDirection(position aStartPosition, position aEndPosition) {
    int8_t tDeltaX = aEndPosition.x - aStartPosition.x;
    int8_t tDeltaY = aEndPosition.y - aStartPosition.y;
    if (tDeltaX > 0) {
        return DIRECTION_RIGHT;
    } else if (tDeltaX < 0) {
        return DIRECTION_LEFT;
    } else if (tDeltaY > 0) {
        return DIRECTION_DOWN;
    } else /* if (tDeltaY < 0) */{
        return DIRECTION_UP;
    }
}

uint8_t computeReverseDirection(uint8_t aDirection) {
    return ((aDirection + 2) % 4);
}

/*********************
 * SNAKE AUTO SOLVER
 *********************/

/*
 * MAIN auto solver algorithm
 * Gives the next direction towards the apple.
 * returns DIRECTION_NONE if no direction is valid.
 * SnakeAutoSolverMode determines the behavior of this algorithm
 */
uint8_t MatrixSnake::findNextDir() {

#ifdef DEBUG
    Serial.print(F("Direction="));
    Serial.println(DirectionToString(Direction));
#endif

    uint8_t tNewDirection = DIRECTION_NONE;
    int8_t tDeltaX = Apple.x - SnakePixelList[0].x;
    int8_t tDeltaY = Apple.y - SnakePixelList[0].y;

#ifdef TRACE
    Serial.print(F("DeltaX="));
    Serial.print(tDeltaX);
    Serial.print(F(" DeltaY="));
    Serial.println(tDeltaY);
#endif

    /*
     * Avoid going to opposite direction, because this is invalid.
     * Eg. if current direction is UP, we must not change to DOWN.
     */
// go shortest delta first
    if ((abs(tDeltaX) > abs(tDeltaY)) && tDeltaY != 0) {
        if (tDeltaY > 0 && Direction != DIRECTION_UP) {
            tNewDirection = DIRECTION_DOWN;
        } else if (tDeltaY < 0 && Direction != DIRECTION_DOWN) {
            tNewDirection = DIRECTION_UP;
        }
    }

    if (tNewDirection == DIRECTION_NONE) {
        /*
         * Here x is shortest delta or y delta is 0 or y move not valid
         */
        if (tDeltaX > 0 && Direction != DIRECTION_LEFT) {
            tNewDirection = DIRECTION_RIGHT;
        } else if (tDeltaX < 0 && Direction != DIRECTION_RIGHT) {
            tNewDirection = DIRECTION_LEFT;
        }
    }

    if (tNewDirection == DIRECTION_NONE) {
        /*
         * Here x delta is 0 or x is shortest delta but x move not valid
         */
        if (tDeltaY > 0 && Direction != DIRECTION_UP) {
            tNewDirection = DIRECTION_DOWN;
        } else if (tDeltaY < 0 && Direction != DIRECTION_DOWN) {
            tNewDirection = DIRECTION_UP;
        }
    }
    if (tNewDirection == DIRECTION_NONE) {
        // Can not go towards apple, so just try current direction
        tNewDirection = Direction;
    }

    /*
     * check if new direction is valid
     */
    uint16_t tIndexOfNewHeadPositionInSnake = checkDirection(tNewDirection);
    if (tIndexOfNewHeadPositionInSnake > 0) {
        /*
         * New direction leads to collision with snake itself, so must try other directions
         */
        bool invalidDirectionsArray[NUMBER_OF_DIRECTIONS];
        memset(invalidDirectionsArray, false, NUMBER_OF_DIRECTIONS);
        invalidDirectionsArray[tNewDirection] = true;
        // mark opposite direction of current moving direction also as invalid
        invalidDirectionsArray[(Direction + 2) % NUMBER_OF_DIRECTIONS] = true;
#ifdef TRACE
        Serial.print(F("Detected invalid direction="));
        Serial.println(tNewDirection);
#endif

        /*
         * just try all other directions
         */
        uint8_t i;
        for (i = 0; i < NUMBER_OF_DIRECTIONS; ++i) {
            tNewDirection = i;
            if (SnakeAutoSolverMode & AUTOSOLVER_CLOCKWISE_FLAG) {
                // try directions the other way
                tNewDirection = (NUMBER_OF_DIRECTIONS - 1) - i;
            }
            if (invalidDirectionsArray[tNewDirection]) {
                continue;
            }
            if (checkDirection(tNewDirection) == 0) {
                break;
            }
#ifdef TRACE
            Serial.print(F("Detected invalid direction="));
            Serial.println(tNewDirection);
#endif
        }

        if (i == NUMBER_OF_DIRECTIONS) {
            tNewDirection = DIRECTION_NONE;
#ifdef TRACE
            Serial.println(F("Give up, no valid direction left"));
#endif
        }
    }
#ifdef DEBUG
    Serial.print(F("NewDirection="));
    Serial.println(DirectionToString(tNewDirection));
#endif
    return tNewDirection;
}

/*
 * Stores state, runs the game with the current settings and restores state.
 * Returns number of steps to get the apple or 0 if impossible.
 */
uint8_t MatrixSnake::runAndCheckIfAppleCanBeReached() {

    uint8_t tResult = 0;
    /*
     * store original snake values
     */
    uint16_t tStoredSnakeLength = SnakeLength;
    uint8_t tStoredDirection = Direction;
    position *tStoredSnakePixelList = new position[Rows * Columns];
    memcpy(tStoredSnakePixelList, SnakePixelList, SnakeLength * sizeof(position));

    /*
     * Test-run the game to the end
     */
    uint8_t tDirection = Direction;
    while (true) {
        tDirection = findNextDir();
        if (tDirection == DIRECTION_NONE) {
            tResult = 0;
            break;
        }
        tResult++;
        position tNewHeadPosition;
        computeNewHeadPosition(tDirection, &tNewHeadPosition);
        if (moveSnakeAndCheckApple(tNewHeadPosition)) {
            // Test ends with eating the apple
            break;
        }
    }

    /*
     * restore snake body and length
     */
    memcpy(SnakePixelList, tStoredSnakePixelList, tStoredSnakeLength * sizeof(position));
    SnakeLength = tStoredSnakeLength;
    Direction = tStoredDirection;

    if (tStoredSnakePixelList != NULL) {
        free(tStoredSnakePixelList);
    }
#ifdef DEBUG
    Serial.print(F("Check with settings=0x"));
    Serial.print(SnakeAutoSolverMode, HEX);
    Serial.print(F(" gives "));
    Serial.println(tResult);
#endif
    return tResult;
}

/*
 * The weak standard implementation. Can be be overridden by the users own implementation.
 */
uint8_t
__attribute__((weak)) getNextSnakeDirection(MatrixSnake *aSnake) {
    return aSnake->getNextSnakeDir();
}

/*
 * SNAKE SOLVER EXAMPLE
 * This example test-plays the game with the standard moving algorithm (findNextDir()) to decide if it gets an apple.
 * If it has no chance to get the apple it changes the behavior of the solver.
 * This is checked only once when a new apple position was detected.
 */
uint8_t MatrixSnake::getNextSnakeDir() {

#ifdef DEBUG
    Serial.print(F("Direction="));
    Serial.print(DirectionToString(Direction));
    Serial.print(F(" head=("));
    Serial.print(SnakePixelList[0].x);
    Serial.print(',');
    Serial.print(SnakePixelList[0].y);
    Serial.println(')');
#endif

    static position lastApplePosition;
    if (lastApplePosition.x != Apple.x || lastApplePosition.y != Apple.y) {
        lastApplePosition = Apple;
        /*
         * New apple position detected, now check settings by running this game with this direction to its end (until no direction left)
         */
        SnakeAutoSolverMode = 0;

        uint8_t tSteps = runAndCheckIfAppleCanBeReached();
        if (tSteps == 0) {
            // just take other mode now
            SnakeAutoSolverMode |= AUTOSOLVER_CLOCKWISE_FLAG;
#ifdef DEBUG
            Serial.print(F("Change SnakeAutoSolverMode to 0x"));
            Serial.println(SnakeAutoSolverMode, HEX);
            // for debugging purposes
            runAndCheckIfAppleCanBeReached();
#endif
        }
    }

    return findNextDir();
}

/****************************************************
 * SNAKE AUTORUN
 * Uses built in or user supplied auto solver
 * SnakeAutorun is implemented as a COMBINED PATTERN
 ****************************************************/
#define AUTORUN_MODE_SHOW_END 0
#define AUTORUN_MODE_SHOW_SCORE 1
#define AUTORUN_MODE_MOVE_SCORE 2
#define AUTORUN_MODE_START_NEW 3

/*
 * Initialize for snake autorun
 * aRepetitions - number of Snake games until original OnPatternCompleteHandler is called.
 * If OnPatternComplete is NULL, Snake game is played forever.
 */
void initSnakeAutorun(MatrixSnake *aLedsPtr, uint16_t aIntervalMillis, color32_t aColor, uint16_t aRepetitions) {

#ifdef INFO
    Serial.print(F("Autorun: "));
#endif
    aLedsPtr->MultipleExtension = AUTORUN_MODE_SHOW_END;
    aLedsPtr->Repetitions = aRepetitions;
    aLedsPtr->NextOnPatternCompleteHandler = aLedsPtr->OnPatternComplete;
    aLedsPtr->OnPatternComplete = &SnakeAutorunCompleteHandler;
    aLedsPtr->Snake(aIntervalMillis, aColor);
    aLedsPtr->Direction = DIRECTION_LEFT;

}

/*
 *  - Show score
 *  - Move score to left
 *  - If specified, switch back to original OnComplete handler after delay of (Index * 2)
 */
void SnakeAutorunCompleteHandler(NeoPatterns *aLedsPtr) {
    static unsigned long sOriginalInterval; // store for original interval, since move needs its separate interval
    MatrixSnake *tLedsPtr = (MatrixSnake*) aLedsPtr;

    uint8_t tStep = tLedsPtr->MultipleExtension;

#ifdef DEBUG
    Serial.print(F("SnakeAutorunCompleteHandler Step="));
    Serial.println(tStep);
#endif

    if (tStep == AUTORUN_MODE_START_NEW) {
        // score is moved out, start a new game or perform delay and switch to original OnPatternComplete handler.
        tLedsPtr->Repetitions--;
        if (tLedsPtr->Repetitions != 0 || tLedsPtr->NextOnPatternCompleteHandler == NULL) {
            // set value to enable next turn
            tLedsPtr->MultipleExtension = AUTORUN_MODE_SHOW_END;
            tLedsPtr->Snake(sOriginalInterval, tLedsPtr->Color1);
        } else {
            // perform delay and then switch back to NextOnComplete
            tLedsPtr->Delay(tLedsPtr->Index * 2);
            tLedsPtr->OnPatternComplete = tLedsPtr->NextOnPatternCompleteHandler;
        }
        return;

    } else if (tStep == AUTORUN_MODE_MOVE_SCORE) {
        // Move score to left
        tLedsPtr->Move(DIRECTION_LEFT, tLedsPtr->Columns, 30);

    } else if (tStep == AUTORUN_MODE_SHOW_SCORE) {
        // Show score
        tLedsPtr->showNumberOnMatrix(tLedsPtr->SnakeLength, COLOR32_GREEN_HALF);
        tLedsPtr->Delay(SHOW_NUMBER_INTERVAL_MILLIS);

    } else if (tStep == AUTORUN_MODE_SHOW_END) {
        // Snake just ended -> delay to show end positions
        sOriginalInterval = tLedsPtr->Interval; // store original snake interval
        tLedsPtr->Delay(SHOW_END_INTERVAL_MILLIS);
    }

    tStep++;
    tLedsPtr->MultipleExtension = tStep;
}

const char DefaultTickerText[] PROGMEM = "I love Neopixel";
const char ILoveNeopatternsString[] PROGMEM = "I love Neopatterns";
const char MatrixDemoString[] PROGMEM = "Matrix Demo";
const char WelcomeString[] PROGMEM = "Welcome";
const char *const TickerStrings[] PROGMEM = { DefaultTickerText, ILoveNeopatternsString, MatrixDemoString, WelcomeString };
const char *sTickerTextPtr;

void setMatrixAndSnakePatternsDemoHandlerRandomTickerText() {
    uint8_t tRandomIndex = random(0, sizeof(TickerStrings) / sizeof(char*));
#if defined(__AVR__) // Let the function work for non AVR platforms
    sTickerTextPtr = (char*) pgm_read_word(&TickerStrings[tRandomIndex]);
#else
    sTickerTextPtr = TickerStrings[tRandomIndex];
#endif
}

void setMatrixAndSnakePatternsDemoHandlerTickerText(const __FlashStringHelper *aTextForTicker) {
    sTickerTextPtr = reinterpret_cast<const char*>(aTextForTicker);
}

/*
 * Sample callback handler for MatrixNeoPatterns + MatrixSnake
 * 1 loop lasts ca. 50 seconds
 * 1. Runs ticker "I love Neopixel" from right to left / from bottom to top
 * 2. Moves heart in from top / bottom, show 2 heart beats, and move heart out
 * 3. Show 2 snake runs / fire. Snake shows up on the odd loops, fire on the even ones
 */
void MatrixAndSnakePatternsDemoHandler(NeoPatterns *aLedsPtr) {
    MatrixSnake *tLedsPtr = (MatrixSnake*) aLedsPtr;
    static int8_t sState = 0;
    static uint8_t sHeartDirection = DIRECTION_DOWN;
//    static uint8_t sHeartDirection = DIRECTION_UP;
    static int8_t sTickerDirection = DIRECTION_LEFT;
    //    static int8_t sTickerDirection = DIRECTION_UP;
    static int8_t sSnakeFireSnowSelector = 0;
//    static int8_t sSnakeFireSnowSelector = 2; // start with snow

    /*
     * implement a delay between each case
     */
    if ((sState & 1) == 1) {
        aLedsPtr->Delay(100); // to separate each pattern - not really needed here
        sState++;
        return;
    }

    uint8_t tState = sState / 2;
    uint8_t tYOffset;
    uint8_t tSteps = HEART_HEIGHT + 1 + (tLedsPtr->Rows - HEART_HEIGHT) / 2;
    uint8_t tXOffset = (tLedsPtr->Columns - HEART_WIDTH) / 2;

    switch (tState) {
    case 0:
//        myLoadTest(tLedsPtr);
        setMatrixAndSnakePatternsDemoHandlerRandomTickerText();
        tLedsPtr->TickerInit(sTickerTextPtr, NeoPatterns::Wheel(0), COLOR32_BLACK, 80, sTickerDirection, FLAG_TICKER_DATA_IN_FLASH);
        sTickerDirection--;
        if (sTickerDirection < 0) {
            sTickerDirection = DIRECTION_LEFT;
        }
        break;
    case 1:
        tYOffset = -1; // direction DOWN
        if (sHeartDirection == DIRECTION_UP) {
            tYOffset = tLedsPtr->Rows + HEART_HEIGHT;
        }
        // move in
        tLedsPtr->MovingPicturePGM(heart8x8, COLOR32_RED_HALF, COLOR32_BLACK, tXOffset, tYOffset, tSteps, 100, sHeartDirection);
        break;
    case 2:
        // Next 4 cases show 2 heart beats
        // take color from picture before
        aLedsPtr->ProcessSelectiveColor(aLedsPtr->Color1, &DimColor, 6, 40);
        break;
    case 3:
        // get last color from dim function
        aLedsPtr->ProcessSelectiveColor(aLedsPtr->LongValue2.ColorTmp, &BrightenColor, 6, 40);
        break;
    case 4:
        aLedsPtr->ProcessSelectiveColor(aLedsPtr->LongValue2.ColorTmp, &DimColor, 6, 40);
        break;
    case 5:
//        aLedsPtr->ProcessSelectiveColor(aLedsPtr->LongValue2.ColorTmp, &BrightenColor, 6, 40);
        aLedsPtr->ProcessSelectiveColor(aLedsPtr->LongValue2.ColorTmp, &BrightenColor, 6, 40);
        break;

    case 6:
        // move out - for move out, we can use Move instead of MovingPicturePGM
        tLedsPtr->Move(sHeartDirection, tLedsPtr->Rows, 100);
        // change direction for next time
        if (sHeartDirection == DIRECTION_DOWN) {
            sHeartDirection = DIRECTION_UP;
        } else {
            sHeartDirection = DIRECTION_DOWN;
        }
        break;
    case 7:
        aLedsPtr->Delay(1500);
        break;
    case 8:
        if (sSnakeFireSnowSelector == 0) {
            initSnakeAutorun(tLedsPtr, 200, COLOR32_BLUE, 1);
        } else if (sSnakeFireSnowSelector == 1) {
            // after 4 minutes show more fire :-)
            if (millis() < (4 * 60 * 1000L)) {
                tLedsPtr->Fire();
            } else {
                tLedsPtr->Fire(800, 30);
            }
        } else {
            // after 4 minutes show more snow :-)
            if (millis() < (4 * 60 * 1000L)) {
                tLedsPtr->Snow();
            } else {
                tLedsPtr->Snow(2000);
            }
        }
        sSnakeFireSnowSelector++;
        if (sSnakeFireSnowSelector >= 3) {
            sSnakeFireSnowSelector = 0;
        }
        break;
    case 9:
        aLedsPtr->clear(); // Clear matrix
        aLedsPtr->show();
        aLedsPtr->Delay(2000);
        // do not forget sState = -1; in last sensible case
        sState = -1; // Start from beginning
        break;
    case 10:
        break;
    case 11:
        // safety net
        sState = -1; // Start from beginning
        break;

// SAMPLE ACTIONS for case
//        aLedsPtr->Delay(40000);
//        tLedsPtr->TickerPGM(PSTR("I love you"), COLOR32_RED, COLOR32_BLACK, 50, DIRECTION_LEFT);
//        tLedsPtr->MovingPicturePGM(heart8x8, COLOR32_RED, COLOR32_BLACK, 0, -8, 9, 100, DIRECTION_UP);
//        tLedsPtr->MovingPicturePGM(heart8x8, COLOR32_RED, COLOR32_BLACK, 0, 0, 9, 100, DIRECTION_UP);
//        tLedsPtr->MovingPicturePGM(heart8x8, COLOR32_RED, COLOR32_BLACK, 0, 8, 9, 100, DIRECTION_DOWN);
//        tLedsPtr->MovingPicturePGM(heart8x8, COLOR32_RED, COLOR32_BLACK, 0, 0, 9, 100, DIRECTION_DOWN);
//        tLedsPtr->MovingPicturePGM(heart8x8, COLOR32_RED, COLOR32_BLACK, -8, 0, 9, 100, DIRECTION_LEFT);
//        tLedsPtr->MovingPicturePGM(heart8x8, COLOR32_RED, COLOR32_BLACK, 0, 0, 9, 100, DIRECTION_LEFT);
//        tLedsPtr->MovingPicturePGM(heart8x8, COLOR32_RED, COLOR32_BLACK, 8, 0, 9, 100, DIRECTION_RIGHT);
//        tLedsPtr->MovingPicturePGM(heart8x8, COLOR32_RED, COLOR32_BLACK, 0, 0, 9, 100, DIRECTION_RIGHT);
//        tLedsPtr->FadeSelective(COLOR32_RED, COLOR32_GREEN, 20, 40);

    default:
        aLedsPtr->Delay(1);
#ifdef ERROR
        Serial.print(F("case "));
        Serial.print(tState);
        Serial.println(F(" not implemented"));
#endif
        break;
    }

#ifdef INFO
    Serial.print(F("Pin="));
    Serial.print(aLedsPtr->getPin());
    Serial.print(F(" Length="));
    Serial.print(aLedsPtr->numPixels());
    Serial.print(F(" ActivePattern="));
    Serial.print(aLedsPtr->ActivePattern);
    Serial.print(F(" State="));
    Serial.println(tState);
#endif

    sState++;
}

