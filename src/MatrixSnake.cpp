/*
 * MatrixSnake.cpp
 *
 * Implements a snake game on NeoPixel matrix.
 * High-Score is reset if both RightButton and Left Button are pressed at start of game by calling Snake()
 *  You need to install "Adafruit NeoPixel" library under Sketch -> Include Library -> Manage Librarys... -> use "neoPixel" as filter string
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
#define INFO
//#define ERROR

#include "MatrixSnake.h"
#include <stdlib.h>         // for utoa() etc.

EEMEM uint16_t HighScoreEEPROM; // is reset if both right and left button are pressed at startup.

// Constructor - calls base-class constructor to initialize strip
MatrixSnake::MatrixSnake(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry, uint8_t aTypeOfPixel, // @suppress("Class members should be properly initialized")
        void (*aPatternCompletionCallback)(NeoPatterns*)) :
        MatrixNeoPatterns(aColumns, aRows, aPin, aMatrixGeometry, aTypeOfPixel, aPatternCompletionCallback) {
}

// Update the pattern returns true if update has happened in order to give the caller a chance to manually change parameters (like color etc.)
bool MatrixSnake::Update() {

    if (ActivePattern == SNAKE) {
        SnakeInputHandler();
    }

    if ((millis() - lastUpdate) > Interval) {
        if (ActivePattern == SNAKE) {
            SnakeUpdate();
        } else {
            MatrixNeoPatterns::Update();
        }
        // time to update
        lastUpdate = millis();
        return true;
    }
    return false;
}

// moves the goal to a new position
void MatrixSnake::newApple() {
    bool AppleIsOnSnake = true;
    uint8_t tNewX;
    uint8_t tNewY;

    // clear old apple
    setMatrixPixelColor(Apple.x, Apple.y, COLOR32_BLACK);

    // chose new position if the chosen position aligns with the snake body
    while (AppleIsOnSnake) {
        tNewX = random(Rows - 1);
        tNewY = random(Columns - 1);
        if (tNewX == Apple.x && tNewY == Apple.y) {
            continue;
        }
        // checks for collision with snake
        AppleIsOnSnake = false;
        for (uint16_t i = 0; i < SnakeLength; i++) {
            // checks if it is on snake part
            if (SnakePixelList[i].x == tNewX && SnakePixelList[i].y == tNewY) {
                AppleIsOnSnake = true;
                break;
            }
        }
    }
    Apple.x = tNewX;
    Apple.y = tNewY;

#ifdef INFO
    Serial.print((const __FlashStringHelper *) PSTR("New apple("));
    Serial.print(tNewX);
    Serial.print(',');
    Serial.print(tNewY);
    Serial.println(')');
#endif

    setMatrixPixelColor(Apple.x, Apple.y, APPLE_COLOR);

}

void MatrixSnake::rotateLeft() {
    Direction = (Direction + 1) % 4;
}

void MatrixSnake::rotateRight() {
    Direction -= 1;
    if (Direction < 0) {
        Direction = 3;
    }
}

// resets the playing field
void MatrixSnake::resetAndDrawSnake() {
#ifdef DEBUG
    Serial.println((const __FlashStringHelper *) PSTR("Reset"));
#endif
    // Clear all pixels
    clear();
    SnakeLength = sizeof(SnakeInitialPixels) / sizeof(position);
    Index = 0; // steps already gone
    Direction = DIRECTION_LEFT;
    // sets the beginning of the snake to init_snake
    memcpy(SnakePixelList, SnakeInitialPixels, sizeof(SnakeInitialPixels));
    newApple();
    Flags &= ~(FLAG_SNAKE_IS_MOVING | FLAG_SNAKE_SHOW_LENGTH);
    drawSnake();
}
/*
 * TotalSteps holds the steps the snake is gone
 */
