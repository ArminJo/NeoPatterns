/*
 * MPU6050IMUData.hpp
 *
 *  Functions for getting IMU data from MPU6050.
 *  Accelerator data are always fetched, gyro data only if DO_NOT_USE_GYRO is NOT activated
 *
 *  Created on: 19.11.2020
 *  Copyright (C) 2020-2021  Armin Joachimsmeyer
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

#include "MPU6050IMUData.h"

//#define DEBUG // Only for development

MPU6050IMUData::MPU6050IMUData() {
    I2CAddress = MPU6050_DEFAULT_ADDRESS;
}

MPU6050IMUData::MPU6050IMUData(uint8_t aI2CAddress) {
    I2CAddress = aI2CAddress;
}

void MPU6050IMUData::setI2CAddress(uint8_t aI2CAddress) {
    I2CAddress = aI2CAddress;
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
#endif
}

/*
 * 400 kHz I2C fast mode timings:
 * 480 us reading per chunk
 * 4.8 ms for 10 chunks a 12 bytes -> 40 us / byte
 * @return Number of chunks/data sets read from FIFO
 */
uint8_t MPU6050IMUData::readFromMPU6050Fifo() {
    // Get FIFO count
    uint16_t tFifoCount = MPU6050ReadWordSwapped(MPU6050_RA_FIFO_COUNTH);
#ifdef DEBUG
    Serial.print(F("FiFoCnt="));
    Serial.print(tFifoCount);
    Serial.print(" ");
#endif
#if !defined(DO_NOT_USE_GYRO)
    int32_t tGyroscope[NUMBER_OF_GYRO_VALUES] = { 0, 0, 0 };
#endif
    if (tFifoCount > 0x300) {
        // Fifo is over 3/4 full -> reset it
        resetMPU6050Fifo();
#ifdef DEBUG
        Serial.println(F("Reset Fifo"));
#endif
        return 0;
    }
    uint8_t tNumberOfChunks = tFifoCount / FIFO_CHUNK_SIZE; // accept only 12 bytes chunks
    if (tNumberOfChunks > 0) {
#if defined(USE_SOFT_I2C_MASTER)
        // Here we have no buffer and can read all chunks in one row
        i2c_start(I2CAddress << 1);
        i2c_write(MPU6050_RA_FIFO_R_W);
        i2c_rep_start((I2CAddress << 1) | I2C_READ); // restart for reading
#endif
        uint8_t tChunckCount;
        for (tChunckCount = 0; tChunckCount < tNumberOfChunks; tChunckCount++) {

#if !defined(USE_SOFT_I2C_MASTER)
            Wire.beginTransmission(I2CAddress);
            Wire.write(MPU6050_RA_FIFO_R_W);
            Wire.endTransmission(false);
            Wire.requestFrom(I2CAddress, (uint8_t) (FIFO_CHUNK_SIZE), (uint8_t) true);
#endif

            // we must read all 3 values
            WordUnion tValue;
            for (uint8_t i = 0; i < NUMBER_OF_ACCEL_VALUES; i++) {
                // read into intermediate variable
#if defined(USE_SOFT_I2C_MASTER)
                tValue.UByte.HighByte = i2c_read(false);
#  if !defined(DO_NOT_USE_GYRO)
                tValue.UByte.LowByte = i2c_read(false);
#  else
                tValue.UByte.LowByte = i2c_read((i == (NUMBER_OF_ACCEL_VALUES - 1) && tChunckCount == (tNumberOfChunks - 1)));
#  endif
#else
                tValue.Byte.HighByte = Wire.read();
                tValue.Byte.LowByte = Wire.read();
#endif
#if !defined(USE_ONLY_ACCEL_FLOATING_OFFSET)
                tValue.Word = tValue.Word - AcceleratorInitialOffset[i];
                // compute speeds
                Speeds[i].Long += tValue.Word;
#endif
                Accelerator[i] = tValue.Word;
                AcceleratorLP8[i].Long += (((int32_t) tValue.Word << 16) - AcceleratorLP8[i].Long) >> 8; // Fixed point 2.0 us

            }
#if !defined(DO_NOT_USE_GYRO)
            for (uint8_t i = 0; i < NUMBER_OF_GYRO_VALUES; i++) {
                // read into intermediate variable
#  if defined(USE_SOFT_I2C_MASTER)
                tValue.UByte.HighByte = i2c_read(false);
                tValue.UByte.LowByte = i2c_read((i == (NUMBER_OF_GYRO_VALUES - 1) && tChunckCount == (tNumberOfChunks - 1)));
#  else
                tValue.Byte.HighByte = Wire.read();
                tValue.Byte.LowByte = Wire.read();
#  endif
                tValue.Word = tValue.Word - GyroscopeOffset[i];
                tGyroscope[i] += tValue.Word;
                // Compute rotations
                Rotations[i].Long += tValue.Word;
            }
#endif
        } // for (uint8_t tChunckCount = 0
#if defined(USE_SOFT_I2C_MASTER)
        i2c_stop();
#endif
#if !defined(DO_NOT_USE_GYRO)
        /*
         * If we have more than one gyro value, use the average
         */
        for (uint8_t i = 0; i < NUMBER_OF_GYRO_VALUES; i++) {
            Gyro[i] = tGyroscope[i] / tChunckCount;
        }
#endif
    } // if (tNumberOfChunks > 0)
    return tNumberOfChunks;
}

