/*
 *  ____                     _      ______ _____    _____
 * / __ \                   | |    |  ____|  __ \  |  __ \
 *| |  | |_ __   ___ _ __   | |    | |__  | |  | | | |__) |__ _  ___ ___
 *| |  | | '_ \ / _ \ '_ \  | |    |  __| | |  | | |  _  // _` |/ __/ _ \
 *| |__| | |_) |  __/ | | | | |____| |____| |__| | | | \ \ (_| | (_|  __/
 * \____/| .__/ \___|_| |_| |______|______|_____/  |_|  \_\__,_|\___\___|
 *       | |
 *       |_|
 */
/*
 *  OpenLedRace.cpp
 *
 *  Extended version of the OpenLedRace "version Basic for PCB Rome Edition. 2 Player, without Boxes Track"
 *
 *  Extensions are:
 *  Accelerator MPU6050 input.
 *  Classes for Car, Bridge, Ramp and Loop.
 *  Dynamic activation of up to 4 cars.
 *  Light effects by NeoPattern library.
 *  Tone generation without dropouts by use of hardware timer output.
 *  Winner melody by PlayRTTTL library.
 *  Compensation for blocked millis() timer during draw.
 *  Checks for RAM availability.
 *  Overlapping of cars is handled by using addPixelColor() for drawing.
 *
 *  One 1200 mA Li-ion Battery lasts for around 6 hours
 *
 *  You need to install "Adafruit NeoPixel" library under "Tools -> Manage Libraries..." or "Ctrl+Shift+I" -> use "neoPixel" as filter string
 *  You also need to install "NeoPatterns" and "PlayRtttl" library under "Tools -> Manage Libraries..." or "Ctrl+Shift+I"
 *
 *  Copyright (C) 2020-2023  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
 *
 *  This file is part of OpenledRace https://github.com/ArminJo/OpenledRace.
 *  This file is part of NeoPatterns https://github.com/ArminJo/NeoPatterns.
 *
 *  NeoPatterns and OpenledRace is free software: you can redistribute it and/or modify
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
 *  along with this program. If not, see <http://www.gnu.org/licenses/gpl.html>.
 */
/*
 * Open LED Race
 * An minimalist cars race for LED strip
 *
 * by gbarbarov@singulardevices.com for Arduino day Seville 2019
 * https://www.hackster.io/gbarbarov/open-led-race-a0331a
 * https://twitter.com/openledrace
 *
 * https://gitlab.com/open-led-race
 * https://openledrace.net/open-software/
 */

#include <Arduino.h>

#include "PlayRtttl.hpp"

/*
 * Enable only these 3 patterns to save program space
 */
#define ENABLE_PATTERN_SCANNER_EXTENDED
#define ENABLE_PATTERN_COLOR_WIPE
#define ENABLE_PATTERN_STRIPES
#include "NeoPatterns.hpp"

//#define TRACE
//#define DEBUG
#define INFO    // Enable additional informative output like e.g. offsets,
#include "DebugLevel.h" // to propagate debug level

#if defined(INFO) && defined(__AVR__)
#include "AVRUtils.h" // for initStackFreeMeasurement() and printRAMInfo()
#endif

// for hunting errors
//#include "AvrTracing.hpp"

#define VERSION_EXAMPLE "1.3"
// 1.4 - work in progress
// 1.3 Moved Bridge and loop, VU Bar animations
// 1.2 Improvements from Hannover Maker Faire
// 1.1 Hannover Maker Faire version

//#define BIG_NUMBERS_ALTERNATIVE         // use alternative font for big numbers

//#define USE_NO_LCD                      // this suppresses the error tone and print if LCD was not found
//#define TEST_MODE
//#define TIMING_TEST
#define ENABLE_ACCELERATOR_INPUT        // Enables readout of MPU6050 IMU as input. Saves 320 bytes program space and 10 bytes RAM.
#define USE_ACCELERATION_NEOPIXEL_BARS  // Shows the low pass value of user acceleration input (button or IMU) on an 8 Neopixel bar
//#define BRIDGE_NO_NEOPATTERNS           // No patterns on bridge. Saves 13 bytes RAM
//#define LOOP_NO_NEOPATTERNS             // No patterns on loop. Saves 4 bytes RAM

#if defined(ENABLE_ACCELERATOR_INPUT)
/*
 * Modifiers for the MPU6050IMUData library to save speed and space
 */
#define DO_NOT_USE_GYRO
#define USE_ONLY_ACCEL_FLOATING_OFFSET
//#define USE_SOFT_I2C_MASTER // Requires SoftI2CMaster.h + SoftI2CMasterConfig.h. Saves 2110 bytes program memory and 200 bytes RAM compared with Arduino Wire
#include "MPU6050IMUData.hpp" // This defines USE_SOFT_I2C_MASTER, if SoftI2CMasterConfig.h is available
#endif // #if defined(ENABLE_ACCELERATOR_INPUT)

#include "LongUnion.h"

#define LCD_I2C_ADDRESS 0x27    // Default LCD address is 0x27 for a 20 chars and 4 line / 2004 display
#include "LiquidCrystal_I2C.hpp"  // This defines USE_SOFT_I2C_MASTER, if SoftI2CMasterConfig.h is available. Use only the modified version delivered with this program!
LiquidCrystal_I2C myLCD(LCD_I2C_ADDRESS, 20, 4);
#define USE_SERIAL_2004_LCD // required by LCDBigNumbers.hpp
#include "LCDBigNumbers.hpp" // Include sources for LCD big number generation

LCDBigNumbers BigNumbers(&myLCD, BIG_NUMBERS_FONT_3_COLUMN_4_ROWS_VARIANT_1);

void checkForLCDConnected();
bool sSerialLCDAvailable;

/*
 * Pin layout - adapt it to your need
 */
#define PIN_PLAYER_1_VU_BAR     2 // RED
#define PIN_PLAYER_2_VU_BAR     3// GREEN
#define PIN_PLAYER_1_BUTTON     4 // RED
#define PIN_PLAYER_2_BUTTON     5 // GREEN

#if !defined(ENABLE_ACCELERATOR_INPUT)
#define PIN_PLAYER_3_BUTTON     6
#endif
#define PIN_RESET_GAME_BUTTON   7

#define PIN_NEOPIXEL_TRACK      8
#if defined(TIMING_TEST)
#define PIN_TIMING              9
#endif
#define PIN_MANUAL_PARAMETER_MODE  9 // if connected to ground, analog inputs for parameters are used

#define PIN_BUZZER              11   // must be pin 11, since we use the direct hardware tone output for ATmega328, which is not disturbed by Neopixel show()

#define PIN_GRAVITY             A0
#define PIN_FRICTION            A1
#define PIN_DRAG                A2

#define ONLY_PLOTTER_OUTPUT_PIN 12 // Verbose output to Arduino Serial Monitor is disabled, if connected to ground. This is intended for Arduino Plotter mode.
bool sOnlyPlotterOutput;

#define ANALOG_OFFSET   20   // Bias/offset to get real 0 analog value, because of high LED current on Breadboard, which cause a ground bias.

/*
 * The track
 * Think of 1 Pixel = 1 meter
 */
#define NUMBER_OF_TRACK_PIXELS  300 // Number of LEDs in strip

/*
 * The cars
 * Maximum number of cars supported. Each car is individually activated at runtime
 * by first press of its button or movement of its accelerator input.
 */
#if defined(ENABLE_ACCELERATOR_INPUT)
#define NUMBER_OF_CARS            2 // Currently we can handle only 2 distinct accelerometers.
#define ACCELERATOR_TRIGGER_VALUE 32 // The value of getAcceleratorValueShift8() to trigger a new race
#else
#define NUMBER_OF_CARS            4
#endif
#define CAR_1_COLOR     (color32_t)COLOR32_RED
#define CAR_2_COLOR     (color32_t)COLOR32_GREEN
#define CAR_3_COLOR     (color32_t)COLOR32_BLUE
const char Car1ColorString[] PROGMEM = "RED";
const char Car2ColorString[] PROGMEM = "GREEN";
const char Car3ColorString[] PROGMEM = "BLUE";