void MatrixSnake::Snake(uint16_t aIntervalMillis, color32_t aColor, uint8_t aPinOfRightButton, uint8_t aPinOfLeftButton,
        uint8_t aPinOfUpButton, uint8_t aPinOfDownButton) {
    ActivePattern = SNAKE;
    Interval = aIntervalMillis;
    Color1 = aColor;
    PinOfRightButton = aPinOfRightButton;
    PinOfLeftButton = aPinOfLeftButton;
    PinOfUpButton = aPinOfUpButton;
    PinOfDownButton = aPinOfDownButton;

    // used for autorun
    if (SnakePixelList != NULL) {
        free(SnakePixelList);
    }
    SnakePixelList = new position[Rows * Columns];

    /*
     * Read high score from EEPROM / reset high score
     */
    HighScore = eeprom_read_word(&HighScoreEEPROM);
    if ((digitalRead(aPinOfRightButton) == LOW && digitalRead(aPinOfLeftButton) == LOW) || HighScore == 0xFFFF) {
#ifdef INFO
        Serial.print((const __FlashStringHelper *) PSTR("Reset High Score"));
#endif
        eeprom_write_word(&HighScoreEEPROM, 0);
        HighScore = 0;
    }

#ifdef INFO
    Serial.print((const __FlashStringHelper *) PSTR("Starting Snake game with refresh interval="));
    Serial.print(aIntervalMillis);
    Serial.print((const __FlashStringHelper *) PSTR("ms. High Score="));
    Serial.println(HighScore);
#endif

    resetAndDrawSnake();

    if (PinOfRightButton != 0) {
        pinMode(aPinOfRightButton, INPUT_PULLUP);
        pinMode(aPinOfLeftButton, INPUT_PULLUP);
        if (PinOfUpButton != 0) {
            pinMode(aPinOfUpButton, INPUT_PULLUP);
            pinMode(aPinOfDownButton, INPUT_PULLUP);
        }
        Flags = 0; // start with 2 button mode unless up or down button is pressed
    } else {
        Flags = FLAG_SNAKE_AUTORUN | FLAG_SNAKE_IS_MOVING; // start in autorun mode
    }
}

void MatrixSnake::drawSnake() {
    for (uint16_t i = 0; i < SnakeLength; i++) {
        position tPosition = SnakePixelList[i];
        // linear color interpolation over the body of the snake between 50 (tail) and 255 (head)
        // Optimize order of operations to minimize truncation error
        uint8_t red = (Red(Color1) * (SnakeLength - i)) / SnakeLength;
        uint8_t green = (Green(Color1) * (SnakeLength - i)) / SnakeLength;
        uint8_t blue = (Blue(Color1) * (SnakeLength - i)) / SnakeLength;
#ifdef TRACE
        Serial.print((const __FlashStringHelper *) PSTR("snake color red="));
        Serial.print(red);
        Serial.print((const __FlashStringHelper *) PSTR(" green="));
        Serial.print(green);
        Serial.print((const __FlashStringHelper *) PSTR(" blue="));
        Serial.println(blue);
#endif
        setMatrixPixelColor(tPosition.x, tPosition.y, COLOR32(red, green, blue));
    }
    show();
    // Clear snake tail pixel in advance for next drawing
    position tPosition = SnakePixelList[SnakeLength - 1];
    setMatrixPixelColor(tPosition.x, tPosition.y, COLOR32_BLACK);

}