/*
 * Read data direct from registers
 * 500 us reading
 * Sets Accelerator, Speeds, Gyro and Rotations
 */
void MPU6050IMUData::readMPU6050Raw() {
//    NumberOfLoopSamples++;
    /*
     * Get data
     */
#if defined(USE_SOFT_I2C_MASTER)
    i2c_start(I2CAddress << 1);
    i2c_write(MPU6050_RA_ACCEL_XOUT_H);
    i2c_rep_start((I2CAddress << 1) | I2C_READ); // restart for reading

#else
    Wire.beginTransmission(I2CAddress);
    Wire.write(MPU6050_RA_ACCEL_XOUT_H);
    Wire.endTransmission(false);
#  if !defined(DO_NOT_USE_GYRO)
    Wire.requestFrom(I2CAddress, (uint8_t) (FIFO_CHUNK_SIZE + 2), (uint8_t) true);
#  else
        Wire.requestFrom(I2CAddress, (uint8_t) (FIFO_CHUNK_SIZE), (uint8_t) true);
#  endif
#endif

// [ax,ay,az,temp,gx,gy,gz]
// Accelerator
    WordUnion tValue;
    for (uint8_t i = 0; i < NUMBER_OF_ACCEL_VALUES; i++) {
        // read into intermediate variable
#if defined(USE_SOFT_I2C_MASTER)
        tValue.UByte.HighByte = i2c_read(false);
#  if !defined(DO_NOT_USE_GYRO)
        tValue.UByte.LowByte = i2c_read(false);
#  else
            tValue.UByte.LowByte = i2c_read(i == (NUMBER_OF_ACCEL_VALUES - 1) );
#  endif
#else
        tValue.Byte.HighByte = Wire.read();
        tValue.Byte.LowByte = Wire.read();
#endif
#if !defined(USE_ONLY_ACCEL_FLOATING_OFFSET)
        tValue.Word = tValue.Word - AcceleratorInitialOffset[i];
        // compute speeds
        Speeds[i].Long += tValue.Word;
#endif
        Accelerator[i] = tValue.Word;

    }

#if !defined(DO_NOT_USE_GYRO)
// Skip temperature
#if defined(USE_SOFT_I2C_MASTER)
    i2c_read(false);
    i2c_read(false);
#else
    Wire.read();
    Wire.read();
#endif

// 30 us read gyroscope data from wire buffer and process
    for (uint8_t i = 0; i < NUMBER_OF_GYRO_VALUES; i++) {
        // read into intermediate variable
#  if defined(USE_SOFT_I2C_MASTER)
        tValue.UByte.HighByte = i2c_read(false);
        tValue.UByte.LowByte = i2c_read(i == (NUMBER_OF_GYRO_VALUES - 1));
#  else
        tValue.Byte.HighByte = Wire.read();
        tValue.Byte.LowByte = Wire.read();
#  endif
        tValue.Word = tValue.Word - GyroscopeOffset[i];
        Gyro[i] = tValue.Word;
        // Compute rotations
        Rotations[i].Long += tValue.Word;
    }
#endif
#if defined(USE_SOFT_I2C_MASTER)
    i2c_stop();
#endif
}

