/*
 * MPU6050IMUData.hpp
 *
 *  Functions for getting IMU data from MPU6050.
 *  Accelerator data are always fetched, gyro data only if DO_NOT_USE_GYRO is NOT activated
 *
 *  Created on: 19.11.2020
 *  Copyright (C) 2020-2022  Armin Joachimsmeyer
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
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/gpl.html>.
 */
#ifndef _MPU6050_IMU_DATA_HPP
#define _MPU6050_IMU_DATA_HPP

//#define DEBUG // Only for development

//#define DO_NOT_USE_GYRO           // Do not read any gyroscope parameter from IMU
//#define USE_ACCEL_FLOATING_OFFSET // Compute a low pass value (floating offset) for each accelerator axis which enables the computeAccelerationWithFloatingOffset() function.

/*
 * Do not compensate the accelerator values with the initial offset.
 * Saves space and time if ONLY computeAccelerationWithFloatingOffset() function is used.
 * Function computeAccelerationWithInitialOffset() will not work as expected!
 */
//#define USE_ONLY_ACCEL_FLOATING_OFFSET
#define ACCEL_RAW_TO_G_FOR_2G_RANGE (4.0/65536.0)
#define ACCEL_RAW_VALUE_OF_ONE_G_FOR_2G_RANGE (65536 / 4)
#define GYRO_RAW_TO_DEGREE_PER_SECOND_FOR_250DPS_RANGE (500.0/65536.0) // 0.00762939 or 1/131.072

#if !defined(NUMBER_OF_OFFSET_CALIBRATION_SAMPLES)
#define NUMBER_OF_OFFSET_CALIBRATION_SAMPLES 512
#endif

#if defined(DO_NOT_USE_GYRO)
#define FIFO_CHUNK_SIZE             ((NUMBER_OF_ACCEL_VALUES) * 2) // size of one complete fifo dataset
#else
#define _USE_GYRO   // do avoid double negations
#define FIFO_CHUNK_SIZE             ((NUMBER_OF_ACCEL_VALUES + NUMBER_OF_GYRO_VALUES) * 2) // size of one complete fifo dataset
#endif

//#define USE_SOFT_I2C_MASTER // Saves 2110 bytes program memory and 200 bytes RAM compared with Arduino Wire
//#define USE_SOFT_WIRE // Saves 1700 bytes program memory and 200 bytes RAM compared with with Arduino Wire
#if !defined(USE_SOFT_I2C_MASTER) && __has_include("SoftI2CMasterConfig.h")
#define USE_SOFT_I2C_MASTER
#endif
#if defined(USE_SOFT_I2C_MASTER)
#include "SoftI2CMasterConfig.h"    // Include configuration for sources
#  if !defined(_SOFTI2C_H)
#include "SoftI2CMaster.h"          // include sources
#  endif
#elif defined(USE_SOFT_WIRE)
#define USE_SOFTWIRE_H_AS_PLAIN_INCLUDE
#include "SoftWire.h" // just for tests :-)
#else
#include "Wire.h"
#endif // defined(USE_SOFT_I2C_MASTER) || defined(USE_SOFT_WIRE)

#include "MPU6050IMUData.h"

MPU6050IMUData::MPU6050IMUData() { // @suppress("Class members should be properly initialized")
#if defined(USE_SOFT_I2C_MASTER)
    I2CAddress = MPU6050_DEFAULT_ADDRESS << 1;
#else
    I2CAddress = MPU6050_DEFAULT_ADDRESS;
#endif
}

MPU6050IMUData::MPU6050IMUData(uint8_t aI2CAddress) { // @suppress("Class members should be properly initialized")
#if defined(USE_SOFT_I2C_MASTER)
    I2CAddress = aI2CAddress << 1;
#else
    I2CAddress = aI2CAddress;
#endif
}

void MPU6050IMUData::setI2CAddress(uint8_t aI2CAddress) { // @suppress("Class members should be properly initialized")
#if defined(USE_SOFT_I2C_MASTER)
    I2CAddress = aI2CAddress << 1;
#else
    I2CAddress = aI2CAddress;
#endif
}

/*
 * I2C fast mode is supported by MPU6050
 * @return false if initialization is not OK
 */
