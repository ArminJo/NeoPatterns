/*
 *  SoundToNeoPixelVU.ino
 *
 *  Copyright (C) 2018-2026 Armin Joachimsmeyer
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <https://www.gnu.org/licenses/gpl-3.0.html>.
 *
 */

#include <Arduino.h>

#define CALLS_PER_PEAK_DROP     (DEFAULT_CALLS_PER_PEAK_DROP * 2)  // we have 20 ms loop Peak drop so use double the default (6) value

//#define DEBUG

// include order matters for Eclipse indexer to work correct :-(
#include "NeoPixelVU.h"
#include "MatrixNeoPixelVU.h"

#include "digitalWriteFast.h"
#include "AVRUtils.h"

#include "ffft.h"

#include "MatrixNeoPixelVU.hpp" // must be last for Eclipse indexer to work correct :-(

#define VERSION_EXAMPLE "3.0"

// This block must be located after the includes of other *.hpp files
//#define LOCAL_DEBUG // This enables debug output only for this file - only for development
#include "LocalDebugLevelStart.h"

#define ADC_CHANNEL 0

#define NUMBER_OF_BAR_PIXELS   16  // Number of pixels on neopixel bar
#define VU_BAR_AVERAGE_PIN      3
#define VU_BAR_PEAK0_PIN        4
#define VU_BAR_PEAK2_PIN        5
#define VU_BAR_LOG_PIN          6
#define NEO_PIXEL_MATRIX_PIN    8

#define NOISE_MEASUREMENT_PIN   9 // if low a noise measurements is taken
#define TIMING_PIN              10 // set at start if ISR and reset at end
// If connected to ground, verbose output for Arduino Serial Monitor is enabled. This is not suitable for Arduino Plotter.
#define ONLY_PLOTTER_OUTPUT_PIN 11 // Verbose output to Arduino Serial Monitor is disabled, if connected to ground. This is intended for Arduino Plotter mode.
bool sOnlyPlotterOutput;

/*
 * do not mark them as volatile, since the values are only accessed by loop when acquisition has finished
 */
struct AudioBufferInfoStruct {
    int16_t AudioSampleBuffer[FFT_N];  // Audio capture buffer for values from -512 to + 511. FFT_N is 128 by default
    int16_t *AudioSampleBufferPointer;  // Next buffer position
    /*
     * Raw 10 bit values
     */
    uint16_t ValueMin;
    uint16_t ValueMax;
    uint16_t PeakToPeakValue;

    uint16_t MidValue; // ValueMin + (1/2 * PeakToPeakValue)
    uint16_t RawAverageValue; // not so noisy, but inaccurate at clipped input
    uint32_t AccumulatorForRawAverage;
    uint32_t RawAverageLowPass5Shift8; // Value with better resolution
    uint16_t RawZeroValue; // := RawAverageLowPass5

    /*
     * Volume value := Raw - Noise
     */
    uint8_t NoiseAverageValue;
    uint8_t NoisePeakValue;
    uint16_t VolumePeakValue; // PeakToPeakValue/2 - NoiseValue

    uint16_t VolumeAverageValue; // not so noisy as VolumePeakValue
    uint32_t AccumulatorForVolumeAverage;
} AudioBufferInfo;

complex_t butterfly_buffer[FFT_N];  // FFT "butterfly" buffer
//64 to 128 bytes are missing
uint16_t spectrum[FFT_N / 2]; // Spectrum output buffer
/*
 These tables were arrived at through testing, modeling and trial and error,
 exposing the unit to assorted music and sounds.  But there's no One Perfect
 EQ Setting to Rule Them All, and the graph may respond better to some
 inputs than others.  The software works at making the graph interesting,
 but some columns will always be less lively than others, especially
 comparing live speech against ambient music of varying genres.
 */
// This is low-level noise that's subtracted from each FFT output column:
static uint8_t noise[64] = { 8, 6, 6, 5, 3, 4, 4, 4, 3, 4, 4, 3, 2, 3, 3, 4, 2, 1, 2, 1, 3, 2, 3, 2, 1, 2, 3, 1, 2, 3, 4, 4, 3, 2,
        2, 2, 2, 2, 2, 1, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 4 };
