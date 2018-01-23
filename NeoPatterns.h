#ifndef NEOPATTERNS_H
#define NEOPATTERNS_H

#include "Adafruit_NeoPixel.h"

// Pattern types supported:
enum pattern {
    NONE, RAINBOW_CYCLE, THEATER_CHASE, COLOR_WIPE, SCANNER, FADE,
    /* extensions */
    FIRE, CYLON, DELAY, PROCESS_SELECTIVE, FADE_SELECTIVE, MOVING_PICTURE, TICKER
};

// Pattern directions supported:
enum direction {
    FORWARD, REVERSE, UP, DOWN
};

uint8_t Red(uint32_t color);
uint8_t Green(uint32_t color);
uint8_t Blue(uint32_t color);

// NeoPattern Class - derived from the Adafruit_NeoPixel class
class NeoPatterns: public Adafruit_NeoPixel {
public:
    NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t type, void (*callback)(NeoPatterns*));
    bool Update();
    void Increment();
    void Reverse();
    void setDirectionAndTotalStepsAndIndex(direction dir, uint16_t totalSteps);
    void RainbowCycle(uint8_t interval, direction dir = FORWARD);
    void TheaterChase(uint32_t color1, uint32_t color2, uint8_t interval, direction dir = FORWARD);
    void ColorWipe(uint32_t color, uint8_t interval, direction dir = FORWARD);
    void Scanner(uint32_t color1, uint8_t interval, uint8_t mode = 0);
    void Fade(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, direction dir = FORWARD);
    uint32_t DimColor(uint32_t color);
    void ColorSet(uint32_t color);

    /*
     * Extensions
     */
    void Fire(uint16_t interval, uint16_t repetitions = 100);
    void Cylon(uint32_t color1, uint16_t interval, uint8_t repetitions = 1);
    void Delay(uint16_t aMillis);
    void ProcessSelectiveColor(uint32_t (*aSingleLEDProcessingFunction)(NeoPatterns*), uint16_t steps, uint16_t interval);
    void FadeSelectiveColor(uint32_t color1, uint32_t color2, uint16_t steps, uint16_t interval);
    void MovingPicturePGM(const uint8_t* aGraphics8x8Array, uint32_t aForegroundColor, uint32_t aBackgroundColor, int8_t aYOffset,
            int8_t aXOffset, uint16_t steps, uint16_t interval, direction dir = UP);
    void TickerPGM(const char* aStringPtrPGM, uint32_t aForegroundColor, uint32_t aBackgroundColor, uint16_t interval,
            direction dir = FORWARD);
    void Ticker(const char* aStringPtr, uint32_t aForegroundColor, uint32_t aBackgroundColor, uint16_t interval, direction dir =
            FORWARD);
    void TickerInit(const char* aStringPtr, uint32_t aForegroundColor, uint32_t aBackgroundColor, uint16_t interval, direction dir =
            FORWARD, bool isPGMString = true);

    void loadPicturePGM(const uint8_t* aGraphics8x8ArrayPGM, uint32_t aForegroundColor, uint32_t aBackgroundColor, int8_t aYOffset =
            0, int8_t aXOffset = 0, bool doFilling = true, int8_t aNumberOfVerticalLinesToProcess = 8);

// Static functions
    static uint32_t Wheel(byte WheelPos);
    static uint32_t HeatColor(uint8_t aTemperature);

    void RainbowCycleUpdate();
    void TheaterChaseUpdate();
    void ColorWipeUpdate();
    void ScannerUpdate();
    void FadeUpdate();

    /*
     * Extensions
     */
    void FireUpdate();
    void CylonUpdate();
    void DelayUpdate();
    void ProcessSelectiveColorUpdate();
    void FadeSelectiveUpdate();
    void MovingPicturePGMUpdate();
    void TickerUpdate();

    // Member Variables:
    pattern ActivePattern;  // which pattern is running
    direction Direction;     // direction to run the pattern

    unsigned long Interval;   // milliseconds between updates
    unsigned long lastUpdate; // last update of position

    uint32_t Color1, Color2;  // What colors are in use
    uint16_t TotalSteps;  // total number of steps in the pattern
    uint16_t Index;  // current step within the pattern

    void (*OnComplete)(NeoPatterns*);  // Callback on completion of pattern

    /*
     * Extensions
     */
    bool isBar; // fire pattern makes no sense on circles

    // For scanner extensions
    // mode 0 -> old scanner starting at 0 - 2 passes
    // mode 1 -> old scanner but starting at numPixels() -1 - one pass (falling star pattern)
    // mode +2 -> starting at both ends
    // mode +4 -> let scanner vanish complete (>=7 additional steps at the end)
    uint8_t Mode;  // special behavior of the pattern - For Ticker: Flag if DataPtr points to RAM or FLASH
    // for Cylon, Fire, multipleHandler
    uint16_t Repetitions;
    // for picture and ticker extension
    const uint8_t* DataPtr; // can hold pointer to PGM or data space string or to PGM space 8x8 graphic array.
    int8_t GraphicsYOffset;
    int8_t GraphicsXOffset;
    uint32_t ColorForSelection; // for FadeSelectiveColor, ProcessSelectiveColor
    uint32_t (*SingleLEDProcessingFunction)(NeoPatterns*); // for ProcessSelectiveColor
    // for multiple pattern extensions
    void (*NextOnCompleteHandler)(NeoPatterns*);  // Next callback after completion of multiple pattern
};

// eases constant color declarations
#define COLOR(r,g,b)   ((uint32_t)(((uint32_t)r<<16)|((uint16_t)g<<8)|b)) // return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;

//  Sample processing functions for ProcessSelectiveColor()
uint32_t FadeColor(NeoPatterns* aLedPtr);
uint32_t DimColor(NeoPatterns* aLedPtr);
uint32_t LightenColor(NeoPatterns* aLedPtr);

// multiple pattern example
void initFallingStar(NeoPatterns * aLedsPtr, uint32_t aColor, uint8_t aRepetitions, void (*aNextOnCompleteHandler)(NeoPatterns*));
void multipleFallingStarCompleteHandler(NeoPatterns * aLedsPtr);

int8_t checkAndTruncateParamValue(int8_t aParam, int8_t aParamMax, int8_t aParamMin);

#endif // NEOPATTERNS_H