bool initWire() {
#if defined(USE_SOFT_I2C_MASTER)
    return i2c_init(); // Initialize everything and check for bus lockup
#else
    Wire.begin();
    Wire.setClock(400000);
    Wire.setTimeout(5000);
    return true;
#endif
}

/*
 * Reset all values, especially Speed[] and Rotation[] for a new offset computation
 */
void MPU6050IMUData::resetOffset() {
#if !defined(USE_ONLY_ACCEL_FLOATING_OFFSET) || defined(_USE_GYRO)
    for (uint_fast8_t i = 0; i < NUMBER_OF_ACCEL_VALUES; i++) {
#  if !defined(USE_ONLY_ACCEL_FLOATING_OFFSET)
        Speed[i].ULong = 0;
        OffsetsJustHaveChanged = false;
#  endif
#  if defined(_USE_GYRO)
        Rotation[i].ULong = 0;
#  endif
    }
#endif
    CountOfFifoChunksForOffset = 0;
}

/*
 * Resets all variables, initializes the MPU, but not the FIFO and sets LowPassShiftValue
 * @param aSampleRateDivider 1 to 256. Divider of the 1 kHz clock used for FiFo
 * @param aLowPassIndex one of: MPU6050_BAND_260_HZ (LP disabled) MPU6050_BAND_184_HZ (184 Hz), 94, 44, 21, 10 to MPU6050_BAND_5_HZ
 * @return false if i2c_start() was not successful / MPU6050 not attached
 */
bool MPU6050IMUData::initMPU6050(uint8_t aSampleRateDivider, mpu6050_bandwidth_t aLowPassType) {

#if !defined(USE_ONLY_ACCEL_FLOATING_OFFSET) || defined(_USE_GYRO)
    for (uint_fast8_t i = 0; i < NUMBER_OF_ACCEL_VALUES; i++) {
#  if defined(USE_ONLY_ACCEL_FLOATING_OFFSET)
        AcceleratorLowpassSubOneHertz[i].Long = 0;
#  else
        Accelerator[i] = 0;
        Speed[i].ULong = 0;
#  endif
#  if defined(_USE_GYRO)
        Gyro[i] = 0;
        Rotation[i].ULong = 0;
#  endif
    }
#endif
#if !defined(USE_ONLY_ACCEL_FLOATING_OFFSET)
    OffsetsJustHaveChanged = false; // To be used for print as flag, that printing once has happened
#endif
    CountOfFifoChunksForOffset = 0;

#if defined(USE_SOFT_I2C_MASTER)
    /*
     * Check if MPU6050 is attached
     */
    if (!i2c_start(I2CAddress)) {
        i2c_stop();
        return false;
    }
    i2c_stop();
#elif defined(USE_SOFT_WIRE)
#warning SoftWire does not support dynamically check of connection because it has no setWireTimeout() function. You should use "#define USE_SOFT_I2C_MASTER" instead.
#else
    Wire.setWireTimeout(); // Sets default timeout of 25 ms.
    Wire.beginTransmission(I2CAddress);
    if (Wire.endTransmission(true) != 0) {
        return false;
    }
#endif
    MPU6050WriteByte(MPU6050_RA_PWR_MGMT_1, MPU6050_CLOCK_PLL_ZGYRO); // use recommended gyro reference: PLL with Z axis gyroscope reference
    MPU6050WriteByte(MPU6050_RA_SMPLRT_DIV, aSampleRateDivider - 1); // parameter 0 => divider 1, 19 -> divider 20
    MPU6050WriteByte(MPU6050_RA_CONFIG, aLowPassType); // ext input disabled, DLPF enabled

#if defined(USE_ACCEL_FLOATING_OFFSET)
    uint_fast8_t tLowPassShiftValue; // 8 for aSampleRateDivider = 1 equivalent to 1 kHz gives 0.6 Hz cutoff frequency
#  if defined(DEBUG)
    uint8_t tSampleRateDivider = aSampleRateDivider;
#  endif
    for (tLowPassShiftValue = 8; tLowPassShiftValue > 0; --tLowPassShiftValue) {
        aSampleRateDivider = aSampleRateDivider >> 1;
        if (aSampleRateDivider == 0) {
            break;
        }
    }
    LowPassShiftValue = tLowPassShiftValue;
#  if defined(DEBUG)
    Serial.print(F("LowPassShiftValue for sample rate "));
    Serial.print(tSampleRateDivider);
    Serial.print(F(" ms = "));
    Serial.println(LowPassShiftValue);
    Serial.flush();
#  endif
#endif

// range select
    // the next is default and never changed by us
//    MPU6050WriteByte(MPU6050_RA_ACCEL_CONFIG,
//    MPU6050_ACCEL_FS_2 << (MPU6050_ACONFIG_AFS_SEL_BIT - MPU6050_ACONFIG_AFS_SEL_LENGTH + 1)); // range +/- 2 g - default
#if defined(_USE_GYRO)
    // the next is default and never changed by us
//    MPU6050WriteByte(MPU6050_RA_GYRO_CONFIG,
//    MPU6050_GYRO_FS_250 << (MPU6050_GCONFIG_FS_SEL_BIT - MPU6050_GCONFIG_FS_SEL_LENGTH + 1)); // range +/- 250 deg/s - default
#endif
    return true;
}