/*
 * The bridge with a ramp up, a flat platform and a ramp down
 */
#define NUMBER_OF_BRIDGES         1
#define BRIDGE_1_START           87
#define BRIDGE_1_RAMP_LENGTH     21 // in pixel
#define BRIDGE_1_PLATFORM_LENGTH 20 // > 0 for bridges with a ramp up a flat bridge and a ramp down
#define BRIDGE_1_HEIGHT          15 // in pixel -> 45 degree slope here
#define RAMP_COLOR               COLOR32_CYAN_QUARTER // COLOR32(0,64,64)
#define RAMP_COLOR_DIMMED        COLOR32(0,8,8)

/*
 * The loop
 */
#define NUMBER_OF_LOOPS           1
#define LOOP_1_UP_START         228
#define LOOP_1_LENGTH            48 // in pixel
#define LOOP_COLOR              COLOR32_PURPLE_QUARTER // COLOR32(64,0,64)
#define LOOP_DIMMED_COLOR       COLOR32(8,0,8)
#define GAMMA_FOR_DIMMED_VALUE  160

NeoPatterns track = NeoPatterns(NUMBER_OF_TRACK_PIXELS, PIN_NEOPIXEL_TRACK, NEO_GRB + NEO_KHZ800);

#if defined(USE_ACCELERATION_NEOPIXEL_BARS)
//#define ACCELERATION_BAR_SCALE_VALUE    100
/*
 * The central NeoPixel object used for every bar, since there is no persistence needed for the bar pixel content
 */
NeoPixel AccelerationCommonNeopixelBar = NeoPixel(8, 0, NEO_GRB + NEO_KHZ800);
#endif

/*
 * Game loop timing
 */
#define MILLISECONDS_PER_LOOP 20 // 50 fps
// Required for correction of the millis() timer
#define MILLIS_FOR_TRACK_TO_SHOW  (NUMBER_OF_TRACK_PIXELS / 33) // 33 pixels can be sent per ms. 9ms for 300 pixels.
#define ANIMATION_INTERVAL_MILLIS 10000

#define FULL_GRAVITY 100 // 100 is gravity for a vertical slope, 0 for a horizontal and 70 (sqrt(0,5)*100) for 45 degree.

/*
 * Sound
 */
unsigned long sBeepEndMillis = 0; // for special sounds - overtaking and leap
int sBeepFrequency = 0;
bool sSoundEnabled = true; // not really used yet - always true

/*
 * Race control
 */
#if defined(TEST_MODE)
#define START_ANIMATION_MILLIS               500 // The duration of the start animation
#define WINNER_ANIMATION_DELAY_MILLIS       1000 // The time to show the end of race situation
#define LAPS_PER_RACE                        255
#else
#define START_ANIMATION_MILLIS              1500 // The duration of the start animation
#define WINNER_ANIMATION_DELAY_MILLIS       2000 // The time to show the end of race situation
#define WINNER_MINIMAL_ANIMATION_DURATION_MILLIS 2000 // The minimum time time to show the winner animation + sound
#define LAPS_PER_RACE                          5
#endif

// Main loop modes
#define MODE_IDLE 0
#define MODE_START 1
#define MODE_RACE 2
uint8_t sLoopMode = MODE_IDLE;
uint16_t sLoopCountForDebugPrint;
uint8_t sIndexOfLeadingCar = 0; // To check for playing overtaking sound.

/*
 * Car control
 * we get a press each 70 to 150 milliseconds / each 3th to 7th loop
 */
#define ENERGY_PER_BUTTON_PRESS     0.2 // As pixel per loop
#define FRICTION_PER_LOOP           0.006
#define AERODYNAMIC_DRAG_PER_LOOP   0.0002 // can also be 0.0
#define GRAVITY_FACTOR_FOR_MAP      0.0007 // gravity constant for gravity values from 0 to 100 => gravity 1 is 0.02 -- 0.001 is too much.

/*
 * Forward declarations
 */
void startRace();
void printStartMessage();
void resetAllCars();
void resetAndDrawTrack(bool aDoAnimation);
void resetAndShowTrackWithoutCars();
bool isInRegion(uint16_t aPixelPosition, unsigned int aRegionFirst, unsigned int aRegionLength);
bool isAnyCarInRegion(unsigned int aRegionFirst, unsigned int aRegionLength);
void playError();
void playShutdownMelody();
void playMelodyAndShutdown();
void checkAndHandleWinner();
void checkForOvertakingLeaderCar();
bool checkAllInputs();
void printConfigPinInfo(Print *aSerial, uint8_t aConfigPinNumber, const __FlashStringHelper *aConfigPinDescription);

extern volatile unsigned long timer0_millis; // Used for ATmega328P to adjust for missed millis interrupts

/*
 * Helper macro for getting a macro definition as string
 */
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

void myTone(int aFrequency) {
    tone(PIN_BUZZER, aFrequency);
#if defined(TCCR2A)
    TCCR2A |= _BV(COM2A0); // switch to direct toggle output at OC2A / pin 11 to enable direct hardware tone output
#endif
}

/*******************************************************************************************
 * The RAMP class
 * Sets gravity for the ramps and allocates NeoPatterns for the ramps to AnimationForIdle
 *
 * Requires 10 bytes RAM + sizeof(NeoPatterns) per ramp
 * One ramp consists of (aRampLength + 1) pixel with the first and last with half the gravity
 * aRampUpStart - First pixel with gravity = 1/2 gravity
 * aRampLength - aRampUpStart + aRampLength is last ramp pixel with gravity = 1/2 gravity
 * Gravity - 100 is full gravity for vertical slope
 *******************************************************************************************/
class Ramp {
public:
#if !defined(BRIDGE_NO_NEOPATTERNS)
    NeoPatterns *TrackPtr;
    NeoPatterns *RampPatterns;
    bool isInitialized;
#endif
    uint16_t StartPositionOnTrack;
    uint8_t RampLength;
    uint8_t RampHeight;
    bool isRampDown;

    void init(NeoPatterns *aTrackPtr, uint16_t aRampUpStartPositionOnTrack, uint8_t aRampHeight, uint8_t aRampLength,
            bool aIsRampDown) {
        StartPositionOnTrack = aRampUpStartPositionOnTrack;
        RampHeight = aRampHeight;
        RampLength = aRampLength;
        isRampDown = aIsRampDown;
#if !defined(BRIDGE_NO_NEOPATTERNS)
        TrackPtr = aTrackPtr;

        /*
         * NeoPatterns segments to control light effects on both ramps
         * Call malloc() and free() before, since the compiler calls the constructor even
         * when the result of malloc() is NULL, which leads to overwrite low memory.
         */
        void *tMallocTest = malloc(sizeof(NeoPatterns)); // 67 + 2
        if (tMallocTest != NULL) {
            free(tMallocTest);
            RampPatterns = new NeoPatterns(TrackPtr, StartPositionOnTrack, RampLength, false);
            isInitialized = true;
        } else {
            Serial.print(F("Not enough heap memory ("));
            Serial.print(sizeof(NeoPatterns) + 2);
            Serial.println(F(") for RampPatterns."));
        }

#else
        (void) aTrackPtr;
#endif // !defined(BRIDGE_NO_NEOPATTERNS)
    }

