/**
 * NeoPatterns.cpp
 *
 * This is an extended version version of the NeoPattern Example by Adafruit
 * https://learn.adafruit.com/multi-tasking-the-arduino-part-3?view=all
 * First you need to install "Adafruit NeoPixel" library under Sketch -> Include Library -> Manage Librarys... -> use "neo" as filter string
 * Extension are made to include more patterns, combined patterns and 8x8 NeoPixel matrix.
 *
 *  Copyright (C) 2018  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/gpl.html>.
 */
#include <Arduino.h>

#include "NeoPatterns.h"

// used for Ticker
#include "fonts.h"

/**********************************************************************************
 * Code from https://learn.adafruit.com/multi-tasking-the-arduino-part-3?view=all
 * Extended for added functionality
 **********************************************************************************/
// Constructor - calls base-class constructor to initialize strip
NeoPatterns::NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t type, void (*callback)(NeoPatterns*)) :
        Adafruit_NeoPixel(pixels, pin, type) {
    OnComplete = callback;
}

// Update the pattern returns true if update has happened in order to give the caller a chance to manually change parameters (like color etc.)
bool NeoPatterns::Update() {
    if ((millis() - lastUpdate) > Interval) {
        // time to update
        lastUpdate = millis();
        switch (ActivePattern) {
        case RAINBOW_CYCLE:
            RainbowCycleUpdate();
            break;
        case THEATER_CHASE:
            TheaterChaseUpdate();
            break;
        case COLOR_WIPE:
            ColorWipeUpdate();
            break;
        case SCANNER:
            ScannerUpdate();
            break;
        case FADE:
            FadeUpdate();
            break;
        case FADE_SELECTIVE:
            FadeSelectiveUpdate();
            break;
        case PROCESS_SELECTIVE:
            ProcessSelectiveColorUpdate();
            break;
        case CYLON:
            CylonUpdate();
            break;
        case FIRE:
            FireUpdate();
            break;
        case TICKER:
            TickerUpdate();
            break;
        case MOVING_PICTURE:
            MovingPicturePGMUpdate();
            break;
        case DELAY:
            DelayUpdate();
            break;
        default:
            break;
        }
        return true;
    }
    return false;
}

// Increment the Index and reset at the end
void NeoPatterns::Increment() {
    if (Direction == FORWARD) {
        Index++;
        if (Index >= TotalSteps) {
            Index = 0;
            if (OnComplete != NULL) {
                OnComplete(this); // call the completion callback
            }
        }
    } else // Direction == REVERSE
    {
        --Index;
        if (Index == 0xFFFF) {
            Index = TotalSteps - 1;
            if (OnComplete != NULL) {
                OnComplete(this); // call the completion callback
            }
        }
    }
}

// Reverse pattern direction
void NeoPatterns::Reverse() {
    if (Direction == FORWARD) {
        Direction = REVERSE;
        Index = TotalSteps - 1;
    } else {
        Direction = FORWARD;
        Index = 0;
    }
}

// Helper to set index accordingly to direction
void NeoPatterns::setDirectionAndTotalStepsAndIndex(direction dir, uint16_t totalSteps) {
    Direction = dir;
    TotalSteps = totalSteps;
    if (Direction == FORWARD) {
        Index = 0;
    } else {
        Index = totalSteps - 1;
    }
}

// Initialize for a RainbowCycle
void NeoPatterns::RainbowCycle(uint8_t interval, direction dir) {
    ActivePattern = RAINBOW_CYCLE;
    Interval = interval;
    setDirectionAndTotalStepsAndIndex(dir, 255);
}

// Update the Rainbow Cycle Pattern
void NeoPatterns::RainbowCycleUpdate() {
    for (uint16_t i = 0; i < numPixels(); i++) {
        setPixelColor(i, Wheel(((i * 256 / numPixels()) + Index) & 255));
    }
    show();
    Increment();
}