void MPU6050IMUData::resetMPU6050Fifo() {
    MPU6050WriteByte(MPU6050_RA_USER_CTRL, _BV(MPU6050_USERCTRL_FIFO_RESET_BIT)); // Reset FIFO
    MPU6050WriteByte(MPU6050_RA_USER_CTRL, _BV(MPU6050_USERCTRL_FIFO_EN_BIT)); // enable FIFO
}

void MPU6050IMUData::initMPU6050Fifo() {
#if defined(_USE_GYRO)
    MPU6050WriteByte(MPU6050_RA_FIFO_EN,
    _BV(MPU6050_ACCEL_FIFO_EN_BIT) | _BV(MPU6050_XG_FIFO_EN_BIT) | _BV(MPU6050_YG_FIFO_EN_BIT) | _BV(MPU6050_ZG_FIFO_EN_BIT)); // FIFO: all Accel + Gyro axes
#else
        MPU6050WriteByte(MPU6050_RA_FIFO_EN, _BV(MPU6050_ACCEL_FIFO_EN_BIT)); // FIFO: only Accel axes
#endif
    resetMPU6050Fifo();
}

/**
 * Sets Accelerator, Speeds, Gyro and Rotations arrays
 *
 * 400 kHz I2C fast mode timings:
 * 480 us reading per chunk (only accelerator values)
 * 4.8 ms for 10 chunks a 12 bytes (only accelerator values) -> 40 us / byte
 * @return Number of chunks/data sets read from FIFO
 */
