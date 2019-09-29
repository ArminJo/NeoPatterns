/*
 * MatrixNeoPatterns.cpp
 *
 * Implements functions especially for 8x8 NeoPixel matrix.
 *
 *  SUMMARY
 *  You need to install "Adafruit NeoPixel" library under "Tools -> Manage Libraries..." or "Ctrl+Shift+I" -> use "neoPixel" as filter string.
 *  Extension are made to include more patterns and combined patterns
 *  and patterns for nxn NeoPixel matrix (tested with 8x8 and 10x10).
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

//#define TRACE
//#define DEBUG
//#define INFO
//#define WARN

#include "MatrixNeoPatterns.h"

// used for Ticker - modify line 12 of fonts.h to change font sizes
#include "fonts.h"

// Constructor - calls base-class constructor to initialize strip
MatrixNeoPatterns::MatrixNeoPatterns(uint8_t aColumns, uint8_t aRows, uint8_t aPin, uint8_t aMatrixGeometry, uint8_t aTypeOfPixel, // @suppress("Class members should be properly initialized")
        void (*aPatternCompletionCallback)(NeoPatterns*)) :
        NeoPixel(aColumns * aRows, aPin, aTypeOfPixel), MatrixNeoPixel(aColumns, aRows, aPin, aMatrixGeometry, aTypeOfPixel), NeoPatterns(
                aColumns * aRows, aPin, aTypeOfPixel, NULL) {
    OnPatternComplete = aPatternCompletionCallback;
}

// Update the pattern returns true if update has happened in order to give the caller a chance to manually change parameters (like color etc.)
bool MatrixNeoPatterns::Update(bool doShow) {
    if ((millis() - lastUpdate) > Interval) {
        switch (ActivePattern) {
        case PATTERN_FIRE:
            FireMatrixUpdate();
            break;
        case PATTERN_TICKER:
            TickerUpdate();
            break;
        case PATTERN_MOVE:
            MoveUpdate();
            break;
        case PATTERN_MOVING_PICTURE:
            MovingPicturePGMUpdate();
            break;
        default:
            NeoPatterns::update(false);
            break;
        }
        if (doShow) {
            show();
        }

        // remember last time of update
        lastUpdate = millis();
        return true;
    }
    return false;
}

int convolutionMatrixIntegerTimes256[3][3];
#define mapXYToArray(x, y, XRowLength) (((y) * (XRowLength)) + (x))

// The bottom line of fire which is set by random values every 4 updates
uint8_t sInitialHeat[8];

void setInitHeat() {
    for (int i = 0; i < 8;) {
        uint8_t tVal = random(40, 255);
        sInitialHeat[i++] = tVal;
    }
}

// initialize for fire -> set all to zero
void MatrixNeoPatterns::Fire(uint16_t aIntervalMillis, uint16_t aRepetitions) {
    ActivePattern = PATTERN_FIRE;
    Interval = aIntervalMillis;
    Direction = DIRECTION_UP;
    Index = aRepetitions;
// plus 1 pixel padding on each side
    if (MatrixNew) {
        free(MatrixNew);
    }
    if (MatrixOld) {
        free(MatrixOld);
    }
    MatrixNew = (uint8_t *) calloc((Rows + 2) * (Columns + 2), 1);
    MatrixOld = (uint8_t *) calloc((Rows + 2) * (Columns + 2), 1);

#ifdef INFO
    Serial.print(F("Starting Fire with refresh interval="));
    Serial.println(aIntervalMillis);
#endif
#ifdef DEBUG
    Serial.print("MatrixNew=0x");
    Serial.println((uint16_t) MatrixNew, HEX);
    Serial.print("MatrixOld=0x");
    Serial.println((uint16_t) MatrixOld, HEX);
#endif

    for (uint8_t cy = 0; cy < CONVOLUTION_MATRIX_SIZE; cy++) {
        for (uint8_t cx = 0; cx < CONVOLUTION_MATRIX_SIZE; cx++) {
            convolutionMatrixIntegerTimes256[cx][cy] = convolutionMatrix[cx][cy] * 256;
        }
    }
    setInitHeat();
}

/*
 * Fire for a 8x8 matrix works on a 10x10 temporary matrix (with borders for the convolution on each side)
 * Every 4 updates a new bottom heat line is generated.
 * Then the new matrix is computed with convolution from the old one.
 */