    /*
     * Compute Gravity acceleration on the fly
     * @ return Values from -100 to +100 with 100 is gravity for a vertical slope, 0 for a horizontal and 70 (sqrt(0,5)*100) for 45 degree.
     *          Positive values increases speed, negative values decreases speed.
     */
    int8_t getGravityAcceleration(uint16_t aPositionOnTrack) {
#if defined(DEBUG)
        if (aPositionOnTrack < StartPositionOnTrack || aPositionOnTrack > StartPositionOnTrack + RampLength) {
            Serial.print(F("Error! position="));
            Serial.print(aPositionOnTrack);
            Serial.print(F(" is not on ramp "));
            Serial.print(StartPositionOnTrack);
            Serial.print(F(" to "));
            Serial.println(StartPositionOnTrack + RampLength);
        }
#endif
        uint8_t tPositionOnRamp = aPositionOnTrack - StartPositionOnTrack;
        int8_t tResultingForce = ((uint16_t) (RampHeight * FULL_GRAVITY)) / RampLength; // results in values from 0 to 100
        if (!isRampDown) {
            tResultingForce = -tResultingForce; // deceleration for ramp up
        }
        if (tPositionOnRamp == 0 || tPositionOnRamp == RampLength) {
            return tResultingForce / 2; // Start and end with half deceleration for ramp up
        } else {
            return tResultingForce; // Deceleration for ramp up
        }
    }

    void startIdleAnimation(long aRandomValue) {
#if !defined(BRIDGE_NO_NEOPATTERNS)
        if (isInitialized) {
            uint8_t tDirection;
            if (isRampDown) {
                tDirection = DIRECTION_DOWN;
            } else {
                tDirection = DIRECTION_UP;
            }
            LongUnion tRandom;
            tRandom.Long = aRandomValue;
            if (tRandom.UBytes[0] & 0x03) {
                if (tRandom.UBytes[0] & 0x01) {
                    //1 + 3
                    RampPatterns->Stripes(NeoPatterns::Wheel(tRandom.UBytes[0]), (tRandom.UBytes[1] & 0x03) + 1, COLOR32_BLACK, 4,
                            (tRandom.UBytes[2] & 0x7F) + 64, (tRandom.UBytes[3] & 0x1F) + 4, tDirection);
                } else {
                    //2
                    initMultipleFallingStars(RampPatterns, COLOR32_WHITE_HALF, 7, (tRandom.UBytes[0] & 0x07) + 1,
                            (tRandom.UBytes[0] & 0x03) + 2, NULL, tDirection);
                }
            } else {
                if (tRandom.UBytes[0] > 0x03) {
                    // 0 - especially used for setup
                    RampPatterns->ColorWipeD(NeoPatterns::Wheel(tRandom.UBytes[0]), START_ANIMATION_MILLIS, 0, tDirection);
                } else {
                    // 0 - especially used for setup
                    RampPatterns->ColorWipeD(RAMP_COLOR, START_ANIMATION_MILLIS, 0, tDirection);
                }

//                RampPatterns->ScannerExtendedD(COLOR32_BLUE_HALF, 8, START_ANIMATION_MILLIS, 2,
//                FLAG_SCANNER_EXT_ROCKET | FLAG_SCANNER_EXT_VANISH_COMPLETE | FLAG_SCANNER_EXT_START_AT_BOTH_ENDS, tDirection);
            }
        }
#endif
    }

    /*
     * Draw ramp and dim brightness, if car is on ramp
     */
    void draw() {
#if !defined(BRIDGE_NO_NEOPATTERNS)
        bool tCarIsOnRamp = isAnyCarInRegion(StartPositionOnTrack, RampLength);
        color32_t tColor = RAMP_COLOR;
        if (tCarIsOnRamp) {
            tColor = TrackPtr->dimColorWithGamma5(tColor, 160);
        }
        TrackPtr->fillRegion(tColor, StartPositionOnTrack, RampLength);
#endif
    }
};

/*******************************************************************************************
 * The BRIDGE class
 * Requires 23 bytes + 2 * sizeof(NeoPatterns) RAM per bridge
 *******************************************************************************************/
class Bridge {
#if !defined(BRIDGE_NO_NEOPATTERNS)
public:
    NeoPatterns *TrackPtr;
    bool isInitialized;
#endif
    Ramp RampUp;
    Ramp RampDown;

public:
    void init(NeoPatterns *aTrackPtr, uint16_t aBridgeStartPositionOnTrack, uint8_t aBridgeHeight, uint8_t aRampLength,
            uint8_t aRampPlatformLength) {
        RampUp.init(aTrackPtr, aBridgeStartPositionOnTrack, aBridgeHeight, aRampLength, false);
        RampDown.init(aTrackPtr, aBridgeStartPositionOnTrack + aRampLength + aRampPlatformLength, aBridgeHeight, aRampLength, true);
#if !defined(BRIDGE_NO_NEOPATTERNS)
        TrackPtr = aTrackPtr;
        isInitialized = true;
#endif
    }

    void startIdleAnimation(bool doRandom) {
#if !defined(BRIDGE_NO_NEOPATTERNS)
        if (isInitialized) {
            // Show the same animation on both ramps
            long tRandom = random();
            if (!doRandom) {
                tRandom &= 0xFFFFFF00; // this forces pattern 0
            }
            RampUp.startIdleAnimation(tRandom);
            RampDown.startIdleAnimation(tRandom);
        }
#endif
    }

    /*
     * Draw both ramps and dim brightness, if car is on ramp
     */
    void draw() {
#if !defined(BRIDGE_NO_NEOPATTERNS)
        if (isInitialized) {
            RampUp.draw();
            RampDown.draw();
        }
#endif
    }
};
Bridge bridges[NUMBER_OF_BRIDGES];

/********************************
 * The LOOP class
 ********************************/
class Loop {
    /*
     * Requires 10 bytes + sizeof(NeoPatterns) RAM per loop
     *
     * Loop consists of (LoopLength) pixel
     * Gravity - 100 is full gravity for vertical slope
     */

public:
    NeoPatterns *TrackPtr;
#if !defined(LOOP_NO_NEOPATTERNS)
    NeoPatterns *LoopPatterns;
    bool isInitialized;
    uint8_t RainbowIndex;
    uint8_t RainbowIndexDividerCounter; // divides the call to RainbowIndex++
#endif
    uint16_t StartPositionOnTrack; // Starting with 0
    uint8_t LoopLength;

    void init(NeoPatterns *aTrackPtr, uint16_t aStartPositionOnTrack, uint8_t aLength) {
        StartPositionOnTrack = aStartPositionOnTrack;
        LoopLength = aLength;
        TrackPtr = aTrackPtr;
#if !defined(LOOP_NO_NEOPATTERNS)
        /*
         * NeoPatterns segments to control light effects on both ramps
         * Call malloc() and free() before, since the compiler calls the constructor even when the result of malloc() is NULL, which leads to overwrite low memory.
         */
        void *tMallocTest = malloc(sizeof(NeoPatterns)); // 67 + 2
        if (tMallocTest != NULL) {
            free(tMallocTest);
            // Create a NeoPattern, which runs on a segment of the existing NeoPattern object.
            LoopPatterns = new NeoPatterns(TrackPtr, StartPositionOnTrack, LoopLength, false);
            isInitialized = true;
#  if defined(__AVR__) && defined(DEBUG)
            printRAMInfo(&Serial);
#  endif
        } else {
            Serial.print(F("Not enough heap memory ("));
            Serial.print(sizeof(NeoPatterns) + 2);
            Serial.println(F(") for LoopPatterns."));
#  if defined(__AVR__)
            printRAMInfo(&Serial);
#  endif
        }
#else
        (void) aTrackPtr;
#endif
    }

    /*
     * Compute Gravity acceleration on the fly
     * This saves the 200 bytes RAM of the old Acceleration map array
     * @ return Values from -100 to +100 with 100 is gravity for a vertical slope, 0 for a horizontal and 70 (sqrt(0,5)*100) for 45 degree.
     *          Positive values increases speed, negative values decreases speed.
     */
    int8_t getGravityAcceleration(uint16_t aPositionOnTrack) {
#if defined(DEBUG)
        if (aPositionOnTrack < StartPositionOnTrack || aPositionOnTrack >= StartPositionOnTrack + LoopLength) {
            Serial.print(F("Error! position="));
            Serial.print(aPositionOnTrack);
            Serial.print(F(" is not on loop "));
            Serial.print(StartPositionOnTrack);
            Serial.print(F(" to "));
            Serial.println(StartPositionOnTrack + LoopLength - 1);
        }
#endif
        uint8_t tPositionOnRamp = aPositionOnTrack - StartPositionOnTrack;
        return -((sin((TWO_PI / LoopLength) * tPositionOnRamp) + 0.005) * FULL_GRAVITY); // we start with deceleration for 1. half of loop
    }

