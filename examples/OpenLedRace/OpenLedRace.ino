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
 *  OpenLedRace.cppp
 *
 *  Extended version of the OpenLedRace "version Basic for PCB Rome Edition. 2 Player, without Boxes Track"
 *  Extensions are:
 *  Classes for Car, Bridge, Ramp and Loop.
 *  Dynamic activation of up to 4 cars.
 *  Light effects by NeoPattern library.
 *  Tone generation without dropouts by use of hardware timer output.
 *  Winner melody by PlayRTTTL library.
 *  Compensation for millis() timer.
 *  Checks for RAM availability.
 *
 *  You need to install "Adafruit NeoPixel" library under "Tools -> Manage Libraries..." or "Ctrl+Shift+I" -> use "neoPixel" as filter string
 *
 *  Copyright (C) 2020  Armin Joachimsmeyer
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

/*
 * Open LED Race
 * An minimalist cars race for LED strip
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * by gbarbarov@singulardevices.com for Arduino day Seville 2019
 * https: //www.hackster.io/gbarbarov/open-led-race-a0331a
 * https://twitter.com/openledrace
 *
 * https://gitlab.com/open-led-race
 * https://openledrace.net/open-software/
 *
 */

#include <Arduino.h>

#include <NeoPatterns.h>
#include "PlayRtttl.h"
#include "AVRUtils.h"

#define VERSION_EXAMPLE "1.0"

//#define TRACE
#define DEBUG
//#define INFO
#include "DebugLevel.h"

#define TIMING_TEST

#ifdef TIMING_TEST
#define PIN_TIMING  9
#endif

/*
 * Pin layout
 */
#define PIN_PLAYER_1_BUTTON 4
#define PIN_PLAYER_2_BUTTON 5
#define PIN_PLAYER_3_BUTTON 6
#define PIN_PLAYER_4_BUTTON 7
#define PIN_NEOPIXEL        8
#define PIN_AUDIO          11

#define PIN_MANUAL_PARAMETER_MODE       9 // if connected to ground, analog inputs for parameters are used
#define PIN_GRAVITY        A0
#define PIN_FRICTION       A1
#define PIN_DRAG           A2

/*
 * The track
 * Think of 1 Pixel = 1 meter
 */