void MatrixNeoPatterns::FireMatrixUpdate() {
    if (Index % 4 == 0) {
        setInitHeat();
    }
    // First refresh (invisible) bottom line on every update
    for (int i = 1; i < Rows + 1; i++) {
        MatrixOld[mapXYToArray(i, Columns + 1, Rows + 2)] = sInitialHeat[i - 1];
    }

    /*
     * Process and Map center without padding from heat cells to LED colors.
     * Using pointer instead of indexing with x and y saves 1 ms!
     */
    for (uint8_t y = 1; y < Columns + 1; y++) {
        for (uint8_t x = 1; x < Rows + 1; x++) {
            // Convolution
            long tConvolutionSumTimes256 = 0;
            // using pointers here saves 1 ms
            int * convolutionMatrixIntegerTimes256Ptr = &convolutionMatrixIntegerTimes256[0][0];
            uint8_t * tFireMatrixOldPtr = &MatrixOld[mapXYToArray((x - 1), (y - 1), Rows + 2)];
            for (uint8_t cy = 0; cy < CONVOLUTION_MATRIX_SIZE; cy++) {
                for (uint8_t cx = 0; cx < CONVOLUTION_MATRIX_SIZE; cx++) {
                    tConvolutionSumTimes256 += *convolutionMatrixIntegerTimes256Ptr++ * *tFireMatrixOldPtr++;
//                    tConvolutionSumTimes256 += convolutionMatrixIntegerTimes256[cy][cx]
//                            * FireGrid8x8Old[map10x10((x - 1) + cx, (y - 1) + cy)];
                }
                tFireMatrixOldPtr += (Rows + 2) - 3;
            }

            uint8_t tNewHeatValue = MatrixOld[mapXYToArray(x, y, Rows + 2)] + (tConvolutionSumTimes256 / 256);

            // - COOLING and clipping to zero
            if (tNewHeatValue > MATRIX_FIRE_COOLING) {
                tNewHeatValue -= MATRIX_FIRE_COOLING;
            } else {
                tNewHeatValue = 0;
            }
            MatrixNew[mapXYToArray(x, y, Rows + 2)] = tNewHeatValue;

            // Heat color mapping
            setMatrixPixelColor(x - 1, y - 1, HeatColor(tNewHeatValue));
#ifdef DEBUG
            Serial.print("x=");
            Serial.print(x);
            Serial.print(" y=");
            Serial.print(y);
            Serial.print(" map=");
            Serial.println(mapXYToArray(x, y, Rows + 2));
#endif

        }
    }
// takes 11 milli seconds with float or 4 ms with integer

// toggle areas
    uint8_t * tPtr = MatrixNew;
    MatrixNew = MatrixOld;
    MatrixOld = tPtr;

    Index--;
    if (Index == 0) {
        /*
         * End of fire pattern -> cleanup
         */
        free(MatrixOld);
        MatrixOld = NULL;
        free(MatrixNew);
        MatrixNew = NULL;

        ActivePattern = PATTERN_NONE; // reset ActivePattern to enable polling for end of pattern.
        if (OnPatternComplete != NULL) {
            OnPatternComplete(this); // call the completion callback
        }
    }
}

/*
 * direction FORWARD is from left to right
 * Currently only 8x8 Graphics are supported
 */
