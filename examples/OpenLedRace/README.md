# [OpenledRace](https://github.com/ArminJo/OpenledRace) - An implementation of the OpenledRace Arduino game
### Version 1.0.0
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Commits since latest](https://img.shields.io/github/commits-since/ArminJo/OpenledRace/latest)](https://github.com/ArminJo/OpenledRace/commits/master)
[![Build Status](https://github.com/ArminJo/OpenledRace/workflows/TestCompile/badge.svg)](https://github.com/ArminJo/OpenledRace/actions)
![Hit Counter](https://visitor-badge.laobi.icu/badge?page_id=ArminJo_OpenledRace)


Extended version of the OpenLedRace "version Basic for PCB Rome Edition. 2 Player, without Boxes Track"

 Extensions are:
 *  Accelerator MPU6050 input.
 *  Classes for Car, Bridge, Ramp and Loop with **natural gravity**.
 *  Light effects by NeoPattern library.
 *  **Tone generation without dropouts** by use of hardware timer output.
 *  Winner melody by PlayRTTTL library.
 *  Compensation for blocked millis() timer during draw.
 *  Checks for RAM availability.
 *  Overlapping of cars is handled by using addPixelColor() for drawing.
 *  Dynamic activation of up to 4 cars.

Based on:
- https://www.hackster.io/gbarbarov/open-led-race-a0331a
- https://twitter.com/openledrace
- https://gitlab.com/open-led-race
- https://openledrace.net/open-software/

#### OpenLedRace at the Cologne public library MINTköln-Festival
![OpenLedRace at the Cologne public library MINTköln-Festival](https://github.com/ArminJo/OpenledRace/pictures/OpenLedRaceAtMintFestival.jpg)

# Compile with the Arduino IDE
Download and extract the repository. In the Arduino IDE open the sketch with File -> Open... and select the OpenledRace folder.<br/>
You need to install *Adafruit NeoPixel* library under "Tools -> Manage Libraries..." or "Ctrl+Shift+I" -> use "neoPixel" as filter string.<br/>
You also need to install *NeoPatterns* and *PlayRtttl* library under "Tools -> Manage Libraries..." or "Ctrl+Shift+I"

#### If you find this program useful, please give it a star.