    void startIdleAnimation(bool doRandom) {
#if !defined(LOOP_NO_NEOPATTERNS)
        if (isInitialized) {
            LongUnion tRandom;
            tRandom.Long = random();
            if (doRandom && (tRandom.UBytes[0] & 0x03)) {
                if (tRandom.UBytes[0] & 0x01) {
                    //1 + 3
                    LoopPatterns->Stripes(NeoPatterns::Wheel(tRandom.UBytes[0]), (tRandom.UBytes[1] & 0x03) + 1, COLOR32_BLACK, 5,
                            (tRandom.UBytes[2] & 0x7F) + 64, (tRandom.UBytes[3] & 0x1F) + 4, DIRECTION_UP);
                } else {
                    //2
                    initMultipleFallingStars(LoopPatterns, COLOR32_WHITE_HALF, 7, (tRandom.UBytes[0] & 0x07) + 1,
                            4 - (tRandom.UBytes[0] & 0x03), NULL, DIRECTION_UP);
                }
            } else {
                // 0 or not random
                LoopPatterns->ScannerExtendedD(COLOR32_BLUE_HALF, 8, START_ANIMATION_MILLIS, 2,
                FLAG_SCANNER_EXT_ROCKET | FLAG_SCANNER_EXT_VANISH_COMPLETE | FLAG_SCANNER_EXT_START_AT_BOTH_ENDS, DIRECTION_UP);
            }
        }
#endif
    }

    /*
     * Draw the loop in a fixed or changing color
     */
    void draw(bool aDoRaceAnimation) {
        bool tCarIsOnLoop = isAnyCarInRegion(StartPositionOnTrack, LoopLength);
        color32_t tColor = LOOP_COLOR;
#if !defined(LOOP_NO_NEOPATTERNS)
        if (isInitialized && aDoRaceAnimation) {
            if (!LoopPatterns->update()) {
                // do not increment RainbowIndex at each MILLISECONDS_PER_LOOP (20 ms) call
                if (RainbowIndexDividerCounter++ >= 6) {
                    RainbowIndexDividerCounter = 0;
                    RainbowIndex++;
                }
                tColor = NeoPixel::Wheel(RainbowIndex);
            }
        }
#else
        (void) aDoAnimation; // to avoid compiler warning
#endif
        if (tCarIsOnLoop) {
            tColor = TrackPtr->dimColorWithGamma5(tColor, 160);
        }
        TrackPtr->fillRegion(tColor, StartPositionOnTrack, LoopLength);
    }
};
Loop loops[NUMBER_OF_LOOPS];

/*******************************************************************************************
 * The CAR class
 * Code related to each car is contained in this class
 * Requires 23 bytes RAM per car
 *******************************************************************************************/
/*
 * Return values for computeNewSpeedAndDistance
 */
#define CAR_NOP 0
#define CAR_LAP_CONDITION 1
class Car {
public:
    NeoPatterns *TrackPtr;
    uint8_t NumberOfThisCar;    // 1, 2...

    uint8_t AcceleratorButtonPin;
#if defined(USE_ACCELERATION_NEOPIXEL_BARS)
    uint8_t AccelerationBarPin;
    uint16_t AcceleratorLowPassValue;
#endif

#if defined(ENABLE_ACCELERATOR_INPUT)
    MPU6050IMUData AcceleratorInput;
    bool AcceleratorInputConnected;  // Dynamically detection of IMU accelerator connection; true -> accelerator is connected
    bool ButtonInputDetected;   // true -> button input was detected and has precedence of acceleration input
#endif // defined(ENABLE_ACCELERATOR_INPUT)

    color32_t Color;            // Car pixel color
    const char *CarColorString; // Car color string

    float SpeedAsPixelPerLoop;  // Reasonable values are 0.5 to 2.0
    float Distance;             // Complete racing distance in pixel
    uint16_t PixelPosition;     // The index of head of car on the track
    uint8_t Laps;

    bool lastButtonState;
    const char *WinnerMelody;

    Car() {  // @suppress("Class members should be properly initialized")
        TrackPtr = NULL;
//        CarIsActive = false;
    }

    void init(NeoPatterns *aTrackPtr, uint8_t aNumberOfThisCar, uint8_t aButtonPin, color32_t aCarColor,
            const char *Car1ColorString, const char *aWinnerMelody) {
        TrackPtr = aTrackPtr;
        NumberOfThisCar = aNumberOfThisCar;
        AcceleratorButtonPin = aButtonPin;
        pinMode(AcceleratorButtonPin, INPUT_PULLUP);
        Color = aCarColor;
        CarColorString = Car1ColorString;
        WinnerMelody = aWinnerMelody;

        reset();

#if defined(ENABLE_ACCELERATOR_INPUT)
        if (aNumberOfThisCar == 2) {
            AcceleratorInput.setI2CAddress(MPU6050_ADDRESS_AD0_HIGH);
        }
        // use maximum filtering. It prefers slow and huge movements :-)
        if (!AcceleratorInput.initMPU6050AndCalculateAllOffsetsAndWait(20, MPU6050_BAND_5_HZ)) {
            AcceleratorInputConnected = false;
            Serial.print(F("No MPU6050 IMU connected at address 0x"));
#if defined(USE_SOFT_I2C_MASTER)
            Serial.print(AcceleratorInput.I2CAddress >> 1, HEX);
#else
            Serial.print(AcceleratorInput.I2CAddress, HEX);
#endif
            Serial.print(F(" for car "));
            Serial.print(aNumberOfThisCar);
            Serial.println(F(". You may want to disable \"#define ENABLE_ACCELERATOR_INPUT\""));
            if (sSerialLCDAvailable) {
                myLCD.setCursor(0, aNumberOfThisCar + 1);
                myLCD.print(F("No IMU for car "));
                myLCD.print(aNumberOfThisCar);
                myLCD.print(' '); // to overwrite button info
            }
            playError();
        } else {
            AcceleratorInputConnected = true;
#  if defined(INFO)
            if (!sOnlyPlotterOutput) {
                Serial.print(NumberOfThisCar);
                Serial.print(' ');
                AcceleratorInput.printAllOffsets(&Serial);
            }
#  endif
        }
#endif // defined(ENABLE_ACCELERATOR_INPUT)
    }

    void reset() {
        SpeedAsPixelPerLoop = 0;
        Distance = 0;
        Laps = 0;
        PixelPosition = 0;
        lastButtonState = 1;
#if defined(ENABLE_ACCELERATOR_INPUT)
        ButtonInputDetected = false;
#endif
#if defined(USE_ACCELERATION_NEOPIXEL_BARS)
        AccelerationCommonNeopixelBar.setPin(AccelerationBarPin);
        AccelerationCommonNeopixelBar.clear();
        AccelerationCommonNeopixelBar.show();
#endif
    }