void MatrixNeoPatterns::MovingPicturePGM(const uint8_t* aGraphics8x8ArrayPGM, color32_t aForegroundColor,
        color32_t aBackgroundColor, int8_t aGraphicsXOffset, int8_t aGraphicsYOffset, uint16_t aSteps, uint16_t aIntervalMillis,
        uint8_t aDirection) {
    ActivePattern = PATTERN_MOVING_PICTURE;
    DataPtr = aGraphics8x8ArrayPGM;
    Color1 = aForegroundColor;
    Color2 = aBackgroundColor;
    GraphicsXOffset = constrain(aGraphicsXOffset, -Rows, Rows);
    GraphicsYOffset = constrain(aGraphicsYOffset, -Columns, Columns);
    Interval = aIntervalMillis;
    TotalStepCounter = aSteps;
    Index = 0;
    Direction = aDirection;
#ifdef INFO
    Serial.print(F("Starting MovingPicturePGM with refresh interval="));
    Serial.print(aIntervalMillis);
    Serial.print(F("ms. Direction="));
    Serial.println(aDirection);
#endif
}

void MatrixNeoPatterns::MovingPicturePGMUpdate() {
    loadPicturePGM(DataPtr, 8, 8, Color1, Color2, GraphicsXOffset, GraphicsYOffset, false, true);
    if (Direction == DIRECTION_UP) {
        GraphicsYOffset++;
    } else if (Direction == DIRECTION_DOWN) {
        GraphicsYOffset--;
    } else if (Direction == DIRECTION_LEFT) {
        GraphicsXOffset++;
    } else if (Direction == DIRECTION_RIGHT) {
        GraphicsXOffset--;
    }
    Index++;
    if (Index >= TotalStepCounter) {
        ActivePattern = PATTERN_NONE; // reset ActivePattern to enable polling for end of pattern.
        if (OnPatternComplete != NULL) {
            OnPatternComplete(this); // call the completion callback
        }
    }
}

void MatrixNeoPatterns::showNumberOnMatrix(uint8_t aNumber, color32_t aColor) {
    char tStringBuffer[4];
    utoa(aNumber, tStringBuffer, 10); // number is aligned left
    uint8_t tStringLength = strlen(tStringBuffer);
    int8_t tXOffset = Columns - NUMBERS_FONT_WIDTH;
    clear();
    for (int8_t i = tStringLength; i > 0; i--) {
        loadPicturePGM(&fontNumbers4x6[(tStringBuffer[i - 1] - '0') * NUMBERS_FONT_HEIGHT], NUMBERS_FONT_WIDTH,
        NUMBERS_FONT_HEIGHT, aColor, COLOR32_BLACK, tXOffset);
        tXOffset -= NUMBERS_FONT_WIDTH;
        if (tXOffset < -3) {
            break;
        }
    }
}

/*
 * 103(UP/DOWN) or 129 (LEFT/RIGHT) usec for moving 8x8
 * show takes ca. 1000 us
 * assumes Z type mapping
 */
void MatrixNeoPatterns::moveArrayContent(uint8_t aDirection) {
    if (LayoutMappingFunction != NULL) {
#ifdef WARN
        Serial.print(F("moveArrayContent with only one parameter does actually not support other than Z type mappings."));
#endif
    } else {

        uint8_t tBytesToSkipForOneRow = Columns * BytesPerPixel;

#ifdef TRACE
        Serial.print("moveArrayContent aDirection=");
        Serial.print(aDirection);
        Serial.print(" tNumPixels=");
        Serial.print(numPixels());
        Serial.print(" BytesPerPixel=");
        Serial.print(BytesPerPixel);
        Serial.print(" tBytesToSkipForOneRow=");
        Serial.println(tBytesToSkipForOneRow);
#endif

        if (aDirection == DIRECTION_UP) {
            memmove(pixels + tBytesToSkipForOneRow, pixels, numBytes - tBytesToSkipForOneRow);
            memset(pixels, 0, tBytesToSkipForOneRow);
        } else if (aDirection == DIRECTION_DOWN) {
            memmove(pixels, pixels + tBytesToSkipForOneRow, numBytes - tBytesToSkipForOneRow);
            memset(pixels + (numBytes - tBytesToSkipForOneRow), 0, tBytesToSkipForOneRow);
        } else if (aDirection == DIRECTION_LEFT) {
            memmove(pixels + BytesPerPixel, pixels, numBytes - BytesPerPixel);
            for (int i = 0; i < Rows; ++i) {
                memset(pixels, 0, BytesPerPixel);
                pixels += tBytesToSkipForOneRow;
            }
        } else if (aDirection == DIRECTION_RIGHT) {
            memmove(pixels, pixels + BytesPerPixel, numBytes - BytesPerPixel);
            pixels = pixels + tBytesToSkipForOneRow - BytesPerPixel;
            for (int i = 0; i < Rows; ++i) {
                memset(pixels, 0, BytesPerPixel);
                pixels += tBytesToSkipForOneRow;
            }
        }
    }
}