void MPU6050IMUData::calculateAllOffsets() {
    uint32_t LastDataMillis;

    int32_t tSumAccel[NUMBER_OF_ACCEL_VALUES] = { 0, 0, 0 };
#if !defined(DO_NOT_USE_GYRO)
    int32_t tSumGyro[NUMBER_OF_GYRO_VALUES] = { 0, 0, 0 };
#endif

    for (int j = 0; j < NUMBER_OF_OFFSET_CALIBRATION_SAMPLES; j++) {

        // get data every ms
        while (millis() == LastDataMillis) {
            ;
        }
        LastDataMillis = millis();

#if defined(USE_SOFT_I2C_MASTER)
        i2c_start(I2CAddress << 1);
        i2c_write(MPU6050_RA_ACCEL_XOUT_H);
        i2c_rep_start((I2CAddress << 1) | I2C_READ); // restart for reading

#else
        Wire.beginTransmission(I2CAddress);
        Wire.write(MPU6050_RA_ACCEL_XOUT_H);
        Wire.endTransmission(false);
#  if !defined(DO_NOT_USE_GYRO)
        Wire.requestFrom(I2CAddress, (uint8_t) (FIFO_CHUNK_SIZE + 2), (uint8_t) true);
#  else
        Wire.requestFrom(I2CAddress, (uint8_t) (FIFO_CHUNK_SIZE), (uint8_t) true);
#  endif
#endif

        WordUnion tValue;
        // Acceleration
        for (uint8_t i = 0; i < NUMBER_OF_ACCEL_VALUES; i++) {
#if defined(USE_SOFT_I2C_MASTER)

            tValue.UByte.HighByte = i2c_read(false);
#  if !defined(DO_NOT_USE_GYRO)
            tValue.UByte.LowByte = i2c_read(false);
#  else
            tValue.UByte.LowByte = i2c_read(i == (NUMBER_OF_ACCEL_VALUES - 1) );
#  endif
#else
            tValue.Byte.HighByte = Wire.read();
            tValue.Byte.LowByte = Wire.read();
#endif
            tSumAccel[i] += tValue.Word;
        }
#if !defined(DO_NOT_USE_GYRO)
        // Temperature
#if defined(USE_SOFT_I2C_MASTER)
        i2c_read(false);
        i2c_read(false);
#else
        Wire.read();
        Wire.read();
#endif

        // Gyro
        for (uint8_t i = 0; i < NUMBER_OF_GYRO_VALUES; i++) {
#if defined(USE_SOFT_I2C_MASTER)
            tValue.UByte.HighByte = i2c_read(false);
            tValue.UByte.LowByte = i2c_read(i == (NUMBER_OF_GYRO_VALUES - 1));
#else
            tValue.Byte.HighByte = Wire.read();
            tValue.Byte.LowByte = Wire.read();
#endif
            tSumGyro[i] += tValue.Word;
        }
#endif
#if defined(USE_SOFT_I2C_MASTER)
        i2c_stop();
#endif
    }

    for (uint8_t i = 0; i < 3; i++) {
        Accelerator[i] = tSumAccel[i] / NUMBER_OF_OFFSET_CALIBRATION_SAMPLES;
#if defined(USE_ONLY_ACCEL_FLOATING_OFFSET)
        // initialize the lowpass with initial offsets since we have no initial offset correction for raw values any more
        AcceleratorLP8[i].Long = (tSumAccel[i] / NUMBER_OF_OFFSET_CALIBRATION_SAMPLES) << 16;
#else
        AcceleratorInitialOffset[i] = tSumAccel[i] / NUMBER_OF_OFFSET_CALIBRATION_SAMPLES;
#  if defined(USE_ACCEL_FLOATING_OFFSET)
        AcceleratorLP8[i].Long = 0;
#  endif
#endif
#if !defined(DO_NOT_USE_GYRO)
        GyroscopeOffset[i] = tSumGyro[i] / NUMBER_OF_OFFSET_CALIBRATION_SAMPLES;
#endif
    }

}

void MPU6050IMUData::printLP8Offsets(Print *aSerial) {
    aSerial->print(F("Acc low pass 8 offsets: X="));
    aSerial->print(AcceleratorLP8[0].Word.HighWord);
    aSerial->print('|');
    aSerial->print(AcceleratorLP8[0].Word.HighWord * ACCEL_RAW_TO_G_FOR_2G_RANGE);
    aSerial->print(F("g Y="));
    aSerial->print(AcceleratorLP8[1].Word.HighWord);
    aSerial->print('|');
    aSerial->print(AcceleratorLP8[1].Word.HighWord * ACCEL_RAW_TO_G_FOR_2G_RANGE);
    aSerial->print(F("g Z="));
    aSerial->print(AcceleratorLP8[2].Word.HighWord);
    aSerial->print('|');
    aSerial->print(AcceleratorLP8[2].Word.HighWord * ACCEL_RAW_TO_G_FOR_2G_RANGE);
    aSerial->println('g');
}

