# NeoPatterns and Snake game for NeoPixel matrix
This is an extended version version of the NeoPattern example by Adafruit https://learn.adafruit.com/multi-tasking-the-arduino-part-3?view=all.
Extension are made to include more patterns, combined patterns and nxn NeoPixel matrix.

## Download
The actual version can be downloaded directly from GitHub [here](https://github.com/ArminJo/NeoPatterns/blob/master/extras/NeoPatterns.zip?raw=true)

# Installation
First you need to install "Adafruit NeoPixel" library with *Sketch -> Include Library -> Manage Librarys...*. Use "neoPixel" as filter string.  
Then download NeoPatterns.zip file or use the GitHub *clone or download -> Download ZIP* button, and add the .zip file with *Sketch -> Include Library -> add .ZIP Library...*.  

# PATTERNS
## Patterns from [Adafruit](https://www.adafruit.com/)
**RAINBOW_CYCLE**, **THEATER_CHASE**, **COLOR_WIPE**, **SCANNER**, **FADE**  
The SCANNER pattern is extended and now has 4 modes.
## Pattern from [FastLed](https://github.com/FastLED/FastLED)
**FIRE** adapted from https://github.com/FastLED/FastLED/tree/master/examples/Fire2012
## New patterns
 **CYLON**, **DELAY**, **PROCESS_SELECTIVE**, **FADE_SELECTIVE**
## Patterns for nxn Matrix
**MOVING_PICTURE**, **MOVE**, **TICKER**, **FIRE**, **SNAKE**

## All pixel mappings except NEO_MATRIX_COLUMNS supported
In case you need NEO_MATRIX_COLUMNS layout, try to rotate your Matrix and use NEO_MATRIX_ROWS or use your own custom mapping function.

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
The game can be controlled by 2 or 4 buttons or by serial input (WASD).
The experimental Python script in the extras folder converts key presses and game controller input to appropriate serial output for the game. After 7 seconds of inactivity it runs the Snake demo with a simple AI.
## SnakeAutorun Example
**With the SnakeAutorun example you can prove your skill to write an AI to solve the Snake game. Just put your code in the computeSnakeDirection() function.**


NeoPatterns on breadboard
![NeoPatterns on breadboard](https://github.com/ArminJo/NeoPatterns/blob/master/extras/Breadboard_complete.jpg)

Youtube Video of NeoPatternsDemo
[![Demonstration of NeoPatterns](https://i.ytimg.com/vi/CsB7FkywCRQ/hqdefault.jpg)](https://www.youtube.com/watch?v=CsB7FkywCRQ)

Youtube Video of MatrixDemo on a 10x10 matrix 
[![Demonstration of MatrixDemo on a 10x10 matrix](https://i.ytimg.com/vi/URsq28l2PEQ/hqdefault.jpg)](https://www.youtube.com/watch?v=URsq28l2PEQ)