uint8_t MPU6050IMUData::readDataFromMPU6050Fifo() {
    // Get FIFO count
    uint16_t tFifoCount = MPU6050ReadWordSwapped(MPU6050_RA_FIFO_COUNTH);
#if defined(DEBUG)
    Serial.print(F("FiFoCnt="));
    Serial.print(tFifoCount);
    Serial.print(" ");
#endif
#if defined(_USE_GYRO)
    int32_t tGyroscope[NUMBER_OF_GYRO_VALUES] = { 0, 0, 0 };
#endif
    if (tFifoCount > 0x300) {
        // Fifo is over 3/4 full -> reset it
        resetMPU6050Fifo();
#if defined(DEBUG)
        Serial.println(F("Reset Fifo"));
#endif
        return 0;
    }
    uint8_t tNumberOfChunks = tFifoCount / FIFO_CHUNK_SIZE;
    if (tNumberOfChunks > 0) {
#if defined(USE_SOFT_I2C_MASTER)
        // Here we have no buffer and can read all chunks in one row
        i2c_start(I2CAddress);
        i2c_write(MPU6050_RA_FIFO_R_W);
        i2c_rep_start((I2CAddress) | I2C_READ); // restart for reading
#endif
        for (uint_fast8_t tChunckCount = 0; tChunckCount < tNumberOfChunks; tChunckCount++) {

#if !defined(USE_SOFT_I2C_MASTER)
            Wire.beginTransmission(I2CAddress);
            Wire.write(MPU6050_RA_FIFO_R_W);
            Wire.endTransmission(false);
            // read chunk by chunk
            Wire.requestFrom(I2CAddress, (uint8_t) (FIFO_CHUNK_SIZE), (uint8_t) true);
#endif

            // we must read all 3 accelerator values
            WordUnion tValue;
            for (uint_fast8_t i = 0; i < NUMBER_OF_ACCEL_VALUES; i++) {
                // read into intermediate variable
#if defined(USE_SOFT_I2C_MASTER)
                tValue.Byte.HighByte = i2c_read(false);
#  if defined(_USE_GYRO)
                // we read gyro values below, so no i2c_read(true) here
                tValue.Byte.LowByte = i2c_read(false);
#  else
                // read the last one with i2c_read(true)
                tValue.Byte.LowByte = i2c_read((i == (NUMBER_OF_ACCEL_VALUES - 1) && tChunckCount == (tNumberOfChunks - 1)));
#  endif
#else
                tValue.Byte.HighByte = Wire.read();
                tValue.Byte.LowByte = Wire.read();
#endif
#if !defined(USE_ONLY_ACCEL_FLOATING_OFFSET)
                // avoid over or underflow - requires 60 bytes
                int32_t tTemp = (int32_t) tValue.Word - AcceleratorOffset[i];
                if (tTemp > __INT16_MAX__) {
                    tValue.Word = __INT16_MAX__;
                } else if (tTemp < (-__INT16_MAX__) - 1) {
                    tValue.Word = (-__INT16_MAX__ - 1);
                } else {
                    tValue.Word = tTemp;
                }

                // compute speeds
                Speed[i].Long += tValue.Word;
#endif
                Accelerator[i] = tValue.Word;
#if defined(USE_ACCEL_FLOATING_OFFSET)
                if ( CountOfFifoChunksForOffset == 1){
                    // Take the second value (a guess) as initial value
                    AcceleratorLowpassSubOneHertz[i].Word.HighWord = tValue.Word;
                } else {
                    // Do not compensate the accelerator values with the initial offset.
                    AcceleratorLowpassSubOneHertz[i].Long += (((int32_t) tValue.Word << 16) - AcceleratorLowpassSubOneHertz[i].Long)
                            >> LowPassShiftValue;
                }
#endif
            }

#if defined(_USE_GYRO)
            for (uint_fast8_t i = 0; i < NUMBER_OF_GYRO_VALUES; i++) {
                // read into intermediate variable
#  if defined(USE_SOFT_I2C_MASTER)
                tValue.Byte.HighByte = i2c_read(false);
                tValue.Byte.LowByte = i2c_read((i == (NUMBER_OF_GYRO_VALUES - 1) && tChunckCount == (tNumberOfChunks - 1)));
#  else
                tValue.Byte.HighByte = Wire.read();
                tValue.Byte.LowByte = Wire.read();
#  endif
                tValue.Word = tValue.Word - GyroscopeOffset[i];
                tGyroscope[i] += tValue.Word;
                // Integrate to rotation
                Rotation[i].Long += tValue.Word;
            }
#endif

        } // for (uint_fast8_t tChunckCount = 0
#if defined(USE_SOFT_I2C_MASTER)
        i2c_stop();
#endif
#if defined(_USE_GYRO)
        /*
         * If we have more than one chunk, use the average
         */
        for (uint_fast8_t i = 0; i < NUMBER_OF_GYRO_VALUES; i++) {
            Gyro[i] = tGyroscope[i] / tNumberOfChunks;
        }
#endif

        CountOfFifoChunksForOffset += tNumberOfChunks;

#if !defined(USE_ONLY_ACCEL_FLOATING_OFFSET)
        /*
         * Automatically get initial offset values once at the beginning
         */
        if (AcceleratorOffset[0] == 0 && CountOfFifoChunksForOffset >= NUMBER_OF_OFFSET_CALIBRATION_SAMPLES) {
            OffsetsJustHaveChanged = true;

            /*
             * Take the first NUMBER_OF_OFFSET_CALIBRATION_SAMPLES values for offset
             */
            for (uint_fast8_t i = 0; i < NUMBER_OF_ACCEL_VALUES; i++) {

                AcceleratorOffset[i] = Speed[i].Long / CountOfFifoChunksForOffset; // speed is the accelerator sum :-)
#  if defined(USE_ACCEL_FLOATING_OFFSET)
                AcceleratorLowpassSubOneHertz[i].Long = 0;
#  endif
#  if defined(_USE_GYRO)
                GyroscopeOffset[i] = Rotation[i].Long / CountOfFifoChunksForOffset;
#  endif
            }
#  if defined(DEBUG)
            Serial.println();
            printAllOffsets(&Serial);
#  endif
        }
#endif // !defined(USE_ONLY_ACCEL_FLOATING_OFFSET)
    } // if (tNumberOfChunks > 0)
    return tNumberOfChunks;
}