    /*
     * The car consists of number of laps pixels
     * Overlapping of cars is handled by using addPixelColor for drawing
     */
    void draw() {

#if defined(TEST_MODE)
        for (int i = 0; i <= 1; i++) {
#else
        for (int i = 0; i <= Laps; i++) {
#endif
            // draw from back to front
            int16_t tDrawPosition = PixelPosition - i;
            if (tDrawPosition < 0) {
                // wrap around
                tDrawPosition += TrackPtr->numPixels();
            }
            TrackPtr->addPixelColor(tDrawPosition, getRedPart(Color), getGreenPart(Color), getBluePart(Color));
        }
    }

    /*
     * @return true if button was pressed or accelerator was moved
     */
    bool checkInput() {
#if !defined(ENABLE_ACCELERATOR_INPUT)
        return checkButton();
#else
        bool tButtonIsPressed = checkButton(); // checkButton() also sets ButtonInputDetected used below
        if (tButtonIsPressed
                || (!ButtonInputDetected && AcceleratorInputConnected && (getAcceleratorValueShift8() >= ACCELERATOR_TRIGGER_VALUE))) {
            return true;
        }
#endif
        return false;
    }

#if defined(ENABLE_ACCELERATOR_INPUT)
    /*
     * @return 4 g for 16 bit full range
     */
    uint8_t getAcceleratorValueShift8() {
        AcceleratorInput.readDataFromMPU6050();
//        AcceleratorInput.readDataFromMPU6050Fifo();
        uint8_t tAcceleration = AcceleratorInput.computeAccelerationWithFloatingOffset() >> 9; // 8 is too easy, 10 is a bit hard
#  if defined(TRACE)
//#if defined(INFO)
        Serial.print(NumberOfThisCar);
        Serial.print(F(" Acc="));
        Serial.println(tAcceleration);
#  endif
#  if defined(USE_ACCELERATION_NEOPIXEL_BARS)
//        AccelerationBar->setBarAndPeakAndShow(tAcceleration);
#  endif
        return tAcceleration;
    }
#endif // defined(ENABLE_ACCELERATOR_INPUT)

    /*
     * Check if button was just pressed
     * activates car and returns true if button was just pressed
     */
    bool checkButton() {
        bool tLastButtonState = lastButtonState;
        lastButtonState = digitalRead(AcceleratorButtonPin);

        if (tLastButtonState == true && lastButtonState == false) {
#if defined(DEBUG)
            Serial.print(NumberOfThisCar);
            Serial.println(F(" Button pressed"));
#endif
#if defined(ENABLE_ACCELERATOR_INPUT)
            ButtonInputDetected = true;
#endif
            return true;
        }
        return false;
    }

    /*
     * @return true if this car is the winner
     */
    bool checkIfWinner(uint8_t aLapsNeededToWin) {
        if (Laps >= aLapsNeededToWin) {
            return true;
        }
        return false;
    }

    /*
     * Play a winner melody and run animations with the car color on the track
     * Returns if melody ends
     * You can stop melody and animation by pressing the car button.
     * @return true if stopped by user input
     */
    bool doWinnerAnimationAndSound() {
        startPlayRtttlPGM(PIN_BUZZER, WinnerMelody);
        TrackPtr->Stripes(Color, 2, COLOR32_BLACK, 8, 300, 50, DIRECTION_UP);
        bool tReturnValue = false;
        auto tStartMillis = millis();

        while (updatePlayRtttl()) {
#if defined(TIMING_TEST)
                /*
                 *  20 microseconds for loop, 300 microseconds if melody updated
                 *  19 ms duration for resetAndDrawTrack() - every 50 ms
                 */
                digitalWrite(PIN_TIMING, HIGH);
#endif
            if (TrackPtr->update()) {
                timer0_millis += MILLIS_FOR_TRACK_TO_SHOW;
                // restore bridge and loop pattern, which might be overwritten by scanner.
                resetAndDrawTrack(false);
            }
#if defined(TIMING_TEST)
                digitalWrite(PIN_TIMING, LOW);
#endif
            if ((millis() - tStartMillis > WINNER_MINIMAL_ANIMATION_DURATION_MILLIS) && checkAllInputs()) {
                // minimal animation time was reached and input was activated
                stopPlayRtttl(); // to stop in a deterministic fashion
                tReturnValue = true;
            }
            yield();
        }
        TrackPtr->stop();
        return tReturnValue;
    }

    void print(float aGravity, float aFricion, float aDrag) {
        Serial.print(NumberOfThisCar);
        Serial.print(F(" Speed="));
        Serial.print(SpeedAsPixelPerLoop, 2);
        Serial.print(F(" - Gravity="));
        Serial.print(aGravity, 6);
        Serial.print(F(" Fricion="));
        Serial.print(aFricion, 6);
        Serial.print(F(" Drag="));
        Serial.println(aDrag, 6);
    }

    /*
     * Check buttons and gravity and get new speed
     */
    uint8_t computeNewSpeedAndDistance() {
        uint8_t tRetval = CAR_NOP;

#if defined(TIMING_TEST)
        digitalWrite(PIN_TIMING, HIGH);
#endif
        uint16_t tAcceleration;

        if (checkButton()) {
            // add fixed amount of energy ->  800 => tAdditionalEnergy is 0.2 per press
            tAcceleration = 600;
        }

#if defined(ENABLE_ACCELERATOR_INPUT)
        else if (!ButtonInputDetected && AcceleratorInputConnected) {
            //Here, no button was pressed before and accelerator is connected
            tAcceleration = getAcceleratorValueShift8();
        } else {
            tAcceleration = 0;
        }
#endif
#if defined(USE_ACCELERATION_NEOPIXEL_BARS)
        AcceleratorLowPassValue += (((int16_t) (tAcceleration - AcceleratorLowPassValue))) >> 3;
        // scale it so that 100 -> 8
        // Parameter NumberOfThisCar == 1 has the effect, that bar for car 2 can be mounted upside down
        AccelerationCommonNeopixelBar.drawBar(AcceleratorLowPassValue / (100 / 8), Color, NumberOfThisCar == 1);
        AccelerationCommonNeopixelBar.setPin(AccelerationBarPin);
        AccelerationCommonNeopixelBar.show();
#endif
        if (tAcceleration > 0) {
            float tAdditionalEnergy = ((float) tAcceleration) / 4096;
            SpeedAsPixelPerLoop = sqrt((SpeedAsPixelPerLoop * SpeedAsPixelPerLoop) + tAdditionalEnergy);
        }
        if (sOnlyPlotterOutput) {
            /*
             * Print data for Arduino plotter
             */
            Serial.print(tAcceleration);
            Serial.print(' ');
            Serial.print(AcceleratorLowPassValue);
            Serial.print(' ');
            Serial.print(int(SpeedAsPixelPerLoop * 100));
            Serial.print(' ');
        }

        bool tIsAnalogParameterInputMode = !digitalRead(PIN_MANUAL_PARAMETER_MODE);
        float tGravity;
        float tFricion;
        float tDrag;

        if (tIsAnalogParameterInputMode) {
            /*
             * Read analog values for gravity etc. from potentiometers
             * 850 us for analogRead + DEBUG print
             */
            uint16_t tGravityRaw = analogRead(PIN_GRAVITY);
            uint16_t tFricionRaw = analogRead(PIN_FRICTION);
            uint16_t tDragRaw = analogRead(PIN_DRAG);
            if (tGravityRaw >= ANALOG_OFFSET) {
                // -ANALOG_OFFSET (20) to get real 0 value even if ground has bias because of high LED current on breadboard
                tGravityRaw -= ANALOG_OFFSET;
            }
            if (tFricionRaw >= ANALOG_OFFSET) {
                tFricionRaw -= ANALOG_OFFSET;
            }
            if (tDragRaw >= ANALOG_OFFSET) {
                tDragRaw -= ANALOG_OFFSET;
            }
            tGravity = tGravityRaw * (GRAVITY_FACTOR_FOR_MAP / 512);
            tFricion = tFricionRaw * (FRICTION_PER_LOOP / 512);
            tDrag = tDragRaw * (AERODYNAMIC_DRAG_PER_LOOP / 512);
#if defined(DEBUG)
            if ((((sLoopCountForDebugPrint & 0x3F) == 0) || checkInput()) && NumberOfThisCar == 1) {
#if defined(TRACE)
                    Serial.print(F("GRaw="));
                    Serial.print(tGravityRaw);
                    Serial.print(F(" FRaw="));
                    Serial.print(tFricionRaw);
                    Serial.print(F(" DRaw="));
                    Serial.println(tDragRaw);
#endif

                print(tGravity, tFricion, tDrag);
            }
#endif

        } else {
            // 100 microseconds
            tGravity = GRAVITY_FACTOR_FOR_MAP;
            tFricion = FRICTION_PER_LOOP;
            tDrag = AERODYNAMIC_DRAG_PER_LOOP;
        }

        /*
         * Compute new position
         */
        Distance += SpeedAsPixelPerLoop; // Take speed to compute new position
        if (Distance < 0.0) {
            Distance = 0.0;
        }
        PixelPosition = (uint16_t) Distance % TrackPtr->numPixels();

        /*
         * Check for lap counter
         */
        if (Distance > TrackPtr->numPixels() * (Laps + 1)) {
            Laps++;
#if defined(INFO)
            if (!sOnlyPlotterOutput) {
                Serial.print(F("Car "));
                Serial.print(NumberOfThisCar);
                Serial.print(F(" starts "));
                Serial.print(Laps + 1);
                Serial.println(F(". lap"));
            }
#endif
            if (sSerialLCDAvailable) {
                BigNumbers.writeAt(Laps, ((NumberOfThisCar - 1) * 13) + 2, 0); // red is left, green is right
            }
            tRetval = CAR_LAP_CONDITION;
        }

        /*
         * Compute new speed:
         * - Acceleration from map and friction are simply subtracted from speed
         * - Aerodynamic drag is subtracted proportional from speed
         */
        int8_t tGravityAcceleration = 0;
        if (PixelPosition >= bridges[0].RampUp.StartPositionOnTrack
                && PixelPosition <= (bridges[0].RampUp.StartPositionOnTrack + bridges[0].RampUp.RampLength)) {
            tGravityAcceleration = bridges[0].RampUp.getGravityAcceleration(PixelPosition);
        } else if (PixelPosition >= bridges[0].RampDown.StartPositionOnTrack
                && PixelPosition <= (bridges[0].RampDown.StartPositionOnTrack + bridges[0].RampDown.RampLength)) {
            tGravityAcceleration = bridges[0].RampDown.getGravityAcceleration(PixelPosition);
        } else if (PixelPosition >= loops[0].StartPositionOnTrack
                && PixelPosition < (loops[0].StartPositionOnTrack + loops[0].LoopLength)) {
            tGravityAcceleration = loops[0].getGravityAcceleration(PixelPosition);
        }

        if (tGravityAcceleration != 0) {
            /*
             * Here we are on a ramp or loop
             */
            SpeedAsPixelPerLoop += tGravity * tGravityAcceleration;
#if defined(DEBUG)
            Serial.print(F(" GravityAcceleration="));
            Serial.print(tGravityAcceleration);
#endif
        }

        if (SpeedAsPixelPerLoop > 0) {
            // forward direction
            if (SpeedAsPixelPerLoop > tFricion) {
                SpeedAsPixelPerLoop -= tFricion;
            } else {
                SpeedAsPixelPerLoop = 0.0;
            }
        } else {
            // backward direction
            if (SpeedAsPixelPerLoop < -tFricion) {
                SpeedAsPixelPerLoop += tFricion;
            } else {
                SpeedAsPixelPerLoop = 0.0;
            }
        }

        SpeedAsPixelPerLoop -= SpeedAsPixelPerLoop * tDrag;

#if defined(TRACE)
        Serial.print(F(" -> "));
        Serial.print(SpeedAsPixelPerLoop, 3);
        Serial.print(F(" => "));
        Serial.println(Distance);
#endif

#if defined(TIMING_TEST)
        digitalWrite(PIN_TIMING, LOW);
#endif
        return tRetval;
    }
}
;
Car cars[NUMBER_OF_CARS];

/********************************
 * Start of program
 ********************************/
extern size_t __malloc_margin;

void setup() {
    __malloc_margin = 120; // 128 is the default value
    pinMode(LED_BUILTIN, OUTPUT);

    pinMode(PIN_RESET_GAME_BUTTON, INPUT_PULLUP);
    pinMode(PIN_MANUAL_PARAMETER_MODE, INPUT_PULLUP);
    pinMode(ONLY_PLOTTER_OUTPUT_PIN, INPUT_PULLUP);

#if defined(TIMING_TEST)
    pinMode(PIN_TIMING, OUTPUT);
#endif

#if defined(INFO) && defined(__AVR__)
    initStackFreeMeasurement();
#endif

    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) /*stm32duino*/|| defined(USBCON) /*STM32_stm32*/|| defined(SERIALUSB_PID) || defined(ARDUINO_attiny3217)
    delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
#endif

