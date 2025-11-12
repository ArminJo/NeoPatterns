<div align = center>

# [OpenledRace](https://github.com/ArminJo/OpenledRace)

**Extended version** of the [OpenledRace](https://twitter.com/openledrace) version ["Basic for PCB Rome Edition. 2 Player, without Boxes Track"](https://gitlab.com/open-led-race/olr-arduino).

[![Open Led Race logo](https://github.com/ArminJo/OpenledRace/blob/master/pictures/OLR-Logo_400x400.png)](https://twitter.com/openledrace)

[![Badge License: GPLv3](https://img.shields.io/badge/License-GPLv3-brightgreen.svg)](https://www.gnu.org/licenses/gpl-3.0)
 &nbsp; &nbsp;
[![Badge Version](https://img.shields.io/github/v/release/ArminJo/OpenledRace?include_prereleases&color=yellow&logo=DocuSign&logoColor=white)](https://github.com/ArminJo/OpenledRace/releases/latest)
 &nbsp; &nbsp;
[![Badge Commits since latest](https://img.shields.io/github/commits-since/ArminJo/OpenledRace/latest?color=yellow)](https://github.com/ArminJo/OpenledRace/commits/master)
 &nbsp; &nbsp;
[![Badge Build Status](https://github.com/ArminJo/OpenledRace/workflows/TestCompile/badge.svg)](https://github.com/ArminJo/OpenledRace/actions)
 &nbsp; &nbsp;
![Badge Hit Counter](https://visitor-badge.laobi.icu/badge?page_id=ArminJo_OpenledRace)
<br/>

Also available as [OpenLedRace example](https://github.com/ArminJo/NeoPatterns/tree/master/examples/OpenLedRace) in the [NeoPatterns library](https://github.com/ArminJo/NeoPatterns).

</div>

#### If you find this library useful, please give it a star.

&#x1F30E; [Google Translate](https://translate.google.com/translate?sl=en&u=https://github.com/ArminJo/OpenLedRace)

<br/>

#### If you find this program useful, please give it a star.

# Extensions to standard version
 *  **Input from MPU6050 Accelerometer**.
 *  Classes for Car, Bridge, Ramp and Loop with **natural gravity**.
 *  **Light effects** by NeoPattern library.
 *  **Tone generation without dropouts** by use of hardware timer output.
 *  2004 **LCD** for instructions and leap counter.
 *  Input feedback by an 8 pixel Neopixel bar.
 *  Winner melody by PlayRTTTL library.
 *  Compensation for blocked millis() timer during draw.
 *  Checks for RAM availability.
 *  Overlapping of cars is handled by using addPixelColor() for drawing.
 *  **Development mode**, where Gravity, Friction and Drag can be set by potentiometers.

 <br/>

# Principle of operation
With every button press or every acceleration of the dumbbells you **add a fixed amount of energy to the "car"**.<br/>
This energy is used to **increase speed**.

Formula is: **Speed = sqrt((OldSpeed * OldSpeed) + AdditionalEnergy)**

On the other hand, **energy is consumed by friction and drag**.<br/>
Gravity also increases or decreases the car as in real life.

Formula is: **NewSpeed = Speed + Gravity + Friction + (Speed * Drag)**

<br/>



# Pictures
| Old version | New version |
| :-: | :-: |
| ![Accelerometer version from MakerFaire 2022](https://github.com/ArminJo/OpenledRace/blob/master/pictures/Overview.jpg) | ![Accelerometer version from MakerFaire 2022](https://github.com/ArminJo/OpenledRace/blob/master/pictures/Details2.jpg) |
| | |
| At the Cologne public library MINTk&ouml;ln-Festival 2025 | At the Cologne public library MINTk&ouml;ln-Festival 2021 |
| ![OpenLedRace at the Cologne public library MINTk&ouml;ln-Festival 2021](https://github.com/ArminJo/OpenledRace/blob/master/pictures/OpenLedRaceAtMintFestival.jpg) | ![OpenLedRace at the Cologne public library MINTk&ouml;ln-Festival 2025](https://github.com/ArminJo/OpenledRace/blob/master/pictures/OpenLedRaceAtMintFestival_2025.jpg) |

<br/>

# Size
The version using a 5 m IP30 - 60 pixel per meter strip with ramp and loop reqires an area of 145 cm x 80 cm plus 20 cm space for breadboard etc.<br/>
The base of the ramp is 83 cm, the height is 23.5 cm. The slope is 45 degree and therefore the top of the ramp is 83 cm - (2 * 23.5 cm) = 36 cm.<br/>
The diameter of the loop is 26 cm.

<br/>

# YouTube Videos
| At the Hannover MakerFaire 2022 | At the Cologne public library MINTk&ouml;ln-Festival 2021 |
| :-: | :-: |
| [![OpenLedRace at the Hannover MakerFaire 2022](https://i.ytimg.com/vi/lYzYpFYJfWI/hqdefault.jpg)](https://www.youtube.com/watch?v=lYzYpFYJfWI) | [![OpenLedRace in action 2021](https://i.ytimg.com/vi/y25rjRkDg0g/hqdefault.jpg)](https://www.youtube.com/watch?v=y25rjRkDg0g) |
| | |
| At the Cologne public library MINTk&ouml;ln-Festival 2025 |  |
| [![OpenLedRace in action 2025](https://i.ytimg.com/vi/mGpJGzWwncQ/hqdefault.jpg)](https://www.youtube.com/watch?v=mGpJGzWwncQ) | |

<br/>

# Arduino plotter output
![Arduino plotter output](https://github.com/ArminJo/OpenledRace/blob/master/pictures/ArduinoPlotterOutput.png)

In this output, both accelerators were initially moving, then only the first, and finally the second alone. `AccelLP` is the low-pass value of `Accel`.<br/>
In the middle, there is a steep decrease in speed at the same input level. This is because the car has reached the ramp and must now overcome gravity.<br/>
At the end, you can see negative speed. This is because the car was on the ramp when the input stopped and gravity moved it backwards down the ramp.

<br/>

# Compile with the Arduino IDE
Download and extract the repository. In the Arduino IDE open the sketch with File -> Open... and select the OpenledRace folder.<br/>
You need to install *Adafruit NeoPixel* library under "Tools -> Manage Libraries..." or "Ctrl+Shift+I" -> use "neoPixel" as filter string.<br/>
You also need to install *NeoPatterns* and *PlayRtttl* library under "Tools -> Manage Libraries..." or "Ctrl+Shift+I"

# BOM
### Required
- Breadboard
- Jumper wire
- Pin header to connect cables to breadboard
- Arduino Nano
- WS2812 strip 5m 60 IP30 - 60 pixel per meter
- 10 kOhm resistor for WS2812 strip data input
- 2 push buttons 16 mm + 2x2 meter flexible 2-wire cable
- 2 Handlebar ends with foam grips to mount the push buttons
- 2004 LCD with serial I2C interface adapter
- Speaker > 32 &ohm;
- 4.7 &micro;F capacitor for speaker
- Power supply - e.g. a 18650 battery + holder
- "Reset / Start Game" Pushbutton
- 1 k&ohm; potentiometer for speaker volume control

### Optional
- 2 8xWS2812 bars + 2 10kOhm resistors for optical feedback of input strength
- 2 GY-521 MPU6050 accelerometer Breakout boards (+ 2 Blink LEDs)
- 2 Dumbbells, each 1 kg
- 2x2 meter 4-Wire cable (+ 2x2 10 kOhm I2C pullup resistors) to connect the accelerometers
- 3 pieces 4 pole magnetic pogo pin connectors


### For development
- 3 100k potentiometers for setting of Gravity, Friction and Drag.

Pin layout is defined [here](https://github.com/ArminJo/NeoPatterns/blob/master/examples/OpenLedRace/OpenLedRace.ino#L129)

| Breadboard overview | Accelerometer connection |
| :-: | :-: |
| ![BreadboardOverview](https://github.com/ArminJo/OpenledRace/blob/master/pictures/BreadboardOverview.jpg) | ![Accelerometer connection](https://github.com/ArminJo/OpenledRace/blob/master/pictures/Accelerometer.jpg) |
| Breadboard top view | Breadboard front view |
| ![Breadboard top view](https://github.com/ArminJo/OpenledRace/blob/master/pictures/BreadboardTop.jpg) | ![Breadboard front view](https://github.com/ArminJo/OpenledRace/blob/master/pictures/BreadboardFront.jpg) |


### Links:
- https://www.hackster.io/gbarbarov/open-led-race-a0331a
- https://twitter.com/openledrace
- https://gitlab.com/open-led-race
- https://openledrace.net/open-software/