#define NUMBER_OF_TRACK_PIXELS  300 // Number of LEDs in strip
NeoPatterns track = NeoPatterns(NUMBER_OF_TRACK_PIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
/*
 * Game loop timing
 */
#define MILLISECONDS_PER_LOOP 20 // 50 fps
#define MILLIS_FOR_TRACK_TO_SHOW  (NUMBER_OF_TRACK_PIXELS / 33) // Needed for correction of the millis() timer

/*
 * This map contains the gravity (deceleration or acceleration) values for each pixel.
 * 100 is gravity for a vertical slope, 0 for a horizontal and 70 (sqrt(0,5)*100) for 45 degree.
 */
int8_t GravityMap[NUMBER_OF_TRACK_PIXELS];
#define FULL_GRAVITY 100

/*
 * The bridges with a ramp up, a flat platform and a ramp down
 */
#define NUMBER_OF_BRIDGES 1
#define BRIDGE_1_START 200
#define BRIDGE_1_RAMP_LENGTH 20 // in pixel
#define BRIDGE_1_TOP_PLATFORM_LENGTH 20 // > 0 for bridges with a ramp up a flat bridge and a ramp down
#define BRIDGE_1_HEIGHT 14 // in pixel -> 45 degree slope here

/*
 * The loops
 */
#define NUMBER_OF_LOOPS 1
#define LOOP_1_UP_START 100
#define LOOP_1_LENGTH 40 // in pixel

/*
 * Sound
 */
unsigned long sBeepEndMillis = 0; // for special sounds - overtaking and leap
int sBeepFrequency = 0;
bool sSoundEnabled = true; // not used yet
#define WINNER_MINIMUM_SOUND_TIME_MILLIS 2000 // minimum time for winner sound before it can be terminated by a button press

/*
 * Race control
 */
#define START_ANIMATION_MILLIS 2000 // the duration of the start animation
#define LAPS_PER_RACE 5
// Loop modes
#define MODE_WAIT 0
#define MODE_RACE 1
uint8_t sMode = MODE_WAIT;
uint16_t sLoopCount;
uint8_t sIndexOfLeadingCar = 0; // index of leading car

/*
 * Car control
 * we get a press each 70 to 150 milliseconds / each 3th to 7th loop
 */
#define NUMBER_OF_CARS              4 // Maximum number of cars supported. Each car is activated by first press of its button, so races with 2 cars are also possible.
#define ACCELERATION_PER_PRESS      0.2 // As pixel per loop
#define FRICTION_PER_LOOP           0.004
#define AERODYNAMIC_DRAG_PER_LOOP   0.004
#define GRAVITY_FACTOR_FOR_MAP      0.0002 // gravity constant for gravity values from 0 to 100 => gravity 1 is 0.02

/*
 * Forward declarations
 */
void resetTrack(bool aDoAnimation);
void resetAndShowTrackWithoutCars();

extern volatile unsigned long timer0_millis; // ATmega328P

/*
 * Code related to each car is contained in this class
 */
/*
 * Return values for computeSpeedAndDistance
 */
#define CAR_NOP 0
#define CAR_LAP_CONDITION 1
class Car {
    /*
     * Needs 23 bytes RAM per car
     */
public:
    NeoPatterns * TrackPtr;

    uint8_t NumberOfThisCar;
    uint8_t AcceleratorButtonPin;
    color32_t Color;

    float SpeedAsPixelPerLoop;
    float Distance;
    uint16_t PixelPosition;
    uint8_t Laps;
    bool CarIsActive;

    bool lastButtonState;
    const char * WinnerMelody;

    Car() {  // @suppress("Class members should be properly initialized")
        TrackPtr = NULL;
        CarIsActive = false;
    }

    void init(NeoPatterns* aTrackPtr, uint8_t aNumberOfThisCar, uint8_t aButtonPin, color32_t aCarColor,
            const char * aWinnerMelody) {
        TrackPtr = aTrackPtr;
        NumberOfThisCar = aNumberOfThisCar;
        AcceleratorButtonPin = aButtonPin;
        pinMode(AcceleratorButtonPin, INPUT_PULLUP);
        Color = aCarColor;
        WinnerMelody = aWinnerMelody;

        reset();
    }

    void reset() {
        SpeedAsPixelPerLoop = 0;
        Distance = 0;
        Laps = 0;
        lastButtonState = 1;
//        CarIsActive = false;
    }

    /*
     * The car consists of Laps pixel
     */
    void draw() {
        if (CarIsActive) {
            for (int i = 0; i <= Laps; i++) {
                int16_t tDrawPosition = PixelPosition - i;
                if (tDrawPosition < 0) {
                    // wrap around
                    tDrawPosition += TrackPtr->numPixels();
                }
//            TrackPtr->setPixelColor(tDrawPosition, Color);
                TrackPtr->addPixelColor(tDrawPosition, Red(Color), Green(Color), Blue(Color));
            }
        }
    }

    /*
     * Check if button was just pressed
     * activates car and returns true if button was just pressed
     */
    bool checkButton() {
        // check if initialized, can not use CarIsActive here!
        if (TrackPtr != NULL) {
            /*
             * Check Button
             */
            bool tLastButtonState = lastButtonState;
            lastButtonState = digitalRead(AcceleratorButtonPin);

            if (tLastButtonState == true && lastButtonState == false) {
#ifdef DEBUG
                Serial.print(NumberOfThisCar);
                Serial.println(F(" Button pressed"));
#endif
                if (!CarIsActive) {
                    CarIsActive = true;
#ifdef DEBUG
                    Serial.print(NumberOfThisCar);
                    Serial.println(F(" Car activated"));
#endif
                }
                return true;
            }
        }
        return false;
    }

    /*
     * @return true if this car is the winner
     */
    bool checkForWinner(uint8_t aLapsNeededToWin) {
        if (CarIsActive && Laps >= aLapsNeededToWin) {
            Serial.print(F("Winner is car "));
            Serial.println(NumberOfThisCar);
            doWinner();
            resetAndShowTrackWithoutCars();
            return true;
        }
        return false;
    }

    void doWinner() {
        // show winner state
        delay(2000);
        uint32_t tStartMillis = millis();
        startPlayRtttlPGM(PIN_AUDIO, WinnerMelody);
        TrackPtr->ScannerExtended(Color, Laps, 10, 3, FLAG_SCANNER_EXT_ROCKET | FLAG_DO_NOT_CLEAR);
        /*
         *  20 microseconds for loop, 300 microseconds if melody updated
         *  19 ms duration for resetAndShowTrackWithoutCars() - every 50 ms
         */
        while (updatePlayRtttl()) {
#ifdef TIMING_TEST
            digitalWrite(PIN_TIMING, HIGH);
#endif
            if (TrackPtr->update()) {
                timer0_millis += MILLIS_FOR_TRACK_TO_SHOW;
                /*
                 * initialize track for next scanner pattern, which will then show it.
                 *
                 */
                resetTrack(false);
            }
#ifdef TIMING_TEST
            digitalWrite(PIN_TIMING, LOW);
#endif
            if (millis() - tStartMillis > WINNER_MINIMUM_SOUND_TIME_MILLIS)
                if (checkButton()) {
                    stopPlayRtttl(); // to stop in a deterministic fashion
                }
            yield();
        }
        reset();
    }

    void print(float aGravity, float aFricion, float aDrag) {
        Serial.print(NumberOfThisCar);
        Serial.print(F(" Speed="));
        Serial.print(SpeedAsPixelPerLoop, 3);
        Serial.print(F(" Gravity="));
        Serial.print(aGravity, 6);
        Serial.print(F(" Fricion="));
        Serial.print(aFricion, 6);
        Serial.print(F(" Drag="));
        Serial.println(aDrag, 6);
    }

    /*
     * Check buttons and gravity and get new speed
     *  850 us for analogRead + DEBUG
     *  100 microseconds
     */
    uint8_t computeSpeedAndDistance() {
        uint8_t tRetval = CAR_NOP;

#ifdef TIMING_TEST
        digitalWrite(PIN_TIMING, HIGH);
#endif
        bool tButtonJustPressed = checkButton(); // This can activate car
        if (CarIsActive) {
            if (tButtonJustPressed) {
                SpeedAsPixelPerLoop += ACCELERATION_PER_PRESS;
            }

            bool tIsAnalogParameterInputMode = !digitalRead(PIN_MANUAL_PARAMETER_MODE);
            float tGravity;
            float tFricion;
            float tDrag;

            if (tIsAnalogParameterInputMode) {
                uint16_t tGravityRaw = analogRead(PIN_GRAVITY);
                uint16_t tFricionRaw = analogRead(PIN_FRICTION);
                uint16_t tDragRaw = analogRead(PIN_DRAG);
                tGravity = tGravityRaw * 0.000001;
                tFricion = tFricionRaw * 0.00001;
                tDrag = tDragRaw * 0.00002;
                if ((((sLoopCount & 0x3F) == 0) || tButtonJustPressed) && NumberOfThisCar == 1) {
#ifdef TRACE
                Serial.print(F(" Gravity="));
                Serial.print(tGravityRaw);
                Serial.print(F(" Fricion="));
                Serial.print(tFricionRaw);
                Serial.print(F(" Drag="));
                Serial.println(tDragRaw);
#endif
#ifdef DEBUG
                    print(tGravity, tFricion, tDrag);
#endif
                }
            } else {
                tGravity = GRAVITY_FACTOR_FOR_MAP;
                tFricion = FRICTION_PER_LOOP;
                tDrag = AERODYNAMIC_DRAG_PER_LOOP;
            }

            /*
             * Compute speed and distance
             * add all acceleration values of next move from map including starting point (needed if we stop at this point)
             */

            /*
             * take old speed to compute new position
             */
            Distance += SpeedAsPixelPerLoop;
            PixelPosition = (uint16_t) Distance % TrackPtr->numPixels();
            /*
             * Check for lap counter
             */
            if (Distance > TrackPtr->numPixels() * (Laps + 1)) {
                Laps++;
#ifdef INFO
                Serial.print(NumberOfThisCar);
                Serial.println(F(" Laps++"));
#endif
                tRetval = CAR_LAP_CONDITION;
            }

            /*
             * Compute new speed
             */
            if (GravityMap[PixelPosition] != 0) {
                /*
                 * Here we are on a ramp
                 */
                SpeedAsPixelPerLoop += tGravity * GravityMap[PixelPosition];
#ifdef TRACE
            Serial.print(F(" Gravity="));
            Serial.print(GravityMap[PixelPosition]);
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

#ifdef TRACE
        Serial.print(F(" -> "));
        Serial.print(SpeedAsPixelPerLoop, 3);
        Serial.print(F(" => "));
        Serial.println(Distance);
#endif

        }
#ifdef TIMING_TEST
        digitalWrite(PIN_TIMING, LOW);
#endif
        return tRetval;
    }
}
;

class Ramp {
    /*
     * Needs 10 bytes RAM + sizeof(NeoPatterns) per ramp
     * Ramp consists of (aRampLength + 1) pixel with the first and last with half the gravity
     * aRampUpStart - First pixel with gravity = 1/2 gravity
     * aRampLength - aRampUpStart + aRampLength is last ramp pixel with gravity = 1/2 gravity
     * Gravity - 100 is full gravity for vertical slope
     */

public:
    NeoPatterns * TrackPtr;
    NeoPatterns * RampPatterns;
    uint16_t StartPositionOnTrack;
    uint8_t RampLength;
    uint8_t RampHeight;
    bool isRampDown;
    bool show;

    void init(NeoPatterns* aTrackPtr, uint16_t aRampUpStartPositionOnTrack, uint8_t aRampHeight, uint8_t aRampLength,
            bool aIsRampDown) {
        TrackPtr = aTrackPtr;
        StartPositionOnTrack = aRampUpStartPositionOnTrack;
        RampHeight = aRampHeight;
        RampLength = aRampLength;
        isRampDown = aIsRampDown;

        /*
         * NeoPatterns segments to control light effects on both ramps
         * Call malloc() and free() before, since the compiler calls the constructor even when the result of malloc() is NULL, which leads to overwrite low memory.
         */
        void * tMallocTest = malloc(sizeof(NeoPatterns));
        if (tMallocTest != NULL) {
            free(tMallocTest);
            RampPatterns = new NeoPatterns(TrackPtr, StartPositionOnTrack, RampLength, false);
            setGravity();
            show = true;
        } else {
#if defined (__AVR__)
            Serial.print(F("Not enough memory for RampPatterns. Free heap="));
            Serial.println(getFreeHeap());
            Serial.flush();
#else
            Serial.print("Not enough memory for new RampPatterns.");
#endif
        }
    }

    void setGravity() {
        if (RampLength < (RampHeight * 10) / 15) {
            Serial.print(F("Error! Ramp length="));
            Serial.print(RampLength);
            Serial.print(F(" must be bigger than 0.6 * ramp height="));
            Serial.println(RampHeight);
        }

        int8_t tResultingForce = RampHeight * FULL_GRAVITY / RampLength; // results in values from 0 to 100
        if (!isRampDown) {
            tResultingForce = -tResultingForce; // deceleration for ramp up
        }
        GravityMap[StartPositionOnTrack] = tResultingForce / 2; // Start with half deceleration for ramp up
        GravityMap[StartPositionOnTrack + RampLength] = tResultingForce / 2; // End with half deceleration for ramp up
        for (int i = 1; i < RampLength; i++) {
            GravityMap[StartPositionOnTrack + i] = tResultingForce; // Deceleration for ramp up
        }

#ifdef DEBUG
        Serial.print(F("Ramp force= "));
        for (int i = 0; i <= RampLength; i++) {
            Serial.print(StartPositionOnTrack + i);
            Serial.print('|');
            Serial.print(GravityMap[StartPositionOnTrack + i]);
            Serial.print(' ');
        }
        Serial.println();
        Serial.flush();
#endif
    }

    void animate() {
        if (show && RampPatterns != NULL) {
            if (isRampDown) {
                RampPatterns->ColorWipeD(COLOR32_CYAN_QUARTER, START_ANIMATION_MILLIS, 0, DIRECTION_DOWN);
            } else {
                RampPatterns->ColorWipeD(COLOR32_CYAN_QUARTER, START_ANIMATION_MILLIS, 0, DIRECTION_UP);
            }
        }
    }

    void draw() {
        if (show && RampPatterns != NULL) {
            RampPatterns->drawBar(RampPatterns->numPixels(), COLOR32_CYAN_QUARTER, true);
        }
    }
};

class Bridge {
    /*
     * Needs 23 bytes + 2 * sizeof(NeoPatterns) RAM per bridge
     */
    NeoPatterns * TrackPtr;
    Ramp RampUp;
    Ramp RampDown;
    bool show;

public:
    /*
     * aRampTopPlatformLength - if 0 then we have 2 pixel with gravity = 1/2 gravity
     *
     */
    void init(NeoPatterns* aTrackPtr, uint16_t aBridgeStartPositionOnTrack, uint8_t aBridgeHeight, uint8_t aRampLength,
            uint8_t aRampPlatformLength) {
        RampUp.init(aTrackPtr, aBridgeStartPositionOnTrack, aBridgeHeight, aRampLength, false);
        RampDown.init(aTrackPtr, aBridgeStartPositionOnTrack + aRampLength + aRampPlatformLength, aBridgeHeight, aRampLength, true);

        TrackPtr = aTrackPtr;
        show = true;
    }

    void animate() {
        if (show) {
            RampUp.animate();
            RampDown.animate();
        }
    }

    void draw() {
        if (show) {
            RampUp.draw();
            RampDown.draw();
        }
    }
};

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
    NeoPatterns * TrackPtr;
    NeoPatterns * LoopPatterns;
    uint16_t StartPositionOnTrack;
    uint8_t Length;
    bool show;
    uint8_t RainbowIndex;
    uint8_t RainbowIndexDividerCounter; // divides the call to RainbowIndex++

    void init(NeoPatterns* aTrackPtr, uint16_t aStartPositionOnTrack, uint8_t aLength) {
        TrackPtr = aTrackPtr;
        StartPositionOnTrack = aStartPositionOnTrack;
        Length = aLength;
        /*
         * NeoPatterns segments to control light effects on both ramps
         * Call malloc() and free() before, since the compiler calls the constructor even when the result of malloc() is NULL, which leads to overwrite low memory.
         */
        void * tMallocTest = malloc(sizeof(NeoPatterns)); // 67
        if (tMallocTest != NULL) {
            free(tMallocTest);
            LoopPatterns = new NeoPatterns(TrackPtr, StartPositionOnTrack, Length, false);
            setGravity();
            show = true;
        } else {
#if defined (__AVR__)
            Serial.print(F("Not enough memory for new LoopPatterns. Free heap="));
            Serial.println(getFreeHeap());
            Serial.flush();
#else
            Serial.print("Not enough memory for new LoopPatterns.");
#endif
        }
    }

    void setGravity() {
        for (int i = 0; i < Length; i++) {
            GravityMap[StartPositionOnTrack + i] = -((sin((TWO_PI / Length) * i) + 0.005) * FULL_GRAVITY); // we start with deceleration for 1. half of loop
        }

#ifdef DEBUG
        Serial.print(F("Loop force= "));
        for (int i = 0; i <= Length; i++) {
            Serial.print(StartPositionOnTrack + i);
            Serial.print('|');
            Serial.print(GravityMap[StartPositionOnTrack + i]);
            Serial.print(' ');
        }
        Serial.println();
        Serial.flush();
#endif
    }

    void animate() {
        if (show && LoopPatterns != NULL) {
            LoopPatterns->ScannerExtendedD(COLOR32_BLUE_QUARTER, 8, START_ANIMATION_MILLIS, 2,
            FLAG_SCANNER_EXT_ROCKET | FLAG_SCANNER_EXT_VANISH_COMPLETE | FLAG_SCANNER_EXT_START_AT_BOTH_ENDS, DIRECTION_UP);
        }
    }

    void draw(bool aDoRainbow) {
        if (show && LoopPatterns != NULL) {
            if (aDoRainbow) {
                // do not increment RainbowIndex at each call
                if (RainbowIndexDividerCounter++ >= 6) {
                    RainbowIndexDividerCounter = 0;
                    RainbowIndex++;
                }
                LoopPatterns->drawBar(Length, NeoPixel::Wheel(RainbowIndex), true);
            } else {
                LoopPatterns->drawBar(Length, COLOR32_PURPLE_QUARTER, true);
            }
        }
    }
};

Car cars[NUMBER_OF_CARS];
Bridge bridges[NUMBER_OF_BRIDGES];
Loop loops[NUMBER_OF_LOOPS];

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);

    pinMode(PIN_MANUAL_PARAMETER_MODE, INPUT_PULLUP);