    sOnlyPlotterOutput = !digitalRead(ONLY_PLOTTER_OUTPUT_PIN);

    if (!sOnlyPlotterOutput) {

        // Just to know which program is running on my Arduino
        Serial.println(F("START " __FILE__ " from " __DATE__));
        Serial.println(
                F(
                        "Connect pin " STR(ONLY_PLOTTER_OUTPUT_PIN) " to ground, to suppress such prints not suited for Arduino plotter"));
        printConfigPinInfo(&Serial, PIN_MANUAL_PARAMETER_MODE, F("AnalogParameterInputMode"));
    }

#if defined(ENABLE_ACCELERATOR_INPUT)
    if (!initWire()) { // Initialize everything and check for bus lockup
        Serial.println(F("I2C init failed"));
    }
#endif
    Serial.flush();

#if !defined(USE_NO_LCD)
    /*
     * LCD initialization
     */
    checkForLCDConnected();

    if (sSerialLCDAvailable) {
        myLCD.init();
        myLCD.clear();
        myLCD.backlight();
        myLCD.print(F("Open LED Race"));
        myLCD.setCursor(0, 1);
        myLCD.print(F(VERSION_EXAMPLE " " __DATE__));
        myLCD.setCursor(0, 2);
        myLCD.print(F("Manual mode pin=" STR(PIN_MANUAL_PARAMETER_MODE)));
        myLCD.setCursor(0, 3);
        myLCD.print(F("Reset  game pin=" STR(PIN_RESET_GAME_BUTTON)));

        BigNumbers.begin(); // Creates custom character used for generating big numbers
        delay(1000); // To show  the message on LCD
    }
#endif

    if (!sOnlyPlotterOutput) {
        Serial.println(F("Initialize track"));
        Serial.flush();
    }

// This initializes the NeoPixel library and checks if enough memory was available
    if (!track.begin(&Serial)) {
        playMelodyAndShutdown();
    }

    if (!sOnlyPlotterOutput) {
        Serial.println(F("Initialize bridges and loops"));
        Serial.flush();
    }
    /*
     * Setup bridges and loops
     */
    bridges[0].init(&track, BRIDGE_1_START, BRIDGE_1_HEIGHT, BRIDGE_1_RAMP_LENGTH, BRIDGE_1_PLATFORM_LENGTH); // Requires 2 x 69 = 138 bytes on heap
//    myTone(64000);
#if defined(INFO) && defined(__AVR__)
    printRAMInfo(&Serial);
#  endif
    loops[0].init(&track, LOOP_1_UP_START, LOOP_1_LENGTH); // Requires 69 bytes on heap
#if defined(INFO) && defined(__AVR__)
    printRAMInfo(&Serial);
#  endif
    /*
     * Setup cars
     */
    if (!sOnlyPlotterOutput) {
        Serial.println(F("Initialize cars"));
        Serial.flush();
    }
    cars[0].init(&track, 1, PIN_PLAYER_1_BUTTON, CAR_1_COLOR, Car1ColorString, MissionImp);
    cars[1].init(&track, 2, PIN_PLAYER_2_BUTTON, CAR_2_COLOR, Car2ColorString, StarWars);
//    cars[2].init(&track, 3, PIN_PLAYER_3_BUTTON, CAR_3_COLOR, Entertainer);

