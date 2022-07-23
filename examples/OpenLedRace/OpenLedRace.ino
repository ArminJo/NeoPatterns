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
 *  You need to install "Adafruit NeoPixel" library under "Tools -> Manage Libraries..." or "Ctrl+Shift+I" -> use "neoPixel" as filter string
 *  You also need to install "NeoPatterns" and "PlayRtttl" library under "Tools -> Manage Libraries..." or "Ctrl+Shift+I"
 *
 *  Copyright (C) 2020-2022  Armin Joachimsmeyer
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
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
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

/*
 * Ideas:
 * improve winner pattern
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
#include "AVRUtils.h" // for initStackFreeMeasurement() and printFreeHeap()
#endif

// for hunting errors
//#include "AvrTracing.hpp"

#define VERSION_EXAMPLE "1.1"

//#define USE_NO_LCD   // this suppresses the error tone and print if LCD was not found
//#define TEST_MODE
//#define TIMING_TEST
#define ENABLE_ACCELERATOR_INPUT
#if defined(ENABLE_ACCELERATOR_INPUT)
#define USE_ACCELERATION_NEOPIXEL_BARS
#endif
//#define BRIDGE_NO_NEOPATTERNS // to save RAM
//#define LOOP_NO_NEOPATTERNS // to save RAM

#define USE_SOFT_I2C_MASTER // Saves 2110 bytes program memory and 200 bytes RAM compared with Arduino Wire

#if defined(ENABLE_ACCELERATOR_INPUT)
/*
 * Modifiers for the MPU6050IMUData library to save speed and space
 */
#define DO_NOT_USE_GYRO
#define USE_ONLY_ACCEL_FLOATING_OFFSET
#include "MPU6050IMUData.hpp" // this configures and includes SoftI2CMaster
#else
// No MPU6050, but SerialLCD -> configure and include SoftI2CMaster here
#include "SoftI2CMasterConfig.h"
#include "SoftI2CMaster.h"
#endif // #if defined(ENABLE_ACCELERATOR_INPUT)

#include "LongUnion.h"

#define LCD_I2C_ADDRESS 0x27
#include "LiquidCrystal_I2C.h" // Use an up to date library version which has the init method
LiquidCrystal_I2C myLCD(LCD_I2C_ADDRESS, 20, 4);  // set the LCD address to 0x27 for a 20 chars and 2 line display
void printBigNumber4(byte digit, byte leftAdjust);
void initBigNumbers();
void checkForLCDConnected();
bool sSerialLCDAvailable;

/*
 * Pin layout
 */
#define PIN_PLAYER_1_BUTTON 2
#define PIN_PLAYER_2_BUTTON 3
#define PIN_VU_BAR_1        4
#define PIN_VU_BAR_2        5
#if !defined(ENABLE_ACCELERATOR_INPUT)
#define PIN_PLAYER_3_BUTTON 6
#define PIN_PLAYER_4_BUTTON 7
#endif

#define PIN_NEOPIXEL        8
#if defined(TIMING_TEST)
#define PIN_TIMING  9
#endif
#define PIN_MANUAL_PARAMETER_MODE       9 // if connected to ground, analog inputs for parameters are used

#define PIN_BUZZER          11   // must be pin 11, since we use the direct hardware tone output for ATmega328

#define PIN_GRAVITY        A0
#define PIN_FRICTION       A1
#define PIN_DRAG           A2
#define ANALOG_OFFSET      20   // To get real 0 analog value, even if ground has bias because of high LED current on Breadboard

#define PIN_ONLY_PLOTTER_OUTPUT 12 // Verbose output to Arduino Serial Monitor is disabled, if connected to ground. This is intended for Arduino Plotter mode.
bool sOnlyPlotterOutput;

/*
 * The track
 * Think of 1 Pixel = 1 meter
 */
#define NUMBER_OF_TRACK_PIXELS  300 // Number of LEDs in strip

/*
 * Maximum number of cars supported. Each car is individually activated at runtime
 * by first press of its button or movement of its accelerator input.
 */