#ifdef TIMING_TEST
    pinMode(PIN_TIMING, OUTPUT);
#endif

#if defined(INFO) && defined(__AVR__)
    initStackFreeMeasurement();
#endif

    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__)
    while (!Serial); //delay for Leonardo, but this loops forever for Maple Serial
#endif
// Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ "\r\nVersion " VERSION_EXAMPLE " from " __DATE__));

// This initializes the NeoPixel library and checks if enough memory was available
    if (!track.begin(&Serial)) {
        // Blink forever
        while (true) {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(500);
            digitalWrite(LED_BUILTIN, LOW);
            delay(500);
        }
    }

    /*
     * Clear track and gravity map
     */
//    track.clear();
    for (int i = 0; i < NUMBER_OF_TRACK_PIXELS; i++) {
        GravityMap[i] = 0;
    }

    /*
     * Setup bridges and loops
     */
    bridges[0].init(&track, BRIDGE_1_START, BRIDGE_1_HEIGHT, BRIDGE_1_RAMP_LENGTH, BRIDGE_1_TOP_PLATFORM_LENGTH); // 138 bytes on heap
//    tone(PIN_AUDIO, 64000);
    loops[0].init(&track, LOOP_1_UP_START, LOOP_1_LENGTH); // 69 bytes on heap

    /*
     * Setup cars
     */
    cars[0].init(&track, 1, PIN_PLAYER_1_BUTTON, COLOR32_RED, MissionImp);
    cars[1].init(&track, 2, PIN_PLAYER_2_BUTTON, COLOR32_GREEN, StarWars);
    cars[2].init(&track, 3, PIN_PLAYER_3_BUTTON, COLOR32_BLUE, Entertainer);
