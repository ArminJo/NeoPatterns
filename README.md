# [NeoPatterns](https://github.com/ArminJo/NeoPatterns) for NeoPixel strips and Snake game for NeoPixel matrix.
### [Version 2.2.0](https://github.com/ArminJo/NeoPatterns/releases)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Installation instructions](https://www.ardu-badge.com/badge/NeoPatterns.svg?)](https://www.ardu-badge.com/NeoPatterns)
[![Commits since latest](https://img.shields.io/github/commits-since/ArminJo/NeoPatterns/latest)](https://github.com/ArminJo/NeoPatterns/commits/master)
[![Build Status](https://github.com/ArminJo/NeoPatterns/workflows/LibraryBuild/badge.svg)](https://github.com/ArminJo/NeoPatterns/actions)
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
**SCANNER**. **STRIPES**, **HEARTBEAT**, **DELAY**, **PROCESS_SELECTIVE**, **FADE_SELECTIVE**, **BOUNCING_BALL**<br/>
The original **SCANNER** pattern is extended and includes the **CYLON** as well as the **ROCKET** or **FALLING_STAR** pattern. The more versatile **STRIPES** pattern replaces the old **THEATER_CHASE** one.
## Pattern from [FastLed](https://github.com/FastLED/FastLED)
**FIRE** adapted from https://github.com/FastLED/FastLED/tree/master/examples/Fire2012
## Patterns only for nxn Matrix
**MOVING_PICTURE**, **MOVE**, **TICKER**, **FIRE**, **SNAKE**
## Your own patterns
**Put your pattern code to the functions UserPattern\[1,2]() and UserPattern\[1,2]Update() in AllPatternOnOneStrip.cpp to realize your own patterns. Enable TEST_USER_PATTERNS on line 41 to test them.**

# NeoPixel library
the included NeoPixel library is an extensions of the Adafruit NeoPixel library and supports multiple virtual NeoPixel (and NeoPattern) objects on one physical strip. It also contains a lot of useful functions like:
- ColorSet()
- drawBar() 
- drawBarFromColorArray() - uses a color array for the different colors of the bar pixel.
- dimColor() - by 50%.
- gamma5() - returns gamma brightness value from a linear input.
- gamma5WithSpecialZero()
- gamma5FromColor() - returns the gamma corrected color.
- Wheel() - returns colors from a color wheel starting ar red.
as well as functions for getting color parts
- Red()
- Green()
- Blue()

# Installation
First you need to install "Adafruit NeoPixel" library with *Tools -> Manage Libraries...* or *Ctrl+Shift+I*. Use "neoPixel" as filter string.
Then install this "NeoPatterns" library with *Tools -> Manage Libraries... (Ctrl+Shift+I)*. Use "NeoPatterns" as filter string.

## Matrix pixel mappings
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

**All matrix pixel mappings except NEO_MATRIX_COLUMNS are supported**
In case you need `NEO_MATRIX_COLUMNS` layout, try to rotate your Matrix and use `NEO_MATRIX_ROWS` or use your own custom mapping function.

## Reducing library size
If you do not have RGBW pixels, then you can save program space by commenting out the line `#define SUPPORT_RGBW` in *NeoPixel.h* or defining `DO_NOT_SUPPORT_RGBW` as global symbol. This saves e.g 400 bytes FLASH for the AllPatternsOnMultiDevices example.

### Modifying library properties
To access the Arduino library files from a sketch, you have to first use *Sketch/Show Sketch Folder (Ctrl+K)* in the Arduino IDE.<br/>
Then navigate to the parallel `libraries` folder and select the library you want to access.<br/>
The library files itself are located in the `src` sub-directory.<br/>
If you did not yet store the example as your own sketch, then with *Ctrl+K* you are instantly in the right library folder.

# SNAKE GAME
## SnakeGame Example
The game can be controlled by 2 or 4 buttons or by serial input (WASD). To enable serial input control you must comment out the line `#define USE_SERIAL_CONTROL` in the library file *MatrixSnake.h* or define global symbol with `-DUSE_SERIAL_CONTROL` which is not yet possible in Arduino IDE:-(.<br/>
The experimental Python script in the extras folder converts key presses and game controller input to appropriate serial output for the game.<br/>
After 7 seconds of inactivity the Snake demo with a simple AI is started.
## SnakeAutorun Example
**With the SnakeAutorun example you can prove your skill to write an AI to solve the Snake game. Just put your code into the getNextSnakeDirection() function.**

NeoPatterns on breadboard
![NeoPatterns on breadboard](https://github.com/ArminJo/NeoPatterns/blob/master/extras/Breadboard_complete.jpg)

# Revision History
### Version 2.2.0
- Added support for RGBW patterns. Requires additional 200 bytes for the AllPatternsOnMultiDevices example. Commenting out `#define SUPPORT_RGBW` or defining `DO_NOT_SUPPORT_RGBW` saves 400 bytes FLASH for the AllPatternsOnMultiDevices example.
- Use type `Print *` instead of `Stream *`.
- Changed function `addPixelColor()`.
- Added function `NeoPixel::printInfo(aSerial)`.
- Added `*D` functions, which take the duration of the whole pattern as argument.
- Added OpenLedRace example.
- Added empty constructor and `init()` functions.
- Added function `updateAllPartialPatterns()`.

### Version 2.1.0
- Ported to ESP8266 and ESP32.
- Changed signature of `NeoPatterns(NeoPixel * aUnderlyingNeoPixelObject)`. Swapped 4. and 5. parameter to make it consistent to the NeoPixel signature.
- Function `setPixelOffsetForPartialNeoPixel()` in NeoPixel.cpp added.

### Version 2.0.0
- Rewrite of most patterns control logic.
- Function `drawBar()` in NeoPixel.cpp added.
- Swapped parameter aNumberOfSteps and aIntervalMillis of `Stripes()`.
- Pattern `HEARTBEAT` and `BOUNCING_BALL` added.
- Swapped first parameter and added parameter aDirection to `Fire()`.
- Changed internal functions.
- Reworked `UserPattern`.

### Version 1.1.0
- Function `getPatternName()` added.
- Function `printPatternName()` added.
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
- ESP8266 boards (tested with LOLIN D1 R2 board)
- ESP32   boards (tested with ESP32 DEVKITV1 board)

## Requests for modifications / extensions
Please write me a PM including your motivation/problem if you need a modification or an extension.

#### If you find this library useful, please give it a star.