/*
 *  156/136(UP/DOWN) or 246 (LEFT/RIGHT)  usec for moving 8x8
 *  assumes LED Rows are in X direction
 */
void MatrixNeoPatterns::moveArrayContent(uint8_t aDirection, color32_t aBackgroundColor) {
    if (Geometry == (NEO_MATRIX_BOTTOM | NEO_MATRIX_RIGHT | NEO_MATRIX_ROWS | NEO_MATRIX_PROGRESSIVE)) {

        uint16_t tNumBytes = numBytes;
        uint8_t tBytesToSkipForOneRow = Columns * BytesPerPixel;

#ifdef TRACE
        Serial.print("moveArrayContent aDirection=");
        Serial.print(aDirection);
        Serial.print(" tNumPixels=");
        Serial.print(numLEDs);
        Serial.print(" PixelColorStorageSize=");
        Serial.print(BytesPerPixel);
        Serial.print(" tBytesToSkipForOneRow=");
        Serial.println(tBytesToSkipForOneRow);
#endif
        if (aDirection == DIRECTION_UP) {
            memmove(pixels + tBytesToSkipForOneRow, pixels, tNumBytes - tBytesToSkipForOneRow);
            for (uint8_t i = 0; i < Rows; ++i) {
                setPixelColor(i, aBackgroundColor);
            }
        } else if (aDirection == DIRECTION_DOWN) {
            memmove(pixels, pixels + tBytesToSkipForOneRow, tNumBytes - tBytesToSkipForOneRow);
            for (uint16_t i = numLEDs - Columns; i < numLEDs; ++i) {
                setPixelColor(i, aBackgroundColor);
            }
        } else if (aDirection == DIRECTION_LEFT) {
            memmove(pixels + BytesPerPixel, pixels, tNumBytes - BytesPerPixel);
            for (uint16_t i = 0; i < numLEDs; i += Columns) {
                setPixelColor(i, aBackgroundColor);
            }
        } else if (aDirection == DIRECTION_RIGHT) {
            memmove(pixels, pixels + BytesPerPixel, tNumBytes - BytesPerPixel);
            for (uint16_t i = Columns - 1; i <= numLEDs; i += Columns) {
                setPixelColor(i, aBackgroundColor);
            }
        }
    } else {
        for (uint8_t y = 0; y < Rows; ++y) {
            for (uint8_t x = 0; x < Columns; ++x) {
                if (aDirection == DIRECTION_UP) {
                    if (y != (Rows - 1)) {
                        setMatrixPixelColor(x, y, getMatrixPixelColor(x, y + 1));
                    } else {
                        //padding lower line
                        setMatrixPixelColor(x, (Rows - 1), aBackgroundColor);
                    }
                } else if (aDirection == DIRECTION_DOWN) {
                    if (y != (Rows - 1)) {
                        setMatrixPixelColor(x, (Rows - 1) - y, getMatrixPixelColor(x, (Rows - 2) - y));
                    } else {
                        //padding upper line
                        setMatrixPixelColor(x, 0, aBackgroundColor);
                    }
                } else if (aDirection == DIRECTION_LEFT) {
                    if (x != (Columns - 1)) {
                        setMatrixPixelColor(x, y, getMatrixPixelColor(x + 1, y));
                    } else {
                        //padding right
                        setMatrixPixelColor(x, y, aBackgroundColor);
                    }
                } else if (aDirection == DIRECTION_RIGHT) {
                    if (x != (Columns - 1)) {
                        setMatrixPixelColor(((Columns - 1) - x), y, getMatrixPixelColor(((Columns - 1) - x) - 1, y));
                    } else {
                        //padding left
                        setMatrixPixelColor(x, y, aBackgroundColor);
                    }
                }
            }
        }
    }
}