//    cars[3].init(&track, 4, PIN_PLAYER_4_BUTTON, COLOR32_BLUE, Entertainer);

    // signal boot
    tone(PIN_AUDIO, 1200, 200);

    /*
     * Boot animation
     */
    delay(300); // to avoid starting animation at power up before USB bootloader delay
    for (uint8_t i = 0; i < NUMBER_OF_BRIDGES; ++i) {
        bridges[i].animate();
    }
    for (uint8_t i = 0; i < NUMBER_OF_LOOPS; ++i) {
        loops[i].animate();
    }
    // wait for animation to end
    track.updateAllPartialPatternsAndWaitForPatternsToStop();

    /*
     * End of setup
     */
    resetAndShowTrackWithoutCars();

#if defined(__AVR__)
    printStackFreeMinimumBytes(&Serial);
#endif
    Serial.println(F("Press any button to start countdown"));
}

void resetTrack(bool aDoAnimation) {
    track.clear();
    for (uint8_t i = 0; i < NUMBER_OF_BRIDGES; ++i) {
        bridges[i].draw();
    }
    for (uint8_t i = 0; i < NUMBER_OF_LOOPS; ++i) {
        loops[i].draw(aDoAnimation);
    }
}

void resetAndShowTrackWithoutCars() {
    resetTrack(false);
    track.show();
    timer0_millis += MILLIS_FOR_TRACK_TO_SHOW;
}