void MatrixSnake::SnakeInputHandler() {
    if (PinOfRightButton == 0) {
        // Demo or AxI mode -> no buttons connected
        return;
    }

    if (PinOfUpButton != 0) {
        /*
         * First check up and down button direct input, no debouncing needed
         */
        if (digitalRead(PinOfUpButton) == LOW) {
            Direction = DIRECTION_UP;
            Flags = FLAG_USE_4_BUTTONS | FLAG_SNAKE_IS_MOVING;
        } else if (digitalRead(PinOfDownButton) == LOW) {
            Direction = DIRECTION_DOWN;
            Flags = FLAG_USE_4_BUTTONS | FLAG_SNAKE_IS_MOVING;
        }
    }

    if (Flags & FLAG_USE_4_BUTTONS) {
        // four buttons direct direction input
        if (digitalRead(PinOfRightButton) == LOW) {
            Direction = DIRECTION_RIGHT;
            Flags |= FLAG_SNAKE_IS_MOVING;
        } else if (digitalRead(PinOfLeftButton) == LOW) {
            Direction = DIRECTION_LEFT;
            Flags |= FLAG_SNAKE_IS_MOVING;
        }
    } else {
        /*
         * 2 buttons turn direction input, debouncing needed
         */
        uint32_t tMillisSinceLastButtonChanged = millis() - MillisOfLastButtonChange;
        if (tMillisSinceLastButtonChanged > MILLIS_FOR_BUTTON_DEBOUNCING) {

            if (DirectionOfLastButtonPressed == DIRECTION_NONE) {
                /*
                 * Check if button pressed
                 */
                if (digitalRead(PinOfRightButton) == LOW) {
                    rotateRight();
                    DirectionOfLastButtonPressed = DIRECTION_RIGHT;
                    Flags |= FLAG_SNAKE_IS_MOVING;
                    MillisOfLastButtonChange = millis();
                } else if (digitalRead(PinOfLeftButton) == LOW) {
                    rotateLeft();
                    DirectionOfLastButtonPressed = DIRECTION_LEFT;
                    Flags |= FLAG_SNAKE_IS_MOVING;
                    MillisOfLastButtonChange = millis();
                }
            } else if (digitalRead(PinOfRightButton) == HIGH && digitalRead(PinOfLeftButton) == HIGH) {
                /*
                 * Button released
                 */
                DirectionOfLastButtonPressed = DIRECTION_NONE;
                MillisOfLastButtonChange = millis();
            }
        }
    }
    /*
     * Serial input (e.g from python controller mapper)
     */
    if (Serial.available()) {
        char tReceivedChar = Serial.read();
        Flags |= FLAG_SNAKE_IS_MOVING;
        if (char(tReceivedChar) == 'd') {
            Direction = DIRECTION_RIGHT;
        } else if (char(tReceivedChar) == 'a') {
            Direction = DIRECTION_LEFT;
        } else if (char(tReceivedChar) == 's') {
            Direction = DIRECTION_DOWN;
        } else if (char(tReceivedChar) == 'w') {
            Direction = DIRECTION_UP;
        }
#ifdef DEBUG
        Serial.println(tReceivedChar);
#endif
    }
}