/*
 * Moving for arrays with 0 at lower right and horizontal rows
 * aSteps == 1 is equivalent to just calling moveArrayContent()
 */
void MatrixNeoPatterns::Move(uint8_t aDirection, uint16_t aSteps, uint16_t aIntervalMillis, bool aMoveDirect,
        color32_t aBackgroundColor) {
    if (aMoveDirect) {
        moveArrayContent(aDirection, aBackgroundColor);
        aSteps--;
    }
    if (aSteps > 0) {
        Color2 = aBackgroundColor;
        Direction = aDirection;
        Interval = aIntervalMillis;
        Index = aSteps;
        ActivePattern = PATTERN_MOVE;
#ifdef INFO
        Serial.print(F("Starting Move with refresh interval="));
        Serial.print(aIntervalMillis);
        Serial.print(F("ms. Direction="));
        Serial.println(aDirection);
#endif
    }
}

void MatrixNeoPatterns::MoveUpdate() {
#ifdef DEBUG
    Serial.print("MoveUpdate TotalSteps=");
    Serial.println(TotalStepCounter);
#endif
    moveArrayContent(Direction, Color2);
    Index--;
    if (Index == 0) {
        ActivePattern = PATTERN_NONE; // reset ActivePattern to enable polling for end of pattern.
        if (OnPatternComplete != NULL) {
            OnPatternComplete(this); // call the completion callback
        }
    }
}

/*
 * only directions DIRECTION_LEFT and DIRECTION_UP are supported yet
 */
void MatrixNeoPatterns::Ticker(const char* aStringPtr, color32_t aForegroundColor, color32_t aBackgroundColor,
        uint16_t aIntervalMillis, uint8_t aDirection) {
    TickerInit(aStringPtr, aForegroundColor, aBackgroundColor, aIntervalMillis, aDirection);
}
void MatrixNeoPatterns::TickerPGM(const char* aStringPtrPGM, color32_t aForegroundColor, color32_t aBackgroundColor,
        uint16_t aIntervalMillis, uint8_t aDirection) {
    TickerInit(aStringPtrPGM, aForegroundColor, aBackgroundColor, aIntervalMillis, aDirection, FLAG_TICKER_DATA_IN_FLASH);
}

void MatrixNeoPatterns::Ticker(__FlashStringHelper * aStringPtrPGM, color32_t aForegroundColor, color32_t aBackgroundColor,
        uint16_t aIntervalMillis, uint8_t aDirection) {
    TickerInit(reinterpret_cast<const char*>(aStringPtrPGM), aForegroundColor, aBackgroundColor, aIntervalMillis, aDirection,
            FLAG_TICKER_DATA_IN_FLASH);
}

void MatrixNeoPatterns::TickerInit(const char* aStringPtr, color32_t aForegroundColor, color32_t aBackgroundColor,
        uint16_t aIntervalMillis, uint8_t aDirection, uint8_t aFlags) {
    ActivePattern = PATTERN_TICKER;
    PatternFlags = aFlags;
    Interval = aIntervalMillis;

    DataPtr = (const uint8_t*) aStringPtr;
    Color1 = aForegroundColor;
    Color2 = aBackgroundColor;
    Direction = aDirection;

#ifdef INFO
    Serial.print(F("Starting Ticker with refresh interval="));
    Serial.print(aIntervalMillis);
    Serial.print(F("ms. Text=\""));
    if (PatternFlags & FLAG_TICKER_DATA_IN_FLASH) {
        Serial.print(reinterpret_cast<const __FlashStringHelper *>(aStringPtr));
    } else {
        Serial.print(aStringPtr);
    }
    Serial.print(F("\" Direction="));
    Serial.println(aDirection);
#endif
// try to position fonts at middle
    GraphicsXOffset = (Rows - FONT_WIDTH) / 2; // positive
    GraphicsYOffset = (FONT_HEIGHT - Columns) / 2; // negative since character must be shifted down
// Set start position for first character to move in
    if (aDirection == DIRECTION_LEFT) {
        GraphicsXOffset = Columns;
    } else if (aDirection == DIRECTION_UP) {
        GraphicsYOffset = -Rows;
    } else {
#ifdef WARN
        Serial.println(F("Direction="));
        Serial.print(aDirection);
        Serial.println(F(" not supported (yet)"));
#endif
        aDirection = DIRECTION_LEFT;
    }
}