void resetAllCars() {
    for (uint8_t i = 0; i < NUMBER_OF_CARS; ++i) {
        cars[i].reset();
    }
}

void startRace() {
    Serial.println(F("Start race countdown"));
//    resetAndShowTrackWithoutCars();
    uint8_t tIndex = 4; // index of last light

    for (int tCountDown = 4; tCountDown >= 0; tCountDown--) {

        // delay at start of loop to enable fast start after last countdown
        for (int tDelayCount = 0; tDelayCount < 100; ++tDelayCount) {
            // check buttons every 10 milliseconds
            for (uint8_t i = 0; i < NUMBER_OF_CARS; ++i) {
                if (cars[i].checkButton()) {
                    cars[i].draw();
                    track.show();
                }
            }
            delay(10);
        }

        track.setPixelColor(tIndex + (2 * tCountDown), COLOR32_RED);
        track.setPixelColor(tIndex + (2 * tCountDown) + 1, COLOR32_RED);
        track.show();
        if (tCountDown != 0) {
            tone(PIN_AUDIO, 600, 200);
        } else {
            sBeepFrequency = 400;
            sBeepEndMillis = millis() + 400;
        }
        Serial.println(tCountDown);
    }
    Serial.println(F("Start race"));
}

/*
 * 10 Milliseconds per loop without delay for
 */