void MatrixSnake::SnakeUpdate() {
    static bool sSnakeHasMoved = false; // for autodemo timeout detection

    if (Flags & FLAG_SNAKE_AUTORUN) {
        Direction = computeSnakeDirection(this, SnakePixelList[0], Apple, Direction, Index, SnakeLength, SnakePixelList);
    } else if (Flags & FLAG_SNAKE_SHOW_LENGTH) {
        Index--;
        if (Index == 0) {
            resetAndDrawSnake();
        }
        return;
    }

    if (Flags & FLAG_SNAKE_IS_MOVING) {
        sSnakeHasMoved = true;

        // move the snake
        position curr_pos = SnakePixelList[0];

        position new_pos;
        new_pos.x = curr_pos.x;
        new_pos.y = curr_pos.y;

        // the actual movement
        switch (Direction) {
        case DIRECTION_UP:
            new_pos.y -= 1;
            break;
        case DIRECTION_DOWN:
            new_pos.y += 1;
            break;
        case DIRECTION_RIGHT:
            new_pos.x += 1;
            break;
        case DIRECTION_LEFT:
            new_pos.x -= 1;
            break;
        default:
#ifdef ERROR
            Serial.print((const __FlashStringHelper *) PSTR("ERROR Direction="));
            Serial.print(Direction);
            Serial.println((const __FlashStringHelper *) PSTR(" is no valid direction!"));
#endif
            break;
        }

        if (new_pos.x == Apple.x and new_pos.y == Apple.y) {
            // ate the apple -> elongate snake
            SnakeLength++;
            newApple();
        }
        // move snake body except head back in array
        memmove(&SnakePixelList[1], &SnakePixelList[0], SnakeLength * 2);

        SnakePixelList[0].x = new_pos.x;
        SnakePixelList[0].y = new_pos.y;

        // check for self intersection of new head with tail
        bool intersection = false;
        // exclude the head
        for (uint16_t i = 1; i < SnakeLength; i++) {
            if (SnakePixelList[i].x == new_pos.x && SnakePixelList[i].y == new_pos.y) {
#ifdef INFO
                Serial.print((const __FlashStringHelper *) PSTR("Intersection at index="));
                Serial.print(i);
                Serial.print((const __FlashStringHelper *) PSTR(" x="));
                Serial.print(new_pos.x);
                Serial.print((const __FlashStringHelper *) PSTR(" y="));
                Serial.print(new_pos.y);
                Serial.print((const __FlashStringHelper *) PSTR(" dir="));
                Serial.print(Direction);
                Serial.print((const __FlashStringHelper *) PSTR(" length="));
                Serial.println(SnakeLength);
#endif
                intersection = true;
                break;
            }
        }
        // also check for out of bounds
        if (intersection || new_pos.x < 0 || new_pos.x >= Columns || new_pos.y < 0 || new_pos.y >= Rows) {
            if (OnPatternComplete == NULL) {
                SnakeDefaultHandler();
            } else {
                free(SnakePixelList);
                SnakePixelList = NULL;
                OnPatternComplete(this); // call the completion callback
            }
            return;
        }
        drawSnake();
        Index++;
    } else {
        long tMillis = millis();
        // +1000 since we are called onlx each Interval millis
        if (tMillis > TIME_TO_SWITCH_TO_AUTO_MODE_MILLIS && tMillis < (TIME_TO_SWITCH_TO_AUTO_MODE_MILLIS + 1000)
                && !sSnakeHasMoved) {
            // switch to demo mode is switching time has reached and snake has not moved
            initSnakeAutorun(this, Interval / 2, Color1);
        }
    }
}

/*
 * Handler is called when game ended with snake head collision with border or snake body
 */
void MatrixSnake::SnakeDefaultHandler() {

    uint16_t tSnakeLength = SnakeLength;
#ifdef INFO
    Serial.print((const __FlashStringHelper *) PSTR("Game Over. Snake length="));
    Serial.print(tSnakeLength);
#endif

    /*
     * handle High Score
     */
    if (tSnakeLength > HighScore) {
#ifdef INFO
        Serial.print((const __FlashStringHelper *) PSTR(" Congratulation! New"));
#endif
        HighScore = tSnakeLength;
        eeprom_write_word(&HighScoreEEPROM, tSnakeLength);
    }
#ifdef INFO
    Serial.print((const __FlashStringHelper *) PSTR(" High Score="));
    Serial.println(HighScore);
#endif

    showScore();
}

/*
 * Show score / length of snake
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
    Flags |= FLAG_SNAKE_SHOW_LENGTH; // signal number shown -> enables delay
    Flags &= ~FLAG_SNAKE_IS_MOVING;
}

/*
 * Sample callback handler for MatrixNeoPatterns + MatrixSnake
 */
