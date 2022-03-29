/*
 * MPU6050IMUData.h
 *
 *  Functions for getting IMU data from MPU6050 for car control.
 *
 *  Created on: 19.11.2019
 *  Copyright (C) 2020  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
 *
 *  This file is part of PWMMotorControl https://github.com/ArminJo/PWMMotorControl.
 *
 *  PWMMotorControl is free software: you can redistribute it and/or modify
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

#ifndef _MPU6050_IMU_DATA_H
#define _MPU6050_IMU_DATA_H

#include <stdint.h>
#include "MPU6050Defines.h"
#include "LongUnion.h"


#define NUMBER_OF_ACCEL_VALUES      3
#define NUMBER_OF_GYRO_VALUES       3


#if defined(USE_ONLY_ACCEL_FLOATING_OFFSET)
#define USE_ACCEL_FLOATING_OFFSET
#endif

// not yet used for car data
class MPU6050IMUData {
public:
    uint8_t I2CAddress;
#if !defined(USE_ONLY_ACCEL_FLOATING_OFFSET)
    int16_t AcceleratorInitialOffset[NUMBER_OF_ACCEL_VALUES]; // stores the initial offsets for the 3 axes
    LongUnion Speeds[NUMBER_OF_ACCEL_VALUES];
#endif
    int16_t Accelerator[NUMBER_OF_ACCEL_VALUES]; // Values compensated with initial offset, +/-2 | 4 g for 16 bit full range
#if defined(USE_ACCEL_FLOATING_OFFSET)
    // The low pass value is in the HighWord, the LowWord holds the fraction
    LongUnion AcceleratorLowpassSubOneHertz[NUMBER_OF_ACCEL_VALUES]; // stores the low pass (0.6 Hz) values (-> floating offsets) for the 3 axes
#endif
    uint8_t LowPassShiftValue; // used to determine the AcceleratorLowpassSubOneHertz filter value

#if !defined(DO_NOT_USE_GYRO)
    int16_t GyroscopeOffset[NUMBER_OF_GYRO_VALUES];
    int16_t Gyro[NUMBER_OF_GYRO_VALUES];
    /*
     * 1000 samples / second
     * => 17 bit LSB per degree at +/-250 dps range
     * The upper word has a resolution of 1/2 degree at 1000 samples per second
     */
    LongUnion Rotations[NUMBER_OF_GYRO_VALUES];
#endif
    MPU6050IMUData();
    MPU6050IMUData(uint8_t aI2CAddress);

    /*
     * Initialization
     */
    void setI2CAddress(uint8_t aI2CAddress);
    bool initMPU6050(uint8_t aSampleRateDivider, mpu6050_bandwidth_t aLowPassIndex);
    void calculateAllOffsets();
    void printLP8Offsets(Print *aSerial);
    void printAllOffsets(Print *aSerial);

    /*
     * Read data with and without FIFO
     */
    void initMPU6050FifoForAccelAndGyro();
    void resetMPU6050Fifo();
    uint8_t readMPU6050Fifo();
    void readMPU6050Raw();

    /*
     * Low level functions
     */
    void MPU6050WriteByte(uint8_t aRegisterNumber, uint8_t aData);
    uint16_t MPU6050ReadWord(uint8_t aRegisterNumber);
    uint16_t MPU6050ReadWordSwapped(uint8_t aRegisterNumber);

    /*
     *
     */
    unsigned int computeAccelerationWithInitialOffset();
    unsigned int computeAccelerationWithFloatingOffset();

};

bool initWire();

#endif // _MPU6050_IMU_DATA_H
#pragma once