    /*
     * Setup the red and green acceleration feedback bars
     */
#if defined(USE_ACCELERATION_NEOPIXEL_BARS)
// initialize the central NeoPixel object
    AccelerationCommonNeopixelBar.begin();
    cars[0].AccelerationBarPin = PIN_PLAYER_1_VU_BAR;
    cars[1].AccelerationBarPin = PIN_PLAYER_2_VU_BAR;
#endif

    /*
     * Print info
     */
    if (!sOnlyPlotterOutput) {
        Serial.println(F(STR(NUMBER_OF_CARS) " cars initialized"));
    }
#if defined(ENABLE_ACCELERATOR_INPUT)
    randomSeed(cars[0].AcceleratorInput.AcceleratorLowpassSubOneHertz[0].ULong);
#endif

// signal boot
    tone(PIN_BUZZER, 1200, 200);

    /*
     * Boot animation
     */
    delay(300); // to avoid starting animation at power up before USB bootloader delay
    for (uint_fast8_t i = 0; i < NUMBER_OF_BRIDGES; ++i) {
        bridges[i].startIdleAnimation(false);
    }
    for (uint_fast8_t i = 0; i < NUMBER_OF_LOOPS; ++i) {
        loops[i].startIdleAnimation(false);
    }
    delay(1000);
    printStartMessage();

// wait for animation to end
    while (track.updateAndShowAlsoAllPartialPatterns()) {
        yield();
        if (checkAllInputs()) {
            break;
        }
    }

    /*
     * End of setup
     */
    resetAndShowTrackWithoutCars();
#if defined(INFO) && defined(__AVR__)
    printRAMInfo(&Serial);
    initStackFreeMeasurement(); // initialize for getting stack usage in loop
#endif
}

/*
 * 10 Milliseconds per loop without delay
 */
void loop() {
    static uint32_t sNextLoopMillis;
    static uint32_t sLastAnimationMillis;

    /*
     * Run loop at a fixed 20 ms period
     */
    while (millis() < sNextLoopMillis) {
        yield();
    }
    sNextLoopMillis += MILLISECONDS_PER_LOOP;

    sLoopCountForDebugPrint++;
    sOnlyPlotterOutput = !digitalRead(ONLY_PLOTTER_OUTPUT_PIN);

#if defined(INFO) && defined(__AVR__)
    if (!sOnlyPlotterOutput) {
        printStackUnusedAndUsedBytesIfChanged(&Serial);
    }
#endif

    if (sLoopMode == MODE_IDLE) {
        /*
         * Do periodic animation
         */
        if (millis() - sLastAnimationMillis > ANIMATION_INTERVAL_MILLIS) {
            sLastAnimationMillis = millis();
            for (uint_fast8_t i = 0; i < NUMBER_OF_BRIDGES; ++i) {
                bridges[i].startIdleAnimation(true);
            }
            for (uint_fast8_t i = 0; i < NUMBER_OF_LOOPS; ++i) {
                loops[i].startIdleAnimation(true);
            }
            if (!sOnlyPlotterOutput) {
                Serial.println(F("Start idle animation"));
            }
        }

        checkAllInputs();
        track.updateAndShowAlsoAllPartialPatterns(); // Show animation

    } else if (sLoopMode == MODE_START) {
        startRace(); // blocking call
        sLoopMode = MODE_RACE;

    } else if (sLoopMode == MODE_RACE) {
        /*
         * Race mode
         * 1. Reset track with loop animation
         * 2. Move cars
         * 3. Check for winner and overtaking the leader
         */
        resetAndDrawTrack(true);

        /*
         * Move each car and start lap sound if one car starts a new lap
         */
        for (uint_fast8_t i = 0; i < NUMBER_OF_CARS; ++i) {
            if (cars[i].computeNewSpeedAndDistance() == CAR_LAP_CONDITION) {
                sBeepEndMillis = millis() + 100;
                sBeepFrequency = 2000;
            }
        }
        if (sOnlyPlotterOutput) {
            Serial.println(); // end of plotter dataset
        }

        /*
         * Draw all cars
         */
        for (uint_fast8_t i = 0; i < NUMBER_OF_CARS; ++i) {
            cars[i].draw();
        }

        checkForOvertakingLeaderCar();
        checkAndHandleWinner();

        /*
         * Show track
         */
        track.show(); // 9 Milliseconds for 300 Pixel
        timer0_millis += MILLIS_FOR_TRACK_TO_SHOW; // compensate Arduino millis() for the time interrupt was disabled for track.show().

        /*
         * Manage sound. Must check for situation after winner
         */
        if (sSoundEnabled && sLoopMode == MODE_RACE) {
            if (millis() < sBeepEndMillis) {
                // Play a single beep, like for overtaking
                myTone(sBeepFrequency);
            } else {
                // tFrequency must be integer since SpeedAsPixelPerLoop can be negative
                int tFrequency = cars[0].SpeedAsPixelPerLoop * 440 + cars[1].SpeedAsPixelPerLoop * 440;
                if (tFrequency > 100) {
                    myTone(tFrequency);
                } else {
                    noTone(PIN_BUZZER);
                }
            }
        }

        /*
         * check for reset button
         */
        if (!digitalRead(PIN_RESET_GAME_BUTTON)) {
            noTone(PIN_BUZZER);
            resetAllCars();
            myLCD.clear();
            printStartMessage();
            resetAndShowTrackWithoutCars();
            Serial.println(F("Reset game button pressed -> start a new race"));
            sLoopMode = MODE_IDLE;
        }
    }
}

void printStartMessage() {
    if (sOnlyPlotterOutput) {
        // print Plotter caption after check for LCD
        Serial.println();
        Serial.println(F("Accel[1] AccelLP[1] Speed[1] Accel[2] AccelLP[2] Speed[2]"));
    } else {
        Serial.print(F("Press any button"));
#if defined(ENABLE_ACCELERATOR_INPUT)
        if (cars[0].AcceleratorInputConnected || cars[1].AcceleratorInputConnected) {
            Serial.print(F(" or move controller"));
        }
#endif
        Serial.println(F(" to start countdown"));
    }
    if (sSerialLCDAvailable) {
        myLCD.setCursor(14, 0);
        myLCD.print(F("5 Laps"));
        myLCD.setCursor(0, 1);
        myLCD.print(F("Press any button"));
        uint8_t tLineIndex = 2;
#if defined(ENABLE_ACCELERATOR_INPUT)
        if (cars[0].AcceleratorInputConnected || cars[1].AcceleratorInputConnected) {
            myLCD.setCursor(0, tLineIndex++);
            myLCD.print(F("or move controller"));
        }
#endif
        myLCD.setCursor(0, tLineIndex);
        myLCD.print(F("to start countdown"));
    }
}

/*
 * @return true if any button pressed or IMU moved
 */
bool checkAllInputs() {
    for (uint_fast8_t i = 0; i < NUMBER_OF_CARS; ++i) {
        if (cars[i].checkInput()) {
            sLoopMode = MODE_START;
            return true;
        }
    }
    if (!digitalRead(PIN_RESET_GAME_BUTTON)) {
        return true;
    }
    return false;
}

/*
 * Check for winner. If one car is the winner it takes a few seconds here
 */
void checkAndHandleWinner() {
    for (uint_fast8_t i = 0; i < NUMBER_OF_CARS; ++i) {
        if (cars[i].checkIfWinner(LAPS_PER_RACE)) {
            if (!sOnlyPlotterOutput) {
                Serial.print(F("Winner is "));
                Serial.print((__FlashStringHelper*) (cars[i].CarColorString));
                Serial.println(F(" car"));
            }

            noTone(PIN_BUZZER);
            resetAllCars();
            /*
             * show winner situation for n seconds
             */
            delay(WINNER_ANIMATION_DELAY_MILLIS);

            /*
             * Show start message and start winner animation and sound
             */
            if (sSerialLCDAvailable) {
                myLCD.clear();
                myLCD.setCursor(0, 0);
                myLCD.print(F("Winner is "));
                myLCD.print((__FlashStringHelper*) (cars[i].CarColorString));
                myLCD.print(F(" car"));
            }
            printStartMessage();
            if (cars[i].doWinnerAnimationAndSound()) { // blocking call until input is received or melody ends
                sLoopMode = MODE_START;
            } else {
                sLoopMode = MODE_IDLE;
            }
            resetAndShowTrackWithoutCars();
            break;
        }
    }
}