void MatrixNeoPatterns::TickerUpdate() {
    char tChar;
    char tNextChar;
// left character
    if (PatternFlags & FLAG_TICKER_DATA_IN_FLASH) {
        tChar = pgm_read_byte(DataPtr);
        tNextChar = pgm_read_byte(DataPtr + 1);
    } else {
        tChar = *DataPtr;
        tNextChar = *(DataPtr + 1);
    }

#ifdef DEBUG
    Serial.print(F("tChar="));
    Serial.print(tChar);
    Serial.print(F(" tNextChar="));
    Serial.println(tNextChar);
#endif

    const uint8_t* tGraphics8x8ArrayPtr = &font_PGM[(tChar - FONT_START) * FONT_HEIGHT];
    loadPicturePGM(tGraphics8x8ArrayPtr, FONT_WIDTH, FONT_HEIGHT, Color1, Color2, GraphicsXOffset, GraphicsYOffset,
            (tNextChar == '\0'));
    if (tNextChar != '\0') {
        // check if next character is moving in
        if (Direction == DIRECTION_LEFT && (GraphicsXOffset + FONT_WIDTH) < Columns) {
            char tNextNextChar;
            if (PatternFlags & FLAG_TICKER_DATA_IN_FLASH) {
                tNextNextChar = pgm_read_byte(DataPtr + 2);
            } else {
                tNextNextChar = *(DataPtr + 2);
            }
            /*
             * Display next character at right
             */
            tGraphics8x8ArrayPtr = &font_PGM[(tNextChar - FONT_START) * FONT_HEIGHT];
            loadPicturePGM(tGraphics8x8ArrayPtr, FONT_WIDTH, FONT_HEIGHT, Color1, Color2, GraphicsXOffset + FONT_WIDTH,
                    GraphicsYOffset, (tNextNextChar == '\0'));
            // check if next/next character can be displayed (needed for small font width)
            // do not check for next/next/next char != '\0' here!
            if ((GraphicsXOffset + (2 * FONT_WIDTH)) < Columns) {
                if (tNextNextChar != '\0') {
                    tGraphics8x8ArrayPtr = &font_PGM[(tNextNextChar - FONT_START) * FONT_HEIGHT];
                    loadPicturePGM(tGraphics8x8ArrayPtr, FONT_WIDTH, FONT_HEIGHT, Color1, Color2,
                            GraphicsXOffset + (2 * FONT_WIDTH), GraphicsYOffset, false);
                }
            }
        }
        if (Direction == DIRECTION_UP && (GraphicsYOffset - FONT_HEIGHT) > -Columns) {
            tGraphics8x8ArrayPtr = &font_PGM[(tNextChar - FONT_START) * FONT_HEIGHT];
            loadPicturePGM(tGraphics8x8ArrayPtr, FONT_WIDTH, FONT_HEIGHT, Color1, Color2, GraphicsXOffset,
                    GraphicsYOffset - FONT_HEIGHT, false);
        }
    }
    /*
     * character is moved out of matrix, so switch to next one
     */
    if (GraphicsXOffset == -FONT_WIDTH || GraphicsYOffset == FONT_HEIGHT) {

        if (tNextChar == '\0') {
            ActivePattern = PATTERN_NONE; // reset ActivePattern to enable polling for end of pattern.
            if (OnPatternComplete != NULL) {
                OnPatternComplete(this); // call the completion callback
                return;
            }
        }
        // switch to next character
        DataPtr++;
        if (Direction == DIRECTION_LEFT) {
            GraphicsXOffset = 0;
        } else if (Direction == DIRECTION_UP) {
            GraphicsYOffset = 0;
        }
    }

// shift offsets
    if (Direction == DIRECTION_LEFT) {
        GraphicsXOffset--;
    } else if (Direction == DIRECTION_UP) {
        GraphicsYOffset++;
    }
}