static const uint8_t PROGMEM // @suppress("Multiple variable declaration")
// These are scaling quotients for each FFT output column, sort of a
// graphic EQ in reverse.  Most music is pretty heavy at the bass end.
eq[64] = { 255, 175, 218, 225, 220, 198, 147, 99, 68, 47, 33, 22, 14, 8, 4, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        // When filtering down to 8 columns, these tables contain indexes
        // and weightings of the FFT spectrum output values to use.  Not all
        // buckets are used -- the bottom-most and several at the top are
        // either noisy or out of range or generally not good for a graph.
        col0data[] = { 2, 1,  // # of spectrum bins to merge, index of first
                111, 8 },           // Weights for each bin
        col1data[] = { 4, 1,  // 4 bins, starting at index 1
                19, 186, 38, 2 }, // Weights for 4 bins.  Got it now?
        col2data[] = { 5, 2, 11, 156, 118, 16, 1 }, //
        col3data[] = { 8, 3, 5, 55, 165, 164, 71, 18, 4, 1 }, //
        col4data[] = { 11, 5, 3, 24, 89, 169, 178, 118, 54, 20, 6, 2, 1 }, //
        col5data[] = { 17, 7, 2, 9, 29, 70, 125, 172, 185, 162, 118, 74, 41, 21, 10, 5, 2, 1, 1 }, //
        col6data[] = { 25, 11, 1, 4, 11, 25, 49, 83, 121, 156, 180, 185, 174, 149, 118, 87, 60, 40, 25, 16, 10, 6, 4, 2, 1, 1, 1 },
        col7data[] = { 37, 16, 1, 2, 5, 10, 18, 30, 46, 67, 92, 118, 143, 164, 179, 185, 184, 174, 158, 139, 118, 97, 77, 60, 45,
                34, 25, 18, 13, 9, 7, 5, 3, 2, 2, 1, 1, 1, 1 },
        // And then this points to the start of the data for each of the columns:
        *const colData[] = { col0data, col1data, col2data, col3data, col4data, col5data, col6data, col7data };
int colDiv[8];    // Used when filtering FFT output to 8 columns

/*
 * The central NeoPixel object used for every bar, since there is no persistence needed for the bar pixel content
 */
NeoPixel LEDBar = NeoPixel(NUMBER_OF_BAR_PIXELS, VU_BAR_AVERAGE_PIN, NEO_GRB + NEO_KHZ800);

color16_t BarColor16ArrayRYG[NUMBER_OF_BAR_PIXELS];
color16_t BarColor16ArrayGreenToRed[NUMBER_OF_BAR_PIXELS];
// Use the same NeoPixel object for all bars, since there is no persistence needed for the bar pixel content
NeoPixelVU VULinearAverageScaled(VU_BAR_AVERAGE_PIN, &LEDBar, 1, FLAG_VU_SCALED_LINEAR_AUTOMATIC, BarColor16ArrayRYG);
NeoPixelVU VULinearPeakScaled(VU_BAR_PEAK0_PIN, &LEDBar, 2, FLAG_VU_SCALED_LINEAR_AUTOMATIC, BarColor16ArrayRYG);
NeoPixelVU VULinearDamp2Scaled(VU_BAR_PEAK2_PIN, &LEDBar, 3, FLAG_VU_SCALED_LINEAR_AUTOMATIC, BarColor16ArrayGreenToRed);
NeoPixelVU VULog(VU_BAR_LOG_PIN, &LEDBar, 2, FLAG_VU_USE_VALUE_LOG, BarColor16ArrayGreenToRed, 512);

/*
 * The 8x8 VUMatrix
 */
color32_t MatrixBarColorArray[8];
MatrixNeoPixelVU VUMatrix;

void initSampling();
void inline startSampling();
void waitForSamplingToEnd();
void setNoiseValues();

//Helper macro for getting a macro definition as string
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(ONLY_PLOTTER_OUTPUT_PIN, INPUT_PULLUP);
    pinMode(NOISE_MEASUREMENT_PIN, INPUT_PULLUP);
    pinMode(TIMING_PIN, OUTPUT);

    initStackFreeMeasurement();

    Serial.begin(115200);
#if defined(__AVR_ATmega32U4__) || defined(SERIAL_USB) || defined(SERIAL_PORT_USBVIRTUAL)
    delay(2000); // To be able to connect Serial monitor after reset and before first printout
#endif
    // Just to know which program is running on my Arduino
    Serial.println(F("START " __FILE__ "\r\nVersion " VERSION_EXAMPLE " from " __DATE__));
    Serial.println(
            F(
                    "Connect pin " STR(ONLY_PLOTTER_OUTPUT_PIN) " to ground, to see the 8 FFT channels and 2 input values in the 1.x Arduino plotter"));
    sOnlyPlotterOutput = !digitalRead(ONLY_PLOTTER_OUTPUT_PIN);

    if (!sOnlyPlotterOutput) {
#if defined(LOCAL_DEBUG)
        printRAMAndStackInfo(&Serial);
#endif
        Serial.println(F("Initialize bar and matrix"));
    }

    // initialize the central NeoPixel object
    LEDBar.begin();
    // Initialize bar colors. Use only one of the bars, since they all share the same BarColorArray
    if (!sOnlyPlotterOutput) {
        Serial.println(F("Initialize bar colors 2x RedYellowGreen, 2x GreenToRed and RYG for Matrix"));
    }
    VULinearAverageScaled.initBarColorRYG16();
    VULinearDamp2Scaled.initBarGreenToRed16();

    /*
     * This initializes the VUMatrix and checks if enough memory was available
     */
    if (!VUMatrix.init(8, 8, NEO_PIXEL_MATRIX_PIN, NEO_MATRIX_BOTTOM | NEO_MATRIX_RIGHT | NEO_MATRIX_ROWS | NEO_MATRIX_PROGRESSIVE,
    NEO_GRB + NEO_KHZ800, FACTOR_FOR_NO_DAMPENING, FLAG_VU_SCALED_LINEAR_AUTOMATIC,
            MatrixBarColorArray, &Serial)) {

        // Blink forever
        while (true) {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(500);
            digitalWrite(LED_BUILTIN, LOW);
            delay(500);
        }
    }
    VUMatrix.initBarColorRYG(); // must be after VUMatrix.init and does nothing if no array attached at init

#if defined(LOCAL_DEBUG)
    if (!sOnlyPlotterOutput) {
        printRAMAndStackInfo(&Serial);
    }
#endif

// Rainbow, in order to see all colors, but take care that the 5 volt supply is sufficient
//    VUMatrix.fillWithRainbow(0);
//    VUMatrix.show();
//    delay(2000);

    /*
     * Fill colDiv
     */
    for (uint_fast8_t i = 0; i < 8; i++) {
        uint8_t *data = (uint8_t*) pgm_read_word(&colData[i]);
        uint8_t nBins = pgm_read_byte(&data[0]) + 2;
        colDiv[i] = 0;
        for (uint_fast8_t j = 2; j < nBins; j++) {
            colDiv[i] += pgm_read_byte(&data[j]);
        }
    }

    delay(300); // to avoid partial patterns and audio signal at power up
    // Print Arduino Plotter caption
    if (sOnlyPlotterOutput) {
        Serial.println(F("Matrix_ValueLeft_1 2 3 4 5 6 7 8 Average Peak"));
    }

    // Wait for 1. audio sampling to finish to settle ADC etc.
    initSampling();
    waitForSamplingToEnd(); // 13.312 ms for 128 samples

// Wait for 2. audio sampling to finish to get values for initializing statistics
    startSampling();
    waitForSamplingToEnd(); // 13.312 ms for 128 samples

    // initialize statistics
    AudioBufferInfo.RawAverageLowPass5Shift8 = AudioBufferInfo.AccumulatorForRawAverage * (256 / FFT_N);
    AudioBufferInfo.RawZeroValue = AudioBufferInfo.RawAverageLowPass5Shift8 >> 8;

    // Start first sample with initialized values for display in loop
    startSampling();
    waitForSamplingToEnd(); // 13.312 ms for 128 samples
}