void MPU6050IMUData::printAllOffsets(Print *aSerial) {
#if defined(USE_ONLY_ACCEL_FLOATING_OFFSET)
    printLP8Offsets(aSerial);
#else
    aSerial->print(F("Acc offsets: X="));
    aSerial->print(AcceleratorInitialOffset[0]);
    aSerial->print('|');
    aSerial->print(AcceleratorInitialOffset[0] * ACCEL_RAW_TO_G_FOR_2G_RANGE);
    aSerial->print(F("g Y="));
    aSerial->print(AcceleratorInitialOffset[1]);
    aSerial->print('|');
    aSerial->print(AcceleratorInitialOffset[1] * ACCEL_RAW_TO_G_FOR_2G_RANGE);
    aSerial->print(F("g Z="));
    aSerial->print(AcceleratorInitialOffset[2]);
    aSerial->print('|');
    aSerial->print(AcceleratorInitialOffset[2] * ACCEL_RAW_TO_G_FOR_2G_RANGE);
    aSerial->println('g');
#endif

#if !defined(DO_NOT_USE_GYRO)
    aSerial->print(F("Gyro offsets: X="));
    aSerial->print(GyroscopeOffset[0]);
    aSerial->print('|');
    aSerial->print(GyroscopeOffset[0] * GYRO_RAW_TO_DEGREE_PER_SECOND_FOR_250DPS_RANGE);
    aSerial->print(F(" Y="));
    aSerial->print(GyroscopeOffset[1]);
    aSerial->print('|');
    aSerial->print(GyroscopeOffset[1] * GYRO_RAW_TO_DEGREE_PER_SECOND_FOR_250DPS_RANGE);
    aSerial->print(F(" Z="));
    aSerial->print(GyroscopeOffset[2]);
    aSerial->print('|');
    aSerial->print(GyroscopeOffset[2] * GYRO_RAW_TO_DEGREE_PER_SECOND_FOR_250DPS_RANGE);
    aSerial->println();
#endif
}

void MPU6050IMUData::resetMPU6050Fifo() {
    MPU6050WriteByte(MPU6050_RA_USER_CTRL, _BV(MPU6050_USERCTRL_FIFO_RESET_BIT)); // Reset FIFO
    MPU6050WriteByte(MPU6050_RA_USER_CTRL, _BV(MPU6050_USERCTRL_FIFO_EN_BIT)); // enable FIFO
}

void MPU6050IMUData::initMPU6050FifoForAccelAndGyro() {
#if defined(DO_NOT_USE_GYRO)
    MPU6050WriteByte(MPU6050_RA_FIFO_EN, _BV(MPU6050_ACCEL_FIFO_EN_BIT)); // FIFO: only Accel axes
#else
    MPU6050WriteByte(MPU6050_RA_FIFO_EN,
    _BV(MPU6050_ACCEL_FIFO_EN_BIT) | _BV(MPU6050_XG_FIFO_EN_BIT) | _BV(MPU6050_YG_FIFO_EN_BIT) | _BV(MPU6050_ZG_FIFO_EN_BIT)); // FIFO: all Accel + Gyro axes
#endif
    resetMPU6050Fifo();
}

void MPU6050IMUData::initMPU6050() {
#if !defined(USE_ONLY_ACCEL_FLOATING_OFFSET)
    for (uint8_t i = 0; i < NUMBER_OF_ACCEL_VALUES; i++) {
        Speeds[i].ULong = 0;
    }
#endif
#if !defined(DO_NOT_USE_GYRO)
    for (uint8_t i = 0; i < NUMBER_OF_GYRO_VALUES; i++) {
        Rotations[i].ULong = 0;
    }
#endif
    MPU6050WriteByte(MPU6050_RA_PWR_MGMT_1, MPU6050_CLOCK_PLL_ZGYRO); // use recommended gyro reference: PLL with Z axis gyroscope reference
    MPU6050WriteByte(MPU6050_RA_SMPLRT_DIV, 0x00); // // Set sample rate to 1 kHz, divider is minimum (1)

//    MPU6050WriteByte(MPU6050_RA_CONFIG, MPU6050_DLPF_BW_20); // ext input disabled, DLPF enabled: ~20 Hz Sample freq = 1kHz
//    MPU6050WriteByte(MPU6050_RA_CONFIG, MPU6050_DLPF_BW_10); // ext input disabled, DLPF enabled: ~10 Hz Sample freq = 1kHz
//    MPU6050WriteByte(MPU6050_RA_CONFIG, MPU6050_DLPF_BW_42); // ext input disabled, DLPF enabled: ~50 Hz Sample freq = 1kHz
//    MPU6050WriteByte(MPU6050_RA_CONFIG, MPU6050_DLPF_BW_98); // ext input disabled, DLPF enabled: accel 184Hz gyro 188Hz @1kHz

    MPU6050WriteByte(MPU6050_RA_CONFIG, MPU6050_DLPF_BW_188); // ext input disabled, DLPF enabled: accel 184Hz gyro 188Hz @1kHz

// range select
    MPU6050WriteByte(MPU6050_RA_ACCEL_CONFIG,
    MPU6050_ACCEL_FS_2 << (MPU6050_ACONFIG_AFS_SEL_BIT - MPU6050_ACONFIG_AFS_SEL_LENGTH + 1)); // range +/- 2 g - default
#if !defined(DO_NOT_USE_GYRO)
    MPU6050WriteByte(MPU6050_RA_GYRO_CONFIG,
    MPU6050_GYRO_FS_250 << (MPU6050_GCONFIG_FS_SEL_BIT - MPU6050_GCONFIG_FS_SEL_LENGTH + 1)); // range +/- 250 deg/s - default
#endif
}