/*
 * Sample callback handler for MatrixNeoPatterns
 */
void MatrixPatternsDemo(NeoPatterns * aLedsPtr) {
    MatrixNeoPatterns* tLedsPtr = (MatrixNeoPatterns*) aLedsPtr;
    static int8_t sState = 0;
    static uint8_t sHeartDirection = DIRECTION_DOWN;
    static int8_t sTickerDirection = DIRECTION_LEFT;

#ifdef INFO
    Serial.print("sState=");
    Serial.println(sState);
#endif
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
        tLedsPtr->TickerPGM(PSTR("I love Neopixel"), NeoPatterns::Wheel(0), COLOR32_BLACK, 80, sTickerDirection);
        sTickerDirection--;
        if (sTickerDirection < 0) {
            sTickerDirection = DIRECTION_LEFT;
        }
        break;
    case 1:
        tYOffset = HEART_HEIGHT;
        if (sHeartDirection == DIRECTION_UP) {
            tYOffset = -HEART_HEIGHT;
        }
        // move in
        tLedsPtr->MovingPicturePGM(heart8x8, COLOR32_RED_HALF, COLOR32_BLACK, tXOffset, tYOffset, tSteps, 100, sHeartDirection);
        break;

    case 2:
// Next 4 cases show 2 heart beats
        aLedsPtr->ProcessSelectiveColor(aLedsPtr->Color1, &DimColor, 6, 40);
        break;
    case 3:
        aLedsPtr->ProcessSelectiveColor(aLedsPtr->Color1, &BrightenColor, 6, 40);
        break;
    case 4:
        aLedsPtr->ProcessSelectiveColor(aLedsPtr->Color1, &DimColor, 6, 40);
        break;
    case 5:
        aLedsPtr->ProcessSelectiveColor(aLedsPtr->Color1, &BrightenColor, 6, 40);
        break;

    case 6:
        // move out
        tLedsPtr->Move(sHeartDirection, tLedsPtr->Rows, 100, true);
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
        tLedsPtr->Fire(100, 50);
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

// EXAMPLE ACTIONS for case
//        aLedsPtr->Delay(40000);
//        aLedsPtr->TickerPGM(PSTR("I love you"), COLOR32_RED, COLOR32_BLACK, 50, DIRECTION_LEFT);
//        aLedsPtr->MovingPicturePGM(heart8x8, COLOR32_RED, COLOR32_BLACK, 0, -8, 9, 100, DIRECTION_UP);
//        aLedsPtr->MovingPicturePGM(heart8x8, COLOR32_RED, COLOR32_BLACK, 0, 0, 9, 100, DIRECTION_UP);
//        aLedsPtr->MovingPicturePGM(heart8x8, COLOR32_RED, COLOR32_BLACK, 0, 8, 9, 100, DIRECTION_DOWN);
//        aLedsPtr->MovingPicturePGM(heart8x8, COLOR32_RED, COLOR32_BLACK, 0, 0, 9, 100, DIRECTION_DOWN);
//        aLedsPtr->MovingPicturePGM(heart8x8, COLOR32_RED, COLOR32_BLACK, -8, 0, 9, 100, DIRECTION_LEFT);
//        aLedsPtr->MovingPicturePGM(heart8x8, COLOR32_RED, COLOR32_BLACK, 0, 0, 9, 100, DIRECTION_LEFT);
//        aLedsPtr->MovingPicturePGM(heart8x8, COLOR32_RED, COLOR32_BLACK, 8, 0, 9, 100, DIRECTION_RIGHT);
//        aLedsPtr->MovingPicturePGM(heart8x8, COLOR32_RED, COLOR32_BLACK, 0, 0, 9, 100, DIRECTION_RIGHT);
//        aLedsPtr->FadeSelective(COLOR32_RED, COLOR32_GREEN, 20, 40);

    default:
        aLedsPtr->Delay(1);
#ifdef WARN
        Serial.print("case ");
        Serial.print(tState);
        Serial.println(" not implemented");
#endif
        break;
    }

    sState++;
}