bool sNoiseMeasurementIsActive = false;
uint_fast8_t sNoiseMeasurementCounter; // count from 16 to 0 for 16 noise samples
uint16_t sNoiseAverageValueMaximum;
uint16_t sNoisePeakValueMaximum;
/*
 * 19 ms for one loop
 */
void loop() {

    /*
     * Here sampling is complete and Raw buffer is filled
     */
    fft_input(AudioBufferInfo.AudioSampleBuffer, butterfly_buffer);   // Samples -> complex #s + hamming window 0.7 ms

    /*
     * Check for reading of noise floor
     * after button release we wait for 500 ms before
     */
    if (digitalRead(NOISE_MEASUREMENT_PIN) == LOW) {
        delay(100); // debounce
        while (digitalRead(NOISE_MEASUREMENT_PIN) == LOW) {
            ; // wait until button release
        }
        if (!sOnlyPlotterOutput) {
            Serial.println(F("FFT noise measurement will start in 1/2 s"));
        }
        for (uint_fast8_t x = 0; x < FFT_N / 2; x++) {
            noise[x] = 0;
        }
        sNoiseAverageValueMaximum = 0;
        AudioBufferInfo.NoiseAverageValue = 0;
        sNoisePeakValueMaximum = 0;
        AudioBufferInfo.NoisePeakValue = 0;
        delay(500);
        sNoiseMeasurementIsActive = true;
        sNoiseMeasurementCounter = 16;
    }

    /*
     * Start a new sampling by ISR and disable millis() and delay()
     * The CPU time not used by sampling is used for computation
     */
    startSampling();

    fft_execute(butterfly_buffer);          // Process complex data
    fft_output(butterfly_buffer, spectrum);          // Complex -> spectrum

    if (sNoiseMeasurementIsActive) {
        // Add to noise
        for (uint_fast8_t x = 0; x < FFT_N / 2; x++) {
            uint16_t tSum = noise[x] + spectrum[x];
            noise[x] = (tSum > 0xFF) ? 0xFF : tSum;
        }
        waitForSamplingToEnd(); // Volume values are valid only after the ISR has finished, and the buffer must not be read while it is filled
        sNoiseAverageValueMaximum = max(sNoiseAverageValueMaximum, AudioBufferInfo.VolumeAverageValue);
        sNoisePeakValueMaximum = max(sNoisePeakValueMaximum, AudioBufferInfo.VolumePeakValue);
        sNoiseMeasurementCounter--;
        if (sNoiseMeasurementCounter == 0) {
            sNoiseMeasurementIsActive = false;
            setNoiseValues();
        }
    } else {
        // Remove noise and apply EQ levels
        for (uint_fast8_t x = 0; x < FFT_N / 2; x++) {
            uint8_t tNoise = noise[x];
            spectrum[x] = (spectrum[x] <= tNoise) ? 0 : (((spectrum[x] - tNoise) * (256L - pgm_read_byte(&eq[x]))) >> 8);
        }

        // Downsample spectrum output to 8 columns:
        for (uint_fast8_t i = 0; i < 8; i++) {
            uint8_t *data = (uint8_t*) pgm_read_word(&colData[i]);
            uint8_t nBins = pgm_read_byte(&data[0]) + 2;
            uint8_t binNum = pgm_read_byte(&data[1]);

            // Weighted Average
            int32_t sum = 0;
            for (uint_fast8_t j = 2; j < nBins; j++) {
                sum += (uint32_t) spectrum[binNum++] * pgm_read_byte(&data[j]);
            }
            int tValue = sum / colDiv[i];

            if (sOnlyPlotterOutput) {
                // print matrix values
                Serial.print(tValue);
                Serial.print(' ');
            }
            VUMatrix.setBarAndPeak(i, tValue / 2);
        }

        /*
         * Wait for sampling to end and then output bar values, since Neopixel output blocks ADC.
         */
        waitForSamplingToEnd(); // 13.312 ms for 128 samples
        VUMatrix.show(); // 2 ms
        VULinearAverageScaled.setBarAndPeakAndShow(AudioBufferInfo.VolumeAverageValue); // 0.8 ms
        VULinearPeakScaled.setBarAndPeakAndShow(AudioBufferInfo.VolumePeakValue);
        VULinearDamp2Scaled.setBarAndPeakAndShow(AudioBufferInfo.VolumePeakValue);
        VULog.setBarAndPeakAndShow(AudioBufferInfo.VolumePeakValue);

        if (sOnlyPlotterOutput) {
            // print global values
            Serial.print(' ');
            Serial.print(AudioBufferInfo.VolumeAverageValue);
            Serial.print(' ');
            Serial.print(AudioBufferInfo.VolumePeakValue);
            Serial.println();
        }
    }
}