/*
 * Sets Accelerator, Speeds, Gyro and Rotations arrays
 *
 * Read data direct from registers
 * 500 us reading
 */
void MPU6050IMUData::readDataFromMPU6050() {
//    NumberOfLoopSamples++;
    /*
     * Get data
     */
#if defined(USE_SOFT_I2C_MASTER)
    // Here we have no buffer and can read all chunks in one row
    i2c_start(I2CAddress);
    i2c_write(MPU6050_RA_ACCEL_XOUT_H);
    i2c_rep_start((I2CAddress) | I2C_READ); // restart for reading
#else
    Wire.beginTransmission(I2CAddress);
    Wire.write(MPU6050_RA_ACCEL_XOUT_H);
    Wire.endTransmission(false);
#  if defined(_USE_GYRO)
    // +2 since we read all data in a row and must read/skip temperature data
    Wire.requestFrom(I2CAddress, (uint8_t) (FIFO_CHUNK_SIZE + 2), (uint8_t) true);
#  else
        Wire.requestFrom(I2CAddress, (uint8_t) (FIFO_CHUNK_SIZE), (uint8_t) true);
#  endif
#endif

// [ax,ay,az,temp,gx,gy,gz]
// Accelerator
    WordUnion tValue;
    for (uint_fast8_t i = 0; i < NUMBER_OF_ACCEL_VALUES; i++) {
        // read into intermediate variable
#if defined(USE_SOFT_I2C_MASTER)
        tValue.Byte.HighByte = i2c_read(false);
#  if !defined(DO_NOT_USE_GYRO)
        tValue.Byte.LowByte = i2c_read(false);
#  else
            tValue.Byte.LowByte = i2c_read(i == (NUMBER_OF_ACCEL_VALUES - 1) );
#  endif
#else
        tValue.Byte.HighByte = Wire.read();
        tValue.Byte.LowByte = Wire.read();
#endif
#if !defined(USE_ONLY_ACCEL_FLOATING_OFFSET)
        // avoid over or underflow - requires 60 bytes
        int32_t tTemp = (int32_t) tValue.Word - AcceleratorOffset[i];
        if (tTemp > __INT16_MAX__) {
            tValue.Word = __INT16_MAX__;
        } else if (tTemp < (-__INT16_MAX__) - 1) {
            tValue.Word = (-__INT16_MAX__ - 1);
        } else {
            tValue.Word = tTemp;
        }
        // compute speeds
        Speed[i].Long += tValue.Word;
#endif
        Accelerator[i] = tValue.Word;
#if defined(USE_ACCEL_FLOATING_OFFSET)
        if ( CountOfFifoChunksForOffset == 1){
            // Take the second value (a guess) as initial value
            AcceleratorLowpassSubOneHertz[i].Word.HighWord = tValue.Word;
        } else {
            // Do not compensate the accelerator values with the initial offset.
            AcceleratorLowpassSubOneHertz[i].Long += (((int32_t) tValue.Word << 16) - AcceleratorLowpassSubOneHertz[i].Long)
                    >> LowPassShiftValue;
        }
#endif
    }

#if defined(_USE_GYRO)
// Skip temperature
#if defined(USE_SOFT_I2C_MASTER)
    i2c_read(false);
    i2c_read(false);
#else
    Wire.read();
    Wire.read();
#endif

// 30 us read gyroscope data from wire buffer and process
    for (uint_fast8_t i = 0; i < NUMBER_OF_GYRO_VALUES; i++) {
        // read into intermediate variable
#  if defined(USE_SOFT_I2C_MASTER)
        tValue.Byte.HighByte = i2c_read(false);
        tValue.Byte.LowByte = i2c_read(i == (NUMBER_OF_GYRO_VALUES - 1));
#  else
        tValue.Byte.HighByte = Wire.read();
        tValue.Byte.LowByte = Wire.read();
#  endif
        tValue.Word = tValue.Word - GyroscopeOffset[i];
        Gyro[i] = tValue.Word;
        // Compute rotations
        Rotation[i].Long += tValue.Word;
    }
#endif
#if defined(USE_SOFT_I2C_MASTER)
    i2c_stop();
#endif
}