void MPU6050IMUData::MPU6050WriteByte(uint8_t aRegisterNumber, uint8_t aData) {

#if defined(USE_SOFT_I2C_MASTER)
    i2c_start(I2CAddress << 1);
    i2c_write(aRegisterNumber);
    i2c_write(aData);
    i2c_stop();

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
    WordUnion tWord;

#if defined(USE_SOFT_I2C_MASTER)
    i2c_start(I2CAddress << 1);
    i2c_write(aRegisterNumber);
    i2c_rep_start((I2CAddress << 1) | I2C_READ); // restart for reading
    tWord.UByte.HighByte = i2c_read(false);
    tWord.UByte.LowByte = i2c_read(true);

#else
    Wire.beginTransmission(I2CAddress);
    Wire.write(aRegisterNumber);
    Wire.endTransmission(false);
    Wire.requestFrom((uint8_t) I2CAddress, (uint8_t) 2, (uint8_t) true);
    tWord.UByte.HighByte = Wire.read();
    tWord.UByte.LowByte = Wire.read();
#endif
    return tWord.UWord;
}

uint16_t MPU6050IMUData::MPU6050ReadWord(uint8_t aRegisterNumber) {
    WordUnion tWord;

#if defined(USE_SOFT_I2C_MASTER)
    i2c_start(I2CAddress << 1);
    i2c_write(aRegisterNumber);
    i2c_rep_start((I2CAddress << 1) | I2C_READ); // restart for reading
    tWord.UByte.LowByte = i2c_read(false);
    tWord.UByte.HighByte = i2c_read(true);

#else
    Wire.beginTransmission(I2CAddress);
    Wire.write(aRegisterNumber);
    Wire.endTransmission(false);
    Wire.requestFrom((uint8_t) I2CAddress, (uint8_t) 2, (uint8_t) true);
    tWord.UByte.LowByte = Wire.read();
    tWord.UByte.HighByte = Wire.read();
#endif
    return tWord.UWord;
}

/**
 * Compute geometric sum of all 3 offset corrected accelerator values
 * 4 g for 16 bit full range
 */
unsigned int MPU6050IMUData::computeAccelerationWithInitialOffset() {
    unsigned long tAcceleration = 0;
    for (uint8_t i = 0; i < NUMBER_OF_ACCEL_VALUES; i++) {
        tAcceleration += (unsigned long) Accelerator[i] * Accelerator[i];
    }
    return sqrt(tAcceleration);
}

/**
 * Compute geometric sum of all 3 offset corrected accelerator values, but use AcceleratorLP8 (low pass 2^8) as offset
 * AcceleratorLP5 is too fast and dampens the signal too much at 1 kHz
 * 4 g for 16 bit full range
 */
unsigned int MPU6050IMUData::computeAccelerationWithFloatingOffset() {
#if defined(USE_ACCEL_FLOATING_OFFSET)
    unsigned long tAcceleration = 0;
    for (uint8_t i = 0; i < NUMBER_OF_ACCEL_VALUES; i++) {
        int16_t tAccelerationOffsetCompensated = Accelerator[i] - (AcceleratorLP8[i].Word.HighWord);
        tAcceleration += (unsigned long) tAccelerationOffsetCompensated * tAccelerationOffsetCompensated;
    }
    return sqrt(tAcceleration);
#else
// fallback
    return computeAccelerationWithInitialOffset();
#endif
}