void setNoiseValues() {
    if (AudioBufferInfo.VolumeAverageValue > 32) {
        if (!sOnlyPlotterOutput) {
            Serial.print(F("Average value "));
            Serial.print(AudioBufferInfo.VolumeAverageValue);
            Serial.println(F(" too high. No noise correction."));
        }
    } else {
        if (!sOnlyPlotterOutput) {
            Serial.print(F("FFT 16 times accumulated noise:"));
        }
        for (uint_fast8_t x = 0; x < FFT_N / 2; x++) {
            uint8_t tNewNoise = (noise[x] + 8) >> 4;
            if (!sOnlyPlotterOutput) {
                Serial.print(' ');
                Serial.print(noise[x]);
                Serial.print('|');
                Serial.print(tNewNoise);
            }
            noise[x] = tNewNoise;
        }
        AudioBufferInfo.NoiseAverageValue = sNoiseAverageValueMaximum;
        AudioBufferInfo.NoisePeakValue = sNoisePeakValueMaximum;
        if (!sOnlyPlotterOutput) {
            Serial.println();
            Serial.print(F("Volume noise set to="));
            Serial.print(AudioBufferInfo.NoiseAverageValue);
            Serial.print(F(" P2P noise set to="));
            Serial.println(AudioBufferInfo.NoisePeakValue);
        }
    }
}

