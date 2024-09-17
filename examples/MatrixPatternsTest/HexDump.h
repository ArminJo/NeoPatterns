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

void printBytePaddedHex(uint8_t aHexValueToPrint);
void printWordPaddedHex(uint16_t aHexValueToPrint);
void printBufferHexDump(uint8_t *aBufferAddress, uint16_t aNumberOfBytesToPrint);
void printBufferHexAndASCIIDump(uint8_t *aBufferAddress, uint16_t aNumberOfBytesToPrint);
void printMemoryHexDump(uint8_t *aMemory, uint16_t aSizeOfMemoryToPrint, uint8_t aBytesPerLine = _16_BYTES_PER_LINE, bool aPrintAscii = true, bool aPrintShortAddress = false, bool aPrintRelativeAddress = false);

#endif // _HEX_DUMP_H