/*
 * This sets the sample rate to 1 ms and the filter to 184 Hz
 */
bool MPU6050IMUData::initMPU6050AndCalculateAllOffsetsAndWait(uint8_t aSampleRateDivider, mpu6050_bandwidth_t aLowPassType) {
    /*
     * Initially set the sample rate to 1 ms and the filter to 184 Hz
     */
    if (!initMPU6050(1, MPU6050_BAND_184_HZ)) {
        return false;
    }
    initMPU6050Fifo();

#if defined(USE_ONLY_ACCEL_FLOATING_OFFSET)
    // Read 256 times
    while (CountOfFifoChunksForOffset < NUMBER_OF_OFFSET_CALIBRATION_SAMPLES) {
        delay(1);
        readDataFromMPU6050Fifo();
    }
#else
    //Read until offset was filled
    while (!OffsetsJustHaveChanged) {
        delay(1);
        readDataFromMPU6050Fifo();
    }
#endif
    /*
     * Finally set the desired values
     */
    initMPU6050(aSampleRateDivider, aLowPassType);
    return true;

//    uint32_t LastDataMillis;
//
//    int32_t tSumAccel[NUMBER_OF_ACCEL_VALUES] = { 0, 0, 0 };
//#if defined(_USE_GYRO)
//    int32_t tSumGyro[NUMBER_OF_GYRO_VALUES] = { 0, 0, 0 };
//#endif
//
//    for (unsigned int j = 0; j < NUMBER_OF_OFFSET_CALIBRATION_SAMPLES; j++) {
//
//        // get data every ms
//        while (millis() == LastDataMillis) {
//            ;
//        }
//        LastDataMillis = millis();
//
//#if defined(USE_SOFT_I2C_MASTER)
//            i2c_start(I2CAddress << 1);
//            i2c_write(MPU6050_RA_ACCEL_XOUT_H);
//            i2c_rep_start((I2CAddress << 1) | I2C_READ); // restart for reading
//#else
//        Wire.beginTransmission(I2CAddress);
//        Wire.write(MPU6050_RA_ACCEL_XOUT_H);
//        Wire.endTransmission(false);
//#  if defined(_USE_GYRO)
//        Wire.requestFrom(I2CAddress, (uint8_t) (FIFO_CHUNK_SIZE + 2), (uint8_t) true);
//#  else
//            Wire.requestFrom(I2CAddress, (uint8_t) (FIFO_CHUNK_SIZE), (uint8_t) true);
//#  endif
//#endif
//
//        WordUnion tValue;
//        // Acceleration
//        for (uint_fast8_t i = 0; i < NUMBER_OF_ACCEL_VALUES; i++) {
//#if defined(USE_SOFT_I2C_MASTER)
//                tValue.Byte.HighByte = i2c_read(false);
//#  if !defined(DO_NOT_USE_GYRO)
//                tValue.Byte.LowByte = i2c_read(false);
//#  else
//                tValue.Byte.LowByte = i2c_read(i == (NUMBER_OF_ACCEL_VALUES - 1) );
//#  endif
//#else
//            tValue.Byte.HighByte = Wire.read();
//            tValue.Byte.LowByte = Wire.read();
//#endif
//            tSumAccel[i] += tValue.Word;
//        }
//#if defined(_USE_GYRO)
//        // Temperature
//#if defined(USE_SOFT_I2C_MASTER)
//            i2c_read(false);
//            i2c_read(false);
//#else
//        Wire.read();
//        Wire.read();
//#endif
//
//        // Gyro
//        for (uint_fast8_t i = 0; i < NUMBER_OF_GYRO_VALUES; i++) {
//#if defined(USE_SOFT_I2C_MASTER)
//                tValue.Byte.HighByte = i2c_read(false);
//                tValue.Byte.LowByte = i2c_read(i == (NUMBER_OF_GYRO_VALUES - 1));
//#else
//            tValue.Byte.HighByte = Wire.read();
//            tValue.Byte.LowByte = Wire.read();
//#endif
//            tSumGyro[i] += tValue.Word;
//        }
//#endif
//#if defined(USE_SOFT_I2C_MASTER)
//            i2c_stop();
//#endif
//    }
//
//    for (uint_fast8_t i = 0; i < NUMBER_OF_ACCEL_VALUES; i++) {
//        Accelerator[i] = 0; // initialize array, since first fifo readout may have no data
//
//#if defined(USE_ONLY_ACCEL_FLOATING_OFFSET)
//            // initialize the lowpass with initial offsets since we have no initial offset correction for raw values any more
//            AcceleratorLowpassSubOneHertz[i].Long = (tSumAccel[i] / NUMBER_OF_OFFSET_CALIBRATION_SAMPLES) << 16;
//#else
//        AcceleratorOffset[i] = tSumAccel[i] / NUMBER_OF_OFFSET_CALIBRATION_SAMPLES;
//#  if defined(USE_ACCEL_FLOATING_OFFSET)
//            AcceleratorLowpassSubOneHertz[i].Long = 0;
//#  endif
//#endif
//#if defined(_USE_GYRO)
//        GyroscopeOffset[i] = tSumGyro[i] / NUMBER_OF_OFFSET_CALIBRATION_SAMPLES;
//#endif
//    }

}

