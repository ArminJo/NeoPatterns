/*
 * CarIMUData.h
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/gpl.html>.
 */

#ifndef CAR_IMU_DATA_H_
#define CAR_IMU_DATA_H_

#include <stdint.h>
#include "MPU6050Defines.h"
#include "LongUnion.h"

//#define DO_NOT_USE_GYRO
//#define USE_ACCEL_FLOATING_OFFSET

//#define USE_SOFT_WIRE     // saves 1700 bytes FLASH + 150 bytes RAM compared with Wire
#define USE_SOFT_I2C_MASTER // saves additional 410 bytes FLASH and 50 bytes RAM compared with USE_SOFT_WIRE
#if defined(USE_SOFT_I2C_MASTER) || defined(USE_SOFT_WIRE)
#define SCL_PIN 5
#define SCL_PORT PORTC
#define SDA_PIN 4
#define SDA_PORT PORTC
#define I2C_HARDWARE 1 // use I2C Hardware
#define I2C_PULLUP 1
//#define I2C_TIMEOUT 5000 // costs 350 bytes
#define I2C_FASTMODE 1
#  if defined(USE_SOFT_WIRE)
#include "SoftWire.h"
SoftWire Wire = SoftWire();
#undef USE_SOFT_I2C_MASTER
#  else
#include "SoftI2CMaster.h"
#  endif
#else
#include "Wire.h"
#endif

#define ACCEL_RAW_TO_G_FOR_2G_RANGE (4.0/65536.0)
#define ACCEL_RAW_VALUE_OF_ONE_G_FOR_2G_RANGE (65536 / 4)
#define GYRO_RAW_TO_DEGREE_PER_SECOND_FOR_250DPS_RANGE (500.0/65536.0) // 0.00762939 or 1/131.072

#if !defined(NUMBER_OF_OFFSET_CALIBRATION_SAMPLES)
#define NUMBER_OF_OFFSET_CALIBRATION_SAMPLES 512
#endif

#define NUMBER_OF_ACCEL_VALUES      3
#define NUMBER_OF_GYRO_VALUES       3

#if !defined(DO_NOT_USE_GYRO)
#define FIFO_CHUNK_SIZE             ((NUMBER_OF_ACCEL_VALUES + NUMBER_OF_GYRO_VALUES) * 2) // size of one complete fifo dataset
#else
#define FIFO_CHUNK_SIZE             ((NUMBER_OF_ACCEL_VALUES) * 2) // size of one complete fifo dataset
#endif

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
    LongUnion AcceleratorLP8[NUMBER_OF_ACCEL_VALUES]; // stores the low pass values (-> floating offsets) for the !offset compensated! 3 axes
#endif

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
    void initMPU6050();
    void calculateAllOffsets();
    void printLP8Offsets(Print *aSerial);
    void printAllOffsets(Print *aSerial);

    /*
     * Read data with and without FIFO
     */
    void initMPU6050FifoForAccelAndGyro();
    void resetMPU6050Fifo();
    uint8_t readFromMPU6050Fifo();
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

#endif /* CAR_IMU_DATA_H_ */

#pragma once