#if defined(ENABLE_ACCELERATOR_INPUT)
#define NUMBER_OF_CARS            2 // Currently we can handle only 2 distinct accelerometers.
#else
#define NUMBER_OF_CARS            4
#endif
#define CAR_1_COLOR     (color32_t)COLOR32_RED
#define CAR_2_COLOR     (color32_t)COLOR32_GREEN
#define CAR_3_COLOR     (color32_t)COLOR32_BLUE
/*
 * The bridge with a ramp up, a flat platform and a ramp down
 */
#define NUMBER_OF_BRIDGES         1
#define BRIDGE_1_START          100
#define BRIDGE_1_RAMP_LENGTH     21 // in pixel
#define BRIDGE_1_PLATFORM_LENGTH 20 // > 0 for bridges with a ramp up a flat bridge and a ramp down
#define BRIDGE_1_HEIGHT          15 // in pixel -> 45 degree slope here
#define RAMP_COLOR               COLOR32_CYAN_QUARTER // COLOR32(0,64,64)
#define RAMP_COLOR_DIMMED        COLOR32(0,8,8)
/*
 * The loop
 */
#define NUMBER_OF_LOOPS 1
#define LOOP_1_UP_START 221
#define LOOP_1_LENGTH   48 // in pixel
#define LOOP_COLOR      COLOR32_PURPLE_QUARTER // COLOR32(64,0,64)
#define LOOP_DIMMED_COLOR      COLOR32(8,0,8)
#define GAMMA_FOR_DIMMED_VALUE 160