void MPU6050IMUData::printLP8Offsets(Print *aSerial) {
#if defined(USE_ACCEL_FLOATING_OFFSET)
        aSerial->print(F("Acc low pass 8 values/offsets: X="));
        aSerial->print(AcceleratorLowpassSubOneHertz[0].Word.HighWord);
        aSerial->print('|');
        aSerial->print(AcceleratorLowpassSubOneHertz[0].Word.HighWord * ACCEL_RAW_TO_G_FOR_2G_RANGE);
        aSerial->print(F("g Y="));
        aSerial->print(AcceleratorLowpassSubOneHertz[1].Word.HighWord);
        aSerial->print('|');
        aSerial->print(AcceleratorLowpassSubOneHertz[1].Word.HighWord * ACCEL_RAW_TO_G_FOR_2G_RANGE);
        aSerial->print(F("g Z="));
        aSerial->print(AcceleratorLowpassSubOneHertz[2].Word.HighWord);
        aSerial->print('|');
        aSerial->print(AcceleratorLowpassSubOneHertz[2].Word.HighWord * ACCEL_RAW_TO_G_FOR_2G_RANGE);
        aSerial->println('g');
#else
    (void) aSerial;
#endif
}

void MPU6050IMUData::printAllOffsets(Print *aSerial) {
#if defined(USE_ACCEL_FLOATING_OFFSET)
        printLP8Offsets(aSerial);
#endif
#if !defined(USE_ONLY_ACCEL_FLOATING_OFFSET)
    aSerial->print(F("Acc offsets: X="));
    aSerial->print(AcceleratorOffset[0]);
    aSerial->print('|');
    aSerial->print(AcceleratorOffset[0] * ACCEL_RAW_TO_G_FOR_2G_RANGE);
    aSerial->print(F("g Y="));
    aSerial->print(AcceleratorOffset[1]);
    aSerial->print('|');
    aSerial->print(AcceleratorOffset[1] * ACCEL_RAW_TO_G_FOR_2G_RANGE);
    aSerial->print(F("g Z="));
    aSerial->print(AcceleratorOffset[2]);
    aSerial->print('|');
    aSerial->print(AcceleratorOffset[2] * ACCEL_RAW_TO_G_FOR_2G_RANGE);
    aSerial->println('g');
#endif

#if defined(_USE_GYRO)
    aSerial->print(F("Gyro offsets: X="));
    aSerial->print(GyroscopeOffset[0]);
    aSerial->print('|');
    aSerial->print(GyroscopeOffset[0] * GYRO_RAW_TO_DEGREE_PER_SECOND_FOR_250DPS_RANGE);
    aSerial->print(F("dps Y="));
    aSerial->print(GyroscopeOffset[1]);
    aSerial->print('|');
    aSerial->print(GyroscopeOffset[1] * GYRO_RAW_TO_DEGREE_PER_SECOND_FOR_250DPS_RANGE);
    aSerial->print(F("dps Z="));
    aSerial->print(GyroscopeOffset[2]);
    aSerial->print('|');
    aSerial->print(GyroscopeOffset[2] * GYRO_RAW_TO_DEGREE_PER_SECOND_FOR_250DPS_RANGE);
    aSerial->println(F("dps"));
#endif
}

