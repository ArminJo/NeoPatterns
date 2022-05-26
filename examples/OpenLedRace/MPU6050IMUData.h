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

#if defined(USE_ONLY_ACCEL_FLOATING_OFFSET)
#define USE_ACCEL_FLOATING_OFFSET
#endif

class MPU6050IMUData {
public:
    uint8_t I2CAddress; // For USE_SOFT_I2C_MASTER this is the full 8 byte address with LSB == 0
#if !defined(USE_ONLY_ACCEL_FLOATING_OFFSET)
    int16_t AcceleratorOffset[NUMBER_OF_ACCEL_VALUES]; // stores the initial offsets for the 3 axes
    LongUnion Speed[NUMBER_OF_ACCEL_VALUES];
    bool OffsetsJustHaveChanged;   // To be used for print as flag, that printing once has happened
#endif
    int16_t Accelerator[NUMBER_OF_ACCEL_VALUES]; // Values compensated with initial offset, +/-2 | 4 g for 16 bit full range
#if defined(USE_ACCEL_FLOATING_OFFSET)
    // The low pass value is in the HighWord, the LowWord holds the fraction
    LongUnion AcceleratorLowpassSubOneHertz[NUMBER_OF_ACCEL_VALUES]; // stores the low pass (0.6 Hz) values (-> floating offsets) for the 3 axes
    uint8_t LowPassShiftValue; // Computed at init dependent of the sample frequency, to have a fixed AcceleratorLowpassSubOneHertz filter value
#endif

#if !defined(DO_NOT_USE_GYRO)
    int16_t GyroscopeOffset[NUMBER_OF_GYRO_VALUES];
    int16_t Gyro[NUMBER_OF_GYRO_VALUES];
    /*
     * 1000 samples / second
     * => 17 bit LSB per degree at +/-250 dps range
     * The upper word has a resolution of 1/2 degree at 1000 samples per second
     * The upper word has a resolution of 1 degree at 500 samples per second
     */
    LongUnion Rotation[NUMBER_OF_GYRO_VALUES];
#endif

    int16_t CountOfFifoChunksForOffset = 0; // signed, since it is used in formulas with other signed values

    MPU6050IMUData();
    MPU6050IMUData(uint8_t aI2CAddress);

    /*
     * Initialization
     */
    void setI2CAddress(uint8_t aI2CAddress);
    bool initMPU6050(uint8_t aSampleRateDivider, mpu6050_bandwidth_t aLowPassIndex);
    bool initMPU6050AndCalculateAllOffsetsAndWait(uint8_t aSampleRateDivider, mpu6050_bandwidth_t aLowPassType);
    void printLP8Offsets(Print *aSerial);
    void printAllOffsets(Print *aSerial);

    /*
     * Read data with and without FIFO
     */
    void initMPU6050Fifo();
    void resetMPU6050Fifo();
    void resetOffset();
    uint8_t readDataFromMPU6050Fifo();
    void readDataFromMPU6050();

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
