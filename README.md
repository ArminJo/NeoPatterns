<div align = center>

# [NeoPatterns](https://github.com/ArminJo/NeoPatterns) for NeoPixel strips and Snake game for NeoPixel matrix.
This is an extended version version of the [NeoPattern example by Adafruit](https://learn.adafruit.com/multi-tasking-the-arduino-part-3?view=all).<br/>
New Patterns are added, a [snake game running on a matrix](https://github.com/ArminJo/NeoPatterns/blob/master/examples/SnakeGame/SnakeGame.ino) is included and you can run [multiple patterns simultaneously on the same strip](https://github.com/ArminJo/NeoPatterns/blob/master/examples/TwoPatternsOnOneStrip/TwoPatternsOnOneStrip.ino).

### [Version 3.1.2](https://github.com/ArminJo/NeoPatterns/archive/master.zip) - work in progress

[![Badge License: GPLv3](https://img.shields.io/badge/License-GPLv3-brightgreen.svg)](https://www.gnu.org/licenses/gpl-3.0)
 &nbsp; &nbsp; 
[![Badge Version](https://img.shields.io/github/v/release/ArminJo/NeoPatterns?include_prereleases&color=yellow&logo=DocuSign&logoColor=white)](https://github.com/ArminJo/NeoPatterns/releases/latest)
 &nbsp; &nbsp; 
[![Badge Commits since latest](https://img.shields.io/github/commits-since/ArminJo/NeoPatterns/latest?color=yellow)](https://github.com/ArminJo/NeoPatterns/commits/master)
 &nbsp; &nbsp; 
[![Badge Build Status](https://github.com/ArminJo/NeoPatterns/workflows/LibraryBuild/badge.svg)](https://github.com/ArminJo/NeoPatterns/actions)
 &nbsp; &nbsp; 
![Badge Hit Counter](https://visitor-badge.laobi.icu/badge?page_id=ArminJo_NeoPatterns)
<br/>
<br/>
[![Stand With Ukraine](https://raw.githubusercontent.com/vshymanskyy/StandWithUkraine/main/badges/StandWithUkraine.svg)](https://stand-with-ukraine.pp.ua)

Available as [Arduino library "NeoPatterns"](https://www.arduinolibraries.info/libraries/neo-patterns).

[![Button Install](https://img.shields.io/badge/Install-brightgreen?logoColor=white&logo=GitBook)](https://www.ardu-badge.com/NeoPatterns)
 &nbsp; &nbsp; 
[![Button Changelog](https://img.shields.io/badge/Changelog-blue?logoColor=white&logo=AzureArtifacts)](https://github.com/ArminJo/NeoPatterns#revision-history)

</div>


| YouTube Video of NeoPatternsDemo | YouTube Video of MatrixDemo on a 10x10 matrix |
| :-: | :-: |
| [![Demonstration of NeoPatterns](https://i.ytimg.com/vi/CsB7FkywCRQ/hqdefault.jpg)](https://www.youtube.com/watch?v=CsB7FkywCRQ) | [![Demonstration of MatrixDemo on a 10x10 matrix](https://i.ytimg.com/vi/URsq28l2PEQ/hqdefault.jpg)](https://www.youtube.com/watch?v=URsq28l2PEQ) |
| YouTube Video of NeoPatternsDemo on a long matrix | YouTube Video of Fire on on a long matrix |
| [![Demonstration of NeoPatterns on a long matrix ](https://i.ytimg.com/vi/y_fmAEUqhFg/hqdefault.jpg)](https://www.youtube.com/watch?v=y_fmAEUqhFg) | [![Demonstration of Fire on a long matrix ](https://i.ytimg.com/vi/CgW5T-mRSvQ/hqdefault.jpg)](https://youtu.be/CgW5T-mRSvQ?t=43) |
| YouTube Video of OpenLedRace at the Cologne public library MINTköln-Festival |  |
| [![OpenLedRace in action](https://i.ytimg.com/vi/y25rjRkDg0g/hqdefault.jpg)](https://www.youtube.com/watch?v=y25rjRkDg0g) |  |

# PATTERNS
## Patterns from [Adafruit](https://www.adafruit.com/)
- RAINBOW_CYCLE
- COLOR_WIPE
- FADE

## New patterns
- DELAY
- SCANNER_EXTENDED
- STRIPES
- FLASH
- PROCESS_SELECTIVE
- HEARTBEAT
- BOUNCING_BALL

The original **SCANNER** pattern is extended and includes the **CYLON** as well as the **ROCKET** or **FALLING_STAR** pattern. The more versatile **STRIPES** pattern replaces the old **THEATER_CHASE** one.

## Pattern from FastLed
- FIRE adapted from [FastLed](https://github.com/FastLED/FastLED/tree/master/examples/Fire2012)

## Patterns only for n x n Matrix
- MOVING_PICTURE*
- MOVE
- TICKER
- FIRE
- SNOW
- SNAKE

## Your own patterns
- USER_PATTERN1
- USER_PATTERN2

**Put your pattern code to the functions UserPattern\[1,2]() and UserPattern\[1,2]Update() in a file UserPattern.cpp to realize your own patterns.**

# NeoPixel library
the included NeoPixel library is an extensions of the Adafruit NeoPixel library and supports multiple virtual NeoPixel (and NeoPattern) objects on one physical strip. It also contains a lot of useful functions like:
- setColor()
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
First, you need to install "Adafruit NeoPixel" library with *Tools -> Manage Libraries...* or *Ctrl+Shift+I*. Use "neoPixel" as filter string.
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

# Using the new *.hpp files / how to avoid `multiple definitions` linker errors
In order to support [compile options](#compile-options--macros-for-this-library) more easily, the line `#include <NeoPatterns.h>`
 or `MatrixNeoPatterns.hpp` or `MatrixSnake.hpp` must be changed to `#include <NeoPatterns.hpp>`,
 but only in your **main program (aka *.ino file with setup() and loop())**, like it is done in the examples.<br/>
In **all other files** you must use `#include <NeoPatterns.h>` etc., otherwise you will get tons of **"multiple definition"** errors.
Take care that all macros you define in your main program before `#include <NeoPatterns.hpp>` etc. ,
e.g. `DO_NOT_USE_MATH_PATTERNS` should also be specified before the *NeoPatterns.h* include,
otherwise the include may not work as expected!

# Compile options / macros for this library
To customize the library to different requirements, there are some compile options / macros available.<br/>

## NeoPixel
These macros must be defined in your program **before** the line `#include <NeoPixel.hpp>` or `#include MatrixNeoPixel.hpp` to take effect.<br/>
Modify them by enabling / disabling them, or change the values if applicable.

| Name | Default value | Description |
|-|-|-|
| `DO_NOT_SUPPORT_RGBW` | disabled | Disables RGBW pixels support. Activate it, if you only have RGB pixels. Saves up to 428 bytes program memory for the AllPatternsOnMultiDevices example. |
| `DO_NOT_SUPPORT_BRIGHTNESS` | disabled | Disables the brightness functions. Saves up to 428 bytes program memory for the AllPatternsOnMultiDevices example. |
| `DO_NOT_SUPPORT_NO_ZERO_BRIGHTNESS` | disabled | Disables the special brightness functions, which sets a dimmed pixel to 0 only if brightness or input color was zero, otherwise it is clipped at e.g. 0x000100. Saves up to 144 bytes program memory for the AllPatternsOnMultiDevices example. |
| `NEO_KHZ400` | 0x0100 | If you do not require the legacy 400 kHz functionality, you can disable the line 138 `#define NEO_KHZ400 0x0100 ///< 400 KHz data transmission` in Adafruit_NeoPixel.h. This saves up to 164 bytes program memory for the AllPatternsOnMultiDevices example. |

## NeoPatterns
These macros must be defined in your program **before** the line `#include <NeoPatterns.hpp>` or `#include MatrixNeoPatterns.hpp` or `#include MatrixSnake.hpp` to take effect.<br/>
Modify them by enabling / disabling them, or change the values if applicable.

| Name | Default value | Description |
|-|-|-|
| `ENABLE_PATTERN_<Pattern name>` | all | Selection of individual pattern(s) to be enabled for your program. You can specify multiple pattern. See [NeoPatterns.h](https://github.com/ArminJo/NeoPatterns/blob/master/src/NeoPatterns.h#L58-L77) |
| `ENABLE_MATRIX_PATTERN_<Pattern name>` | all | Selection of individual matrix pattern(s) to be enabled for your program. You can specify multiple pattern. See [MatrixNeoPatterns.h](https://github.com/ArminJo/NeoPatterns/blob/master/src/MatrixNeoPatterns.h#L41-L51) |
| `ENABLE_SPECIAL_PATTERN_<Pattern name>` | all | Selection of individual special pattern(s) (currently only snake pattern) to be enabled for your program. You can specify multiple pattern. See  [MatrixSnake.h](https://github.com/ArminJo/NeoPatterns/blob/master/src/MatrixSnake.h#L41-L48) |
| `ENABLE_NO_NEO_PATTERN_BY_DEFAULT` | disabled | Disables the default selection of all non matrix NeoPattern patterns if no ENABLE_PATTERN_<Pattern name> is specified. Enables the exclusively use compilation of matrix NeoPattern. |
| `ENABLE_NO_MATRIX_AND_NEO_PATTERN_BY_DEFAULT` | disabled | Disables default selection of all matrix and non matrix NeoPattern patterns if no ENABLE_PATTERN_<Pattern name> or ENABLE_MATRIX_PATTERN_<Pattern name> is specified. Thus it enables the exclusively use of special Snake pattern which saves program memory. |
| `DO_NOT_USE_MATH_PATTERNS` | disabled | Disables the `BOUNCING_BALL` pattern. Saves from 0 bytes up to 1140 bytes program memory, depending if floating point and sqrt() are already used otherwise. |
| `SUPPORT_ONLY_DEFAULT_GEOMETRY` | disabled | Disables other than default geometry, i.e. Pixel 0 is at bottom right of matrix, matrix is row major (horizontal) and same pixel order across each line (no zig-zag). Saves up to 560 bytes program memory and 3 bytes RAM. |

## Snake
These macros must be defined in your program **before** the line `#include MatrixSnake.hpp` to take effect.<br/>
Modify them by enabling / disabling them, or change the values if applicable.

| Name | Default value | Description |
|-|-|-|
| `ENABLE_PATTERNS_FOR_SNAKE_AUTORUN` | disabled | Selects all matrix and non matrix NeoPattern patterns used for the snake game. |
| `ENABLE_USER_SNAKE_SOLVER` | disabled | Disables the built in solver function getNextSnakeDirection() and enables the [user provided solver function](https://github.com/ArminJo/NeoPatterns/blob/master/examples/SnakeSolver/SnakeSolver.ino#L56). |

### Changing include (*.h) files with Arduino IDE
First, use *Sketch > Show Sketch Folder (Ctrl+K)*.<br/>
If you have not yet saved the example as your own sketch, then you are instantly in the right library folder.<br/>
Otherwise you have to navigate to the parallel `libraries` folder and select the library you want to access.<br/>
In both cases the library source and include files are located in the libraries `src` directory.<br/>
The modification must be renewed for each new library version!

### Modifying compile options / macros with PlatformIO
If you are using PlatformIO, you can define the macros in the *[platformio.ini](https://docs.platformio.org/en/latest/projectconf/section_env_build.html)* file with `build_flags = -D MACRO_NAME` or `build_flags = -D MACRO_NAME=macroValue`.

### Modifying compile options / macros with Sloeber IDE
If you are using [Sloeber](https://eclipse.baeyens.it) as your IDE, you can easily define global symbols with *Properties > Arduino > CompileOptions*.<br/>
![Sloeber settings](https://github.com/Arduino-IRremote/Arduino-IRremote/blob/master/pictures/SloeberDefineSymbols.png)

WOKWI online simulation of the AllPatternOnOneBar example.<br/>
[![WOKWI online simulation of the AllPatternOnOneBar example](https://github.com/ArminJo/NeoPatterns/blob/master/pictures/Wokwi_AllPatternOnOneBar.png)](https://wokwi.com/arduino/projects/299556508969992714).

WOKWI online simulation of the MatrixDemo example.<br/>
[![WOKWI online simulation of the MatrixDemo example](https://github.com/ArminJo/NeoPatterns/blob/master/pictures/Wokwi_MatrixDemo.png)](https://wokwi.com/arduino/projects/299560666027524617).

# Examples
## SnakeGame Example
The game can be controlled by 2 or 4 buttons or by serial input (WASD) on the keboard.<br/>
For keyboard control, start the Python script in the extras folder of the library with *RunPythonKeybordForInput.cmd*.
This script **sends a wasd key press immediately** and does not wait for a return or a press of the send button, as the Arduino Serial Monitor does.<br/>
The experimental script *Joystick2Serial.py* converts game controller input to appropriate serial output for the game.<br/>
After 7 seconds of inactivity after boot, the Snake demo with a simple AI is started.

SnakeGame with 4 buttons on breadboard
![SnakeGame with 4 buttons on breadboard](https://github.com/ArminJo/NeoPatterns/blob/master/pictures/SnakeGame.jpg)

## SnakeAutorun Example
**With the SnakeAutorun example you can prove your skill to write an AI to solve the Snake game. Just put your code into the getNextSnakeDirection() function.**

## AllPatternsOnMultiDevices
Shows all patterns for strips rings and matrixes included in the NeoPattern MatrixNeoPattern and Snake library.<br/>
Brightnes can be set by a voltage at pin A0.
Uses the included `allPatternsRandomHandler()` to [show all available patterns](https://www.youtube.com/watch?v=CsB7FkywCRQ).

AllPatternsOnMultiDevices on breadboard
![AllPatternsOnMultiDevices on breadboard](https://github.com/ArminJo/NeoPatterns/blob/master/pictures/Breadboard_complete.jpg)

## OpenLedRace
Extended version of the OpenLedRace "version Basic for PCB Rome Edition. 2 Player, without Boxes Track".<br/>
See also the [dedicated repository for OpenLedRace](https://github.com/ArminJo/OpenledRace).

OpenLedRace at the Cologne public library MINTköln-Festival
![OpenLedRace at the Cologne public library MINTköln-Festival](https://github.com/ArminJo/OpenledRace/blob/master/pictures/OpenLedRaceAtMintFestival.jpg)

## TwoPatternsOnOneStrip
This example renders a slow "background pattern" and a fast "foreground pattern" on the same strip.<br/>
It also shows, how to dynamically **determine the length of the attached strip** und to resize the underlying pixel buffer.

# Revision History
### Version 3.1.2 - work in progress

### Version 3.1.1
- Added parameter `aRepetitions` to pattern `RainbowCycle`.
- Improved layout of character c.
 
### Version 3.1.0
- Added functions `printConnectionInfo()`, `fillRegion()`, `stop()` and `stopAllPatterns()`.
- Fixed brightness initialization bug for Neopixel with UnderlyingNeoPixelObjects.
- Renamed `updateAll*` and `updateAndWait*` functions.
- Now all NeoPattern objects are contained in NeoPatterns list.
- Now `updateOrRedraw()` does never call `show()`.
- New pattern `FLASH`.
- Renamed ColorSet() to setColor().

### Version 3.0.0
- Enabled individual selection of patterns to save program memory.
- Renamed *NeoPatterns.cpp*, *MatrixNeoPatterns.cpp* and *MatrixSnake.cpp* to *NeoPatterns.hpp*, *MatrixNeoPatterns.hpp* and *MatrixSnake.hpp*.
- Renamed matrix pattern macros from `PATTERN_*` to `MATRIX_PATTERN_*`.
- Changed parameter for endless repeats in `initMultipleFallingStars()`.
- Improved usage of `random()`.
- Added function `fillRegion()`, `isActive()` and `setAdafruitBrightnessValue()`.
- Added support for brightness and brightness non zero mode.
- Fixed aDoUpdate bug for `FADE`.
- Fixed bugs in *Colors.h*.

### Version 2.4.0
- Added macros `ENABLE_PATTERN_<pattern_name>` to enable reducing size, if some patterns are not used.
- Renamed `NeoPatterns.cpp` to `NeoPatterns.hpp` to enable easy configuration by main program.
- Fixed brightness bug in `decrementTotalStepCounter()`.
- Changed parameter for endless repeats in `NeoPatterns::initMultipleFallingStars()`.
- Improved usage of random().
- Added function `NeoPixel::fillRegion()`, `NeoPatterns::isActive()` and `NeoPixel::setAdafruitBrightnessValue()`.
 
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
- Added support for RGBW patterns. Requires additional 200 bytes for the AllPatternsOnMultiDevices example. Deactivate the line `#define SUPPORT_RGBW` or defining `DO_NOT_SUPPORT_RGBW` saves 400 bytes program memory for the AllPatternsOnMultiDevices example.
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

## CI
The library examples are tested with GitHub Actions for the following boards:

- Arduino Uno
- Arduino Leonardo
- Arduino Mega 2560
- ESP8266 boards (tested with LOLIN D1 R2 board)
- ESP32   boards (tested with ESP32 DEVKITV1 board)

## Requests for modifications / extensions
Please write me a PM including your motivation/problem if you need a modification or an extension.

#### If you find this library useful, please give it a star.