/*
 * Check for overtaking current leader car
 */
void checkForOvertakingLeaderCar() {
    for (uint_fast8_t i = 0; i < NUMBER_OF_CARS; ++i) {
        if (cars[i].Distance > cars[sIndexOfLeadingCar].Distance) {
            // do not output first match
            if (cars[sIndexOfLeadingCar].Distance > 0) {
#if defined(INFO)
                if (!sOnlyPlotterOutput) {
                    Serial.print(F("Car "));
                    Serial.print(i + 1);
                    Serial.print(F(" overtakes leader car "));
                    Serial.println(sIndexOfLeadingCar + 1);
                }
#endif
                // play overtaking sound
                sBeepFrequency = 1000;
                sBeepEndMillis = millis() + 50;
            }
            sIndexOfLeadingCar = i;
        }
    }
}

void playError() {
    tone(PIN_BUZZER, 2200, 50);
    delay(100);
    tone(PIN_BUZZER, 2200, 50);
    delay(500);
}

void playShutdownMelody() {
    tone(PIN_BUZZER, 2000, 200);
    delay(400);
    tone(PIN_BUZZER, 1400, 300);
    delay(600);
    tone(PIN_BUZZER, 1000, 400);
    delay(800);
    tone(PIN_BUZZER, 700, 500);
    delay(800);
}

void playMelodyAndShutdown() {
    playShutdownMelody();
    while (true) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(500);
        digitalWrite(LED_BUILTIN, LOW);
        delay(500);
    }
}

bool isAnyCarInRegion(unsigned int aRegionFirst, unsigned int aRegionLength) {
    for (uint_fast8_t i = 0; i < NUMBER_OF_CARS; ++i) {
        if (isInRegion(cars[i].PixelPosition, aRegionFirst, aRegionLength)) {
            return true;
        }
    }
    return false;
}

bool isInRegion(uint16_t aPixelPosition, unsigned int aRegionFirst, unsigned int aRegionLength) {
    if (aPixelPosition >= aRegionFirst && aPixelPosition <= (aRegionFirst + aRegionLength)) {
        return true;
    }
    return false;
}

/*
 * Clear track and redraw bridges and loops
 * @param aDoAnimation if true draw animated loops
 */
void resetAndDrawTrack(bool aDoAnimation) {
    track.clear();
    for (uint_fast8_t i = 0; i < NUMBER_OF_BRIDGES; ++i) {
        bridges[i].draw(); // no sensible animation for ramps
    }
    for (uint_fast8_t i = 0; i < NUMBER_OF_LOOPS; ++i) {
        loops[i].draw(aDoAnimation);
    }
}

void resetAndShowTrackWithoutCars() {
    resetAndDrawTrack(false);
    track.show();
    timer0_millis += MILLIS_FOR_TRACK_TO_SHOW;
}

void resetAllCars() {
    for (uint_fast8_t i = 0; i < NUMBER_OF_CARS; ++i) {
        cars[i].reset();
    }
}

void startRace() {
    if (!sOnlyPlotterOutput) {
        Serial.println(F("Start race countdown"));
    }

    /*
     * Draw all cars and track
     */
    for (uint_fast8_t i = 0; i < NUMBER_OF_CARS; ++i) {
        cars[i].draw();
    }
    track.show();

    uint8_t tIndex = 4; // index of last light
    if (sSerialLCDAvailable) {
        myLCD.clear();
    }
    for (int tCountDown = 4; tCountDown >= 0; tCountDown--) {
        // delay at start of loop to enable fast start after last countdown

        delay(900);
        AccelerationCommonNeopixelBar.drawBar(8, CAR_1_COLOR);
        AccelerationCommonNeopixelBar.setPin(PIN_PLAYER_1_VU_BAR);
        AccelerationCommonNeopixelBar.show();
        AccelerationCommonNeopixelBar.drawBar(8, CAR_2_COLOR);
        AccelerationCommonNeopixelBar.setPin(PIN_PLAYER_2_VU_BAR);
        AccelerationCommonNeopixelBar.show();

        delay(100);
        AccelerationCommonNeopixelBar.clear();
        AccelerationCommonNeopixelBar.setPin(PIN_PLAYER_1_VU_BAR);
        AccelerationCommonNeopixelBar.show();
        AccelerationCommonNeopixelBar.setPin(PIN_PLAYER_2_VU_BAR);
        AccelerationCommonNeopixelBar.show();

        track.setPixelColor(tIndex + (2 * tCountDown), COLOR32_RED);
        track.setPixelColor(tIndex + (2 * tCountDown) + 1, COLOR32_RED);
        track.show();
        if (tCountDown != 0) {
            tone(PIN_BUZZER, 600, 200);
#if defined(TCCR2A)
            TCCR2A |= _BV(COM2A0); // switch to direct toggle output at OC2A / pin 11 to enable direct hardware tone output
#endif
        } else {
            // start the start beep for 400 ms. It is ended by main loop
            sBeepFrequency = 400;
            sBeepEndMillis = millis() + 400; // Is ended by main loop
        }
        if (!sOnlyPlotterOutput) {
            Serial.println(tCountDown);
        }
        if (sSerialLCDAvailable) {
            BigNumbers.writeAt(tCountDown, 9);
        }
    }

    /*
     * Now start race
     */
    if (!sOnlyPlotterOutput) {
        Serial.println(F("Start race"));
    }
    myLCD.clear();
    if (sSerialLCDAvailable) {
        // print initial lap counters
        myLCD.clear();
        BigNumbers.writeAt(0, 2);
        BigNumbers.writeAt(0, 15);
    }
}

void checkForLCDConnected() {
    if (!sOnlyPlotterOutput) {
        Serial.println(F("Try to connect to I2C LCD"));
        Serial.flush();
    }
#if defined(USE_SOFT_I2C_MASTER)
    if (!i2c_start(LCD_I2C_ADDRESS << 1)) {
        if (!sOnlyPlotterOutput) {
            Serial.println(F("No I2C LCD connected at address " STR(LCD_I2C_ADDRESS)));
        }
        playError();
        sSerialLCDAvailable = false;
    } else {
        sSerialLCDAvailable = true;
    }
    i2c_stop();
#elif defined(USE_SOFT_WIRE)
#warning SoftWire does not support dynamically check of connection because it has no setWireTimeout() function. You should use "#define USE_SOFT_I2C_MASTER" instead.
#else
    Wire.setWireTimeout(); // Sets default timeout of 25 ms.
    Wire.beginTransmission(LCD_I2C_ADDRESS);
    if (Wire.endTransmission(true) != 0) {
        if (!sOnlyPlotterOutput) {
            Serial.println(F("No I2C LCD connected at address " STR(LCD_I2C_ADDRESS)));
        }
        playError();
        sSerialLCDAvailable = false;
    } else {
        sSerialLCDAvailable = true;
    }
#endif
}

void printConfigPinInfo(Print *aSerial, uint8_t aConfigPinNumber, const __FlashStringHelper *aConfigPinDescription) {
    aSerial->print(F("Pin "));
    aSerial->print(aConfigPinNumber);
    aSerial->print(F(" is"));
    bool tIsEnabled = digitalRead(aConfigPinNumber) == LOW;
    if (!tIsEnabled) {
        aSerial->print(F(" not"));
    }
    aSerial->print(F(" connected to ground, "));
    aSerial->print(aConfigPinDescription);
    aSerial->print(F(" is "));
    if (tIsEnabled) {
        aSerial->println(F("enabled"));
    } else {
        aSerial->println(F("disabled"));
    }
}
