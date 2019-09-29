# NeoPatterns for NeoPixel strips and Snake game for NeoPixel matrix.

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Installation instructions](https://www.ardu-badge.com/badge/NeoPatterns.svg?)](https://www.ardu-badge.com/NeoPatterns)
[![Build Status](https://travis-ci.org/ArminJo/NeoPatterns.svg?branch=master)](https://travis-ci.org/ArminJo/NeoPatterns)
[![Hit Counter](https://hitcounter.pythonanywhere.com/count/tag.svg?url=https%3A%2F%2Fgithub.com%2FArminJo%2FNeoPatterns)](https://github.com/brentvollebregt/hit-counter)

This is an extended version version of the NeoPattern example by Adafruit https://learn.adafruit.com/multi-tasking-the-arduino-part-3?view=all.
New Patterns are added, a snake game running on a matrix is included and you can run multiple patterns simultaneously on the same strip.

| YouTube Video of NeoPatternsDemo | YouTube Video of MatrixDemo on a 10x10 matrix |
| :-: | :-: |
| [![Demonstration of NeoPatterns](https://i.ytimg.com/vi/CsB7FkywCRQ/hqdefault.jpg)](https://www.youtube.com/watch?v=CsB7FkywCRQ) | [![Demonstration of MatrixDemo on a 10x10 matrix](https://i.ytimg.com/vi/URsq28l2PEQ/hqdefault.jpg)](https://www.youtube.com/watch?v=URsq28l2PEQ) |

# PATTERNS
## Patterns from [Adafruit](https://www.adafruit.com/)
**RAINBOW_CYCLE**, **COLOR_WIPE**, **FADE**
## New patterns
**SCANNER**. **STRIPES**, **DELAY**, **PROCESS_SELECTIVE**, **FADE_SELECTIVE**<br/>
The original **SCANNER** pattern is extended and includes the **CYLON** as well as the **ROCKET** or **FALLING_STAR** pattern. The more versatile **STRIPES** pattern replaces the old **THEATER_CHASE** one.
## Pattern from [FastLed](https://github.com/FastLED/FastLED)
**FIRE** adapted from https://github.com/FastLED/FastLED/tree/master/examples/Fire2012
## Patterns only for nxn Matrix
**MOVING_PICTURE**, **MOVE**, **TICKER**, **FIRE**, **SNAKE**
## Your own patterns
**Just put your pattern code to the functions UserPattern\[1,2]() and UserPattern\[1,2]Update() in NeoPatternsSimpleDemo.cpp to realize your own patterns. Enable TEST_USER_PATTERNS on line 39 to test them.**

# Installation
First you need to install "Adafruit NeoPixel" library with *Tools -> Manage Libraries...* or *Ctrl+Shift+I*. Use "neoPixel" as filter string.
Then install this "NeoPatterns" library with *Tools -> Manage Libraries...* or *Ctrl+Shift+I*. Use "NeoPatterns" as filter string.

### All pixel mappings except NEO_MATRIX_COLUMNS supported
In case you need `NEO_MATRIX_COLUMNS` layout, try to rotate your Matrix and use `NEO_MATRIX_ROWS` or use your own custom mapping function.

Pixel mappings definitions and semantics are taken from https://github.com/adafruit/Adafruit_NeoMatrix/blob/master/Adafruit_NeoMatrix.h
Here you find also mappings for tiled display with multiple matrices.

Examples:
```
     ProgressiveMapping                  ZigzagTypeMapping
   Regular        Mirrored           Regular        Mirrored
   Bottom/Right   Bottom/Left                                                             
   15 14 13 12    12 13 14 15        12 13 14 15    15 14 13 12    
   11 10  9  8     8  9 10 11        11 10  9  8     8  9 10 11    
    7  6  5  4     4  5  6  7         4  5  6  7     7  6  5  4    
    3  2  1  0     0  1  2  3         3  2  1  0     0  1  2  3   
```
# SNAKE GAME
## SnakeGame Example
The game can be controlled by 2 or 4 buttons or by serial input (WASD). To enable serial input control you must define the symbol `USE_SERIAL_CONTROL` or comment out line 33 in `MatrixSnake.h`.
The experimental Python script in the extras folder converts key presses and game controller input to appropriate serial output for the game.<br\>
After 7 seconds of inactivity the Snake demo with a simple AI is started.
## SnakeAutorun Example
**With the SnakeAutorun example you can prove your skill to write an AI to solve the Snake game. Just put your code into the getNextSnakeDirection() function.**

NeoPatterns on breadboard
![NeoPatterns on breadboard](https://github.com/ArminJo/NeoPatterns/blob/master/extras/Breadboard_complete.jpg)

# Revision History
### Version 1.1.0
- Fuction `getPatternName()` added.
- Fuction `printPatternName()` added.
- Improved support for pattern running on parts of NeoPixel bars etc.
- New example for 3 pattern running on parts of one NeoPixel bar.
- New simple example.
- Changed signature of `Stripes()` and reset `ActivePattern` to `PATTERN_NONE` at the end of each pattern.
### Version 1.0.1
- Support of pattern running on parts of NeoPixel bars etc.
### Version 1.0.0
- Initial Arduino library version

## Travis CI
The NeoPatterns library examples are built on Travis CI for the following boards:

- Arduino Uno
- Arduino Leonardo
- Arduino Mega 2560

## Requests for modifications / extensions
Please write me a PM including your motivation/problem if you need a modification or an extension.