NeoPatterns track = NeoPatterns(NUMBER_OF_TRACK_PIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

#if defined(USE_ACCELERATION_NEOPIXEL_BARS)
#define ACCELERATION_BAR_SCALE_VALUE 100
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

/*
 * This map contains the gravity (deceleration or acceleration) values for each pixel.
 * 100 is gravity for a vertical slope, 0 for a horizontal and 70 (sqrt(0,5)*100) for 45 degree.
 */
#define ACCEL_MAP_OFFSET BRIDGE_1_START // all values before this are zero -> saves RAM requires program space.
//#define ACCEL_MAP_OFFSET 0 // disable it
int8_t AccelerationMap[NUMBER_OF_TRACK_PIXELS - ACCEL_MAP_OFFSET];
#define FULL_GRAVITY 100

/*
 * Sound
 */
unsigned long sBeepEndMillis = 0; // for special sounds - overtaking and leap
int sBeepFrequency = 0;
bool sSoundEnabled = true; // not really used yet - always true
#define WINNER_MINIMUM_SOUND_TIME_MILLIS 3000 // minimum time for winner sound and animation before it can be terminated by a button press

/*
 * Race control
 */
#if defined(TEST_MODE)
#define START_ANIMATION_MILLIS 500 // the duration of the start animation
#define LAPS_PER_RACE 255
#else
#define START_ANIMATION_MILLIS 2000 // the duration of the start animation
#define LAPS_PER_RACE 5
#endif

// Loop modes
#define MODE_WAIT 0
#define MODE_RACE 1
uint8_t sMode = MODE_WAIT;
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
void resetAllCars();
void resetTrack(bool aDoAnimation);
void resetAndShowTrackWithoutCars();
bool isCarInRegion(unsigned int aRegionFirst, unsigned int aRegionLength);
void playError();
void playShutdownMelody();
void playMelodyAndShutdown();

extern volatile unsigned long timer0_millis; // Used for ATmega328P to adjust for missed millis interrupts

/*
 * Helper macro for getting a macro definition as string
 */
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

void myTone(int aFrequency) {
    tone(PIN_BUZZER, aFrequency);
#if defined(TCCR2A)
    // switch to direct toggle output at OC2A / pin 11 to enable direct hardware tone output
    TCCR2A |= _BV(COM2A0);
#endif
}

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
#if defined(ENABLE_ACCELERATOR_INPUT)
    MPU6050IMUData AcceleratorInput;
    bool AcceleratorConnected; // Dynamically detection of accelerator connection true -> accelerator is connected
    bool ButtonInputDetected; // true -> button input was detected and has precedence of acceleration input
    uint16_t AcceleratorLowPassValue;
#  if defined(USE_ACCELERATION_NEOPIXEL_BARS)
    uint8_t AccelerationBarPin;
#  endif
#endif // defined(ENABLE_ACCELERATOR_INPUT)

    uint8_t NumberOfThisCar; // 1, 2...
    uint8_t AcceleratorButtonPin;
    color32_t Color;

    float SpeedAsPixelPerLoop; // Reasonable values are 0.5 to 2.0
    float Distance;
    uint16_t PixelPosition; // the index of head of car on the track
    uint8_t Laps;

    bool lastButtonState;
    const char *WinnerMelody;

    Car() {  // @suppress("Class members should be properly initialized")
        TrackPtr = NULL;
//        CarIsActive = false;
    }

    void init(NeoPatterns *aTrackPtr, uint8_t aNumberOfThisCar, uint8_t aButtonPin, color32_t aCarColor,
            const char *aWinnerMelody) {
        TrackPtr = aTrackPtr;
        NumberOfThisCar = aNumberOfThisCar;
        AcceleratorButtonPin = aButtonPin;
        pinMode(AcceleratorButtonPin, INPUT_PULLUP);
        Color = aCarColor;
        WinnerMelody = aWinnerMelody;

        reset();

#if defined(ENABLE_ACCELERATOR_INPUT)
        if (aNumberOfThisCar == 2) {
            AcceleratorInput.setI2CAddress(MPU6050_ADDRESS_AD0_HIGH);
        }
        // use maximum filtering. It prefers slow and huge movements :-)

        if (!AcceleratorInput.initMPU6050AndCalculateAllOffsetsAndWait(20, MPU6050_BAND_5_HZ)) {
            AcceleratorConnected = false;
            Serial.print(F("No MPU6050 IMU connected at address 0x"));
            Serial.print(AcceleratorInput.I2CAddress >> 1, HEX);
            Serial.print(F(" for car "));
            Serial.print(aNumberOfThisCar);
            Serial.println(F(". You may want to disable \"#define ENABLE_ACCELERATOR_INPUT\""));
            if (sSerialLCDAvailable) {
                myLCD.setCursor(0, aNumberOfThisCar + 1);
                myLCD.print(F("No IMU for car "));
                myLCD.print(aNumberOfThisCar);
            }
            playError();
        } else {
            AcceleratorConnected = true;
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
        lastButtonState = 1;
#if defined(ENABLE_ACCELERATOR_INPUT)
        ButtonInputDetected = false;
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
        if (tButtonIsPressed || (!ButtonInputDetected && AcceleratorConnected && (getAcceleratorValueShift8() >= 4))) {
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
        uint8_t tAcceleration = AcceleratorInput.computeAccelerationWithFloatingOffset() >> 8;
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
    bool checkForWinner(uint8_t aLapsNeededToWin) {
        if (Laps >= aLapsNeededToWin) {
            if (!sOnlyPlotterOutput) {
                Serial.print(F("Winner is car "));
                Serial.println(NumberOfThisCar);
            }
            doWinner();
            resetAndShowTrackWithoutCars();
            return true;
        }
        return false;
    }

    /*
     * Start with a delay to isInitialized the winner situation
     * then Play a winner melody and run 3 scanner animations with the car color on the track
     * You can stop melody and animation after 2 seconds, by pressing the car button.
     * The 2 seconds are introduced, to avoid direct abort by button action just after the finish.
     */
    void doWinner() {
        noTone(PIN_BUZZER);
        // isInitialized winner situation
        delay(3000);
        startPlayRtttlPGM(PIN_BUZZER, WinnerMelody);
        TrackPtr->ScannerExtended(Color, Laps, 10, 3, FLAG_SCANNER_EXT_ROCKET | FLAG_DO_NOT_CLEAR);

        while (updatePlayRtttl()) {
#if defined(TIMING_TEST)
                /*
                 *  20 microseconds for loop, 300 microseconds if melody updated
                 *  19 ms duration for resetTrack() - every 50 ms
                 */
                digitalWrite(PIN_TIMING, HIGH);
#endif
            if (TrackPtr->update()) {
                timer0_millis += MILLIS_FOR_TRACK_TO_SHOW;
                // restore bridge and loop pattern, which might be overwritten by scanner.
                resetTrack(false);
            }
#if defined(TIMING_TEST)
                digitalWrite(PIN_TIMING, LOW);
#endif
            uint32_t tStartMillis = millis();
            // wait at least 3 seconds
            if (millis() - tStartMillis > WINNER_MINIMUM_SOUND_TIME_MILLIS)
                if (checkInput()) {
                    stopPlayRtttl(); // to stop in a deterministic fashion
                }
            yield();
        }
        reset();
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
        else if (!ButtonInputDetected && AcceleratorConnected) {
            //Here, no button was pressed before and accelerator is connected
            tAcceleration = getAcceleratorValueShift8();
        } else {
            tAcceleration = 0;
        }
#endif
#  if defined(USE_ACCELERATION_NEOPIXEL_BARS)
        AcceleratorLowPassValue += (((int16_t) (tAcceleration - AcceleratorLowPassValue))) >> 3;
        // scale it so that 100 -> 8
        AccelerationCommonNeopixelBar.drawBar(AcceleratorLowPassValue / (100 / 8), Color);
        AccelerationCommonNeopixelBar.setPin(AccelerationBarPin);
        AccelerationCommonNeopixelBar.show();
#  endif
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
                printBigNumber4(Laps, ((NumberOfThisCar - 1) * 13) + 2);
            }
            tRetval = CAR_LAP_CONDITION;
        }

        /*
         * Compute new speed:
         * - Acceleration from map and friction are simply subtracted from speed
         * - Aerodynamic drag is subtracted proportional from speed
         */
        if (PixelPosition >= ACCEL_MAP_OFFSET && AccelerationMap[PixelPosition - ACCEL_MAP_OFFSET] != 0) {
            /*
             * Here we are on a ramp or loop
             */
            SpeedAsPixelPerLoop += tGravity * AccelerationMap[PixelPosition - ACCEL_MAP_OFFSET];
#if defined(TRACE)
            Serial.print(F(" Gravity="));
            Serial.print(AccelerationMap[PixelPosition]);
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

/*******************************************************************************************
 * The RAMP class
 * Sets gravity for the ramps and allocates NeoPatterns for the ramps to startAnimation
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
#  if defined(__AVR__) && defined(DEBUG)
            printFreeHeap(&Serial);
#  endif
        } else {
            Serial.print(F("Not enough heap memory ("));
            Serial.print(sizeof(NeoPatterns) + 2);
            Serial.println(F(") for RampPatterns."));
#  if defined(__AVR__)
            printFreeHeap(&Serial);
#  endif
        }

#else
        (void) aTrackPtr;
#endif // !defined(BRIDGE_NO_NEOPATTERNS)
        setGravity();
    }

    void setGravity() {
#if defined(DEBUG)
        if (RampLength < RampHeight) {
            Serial.print(F("Error! Ramp length="));
            Serial.print(RampLength);
            Serial.print(F(" must be bigger than ramp height="));
            Serial.println(RampHeight);
        }
#endif
        int8_t tResultingForce = ((uint16_t) (RampHeight * FULL_GRAVITY)) / RampLength; // results in values from 0 to 100
        if (!isRampDown) {
            tResultingForce = -tResultingForce; // deceleration for ramp up
        }
        AccelerationMap[StartPositionOnTrack - ACCEL_MAP_OFFSET] = tResultingForce / 2; // Start with half deceleration for ramp up
        AccelerationMap[StartPositionOnTrack - ACCEL_MAP_OFFSET + RampLength] = tResultingForce / 2; // End with half deceleration for ramp up
        for (int i = 1; i < RampLength; i++) {
            AccelerationMap[StartPositionOnTrack - ACCEL_MAP_OFFSET + i] = tResultingForce; // Deceleration for ramp up
        }

#if defined(DEBUG)
        Serial.print(F("Ramp force= "));
        for (int i = 0; i <= RampLength; i++) {
            Serial.print(StartPositionOnTrack - ACCEL_MAP_OFFSET + i);
            Serial.print('|');
            Serial.print(AccelerationMap[StartPositionOnTrack - ACCEL_MAP_OFFSET + i]);
            Serial.print(' ');
        }
        Serial.println();
        Serial.flush();
#endif
    }

    void animate() {
#if !defined(BRIDGE_NO_NEOPATTERNS)
        if (isInitialized) {
            if (isRampDown) {
                RampPatterns->ColorWipeD(RAMP_COLOR, START_ANIMATION_MILLIS, 0, DIRECTION_DOWN);
            } else {
                RampPatterns->ColorWipeD(RAMP_COLOR, START_ANIMATION_MILLIS, 0, DIRECTION_UP);
            }
        }
#endif
    }

    void draw(bool aDoAnimation) {
        bool tCarIsOnRamp = isCarInRegion(StartPositionOnTrack, RampLength);
        color32_t tColor = RAMP_COLOR;
        if (tCarIsOnRamp) {
            tColor = TrackPtr->dimColorWithGamma5(tColor, 160);
        }
#if !defined(BRIDGE_NO_NEOPATTERNS)
        if (isInitialized && aDoAnimation) {
// to be extended :-)
        }
#else
        (void) aDoAnimation; // to avoid compiler warning
#endif
        TrackPtr->fillRegion(tColor, StartPositionOnTrack, RampLength);
    }
};

/*******************************************************************************************
 * The BRIDGE class
 * Requires 23 bytes + 2 * sizeof(NeoPatterns) RAM per bridge
 *******************************************************************************************/
class Bridge {
#if !defined(BRIDGE_NO_NEOPATTERNS)
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

    void animate() {
#if !defined(BRIDGE_NO_NEOPATTERNS)
        if (isInitialized) {
            RampUp.animate();
            RampDown.animate();
        }
#endif
    }

    void draw(bool aDoAnimation) {
#if !defined(BRIDGE_NO_NEOPATTERNS)
        if (isInitialized) {
            RampUp.draw(aDoAnimation);
            RampDown.draw(aDoAnimation);
        }
#endif
    }
};

/********************************
 * The LOOP class
 ********************************/
class Loop {
    /*
     * Needs 10 bytes + sizeof(NeoPatterns) RAM per loop
     *
     * Ramp consists of (aRampLength + 1) pixel with the first and last with half the gravity
     * aRampUpStart - First pixel with gravity = 1/2 gravity
     * aRampLength - aRampUpStart + aRampLength is last ramp pixel with gravity = 1/2 gravity
     * aRampTopPlatformLength - if 0 then we have 2 pixel with gravity = 1/2 gravity
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
            LoopPatterns = new NeoPatterns(TrackPtr, StartPositionOnTrack, LoopLength, false);
            isInitialized = true;
#  if defined(__AVR__) && defined(DEBUG)
            printFreeHeap(&Serial);
#  endif
        } else {
            Serial.print(F("Not enough heap memory ("));
            Serial.print(sizeof(NeoPatterns) + 2);
            Serial.println(F(") for LoopPatterns."));
#  if defined(__AVR__)
            printFreeHeap(&Serial);
#  endif
        }
#else
        (void) aTrackPtr;
#endif
        setGravity();
    }

    void setGravity() {
        for (int i = 0; i < LoopLength; i++) {
            AccelerationMap[StartPositionOnTrack - ACCEL_MAP_OFFSET + i] =
                    -((sin((TWO_PI / LoopLength) * i) + 0.005) * FULL_GRAVITY); // we start with deceleration for 1. half of loop
        }

#if defined(DEBUG)
        Serial.print(F("Loop force= "));
        for (int i = 0; i <= LoopLength; i++) {
            Serial.print(StartPositionOnTrack - ACCEL_MAP_OFFSET + i);
            Serial.print('|');
            Serial.print(AccelerationMap[StartPositionOnTrack - ACCEL_MAP_OFFSET + i]);
            Serial.print(' ');
        }
        Serial.println();
        Serial.flush();
#endif
    }

    void startAnimation() {
#if !defined(LOOP_NO_NEOPATTERNS)
        if (isInitialized) {
            LongUnion tRandom;
            tRandom.Long = random();
            if (tRandom.UBytes[0] & 0x03) {
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
                // 0
                LoopPatterns->ScannerExtendedD(COLOR32_BLUE_HALF, 8, START_ANIMATION_MILLIS, 2,
                FLAG_SCANNER_EXT_ROCKET | FLAG_SCANNER_EXT_VANISH_COMPLETE | FLAG_SCANNER_EXT_START_AT_BOTH_ENDS, DIRECTION_UP);
            }

        }
#endif
    }

    /*
     * Draw the loop in a fixed or changing color
     */
    void draw(bool aDoAnimation) {
        bool tCarIsOnLoop = isCarInRegion(StartPositionOnTrack, LoopLength);
        color32_t tColor = LOOP_COLOR;
#if !defined(LOOP_NO_NEOPATTERNS)
        if (isInitialized && aDoAnimation) {
            if (!LoopPatterns->update()) {
                // do not increment RainbowIndex at each call
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
}
;

/********************************
 * Start of program
 ********************************/
Car cars[NUMBER_OF_CARS];
Bridge bridges[NUMBER_OF_BRIDGES];
Loop loops[NUMBER_OF_LOOPS];
extern size_t __malloc_margin;
void setup() {
    __malloc_margin = 120; // 128 is the default value
    pinMode(LED_BUILTIN, OUTPUT);

    //
    pinMode(PIN_MANUAL_PARAMETER_MODE, INPUT_PULLUP);
    pinMode(PIN_ONLY_PLOTTER_OUTPUT, INPUT_PULLUP);

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

    sOnlyPlotterOutput = !digitalRead(PIN_ONLY_PLOTTER_OUTPUT);
    bool tIsAnalogParameterInputMode = !digitalRead(PIN_MANUAL_PARAMETER_MODE);

    if (!sOnlyPlotterOutput) {

        // Just to know which program is running on my Arduino
        Serial.println(F("START " __FILE__ " from " __DATE__));
        Serial.println(
                F(
                        "Connect pin " STR(PIN_ONLY_PLOTTER_OUTPUT) " to ground, to suppress such prints not suited for Arduino plotter"));
        Serial.print(F("Pin " STR(PIN_MANUAL_PARAMETER_MODE) " is "));
        if (!tIsAnalogParameterInputMode) {
            Serial.print(F("dis"));
        }
        Serial.print(F("connected from ground -> AnalogParameterInputMode is "));

        if (tIsAnalogParameterInputMode) {
            Serial.println(F("enabled"));
        } else {
            Serial.println(F("disabled"));
        }
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
        initBigNumbers();
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

    /*
     * Clear acceleration map
     */
    for (int i = 0; i < NUMBER_OF_TRACK_PIXELS - ACCEL_MAP_OFFSET; i++) {
        AccelerationMap[i] = 0;
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
    loops[0].init(&track, LOOP_1_UP_START, LOOP_1_LENGTH); // Requires 69 bytes on heap

    /*
     * Setup cars
     */
    if (!sOnlyPlotterOutput) {
        Serial.println(F("Initialize cars"));
        Serial.flush();
    }
    cars[0].init(&track, 1, PIN_PLAYER_1_BUTTON, CAR_1_COLOR, MissionImp);
    cars[1].init(&track, 2, PIN_PLAYER_2_BUTTON, CAR_2_COLOR, StarWars);
//    cars[2].init(&track, 3, PIN_PLAYER_3_BUTTON, CAR_3_COLOR, Entertainer);

#if defined(USE_ACCELERATION_NEOPIXEL_BARS)
// initialize the central NeoPixel object
    AccelerationCommonNeopixelBar.begin();
    cars[0].AccelerationBarPin = PIN_VU_BAR_1;
    cars[1].AccelerationBarPin = PIN_VU_BAR_2;
#endif

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
        bridges[i].animate();
    }
    for (uint_fast8_t i = 0; i < NUMBER_OF_LOOPS; ++i) {
        loops[i].startAnimation();
    }
// wait for animation to end
    track.updateAllPartialPatternsAndWaitForPatternsToStop();

    /*
     * End of setup
     */
    resetAndShowTrackWithoutCars();

    if (sOnlyPlotterOutput) {
        // print Plotter caption after check for LCD
        Serial.println();
        Serial.println(F("Accel[1] AccelLP[1] Speed[1] Accel[2] AccelLP[2] Speed[2]"));
    } else {
        Serial.print(F("Press any button"));
#  if defined(ENABLE_ACCELERATOR_INPUT)
        Serial.print(F(" or move controller"));
#endif
        Serial.println(F(" to start countdown"));
    }

    if (sSerialLCDAvailable) {
        uint8_t tLineIndex = 1;
        myLCD.setCursor(0, tLineIndex++);
        myLCD.print(F("Press any button"));
#  if defined(ENABLE_ACCELERATOR_INPUT)
        myLCD.setCursor(0, tLineIndex++);
        myLCD.print(F("or move controller"));
#  endif
        myLCD.setCursor(0, tLineIndex);
        myLCD.print(F("to start countdown"));
    }
}

/*
 * 10 Milliseconds per loop without delay for
 */
void loop() {
    static uint32_t sNextLoopMillis;
    static uint32_t sLastAnimationMillis;

    sLoopCountForDebugPrint++;
    sOnlyPlotterOutput = !digitalRead(PIN_ONLY_PLOTTER_OUTPUT);

#if defined(INFO) && defined(__AVR__)
    if (!sOnlyPlotterOutput) {
        printStackUnusedAndUsedBytesIfChanged(&Serial);
    }
#endif

    if (sMode == MODE_WAIT) {
        /*
         * Do periodic animation
         */
        if (millis() - sLastAnimationMillis > ANIMATION_INTERVAL_MILLIS) {
            sLastAnimationMillis = millis();
            loops[0].startAnimation();
            if (!sOnlyPlotterOutput) {
                Serial.println(F("Start loop Animation"));
            }
        }

        /*
         * Wait for start (first button pressed)
         */
        for (uint_fast8_t i = 0; i < NUMBER_OF_CARS; ++i) {
            if (cars[i].checkInput()) {
                sMode = MODE_RACE;
                startRace(); // blocking call, which checks also for other cars to start :-)
                break;
            }
        }

        track.updateAllPartialPatterns();

        // continue to wait if not started
        if (sMode == MODE_WAIT) {
            delay(10); // check every 10 ms
            return;
        }
    }

    /*
     * Race mode
     * 1. Reset track - run animation and show
     * 2. Move cars
     * 3. Check for winner and overtaking the leader
     */
    resetTrack(true);

    /*
     * Check input / buttons and get new speed
     */

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
     * Check for winner. Blocking call, if one car is the winner
     */
    for (uint_fast8_t i = 0; i < NUMBER_OF_CARS; ++i) {
        if (cars[i].checkForWinner(LAPS_PER_RACE)) {
            resetAllCars();
            sMode = MODE_WAIT;
            break;
        }
    }

    /*
     * Check for overtaking current leader car
     */
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

    /*
     * Draw all cars
     */
    for (uint_fast8_t i = 0; i < NUMBER_OF_CARS; ++i) {
        cars[i].draw();
    }

    /*
     * Show track
     */
    track.show(); // 9 Milliseconds for 300 Pixel
    timer0_millis += MILLIS_FOR_TRACK_TO_SHOW; // compensate Arduino millis() for the time interrupt was disabled for track.isInitialized().

    /*
     * Manage sound. Must check for situation after winner
     */
    if (sSoundEnabled && sMode != MODE_WAIT) {
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
     * Start each loop in 20 ms distance
     */
    while (millis() < sNextLoopMillis) {
        yield();
    }
    sNextLoopMillis += MILLISECONDS_PER_LOOP;
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

bool isCarInRegion(unsigned int aRegionFirst, unsigned int aRegionLength) {
    for (uint_fast8_t i = 0; i < NUMBER_OF_CARS; ++i) {
        if (cars[i].PixelPosition >= aRegionFirst && cars[i].PixelPosition <= (aRegionFirst + aRegionLength)) {
            return true;
        }
    }
    return false;
}

/*
 * Clear track and redraw bridges and loops
 * @param aDoAnimation if true draw animated loops
 */
void resetTrack(bool aDoAnimation) {
    track.clear();
    for (uint_fast8_t i = 0; i < NUMBER_OF_BRIDGES; ++i) {
        bridges[i].draw(aDoAnimation);
    }
    for (uint_fast8_t i = 0; i < NUMBER_OF_LOOPS; ++i) {
        loops[i].draw(aDoAnimation);
    }
}

void resetAndShowTrackWithoutCars() {
    resetTrack(false);
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
        // First checkInput(), to have input feedback enabled while starting
        cars[i].draw();
    }
    track.show();

    uint8_t tIndex = 4; // index of last light
    if (sSerialLCDAvailable) {
        myLCD.clear();
    }
    for (int tCountDown = 4; tCountDown >= 0; tCountDown--) {
        // delay at start of loop to enable fast start after last countdown
        delay(1000);
        track.setPixelColor(tIndex + (2 * tCountDown), COLOR32_RED);
        track.setPixelColor(tIndex + (2 * tCountDown) + 1, COLOR32_RED);
        track.show();
        if (tCountDown != 0) {
            tone(PIN_BUZZER, 600, 200);
#if defined(TCCR2A)
            // switch to direct toggle output at OC2A / pin 11 to enable direct hardware tone output
            TCCR2A |= _BV(COM2A0);
#endif
        } else {
            sBeepFrequency = 400;
            sBeepEndMillis = millis() + 400;
        }
        if (!sOnlyPlotterOutput) {
            Serial.println(tCountDown);
        }
        if (sSerialLCDAvailable) {
            printBigNumber4(tCountDown, 9);
        }
    }
    if (!sOnlyPlotterOutput) {
        Serial.println(F("Start race"));
    }
    myLCD.clear();
    if (sSerialLCDAvailable) {
        myLCD.clear();
        printBigNumber4(0, 2);
        printBigNumber4(0, 15);
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
#endif
}

const uint8_t LCDCustomPatterns[][8] PROGMEM = { { 0x01, 0x07, 0x0F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F }, // char 1: bottom right triangle
        { 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1F, 0x1F, 0x1F },      // char 2: bottom block
        { 0x10, 0x1C, 0x1E, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F },      // char 3: bottom left triangle
        { 0x1F, 0x0F, 0x07, 0x01, 0x00, 0x00, 0x00, 0x00 },      // char 4: top right triangle
        { 0x1F, 0x1E, 0x1C, 0x10, 0x00, 0x00, 0x00, 0x00 },      // char 5: top left triangle
        { 0x1F, 0x1F, 0x1F, 0x1F, 0x00, 0x00, 0x00, 0x00 },      // char 6: upper block
        { 0x1F, 0x1F, 0x1E, 0x1C, 0x18, 0x18, 0x10, 0x10 },      // char 7: full top right triangle
        { 0x01, 0x07, 0x0F, 0x1F, 0x00, 0x00, 0x00, 0x00 }       // char 8: top right triangle
};

// !!! Must be without comment and closed by @formatter:on
// @formatter:off
const char bigNumbers4[][30] PROGMEM = {                         // 4-line numbers
//         0               1               2               3               4              5               6                7               8               9
    {0x01,0x06,0x03, 0x08,0xFF,0xFE, 0x08,0x06,0x03, 0x08,0x06,0x03, 0xFF,0xFE,0xFF, 0xFF,0x06,0x06, 0x01,0x06,0x03, 0x06,0x06,0xFF, 0x01,0x06,0x03, 0x01,0x06,0x03},
    {0xFF,0xFE,0xFF, 0xFE,0xFF,0xFE, 0x02,0x02,0xFF, 0xFE,0x02,0xFF, 0xFF,0x02,0xFF, 0xFF,0x02,0x02, 0xFF,0x02,0x02, 0xFE,0x01,0x07, 0xFF,0x02,0xFF, 0xFF,0x02,0xFF},
    {0xFF,0xFE,0xFF, 0xFE,0xFF,0xFE, 0xFF,0xFE,0xFE, 0xFE,0xFE,0xFF, 0xFE,0xFE,0xFF, 0xFE,0xFE,0xFF, 0xFF,0xFE,0xFF, 0xFE,0xFF,0xFE, 0xFF,0xFE,0xFF, 0xFE,0xFE,0xFF},
    {0x04,0x06,0x05, 0xFE,0x06,0xFE, 0x06,0x06,0x06, 0x04,0x06,0x05, 0xFE,0xFE,0x06, 0x04,0x06,0x05, 0x04,0x06,0x05, 0xFE,0x06,0xFE, 0x04,0x06,0x05, 0x04,0x06,0x05}
};
// @formatter:on

void initBigNumbers() {
    for (uint_fast8_t i = 0; i < 8; i++) {                     // create 8 custom characters
        myLCD.createChar(i + 1, (const char*) &LCDCustomPatterns[i]);
    }
}

void printBigNumber4(byte digit, byte leftAdjust) {
    for (uint_fast8_t row = 0; row < 4; row++) {
        myLCD.setCursor(leftAdjust, row);
        for (byte num = digit * 3; num < digit * 3 + 3; num++) {
            myLCD.write(pgm_read_byte(&bigNumbers4[row][num]));
        }
    }
}