void loop() {
    static uint32_t sNextLoopMillis;

    sLoopCount++;

    if (sMode == MODE_WAIT) {
        /*
         * Wait for start (first button pressed)
         */
        for (uint8_t i = 0; i < NUMBER_OF_CARS; ++i) {
            if (cars[i].checkButton()) {
                cars[i].draw();
                track.show();
                sMode = MODE_RACE;
                startRace(); // blocking call
            }
        }
        if (sMode == MODE_WAIT) {
            delay(10); // check every 10 ms
            return;
        }
    }

    /*
     * Race mode
     * 1. Reset track
     * 2. Move cars
     * 3. Check for winner and overtaking the leader
     */
    resetTrack(true);

    /*
     * Check buttons and gravity and get new speed
     */

    /*
     * Move each car and start lap sound if one car starts a new lap
     */
    for (uint8_t i = 0; i < NUMBER_OF_CARS; ++i) {
        if (cars[i].computeSpeedAndDistance() == CAR_LAP_CONDITION) {
            sBeepEndMillis = millis() + 100;
            sBeepFrequency = 2000;
        }
    }

    /*
     * Check for winner. Blocking call, if one car is the winner
     */
    for (uint8_t i = 0; i < NUMBER_OF_CARS; ++i) {
        if (cars[i].checkForWinner(LAPS_PER_RACE)) {
            resetAllCars();
            sMode = MODE_WAIT;
#if defined(INFO) && defined(__AVR__)
            Serial.print(F("Min stack free[bytes]="));
            Serial.println(getStackFreeMinimumBytes());
#endif
            break;
        }
    }

    /*
     * Check for overtaking leader
     */
    for (uint8_t i = 0; i < NUMBER_OF_CARS; ++i) {
        if (cars[i].Distance > cars[sIndexOfLeadingCar].Distance) {
            // do not output first match
            if (cars[sIndexOfLeadingCar].Distance > 0) {
#ifdef INFO
                Serial.print(F("Overtaking leader car "));
                Serial.println(cars[sIndexOfLeadingCar].NumberOfThisCar);
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
    for (uint8_t i = 0; i < NUMBER_OF_CARS; ++i) {
        cars[i].draw();
    }

    /*
     * Show track
     */
    track.show(); // 9 Milliseconds for 300 Pixel
    timer0_millis += MILLIS_FOR_TRACK_TO_SHOW; // compensate millis() for the time interrupt was disabled for track.show().

    /*
     * Manage sound. Must check for situation after winner
     */
    if (sSoundEnabled && sMode != MODE_WAIT) {
        if (millis() < sBeepEndMillis) {
            tone(PIN_AUDIO, sBeepFrequency);
        } else {
            unsigned int tFrequency = cars[0].SpeedAsPixelPerLoop * 440 + cars[1].SpeedAsPixelPerLoop * 440;
            if (tFrequency > 100) {
                tone(PIN_AUDIO, tFrequency);
            } else {
                noTone(PIN_AUDIO);
            }
        }
#if defined(TCCR2A)
        // switch to direct toggle output at OC2A / pin 11 to enable direct hardware tone output
        TCCR2A |= _BV(COM2A0);
#endif
    }

    /*
     * Start each loop in 20 ms distance
     */
    while (millis() < sNextLoopMillis) {
        yield();
    }
    sNextLoopMillis += MILLISECONDS_PER_LOOP;
}
