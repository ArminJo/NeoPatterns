# [NeoPatterns](https://github.com/ArminJo/NeoPatterns) for NeoPixel strips and Snake game for NeoPixel matrix.
Available as Arduino library "NeoPatterns"

### [Version 2.3.2](https://github.com/ArminJo/NeoPatterns/archive/master.zip) - work in progress

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
| YouTube Video of NeoPatternsDemo on a long matrix | YouTube Video of Fire on on a long matrix |
| [![Demonstration of NeoPatterns on a long matrix ](https://i.ytimg.com/vi/y_fmAEUqhFg/hqdefault.jpg)](https://www.youtube.com/watch?v=y_fmAEUqhFg) | [![Demonstration of Fire on a long matrix ](https://i.ytimg.com/vi/CgW5T-mRSvQ/hqdefault.jpg)](https://youtu.be/CgW5T-mRSvQ?t=43) |

# PATTERNS
## Patterns from [Adafruit](https://www.adafruit.com/)
**RAINBOW_CYCLE**, **COLOR_WIPE**, **FADE**
## New patterns
**SCANNER**. **STRIPES**, **HEARTBEAT**, **DELAY**, **PROCESS_SELECTIVE**, **FADE_SELECTIVE**, **BOUNCING_BALL**<br/>
The original **SCANNER** pattern is extended and includes the **CYLON** as well as the **ROCKET** or **FALLING_STAR** pattern. The more versatile **STRIPES** pattern replaces the old **THEATER_CHASE** one.
## Pattern from [FastLed](https://github.com/FastLED/FastLED)
**FIRE** adapted from https://github.com/FastLED/FastLED/tree/master/examples/Fire2012
## Patterns only for nxn Matrix
**MOVING_PICTURE**, **MOVE**, **TICKER**, **FIRE**, **SNOW**, **SNAKE**
## Your own patterns
**Put your pattern code to the functions UserPattern\[1,2]() and UserPattern\[1,2]Update() in AllPatternOnOneStrip.cpp to realize your own patterns. Enable TEST_USER_PATTERNS on line 41 to test them.**

# NeoPixel library
the included NeoPixel library is an extensions of the Adafruit NeoPixel library and supports multiple virtual NeoPixel (and NeoPattern) objects on one physical strip. It also contains a lot of useful functions like:
- ColorSet()
- drawBar() 
- drawBarFromColorArray() - uses a color array for the different colors of the bar pixel.
- dimColor() - by 50%.
- gamma5() - returns gamma brightness value from a linear input.
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
**Origin (0,0) of x and y values is at the top left corner and the positive direction is right and DOWN.**

Pixel mappings definitions and semantics are taken from https://github.com/adafruit/Adafruit_NeoMatrix/blob/master/Adafruit_NeoMatrix.h
Here you find also mappings for tiled display with multiple matrices.

Examples for LED index to position mappings:
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

# Compile options / macros for this library
To customize the library to different requirements, there are some compile options / makros available.<br/>
Modify it by commenting them out or in, or change the values if applicable. Or define the macro with the -D compiler option for global compile (the latter is not possible with the Arduino IDE, so consider using [Sloeber](https://eclipse.baeyens.it).
| Macro | Default | File | Description |
|-|-|-|-|
| `SUPPORT_RGBW` | enabled | NeoPixel.h | Can be disabled by commenting out `#define SUPPORT_RGBW` or defining `DO_NOT_SUPPORT_RGBW`. Disable it if you only have RGB pixels and do not require RGBW pixels support. Disabling saves up to 400 bytes FLASH for the AllPatternsOnMultiDevices example. |
| `DO_NOT_USE_MATH_PATTERNS` | disabled | NeoPatterns.h | Disables the `BOUNCING_BALL` pattern. Saves up to 640 to 1140 bytes FLASH, depending if floating point and sqrt() are already used otherwise. |
| `SUPPORT_ONLY_DEFAULT_GEOMETRY` | disabled | MatrixNeoPixel.h | If you have only default geometry, i.e. Pixel 0 is at bottom right of matrix, matrix is row major (horizontal) and same pixel order across each line (no zig-zag) you can save 560 bytes (and more) FLASH and 3 bytes RAM. |

### Modifying compile options with Arduino IDE
First use *Sketch > Show Sketch Folder (Ctrl+K)*.<br/>
If you did not yet stored the example as your own sketch, then you are instantly in the right library folder.<br/>
Otherwise you have to navigate to the parallel `libraries` folder and select the library you want to access.<br/>
In both cases the library files itself are located in the `src` directory.<br/>

### Modifying compile options with Sloeber IDE
If you are using Sloeber as your IDE, you can easily define global symbols with *Properties > Arduino > CompileOptions*.<br/>
![Sloeber settings](https://github.com/ArminJo/ServoEasing/blob/master/pictures/SloeberDefineSymbols.png)

# SNAKE GAME
## SnakeGame Example
The game can be controlled by 2 or 4 buttons or by serial input (WASD). To enable serial input control you must activate the line `#define USE_SERIAL_CONTROL` in the library file *MatrixSnake.h* or define global symbol with `-DUSE_SERIAL_CONTROL` which is not yet possible in Arduino IDE:-(.<br/>
The experimental Python script in the extras folder converts key presses and game controller input to appropriate serial output for the game.<br/>
After 7 seconds of inactivity the Snake demo with a simple AI is started.
## SnakeAutorun Example
**With the SnakeAutorun example you can prove your skill to write an AI to solve the Snake game. Just put your code into the getNextSnakeDirection() function.**

NeoPatterns on breadboard
![NeoPatterns on breadboard](https://github.com/ArminJo/NeoPatterns/blob/master/extras/Breadboard_complete.jpg)

# Revision History
### Version 2.3.2 - work in progress

### Version 2.3.1
- Changed type of TotalStepCounter from uint16_t to int16_t.
- Added `SnowMatrix` pattern.
- Improved debugging.
- Fixed random() bug for ESP32.
- Improved Fire cooling.

### Version 2.3.0
- Changed TickerUpdate() and loadPicture() and their callers to achieve that YOffset is consistent with Y direction and origin.
- Removed restriction to 8 columns for `FireMatrix` pattern.

### Version 2.2.2 + 2.2.3 (2.2.2 was an incomplete version)
- Fixed bugs if rows are not equal columns.
- Fixed bug in MatrixNeoPatterns constructor.
- Added `SUPPORT_ONLY_DEFAULT_GEOMETRY` compile option.
- Added loadPicture() for 16 bit pictures.
- Ticker now supports multiple characters and `DIRECTION_NONE`.

### Version 2.2.1
- Removed blocking wait for ATmega32U4 Serial in examples.

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
- Changed signature of `NeoPatterns(NeoPixel *aUnderlyingNeoPixelObject)`. Swapped 4. and 5. parameter to make it consistent to the NeoPixel signature.
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