void MatrixAndSnakePatternsDemo(NeoPatterns * aLedsPtr) {
    MatrixSnake* tLedsPtr = (MatrixSnake*) aLedsPtr;
    static uint8_t sState = 0;
    static uint8_t sHeartDirection = DIRECTION_DOWN;
    static int8_t sTickerDirection = DIRECTION_LEFT;

    sState++;
    /*
     * implement a delay between each case
     */
    if (sState % 2 == 1) {
        aLedsPtr->Delay(100); // not really needed
        return;
    }

    uint8_t tState = sState / 2;
    uint8_t tYOffset;
    uint8_t tSteps = HEART_HEIGHT + 1 + (tLedsPtr->Rows - HEART_HEIGHT) / 2;
    uint8_t tXOffset = (tLedsPtr->Columns - HEART_WIDTH) / 2;

#ifdef DEBUG
    Serial.print((const __FlashStringHelper *) PSTR("tState="));
    Serial.println(tState);
#endif

    switch (tState) {
    case 1:
//        myLoadTest(tLedsPtr);
        tLedsPtr->TickerPGM(PSTR("I love Neopixel"), NeoPatterns::Wheel(0), COLOR32_BLACK, 80, sTickerDirection);
        sTickerDirection--;
        if (sTickerDirection < 0) {
            sTickerDirection = DIRECTION_LEFT;
        }
        break;
    case 2:
        tYOffset = HEART_HEIGHT;
        if (sHeartDirection == DIRECTION_UP) {
            tYOffset = -HEART_HEIGHT;
        }
        // move in

        tLedsPtr->MovingPicturePGM(heart8x8, COLOR32_RED_HALF, COLOR32_BLACK, tXOffset, tYOffset, tSteps, 100, sHeartDirection);
        break;
    case 3:
        // Next 4 cases show 2 heart beats
        aLedsPtr->ColorForSelection = aLedsPtr->Color1;
        aLedsPtr->ProcessSelectiveColor(&DimColor, 6, 40);
        break;
    case 4:
        aLedsPtr->ProcessSelectiveColor(&LightenColor, 6, 40);
        break;
    case 5:
        aLedsPtr->ProcessSelectiveColor(&DimColor, 6, 40);
        break;
    case 6:
        aLedsPtr->ProcessSelectiveColor(&LightenColor, 6, 40);
        break;
    case 7:
        // move out
        tLedsPtr->Move(sHeartDirection, tLedsPtr->Rows, 100, true);
        // change direction for next time
        if (sHeartDirection == DIRECTION_DOWN) {
            sHeartDirection = DIRECTION_UP;
        } else {
            sHeartDirection = DIRECTION_DOWN;
        }
        break;
    case 8:
        aLedsPtr->Delay(1500);
        break;
    case 9:
        if (sTickerDirection == DIRECTION_LEFT) {
            tLedsPtr->Fire(100, 50);
        } else {
            initSnakeAutorun(tLedsPtr, 200, COLOR32_BLUE, 3);
        }
        break;
    case 10:
        aLedsPtr->clear(); // Clear matrix
        aLedsPtr->show();
        aLedsPtr->Delay(2000);
        // do not forget sState = 0; in last sensible case
        sState = 0;
        break;
    case 11:
        break;
    case 12:
        // safety net
        sState = 0;
        break;

// EXAMPLE ACTIONS for case
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
        Serial.print((const __FlashStringHelper *) PSTR("case "));
        Serial.print(tState);
        Serial.println((const __FlashStringHelper *) PSTR(" not implemented"));
#endif
        break;
    }
}

/*****************************************************************
 * COMBINED PATTERN EXAMPLE
 * overwrites the OnComplete Handler pointer and sets it
 * to aNextOnComplete after completion of combined patterns
 *****************************************************************/
