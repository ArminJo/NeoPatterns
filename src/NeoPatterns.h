#ifndef NEOPATTERNS_H
#define NEOPATTERNS_H

// Propagate debug level
#ifdef TRACE
#define DEBUG
#endif
#ifdef DEBUG
#define INFO
#endif
#ifdef INFO
#define WARN
#endif
#ifdef WARN
#define ERROR
#endif

#include "Adafruit_NeoPixel.h"
#include "Colors.h"

extern char VERSION_NEOPATTERNS[4];

// Pattern types supported:
enum pattern {
    NONE, RAINBOW_CYCLE, THEATER_CHASE, COLOR_WIPE, SCANNER, FADE,
    /* extensions */
    FIRE, CYLON, DELAY, MOVE, PROCESS_SELECTIVE, FADE_SELECTIVE, MOVING_PICTURE, TICKER, SNAKE, PATTERN1, PATTERN2,
};

/*
 * Values for Direction
 */
#define DIRECTION_UP 0
#define DIRECTION_LEFT 1
#define DIRECTION_DOWN 2
#define DIRECTION_RIGHT 3
#define NUMBER_OF_DIRECTIONS 4
#define DIRECTION_NONE 4    // No button pressed

// only for Backwards compatibility
#define FORWARD DIRECTION_UP
#define REVERSE DIRECTION_DOWN

uint8_t Red(color32_t color);
uint8_t Green(color32_t color);
uint8_t Blue(color32_t color);

// NeoPattern Class - derived from the Adafruit_NeoPixel class
class NeoPatterns: public Adafruit_NeoPixel {
public:
    NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t aTypeOfPixel, void (*aPatternCompletionCallback)(NeoPatterns*)=NULL);
    //
    void setCallback(void (*callback)(NeoPatterns*));
    //
    bool Update();
    void Increment();
    void Reverse();
    void setDirectionAndTotalStepsAndIndex(uint8_t aDirection, uint16_t totalSteps);
    void RainbowCycle(uint8_t interval, uint8_t aDirection = DIRECTION_UP);
    void TheaterChase(color32_t color1, color32_t color2, uint8_t interval, uint8_t aDirection = DIRECTION_UP);
    void ColorWipe(color32_t color, uint8_t interval, uint8_t aDirection = DIRECTION_UP);
    void Scanner(color32_t color1, uint8_t interval, uint8_t mode = 0);
    void Fade(color32_t color1, color32_t color2, uint16_t steps, uint8_t interval, uint8_t aDirection = DIRECTION_UP);
    uint32_t DimColor(color32_t color);
    void ColorSet(color32_t color);

    /*
     * Extensions
     */
    void Fire(uint16_t interval, uint16_t repetitions = 100);
    void Cylon(color32_t color1, uint16_t interval, uint8_t repetitions = 1);
    void Delay(uint16_t aMillis);
    void ProcessSelectiveColor(uint32_t (*aSingleLEDProcessingFunction)(NeoPatterns*), uint16_t steps, uint16_t interval);
    void FadeSelectiveColor(color32_t color1, color32_t color2, uint16_t steps, uint16_t interval);

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
    void ProcessSelectiveColorForAllPixelAndShow();
    void FadeSelectiveUpdate();

    // Member Variables:
    pattern ActivePattern;  // which pattern is running
    int8_t Direction;     // direction to run the pattern

    color32_t Color1, Color2;  // What colors are in use
#define PIXEL_NEEDS_3_BYTES 3
#define PIXEL_NEEDS_4_BYTES 4 // has also a byte for white
    uint8_t PixelColorStorageSize; // can be 3 or 4

    unsigned long Interval;   // milliseconds between updates
    unsigned long lastUpdate; // last update of position

    uint16_t TotalSteps;  // total number of steps in the pattern. will be compared with Index for basic patterns.
    uint16_t Index;  // Counter for basic pattern. Current step within the pattern. Counter for basic patterns. Step counter of snake.

    void (*OnPatternComplete)(NeoPatterns*);  // Callback on completion of pattern

    /*
     * Extensions
     */
#define GEOMETRY_BAR 1
    uint8_t PatternsGeometry; // fire pattern makes no sense on circles

    // For scanner extensions
    // Flags 0 -> old scanner starting at 0 - 2 passes
    // Flags 1 -> old scanner but starting at numPixels() -1 - one pass (falling star pattern)
    // Flags +2 -> starting at both ends
    // Flags +4 -> let scanner vanish complete (>=7 additional steps at the end)
#define FLAG_SCANNER_ONE_PASS               0x01
#define FLAG_SCANNER_STARTING_AT_BOTH_ENDS  0x02
#define FLAG_SCANNER_VANISH_COMPLETE        0x04
#define FLAG_SCANNER_FALLING_STAR           0x05

    uint8_t Flags;  // special behavior of the pattern
    // for Cylon, Fire, multipleHandler
    uint16_t Repetitions; // counter for multipleHandler

    color32_t ColorForSelection; // for FadeSelectiveColor, ProcessSelectiveColor
    uint32_t (*SingleLEDProcessingFunction)(NeoPatterns*); // for ProcessSelectiveColor
    /*
     * for multiple pattern extensions
     */
    uint8_t Duration;
    void (*NextOnPatternCompleteHandler)(NeoPatterns*);  // Next callback after completion of multiple pattern
};

//  Sample processing functions for ProcessSelectiveColor()
uint32_t FadeColor(NeoPatterns* aLedPtr);
uint32_t DimColor(NeoPatterns* aLedPtr);
uint32_t LightenColor(NeoPatterns* aLedPtr);

// multiple pattern example
void initFallingStar(NeoPatterns * aLedsPtr, color32_t aColor, uint8_t aDuration, uint8_t aRepetitions,
        void (*aNextOnCompleteHandler)(NeoPatterns*));
void multipleFallingStarCompleteHandler(NeoPatterns * aLedsPtr);

void allPatternsRandomExample(NeoPatterns * aLedsPtr);

int8_t checkAndTruncateParamValue(int8_t aParam, int8_t aParamMax, int8_t aParamMin);

#endif // NEOPATTERNS_H