// Initialize for a Theater Chase
void NeoPatterns::TheaterChase(uint32_t color1, uint32_t color2, uint8_t interval, direction dir) {
    ActivePattern = THEATER_CHASE;
    Interval = interval;
    Color1 = color1;
    Color2 = color2;
    setDirectionAndTotalStepsAndIndex(dir, numPixels());
}

// Update the Theater Chase Pattern
void NeoPatterns::TheaterChaseUpdate() {
    for (uint16_t i = 0; i < numPixels(); i++) {
        if ((i + Index) % 3 == 0) {
            setPixelColor(i, Color1);
        } else {
            setPixelColor(i, Color2);
        }
    }
    show();
    Increment();
}

// Initialize for a ColorWipe
void NeoPatterns::ColorWipe(uint32_t color, uint8_t interval, direction dir) {
    ActivePattern = COLOR_WIPE;
    Interval = interval;
    Color1 = color;
    setDirectionAndTotalStepsAndIndex(dir, numPixels());
}

// Update the Color Wipe Pattern
void NeoPatterns::ColorWipeUpdate() {
    setPixelColor(Index, Color1);
    show();
    Increment();
}

// Initialize for a Fade
void NeoPatterns::Fade(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, direction dir) {
    ActivePattern = FADE;
    Interval = interval;
    TotalSteps = steps;
    Color1 = color1;
    Color2 = color2;
    Index = 0;
    Direction = dir;
}

// Update the Fade pattern
void NeoPatterns::FadeUpdate() {
// Calculate linear interpolation between Color1 and Color2
// Optimize order of operations to minimize truncation error
    uint8_t red = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
    uint8_t green = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
    uint8_t blue = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;

    ColorSet(Color(red, green, blue));
    show();
    Increment();
}

// Initialize for a SCANNNER
// extended with modes:
// mode 0 -> old scanner starting at 0 - 2 passes
// mode 1 -> old scanner but starting at numPixels() -1 - one pass (falling star pattern)
// mode +2 -> starting at both ends
// mode +4 -> let scanner vanish complete (>=7 additional steps at the end)
void NeoPatterns::Scanner(uint32_t color1, uint8_t interval, uint8_t mode) {
    clear();
    ActivePattern = SCANNER;
    TotalSteps = numPixels() + 42; // not really needed - for increment() it must be greater than numPixels() +1
    Interval = interval;
    Color1 = color1;
    Index = 0;
    Direction = FORWARD;
    if (mode & 0x01) {
        // only one pass -> falling star pattern
        Index = numPixels() - 1;
        Direction = REVERSE;
    }
    Mode = mode;
}

// Update the Scanner Pattern
void NeoPatterns::ScannerUpdate() {
    for (uint16_t i = 0; i < numPixels(); i++) {
        if (i == Index || ((Mode & 0x02) && (i + numPixels() - 1) == ((numPixels() - 1) * 2) - Index)) {
            // Scan Pixel forth (and simultaneously back, if Special == 1)
            setPixelColor(i, Color1);
        } else { // Fading tail
            setPixelColor(i, DimColor(getPixelColor(i)));
        }
    }

    if (Index >= numPixels() - 1) {
        Direction = REVERSE;
    }
    show();

    if ((Index == 0 || Index == 0xFFFF) && (Mode & 0x04) && Direction == REVERSE) {
        // let tail vanish
        Index = 0xFFFF;
        if (getPixelColor(0) == 0) {
            if (OnComplete != NULL) {
                OnComplete(this); // call the completion callback
            }
        }
    } else {
        Increment();
    }
}

// Calculate 50% dimmed version of a color (used by ScannerUpdate)
uint32_t NeoPatterns::DimColor(uint32_t color) {
// Shift R, G and B components one bit to the right
    uint32_t dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
    return dimColor;
}

// Set all pixels to a color (synchronously)
void NeoPatterns::ColorSet(uint32_t color) {
    for (uint16_t i = 0; i < numPixels(); i++) {
        setPixelColor(i, color);
    }
    show();
}