uint8_t sOriginalTIMSK0;

/*
 * 9615 Hz sampling frequency
 * 13.312 ms for 128 samples
 */
void initSampling() {
    sOriginalTIMSK0 = TIMSK0;
// Init ADC free-run mode; f = ( 16 MHz/prescaler ) / 13 cycles/conversion
    AudioBufferInfo.AudioSampleBufferPointer = &AudioBufferInfo.AudioSampleBuffer[0];

    ADMUX = ADC_CHANNEL | (INTERNAL << REFS0); // Channel sel, right-adj, use 1.1 V reference
    ADCSRA = _BV(ADEN) | // ADC enable
            _BV(ADSC) |  // ADC start
            _BV(ADATE) | // Auto trigger
            _BV(ADIE) |  // Interrupt enable
            _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);   // prescaler is 128. (16 / 128) / 13 = 9615 Hz sampling frequency
    ADCSRB = 0;                                     // Free run mode, no high MUX bit
    DIDR0 = _BV(ADC_CHANNEL);                       // Turn off digital input for ADC pin
}

/*
 * Timer0 interrupts off => no millis() and delay()
 * This is reverted at end of sampling
 */
void inline startSampling() {
    ADCSRA |= _BV(ADIE);
    TIMSK0 = 0; // Timer0 interrupts off
}

void waitForSamplingToEnd() {
// Wait for audio sampling to finish
    while (ADCSRA & _BV(ADIE)) {
        ;
    }
}

/*
 * collect audio samples and compute some values on the fly
 * 6 us + push + pop every 104 us at 16 MHz
 * 13.312 ms for 128 samples
 */