/**
 * Compute geometric sum of all 3 offset corrected accelerator values
 * 4 g for 16 bit full range
 */
unsigned int MPU6050IMUData::computeAccelerationWithInitialOffset() {
    unsigned long tAcceleration = 0;
    for (uint_fast8_t i = 0; i < NUMBER_OF_ACCEL_VALUES; i++) {
        tAcceleration += (unsigned long) Accelerator[i] * Accelerator[i];
    }
    return sqrt(tAcceleration);
}

/**
 * Compute geometric sum of all 3 offset corrected accelerator values, but use AcceleratorLowpassSubOneHertz (low pass 2^8) as offset
 * AcceleratorLP5 is too fast and dampens the signal too much at 1 kHz
 * 4 g for 16 bit full range
 */
unsigned int MPU6050IMUData::computeAccelerationWithFloatingOffset() {
#if defined(USE_ACCEL_FLOATING_OFFSET)
        unsigned long tAcceleration = 0;
        for (uint_fast8_t i = 0; i < NUMBER_OF_ACCEL_VALUES; i++) {
            int16_t tAccelerationOffsetCompensated = Accelerator[i] - (AcceleratorLowpassSubOneHertz[i].Word.HighWord);
            tAcceleration += (unsigned long) tAccelerationOffsetCompensated * tAccelerationOffsetCompensated;
        }
        return sqrt(tAcceleration);
#else
// fallback
    return computeAccelerationWithInitialOffset();
#endif
}

void MPU6050IMUData::MPU6050WriteByte(uint8_t aRegisterNumber, uint8_t aData) {

#if defined(USE_SOFT_I2C_MASTER)
    i2c_write_byte_to_register((I2CAddress), aRegisterNumber, aData);
#else
    Wire.beginTransmission(I2CAddress);
    Wire.write(aRegisterNumber);
    Wire.write(aData);
    Wire.endTransmission();
#endif
}

/*
 * Read high byte first
 */
uint16_t MPU6050IMUData::MPU6050ReadWordSwapped(uint8_t aRegisterNumber) {

#if defined(USE_SOFT_I2C_MASTER)
    return i2c_read_word_swapped_from_register((I2CAddress), aRegisterNumber);
#else
    WordUnion tWord;

    Wire.beginTransmission(I2CAddress);
    Wire.write(aRegisterNumber);
    Wire.endTransmission(false);
    Wire.requestFrom((uint8_t) I2CAddress, (uint8_t) 2, (uint8_t) true);
    tWord.UByte.HighByte = Wire.read();
    tWord.UByte.LowByte = Wire.read();
    return tWord.UWord;
#endif
}

uint16_t MPU6050IMUData::MPU6050ReadWord(uint8_t aRegisterNumber) {

#if defined(USE_SOFT_I2C_MASTER)
    return i2c_read_word_from_register((I2CAddress), aRegisterNumber);
#else
    WordUnion tWord;

    Wire.beginTransmission(I2CAddress);
    Wire.write(aRegisterNumber);
    Wire.endTransmission(false);
    Wire.requestFrom((uint8_t) I2CAddress, (uint8_t) 2, (uint8_t) true);
    tWord.UByte.LowByte = Wire.read();
    tWord.UByte.HighByte = Wire.read();
    return tWord.UWord;
#endif
}

#endif // _MPU6050_IMU_DATA_HPP