#define TEST_DELAY_MILLIS 2000
void myMoveTest1(MatrixNeoPatterns* aLedsPtr) {
    aLedsPtr->moveArrayContent(DIRECTION_UP, COLOR32_RED_HALF);
    delay(TEST_DELAY_MILLIS);
    aLedsPtr->moveArrayContent(DIRECTION_DOWN, COLOR32_BLUE_HALF);
    delay(TEST_DELAY_MILLIS);
    aLedsPtr->moveArrayContent(DIRECTION_LEFT, COLOR32_GREEN_HALF);
    delay(TEST_DELAY_MILLIS);
    aLedsPtr->moveArrayContent(DIRECTION_RIGHT);
    delay(TEST_DELAY_MILLIS);
}

void myMoveTest2(MatrixNeoPatterns* aLedsPtr) {
    aLedsPtr->moveArrayContent(DIRECTION_DOWN, COLOR32_BLUE_HALF);
    delay(TEST_DELAY_MILLIS);
    aLedsPtr->moveArrayContent(DIRECTION_UP, COLOR32_RED_HALF);
    delay(TEST_DELAY_MILLIS);
    aLedsPtr->moveArrayContent(DIRECTION_RIGHT);
    delay(TEST_DELAY_MILLIS);
    aLedsPtr->moveArrayContent(DIRECTION_LEFT, COLOR32_GREEN_HALF);
    delay(TEST_DELAY_MILLIS);
}

void myLoadTest(MatrixNeoPatterns* aLedsPtr) {
    aLedsPtr->loadPicturePGM(heart8x8, 8, 8, COLOR32_RED_HALF, COLOR32_BLACK, 0, 0, false, true);
    delay(TEST_DELAY_MILLIS);
    myMoveTest1(aLedsPtr);
    aLedsPtr->loadPicturePGM(heart8x8, 8, 8, COLOR32_RED_HALF, COLOR32_BLACK, 2, 0, false, true);
    delay(TEST_DELAY_MILLIS);
    myMoveTest2(aLedsPtr);
    aLedsPtr->loadPicturePGM(heart8x8, 8, 8, COLOR32_RED_HALF, COLOR32_BLACK, 0, 2, false, true);
    delay(TEST_DELAY_MILLIS);
    aLedsPtr->loadPicturePGM(heart8x8, 8, 8, COLOR32_RED_HALF, COLOR32_BLACK, -2, -2, false, true);
    delay(TEST_DELAY_MILLIS);
}

const uint8_t fontNumbers4x6[] PROGMEM = { 0x03, 0x05, 0x05, 0x05, 0x06, 0x00, // 0x30 0
        0x02, 0x06, 0x02, 0x02, 0x07, 0x00, // 0x31 1
        0x06, 0x01, 0x02, 0x04, 0x07, 0x00, // 0x32 2
        0x06, 0x01, 0x02, 0x01, 0x06, 0x00, // 0x33
        0x01, 0x05, 0x07, 0x01, 0x01, 0x00, // 0x34
        0x07, 0x04, 0x06, 0x01, 0x06, 0x00, // 0x35
        0x02, 0x04, 0x06, 0x05, 0x02, 0x00, // 0x36
        0x07, 0x01, 0x03, 0x02, 0x02, 0x00, // 0x37
        0x02, 0x05, 0x02, 0x05, 0x02, 0x00, // 0x38 8
        0x02, 0x05, 0x03, 0x01, 0x02, 0x00 // 0x39 9
        };