ISR(ADC_vect) { // Audio-sampling interrupt
    uint16_t ADCValue = ADCW; // 0-1023
    digitalWriteFast(TIMING_PIN, HIGH);

    /*
     * Min + max computation
     */
    if (AudioBufferInfo.AudioSampleBufferPointer == &AudioBufferInfo.AudioSampleBuffer[0]) {
        // first sample
        AudioBufferInfo.ValueMin = ADCValue;
        AudioBufferInfo.ValueMax = ADCValue;
        AudioBufferInfo.AccumulatorForRawAverage = 0;
        AudioBufferInfo.AccumulatorForVolumeAverage = 0;
    } else if (ADCValue > AudioBufferInfo.ValueMax) {
        AudioBufferInfo.ValueMax = ADCValue;
    } else if (ADCValue < AudioBufferInfo.ValueMin) {
        AudioBufferInfo.ValueMin = ADCValue;
    }

// raw average
    AudioBufferInfo.AccumulatorForRawAverage += ADCValue;

// Sign-convert and store for FFT; -512 to +511
    int16_t tSignal = ADCValue - AudioBufferInfo.RawZeroValue;
    *AudioBufferInfo.AudioSampleBufferPointer = tSignal;
    AudioBufferInfo.AudioSampleBufferPointer++;

    /*
     * Rectify and add for volume average
     */
    uint16_t tRectifiedSignal = tSignal; // using this saves 4 bytes
    if (tSignal < 0) {
        tRectifiedSignal = -tSignal;
    }
    AudioBufferInfo.AccumulatorForVolumeAverage += tRectifiedSignal;

    if (AudioBufferInfo.AudioSampleBufferPointer >= &AudioBufferInfo.AudioSampleBuffer[FFT_N]) {
        /*
         * Buffer full -> Disable interrupt compute values and prepare for next run
         */
        ADCSRA &= ~_BV(ADIE);
        TIMSK0 = sOriginalTIMSK0; // restore Timer0 interrupts => enable millis() and delay()

        /*
         * Average volume
         */
        AudioBufferInfo.VolumeAverageValue = AudioBufferInfo.AccumulatorForVolumeAverage / FFT_N;

        if (AudioBufferInfo.VolumeAverageValue >= AudioBufferInfo.NoiseAverageValue) {
            AudioBufferInfo.VolumeAverageValue = AudioBufferInfo.VolumeAverageValue - AudioBufferInfo.NoiseAverageValue;
        } else {
            AudioBufferInfo.VolumeAverageValue = 0;
        }

        /*
         * Peak to peak volume
         */
        AudioBufferInfo.PeakToPeakValue = AudioBufferInfo.ValueMax - AudioBufferInfo.ValueMin;
        AudioBufferInfo.MidValue = AudioBufferInfo.ValueMin + (AudioBufferInfo.PeakToPeakValue / 2);

        if (AudioBufferInfo.PeakToPeakValue > AudioBufferInfo.NoisePeakValue) {
            AudioBufferInfo.VolumePeakValue = AudioBufferInfo.PeakToPeakValue - AudioBufferInfo.NoisePeakValue;
        } else {
            AudioBufferInfo.VolumePeakValue = 0;
        }

        /*
         * Compute raw average for zero value
         */
        AudioBufferInfo.RawAverageValue = (AudioBufferInfo.AccumulatorForRawAverage + (FFT_N / 2)) / FFT_N; // with rounding
        AudioBufferInfo.RawAverageLowPass5Shift8 += ((int32_t) ((AudioBufferInfo.AccumulatorForRawAverage * (256 / FFT_N))
                - AudioBufferInfo.RawAverageLowPass5Shift8)) >> 5; // (256 / FFT_N) to scale input for 256 times (shift 8) of one single value
        AudioBufferInfo.RawZeroValue = AudioBufferInfo.RawAverageLowPass5Shift8 >> 8;

        AudioBufferInfo.AudioSampleBufferPointer = &AudioBufferInfo.AudioSampleBuffer[0];
    }
    digitalWriteFast(TIMING_PIN, LOW);
}