// Input a value 0 to 255 to get a color value.
// The colors are a transition r - g - b - back to r.
uint32_t NeoPatterns::Wheel(byte WheelPos) {
    WheelPos = 255 - WheelPos;
    if (WheelPos < 85) {
        return Color(255 - WheelPos * 3, 0, WheelPos * 3);
    } else if (WheelPos < 170) {
        WheelPos -= 85;
        return Color(0, WheelPos * 3, 255 - WheelPos * 3);
    } else {
        WheelPos -= 170;
        return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    }
}
/****************************************************************************
 * START OF EXTENSIONS
 ****************************************************************************/

/********************************************************
 * The original Fire code is from: Fire2012 by Mark Kriegsman, July 2012
 * https://github.com/FastLED/FastLED/tree/master/examples/Fire2012
 * Adapted to NeoPatterns Class
 ********************************************************/
// initialize for fire -> set all to zero
void NeoPatterns::Fire(uint16_t interval, uint16_t repetitions) {
    ActivePattern = FIRE;
    Interval = interval;
    Direction = FORWARD;
    Index = 0;
    Repetitions = repetitions;
    // reset colors
    ColorSet(0);
}

// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 55, suggested range 20-100
#define COOLING  55

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 120

// Update the Fire Pattern
void NeoPatterns::FireUpdate() {
    static byte heat[24];
    // Step 1.  Cool down every cell a little
    for (uint16_t i = 0; i < numPixels(); i++) {
        uint8_t tChill = random(((COOLING * 10) / numPixels()) + 2);
        if (tChill >= heat[i]) {
            heat[i] = 0;
        } else {
            heat[i] = heat[i] - tChill;
        }
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for (uint16_t k = numPixels() - 1; k >= 2; k--) {
        heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
    }

    // Step 3.  Randomly ignite one new 'spark' of heat near the bottom
    if (random(255) < SPARKING) {
        int y = random(numPixels() / 4);
        uint8_t tNewHeat = random(160, 255);
        if (heat[y] + tNewHeat < heat[y]) {
            heat[y] = 0xFF;
        } else {
            heat[y] += tNewHeat;
        }
    }

    // Step 4.  Map from heat cells to LED colors
    for (uint16_t j = 0; j < numPixels(); j++) {
        setPixelColor(j, HeatColor(heat[j]));
    }
    show();
    Repetitions--;
    if (Repetitions == 0) {
        if (OnComplete != NULL) {
            OnComplete(this); // call the completion callback
        }
    }
}

uint32_t NeoPatterns::HeatColor(uint8_t aTemperature) {

// Scale 'heat' down from 0-255 to 0-191,
// which can then be easily divided into three
// equal 'thirds' of 64 units each.
    uint8_t t192 = (aTemperature == 0) ? 0 : (((int) aTemperature * (int) (192)) >> 8) + 1;

// calculate a value that ramps up from
// zero to 255 in each 'third' of the scale.
    uint8_t heatramp = t192 & 0x3F; // 0..63
    heatramp <<= 2; // scale up to 0..252

// now figure out which third of the spectrum we're in:
    if (t192 & 0x80) {
        // we're in the hottest third
        return Adafruit_NeoPixel::Color(255, 255, heatramp);
//        heatcolor.r = 255; // full red
//        heatcolor.g = 255; // full green
//        heatcolor.b = heatramp; // ramp up blue

    } else if (t192 & 0x40) {
        // we're in the middle third
        return Adafruit_NeoPixel::Color(255, heatramp, 0);
//        heatcolor.r = 255; // full red
//        heatcolor.g = heatramp; // ramp up green
//        heatcolor.b = 0; // no blue

    } else {
        // we're in the coolest third
        return Adafruit_NeoPixel::Color(heatramp, 0, 0);
//        heatcolor.r = heatramp; // ramp up red
//        heatcolor.g = 0; // no green
//        heatcolor.b = 0; // no blue
    }
}

/********************************************************
 * End of code from: Fire2012 by Mark Kriegsman, July 2012
 ********************************************************/

// Initialize for a delay -> just keep the old pattern displayed
void NeoPatterns::Delay(uint16_t aMillis) {
    ActivePattern = DELAY;
    Interval = aMillis;
    //lastUpdate = millis();
    TotalSteps = 1;
    Index = 0;
    Direction = FORWARD;
}

// Update / End the Delay pattern
void NeoPatterns::DelayUpdate() {
    Increment();
}

// Initialize for a CYLON
void NeoPatterns::Cylon(uint32_t color1, uint16_t interval, uint8_t repetitions) {
    clear();
    ActivePattern = CYLON;
    Interval = interval;
    Direction = FORWARD;
    Index = 0;
    Repetitions = repetitions;
    uint8_t tStartIndex = 4;

    // Used as steps per direction
    TotalSteps = (numPixels() - 1) - (2 * tStartIndex);
    Color1 = color1;
    uint8_t i = tStartIndex - 1;
    uint8_t j = tStartIndex + 1;
    setPixelColor(i, color1);
    setPixelColor(tStartIndex, color1);
    setPixelColor(j, color1);
    i--;
    j++;
    color1 = DimColor(DimColor(color1));
    setPixelColor(i, color1);
    setPixelColor(j, color1);
    i--;
    j++;
    color1 = DimColor(DimColor(color1));
    setPixelColor(i, color1);
    setPixelColor(j, color1);
    i--;
    j++;
    color1 = DimColor(DimColor(color1));
    setPixelColor(i, color1);
    setPixelColor(j, color1);
    j++;
}

// Update the Cylon Pattern
void NeoPatterns::CylonUpdate() {
    if (Direction == FORWARD) {
        uint16_t i = numPixels() - 1;
        while (i > 0) {
            uint8_t t = i - 1;
            setPixelColor(i, getPixelColor(t));
            i = t;
        }
        setPixelColor(0, 0);
    } else {
        uint16_t i = 0;
        while (i < numPixels()) {
            uint8_t t = i + 1;
            setPixelColor(i, getPixelColor(t));
            i = t;
        }
        setPixelColor(i, 0);
    }
    show();
    Index++;
    // toggle directions and handle repetitions
    if (Index >= TotalSteps) {
        Index = 0;
        if (Direction == FORWARD) {
            Direction = REVERSE;
        } else {
            Direction = FORWARD;
            Repetitions--;
            if (Repetitions == 0) {
                if (OnComplete != NULL) {
                    OnComplete(this); // call the completion callback
                }
            }
        }
    }
}

/*
 * call provided processing routine only for pixel which have color equal to ColorForSelection
 */
void NeoPatterns::ProcessSelectiveColor(uint32_t (*aSingleLEDProcessingFunction)(NeoPatterns*), uint16_t steps, uint16_t interval) {
    ActivePattern = PROCESS_SELECTIVE;
    Interval = interval;
    TotalSteps = steps;
    Index = 0;
    SingleLEDProcessingFunction = aSingleLEDProcessingFunction;
    Direction = FORWARD;
}

/*
 * Process only pixels with ColorForSelection
 */
void NeoPatterns::ProcessSelectiveColorUpdate() {

    uint32_t tNewColor = SingleLEDProcessingFunction(this);
    for (uint16_t i = 0; i < numPixels(); i++) {
        uint32_t tOldColor = getPixelColor(i);
        if (tOldColor == ColorForSelection) {
            setPixelColor(i, tNewColor);
        }
    }
    show();
    ColorForSelection = tNewColor;
    Increment();
}

/*
 * fade only pixel which have color1
 */
void NeoPatterns::FadeSelectiveColor(uint32_t color1, uint32_t color2, uint16_t steps, uint16_t interval) {
    ActivePattern = FADE_SELECTIVE;
    Interval = interval;
    TotalSteps = steps + 1; // to include color2
    Color1 = color1;
    ColorForSelection = color1;
    Color2 = color2;
    Index = 1; // since start color is already displayed
}

// Update only pixels with the right color
void NeoPatterns::FadeSelectiveUpdate() {
// Calculate linear interpolation between Color1 and Color2
// Optimize order of operations to minimize truncation error
    uint16_t tOriginalTotalSteps = TotalSteps - 1;
    uint8_t red = ((Red(Color1) * (tOriginalTotalSteps - Index)) + (Red(Color2) * Index)) / tOriginalTotalSteps;
    uint8_t green = ((Green(Color1) * (tOriginalTotalSteps - Index)) + (Green(Color2) * Index)) / tOriginalTotalSteps;
    uint8_t blue = ((Blue(Color1) * (tOriginalTotalSteps - Index)) + (Blue(Color2) * Index)) / tOriginalTotalSteps;

    uint32_t tNewColor = Color(red, green, blue);
    for (uint16_t i = 0; i < numPixels(); i++) {
        uint32_t tOldColor = getPixelColor(i);
        if (tOldColor == ColorForSelection) {
            setPixelColor(i, tNewColor);
        }
    }
    show();
    ColorForSelection = tNewColor;
    Increment();
}

/*
 * direction FORWARD is from left to right
 */
void NeoPatterns::MovingPicturePGM(const uint8_t* aGraphics8x8ArrayPGM, uint32_t aForegroundColor, uint32_t aBackgroundColor,
        int8_t aGraphicsXOffset, int8_t aGraphicsYOffset, uint16_t steps, uint16_t interval, direction dir) {
    ActivePattern = MOVING_PICTURE;
    DataPtr = aGraphics8x8ArrayPGM;
    Color1 = aForegroundColor;
    Color2 = aBackgroundColor;
    GraphicsXOffset = checkAndTruncateParamValue(aGraphicsXOffset, 8, -8);
    GraphicsYOffset = checkAndTruncateParamValue(aGraphicsYOffset, 8, -8);
    Interval = interval;
    TotalSteps = steps;
    Index = 0;
    Direction = dir;
}

void NeoPatterns::MovingPicturePGMUpdate() {
    loadPicturePGM(DataPtr, Color1, Color2, GraphicsXOffset, GraphicsYOffset);
    if (Direction == UP) {
        GraphicsYOffset++;
    } else if (Direction == DOWN) {
        GraphicsYOffset--;
    } else if (Direction == FORWARD) {
        GraphicsXOffset++;
    } else if (Direction == REVERSE) {
        GraphicsXOffset--;
    }
    Index++;
    if (Index >= TotalSteps) {
        if (OnComplete != NULL) {
            OnComplete(this); // call the completion callback
        }
    }
}

/*
 * directionFORWARD is from right to left
 */
void NeoPatterns::Ticker(const char* aStringPtr, uint32_t aForegroundColor, uint32_t aBackgroundColor, uint16_t interval,
        direction dir) {
    TickerInit(aStringPtr, aForegroundColor, aBackgroundColor, interval, dir, false);
}
void NeoPatterns::TickerPGM(const char* aStringPtrPGM, uint32_t aForegroundColor, uint32_t aBackgroundColor, uint16_t interval,
        direction dir) {
    TickerInit(aStringPtrPGM, aForegroundColor, aBackgroundColor, interval, dir, true);
}
void NeoPatterns::TickerInit(const char* aStringPtr, uint32_t aForegroundColor, uint32_t aBackgroundColor, uint16_t interval,
        direction dir, bool isPGMString) {
    ActivePattern = TICKER;
    DataPtr = (const uint8_t*) aStringPtr;
    Color1 = aForegroundColor;
    Color2 = aBackgroundColor;
    Direction = dir;
    // Set start position for first character to move in
    GraphicsXOffset = 0;
    GraphicsYOffset = 0;
    if (dir == FORWARD) {
        GraphicsXOffset = FONT_WIDTH;
    } else if (dir == UP) {
        GraphicsYOffset = -8;
        if (FONT_WIDTH < 8) {
            // try to position fonts from right to middle
            GraphicsXOffset = -1;
        }
    }
    Mode = isPGMString;
    Interval = interval;
}

void NeoPatterns::TickerUpdate() {
    char tChar;
    char tNextChar;
    // left character
    if (Mode) {
        tChar = pgm_read_byte(DataPtr);
        tNextChar = pgm_read_byte(DataPtr + 1);
    } else {
        tChar = *DataPtr;
        tNextChar = *(DataPtr + 1);
    }

#ifdef DEBUG
    Serial.print(" tChar=");
    Serial.print(tChar);
    Serial.print(" tNextChar=");
    Serial.println(tNextChar);
#endif

    const uint8_t* tGraphics8x8ArrayPtr = &font_PGM[(tChar - FONT_START) * 8];
    // padding for the last character
    loadPicturePGM(tGraphics8x8ArrayPtr, Color1, Color2, GraphicsXOffset, GraphicsYOffset, (tNextChar == '\0'));
    if (tNextChar != '\0') {
        // next character moving in
        if (Direction == FORWARD && GraphicsXOffset < 0) {
            tGraphics8x8ArrayPtr = &font_PGM[(tNextChar - FONT_START) * 8];
            loadPicturePGM(tGraphics8x8ArrayPtr, Color1, Color2, FONT_WIDTH + GraphicsXOffset, GraphicsYOffset, false,
                    min(FONT_WIDTH, -GraphicsXOffset));
            // check if next next character can be displayed (needed for small font width)
            if (GraphicsXOffset + FONT_WIDTH < 0) {
                char tNextNextChar;
                if (Mode) {
                    tNextNextChar = pgm_read_byte(DataPtr + 2);
                } else {
                    tNextNextChar = *(DataPtr + 2);
                }
                if (tNextNextChar != '\0') {
                    tGraphics8x8ArrayPtr = &font_PGM[(tNextNextChar - FONT_START) * 8];
                    loadPicturePGM(tGraphics8x8ArrayPtr, Color1, Color2, 2 * FONT_WIDTH + GraphicsXOffset, GraphicsYOffset, false,
                            -GraphicsXOffset - FONT_WIDTH);
                }
            }
        }
        if (Direction == UP && GraphicsYOffset > 0) {
            tGraphics8x8ArrayPtr = &font_PGM[(tNextChar - FONT_START) * 8];
            loadPicturePGM(tGraphics8x8ArrayPtr, Color1, Color2, GraphicsXOffset, GraphicsYOffset - 8, false);
        }
    }
    /*
     * first character is moved out left so switch to next one
     */
    if (GraphicsXOffset == -8 || GraphicsYOffset == 8) {

        if (tNextChar == '\0') {
            if (OnComplete != NULL) {
                OnComplete(this); // call the completion callback
                return;
            }
        }
        // switch to next character
        DataPtr++;
        GraphicsXOffset = FONT_WIDTH - 8;
        GraphicsYOffset = 0;
        if (Direction == UP && FONT_WIDTH < 8) {
            // try to position fonts from right to middle
            GraphicsXOffset = -1;
        }
    }

    // shift offsets
    if (Direction == FORWARD) {
        GraphicsXOffset--;
    } else if (Direction == UP) {
        GraphicsYOffset++;
    }
}

/*
 * aYOffset positive -> picture is shift up and truncated and bottom is padded with aBackgroundColor
 *          negative picture is shift down and truncated and top lines are padded with aBackgroundColor
 * aXOffset positive -> picture is shift right and left lines are padded -> rightmost part of graphic is not displayed
 *          negative -> picture is shift left and right lines are padded
 * aNumberOfVerticalLinesToProcess lines from rightmost to process in pattern. Is needed for small fonts which have leftmost lines blank.
 */
void NeoPatterns::loadPicturePGM(const uint8_t* aGraphics8x8ArrayPtr, uint32_t aForegroundColor, uint32_t aBackgroundColor,
        int8_t aXOffset, int8_t aYOffset, bool doPadding, int8_t aNumberOfVerticalLinesToProcess) {
    if (numPixels() < 64) {
        return;
    }
#ifdef DEBUG
    Serial.print("aGraphicsPtr=");
    Serial.print((uint16_t) aGraphics8x8ArrayPtr, HEX);
    Serial.print(" aXOffset=");
    Serial.print(aXOffset);
    Serial.print(" aYOffset=");
    Serial.print(aYOffset);
    Serial.print(" aNumber=");
    Serial.println(aNumberOfVerticalLinesToProcess);
#endif
    /*
     * loop pixels from bottom to top (from index 0 to 63)
     */

    int tPixelIndex = 0;
    const uint8_t* tGraphicsPointer = aGraphics8x8ArrayPtr + 7;

    uint8_t tLinesFromGraphic = 8;
    checkAndTruncateParamValue(aXOffset, 8, -8);
    checkAndTruncateParamValue(aYOffset, 8, -8);

    /*
     * YOffset handling
     */
    if (aYOffset > 0) {
        // shift up
        if (doPadding) {
            // fill bottom lines with background color
            for (int i = 0; i < aYOffset; ++i) {
                for (uint8_t j = 0; j < 8; ++j) {
                    setPixelColor(tPixelIndex, aBackgroundColor);
                    tPixelIndex++;
                }
            }
        } else {
            tPixelIndex += 8 * aYOffset;
        }
        tLinesFromGraphic -= aYOffset;
    } else if (aYOffset < 0) {
        // shift down -> bottom line is taken from within the graphic
        tGraphicsPointer += +aYOffset;
        tLinesFromGraphic += aYOffset;
    }

    /*
     * XOffset handling
     * Use the upper 8 bit of a 16bit bitmask
     */
    uint16_t tBitmaskToStart = 0x0100;
    if (aXOffset > 0) {
        tBitmaskToStart = tBitmaskToStart << aXOffset;
    } else if (aXOffset < 0) {
        tBitmaskToStart = tBitmaskToStart >> -aXOffset;
    }

    for (uint8_t i = 0; i < tLinesFromGraphic; ++i) {
        uint8_t tLineBitPattern = pgm_read_byte(tGraphicsPointer);
        uint16_t tBitmaskExtended = tBitmaskToStart; // shifting mask
        int8_t tNumberOfVerticalLinesToProcess = aNumberOfVerticalLinesToProcess;

#ifdef DEBUG
        Serial.print("tGraphicsPointer=");
        Serial.print((uint16_t) tGraphicsPointer, HEX);
        Serial.print(" tLineBitPattern=");
        Serial.println(tLineBitPattern, HEX);
#endif
        /*
         * Process one horizontal line
         */
        for (int j = 0; j < 8; ++j) {
            uint8_t tBitmask = tBitmaskExtended >> 8;
            if (tBitmask != 00 && tNumberOfVerticalLinesToProcess > 0) {
                if (tBitmask & tLineBitPattern) {
                    // bit in pattern is 1
                    setPixelColor(tPixelIndex, aForegroundColor);
                } else {
                    setPixelColor(tPixelIndex, aBackgroundColor);
                }
#ifdef DEBUG
                Serial.print(" tBitmask=");
                Serial.print(tBitmask);
                Serial.print(" tPixelIndex=");
                Serial.println(tPixelIndex);
#endif
                tNumberOfVerticalLinesToProcess--;
            } else if (doPadding) {
                // padding left
                setPixelColor(tPixelIndex, aBackgroundColor);
            }
            tBitmaskExtended = tBitmaskExtended << 1;
            tPixelIndex++;
        }
        tGraphicsPointer--;
    }

    if (aYOffset < 0 && doPadding) {
        // fill top lines with background color
        for (int i = aYOffset; i < 0; ++i) {
            for (uint8_t j = 0; j < 8; ++j) {
                setPixelColor(tPixelIndex, aBackgroundColor);
                tPixelIndex++;
            }
        }
    }
    show();
}

/***********************************************************
 * Sample processing functions for ProcessSelectiveColor()
 ***********************************************************/
uint32_t FadeColor(NeoPatterns* aLedPtr) {
    uint16_t tIndex = aLedPtr->Index + 1;
    uint16_t tTotalSteps = aLedPtr->TotalSteps;
    uint32_t tColor1 = aLedPtr->Color1;
    uint32_t tColor2 = aLedPtr->Color2;
    uint8_t red = ((Red(tColor1) * (tTotalSteps - tIndex)) + (Red(tColor2) * tIndex)) / tTotalSteps;
    uint8_t green = ((Green(tColor1) * (tTotalSteps - tIndex)) + (Green(tColor2) * tIndex)) / tTotalSteps;
    uint8_t blue = ((Blue(tColor1) * (tTotalSteps - tIndex)) + (Blue(tColor2) * tIndex)) / tTotalSteps;
    return NeoPatterns::Color(red, green, blue);
    // return COLOR(red, green, blue);
}

uint32_t DimColor(NeoPatterns* aLedPtr) {
    uint32_t tColor = aLedPtr->ColorForSelection;
    uint8_t red = Red(tColor) >> 1;
    uint8_t green = Green(tColor) >> 1;
    uint8_t blue = Blue(tColor) >> 1;
    // call to function saves 76 byte program space
    return NeoPatterns::Color(red, green, blue);
//    return COLOR(red, green, blue);
}

uint32_t LightenColor(NeoPatterns* aLedPtr) {
    uint32_t tColor = aLedPtr->ColorForSelection;
    uint8_t red = Red(tColor) << 1;
    uint8_t green = Green(tColor) << 1;
    uint8_t blue = Blue(tColor) << 1;
    // call to function saves 44 byte program space
    return NeoPatterns::Color(red, green, blue);
//    return COLOR(red, green, blue);
}

/*****************************************************************
 * COMBINED PATTERN EXAMPLE
 * overwrites the OnComplete Handler pointer and sets it
 * to aNextOnComplete after completion of combined patterns
 *****************************************************************/
// initialize for falling star (scanner with delay after pattern)
void initFallingStar(NeoPatterns * aLedsPtr, uint32_t aColor, uint8_t aRepetitions, void (*aNextOnCompleteHandler)(NeoPatterns*)) {
    uint8_t tDuration = random(20, 41);

    aLedsPtr->Color2 = aColor;
    aLedsPtr->Repetitions = (aRepetitions * 2) - 1;
    aLedsPtr->OnComplete = &multipleFallingStarCompleteHandler;
    aLedsPtr->NextOnCompleteHandler = aNextOnCompleteHandler;

    /*
     * Start with one scanner
     */
    aLedsPtr->Scanner(aColor, tDuration, 5);
}

/*
 * if all falling stars are completed switch back to NextOnComplete
 */
void multipleFallingStarCompleteHandler(NeoPatterns * aLedsPtr) {
    uint8_t tDuration = random(20, 41);
    uint16_t tRepetitions = aLedsPtr->Repetitions;
    if (tRepetitions == 1) {
        // perform delay and then switch back to NextOnComplete
        aLedsPtr->Delay(tDuration * 2);
        aLedsPtr->OnComplete = aLedsPtr->NextOnCompleteHandler;
    } else {
        /*
         * Next falling star
         */
        tRepetitions = tRepetitions % 2;
        if (tRepetitions == 1) {
            // for odd Repetitions 3,5,7, etc. -> do delay
            aLedsPtr->Delay(tDuration * 2);
        } else {
            // for even Repetitions 2,4,6, etc. -> do scanner
            aLedsPtr->Scanner(aLedsPtr->Color2, tDuration, 5);
        }
        aLedsPtr->Repetitions--;
    }
}

// Returns the Red component of a 32-bit color
uint8_t Red(uint32_t color) {
    return (color >> 16) & 0xFF;
}

// Returns the Green component of a 32-bit color
uint8_t Green(uint32_t color) {
    return (color >> 8) & 0xFF;
}

// Returns the Blue component of a 32-bit color
uint8_t Blue(uint32_t color) {
    return color & 0xFF;
}

int8_t checkAndTruncateParamValue(int8_t aParam, int8_t aParamMax, int8_t aParamMin) {
    if (aParam > aParamMax) {
        aParam = aParamMax;
    } else if (aParam < aParamMin) {
        aParam = aParamMin;
    }
    return aParam;
}

