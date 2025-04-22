/*
 * HexDump.h
 *
 *  Copyright (C) 2022  Armin Joachimsmeyer
 *  Email: armin.joachimsmeyer@gmail.com
 *
 *  This file is part of Arduino-Utils https://github.com/ArminJo/Arduino-Utils.
 *
 *  Arduino-Utils is free software: you can redistribute it and/or modify
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
 *
 */
#ifndef _HEX_DUMP_H
#define _HEX_DUMP_H

#include <stdint.h>
#include <stddef.h>

#define _16_BYTES_PER_LINE  16

#define HEX_DUMP_FORMAT_16_BIT_ABSOLUTE_ADDRESS 0x00 // Print 16 bit absolute address
#define HEX_DUMP_FORMAT_NO_ADDRESS_AT_ALL       0x01 // Bit 0: else print
#define HEX_DUMP_FORMAT_RELATIVE_ADDRESS        0x02 // Bit 1: else absolute address
#define HEX_DUMP_FORMAT_8_BIT_ADDRESS           0x04 // Bit 2: else 16 bit Address
#define HEX_DUMP_FORMAT_ASCII_VALUES            0x08 // default

void printBufferHex(uint8_t *aBufferAddress, uint16_t aNumberOfBytesToPrint);     // Prints no address and hex bytes without ASCII representation.
void printBufferHexDump(uint8_t *aBufferAddress, uint16_t aNumberOfBytesToPrint); // Prints short relative address and hex bytes without ASCII representation.
void printBufferHexAndASCIIDump(uint8_t *aBufferAddress, uint16_t aNumberOfBytesToPrint); // Prints short relative address and hex bytes without ASCII representation.
void printMemoryHexNoASCIIDump(uint8_t *aMemoryAddress, uint16_t aNumberOfBytesToPrint);  // Prints 16 bit address and hex bytes with ASCII representation.
void printMemoryHexAndASCIIDump(uint8_t *aMemoryAddress, uint16_t aNumberOfBytesToPrint); // Prints 16 bit address and hex bytes with ASCII representation.
void printStackMemory(uint16_t aNumberOfBytesToPrint); // Prints 16 bit address and hex bytes ending at top of stack / RAM end.
void printStackDump(); // Prints 16 bit address and hex bytes starting at current stackpointer and ending at ending at top of stack / RAM end.
void printMemoryHexDump(uint8_t *aMemory, uint16_t aSizeOfMemoryToPrint, uint8_t aBytesPerLine = _16_BYTES_PER_LINE,
        uint8_t aFormatFlags = HEX_DUMP_FORMAT_ASCII_VALUES);
void printBytePaddedHex(uint8_t aHexValueToPrint);
void printWordPaddedHex(uint16_t aHexValueToPrint);
#endif // _HEX_DUMP_H