//  __attribute__((weak))
uint8_t __attribute__((weak)) computeSnakeDirection(MatrixSnake * aSnake, position aSnakeHeadPosition, position aApplePosition,
        uint8_t aActualDirection, uint8_t aStepsDone, uint16_t aSnakeLength, position * aSnakeBodyArray) {

#ifdef DEBUG
    Serial.print((const __FlashStringHelper *) PSTR("computeSnakeDirection aActualDirection="));
    Serial.println(aActualDirection);
#endif

    uint8_t tNewDirection = aActualDirection;
    int8_t tDeltaX = aApplePosition.x - aSnakeHeadPosition.x;
    int8_t tDeltaY = aApplePosition.y - aSnakeHeadPosition.y;
    bool tMatched = false;

    // go shortest delta first
    if ((abs(tDeltaX) > abs(tDeltaY)) && tDeltaY != 0) {
        if (tDeltaY > 0 && aActualDirection != DIRECTION_UP) {
            tNewDirection = DIRECTION_DOWN;
            tMatched = true;
        } else if (tDeltaY < 0 && aActualDirection != DIRECTION_DOWN) {
            tMatched = true;
            tNewDirection = DIRECTION_UP;
        }
    }

    if (!tMatched) {
        /*
         * x is shortest delta or y delta is 0 or y move not allowed
         */
        if (tDeltaX > 0 && aActualDirection != DIRECTION_LEFT) {
            tMatched = true;
            tNewDirection = DIRECTION_RIGHT;
        } else if (tDeltaX < 0 && aActualDirection != DIRECTION_RIGHT) {
            tMatched = true;
            tNewDirection = DIRECTION_LEFT;
        }
    }

    if (!tMatched) {
        /*
         * x is shortest delta and x is 0 or x move not allowed
         */
        if (tDeltaY > 0 && aActualDirection != DIRECTION_UP) {
            tNewDirection = DIRECTION_DOWN;
        } else if (tDeltaY < 0 && aActualDirection != DIRECTION_DOWN) {
            tNewDirection = DIRECTION_UP;
        }
    }
#ifdef DEBUG
    Serial.print((const __FlashStringHelper *) PSTR("tDeltaX="));
    Serial.print(tDeltaX);
    Serial.print((const __FlashStringHelper *) PSTR(" tDeltaY="));
    Serial.print(tDeltaY);
    Serial.print((const __FlashStringHelper *) PSTR(" tNewDirection="));
    Serial.println(tNewDirection);
#endif
    return tNewDirection;
}

// initialize for snake autorun
void initSnakeAutorun(MatrixSnake * aLedsPtr, uint16_t aIntervalMillis, color32_t aColor, uint16_t aRepetitions) {

    aLedsPtr->Duration = 2;
    aLedsPtr->Repetitions = aRepetitions;
    aLedsPtr->NextOnPatternCompleteHandler = aLedsPtr->OnPatternComplete;
    aLedsPtr->OnPatternComplete = &SnakeAutorunCompleteHandler;
    aLedsPtr->Snake(aIntervalMillis, aColor);
}

/*
 *  - Free memory and show score
 *  - Move score to left
 *  - Switch back to original OnComplete handler
 */
void SnakeAutorunCompleteHandler(NeoPatterns * aLedsPtr) {
    static unsigned long sInterval; // since move needs its separate in interval
    MatrixSnake* tLedsPtr = (MatrixSnake*) aLedsPtr;

    uint16_t tStep = tLedsPtr->Duration;

#ifdef DEBUG
    Serial.print((const __FlashStringHelper *) PSTR("SnakeAutorunCompleteHandler tStep="));
    Serial.println(tStep);
#endif

    if (tStep == 0) {
        tLedsPtr->Repetitions--;
        if (tLedsPtr->Repetitions != 0 || tLedsPtr->NextOnPatternCompleteHandler == NULL) {
            // set value to enable next turn
            tLedsPtr->Duration = 2;
            tLedsPtr->Snake(sInterval, tLedsPtr->Color1);
        } else {
            // perform delay and then switch back to NextOnComplete
            tLedsPtr->Delay(tLedsPtr->Index * 2);
            tLedsPtr->OnPatternComplete = tLedsPtr->NextOnPatternCompleteHandler;
        }
        return;

    } else if (tStep == 1) {
        // Move score to left
        tLedsPtr->Move(DIRECTION_LEFT, tLedsPtr->Columns, 30);

    } else if (tStep >= 2) {
        sInterval = tLedsPtr->Interval;
        // Snake just ended -> show score
        tLedsPtr->showNumberOnMatrix(tLedsPtr->SnakeLength, COLOR32_GREEN_HALF);
        tLedsPtr->Delay(SHOW_NUMBER_INTERVAL_MILLIS);
    }

    tStep--;
    tLedsPtr->Duration = tStep;
}

