# dsaver
Arduino based Nintendo DS savegame hardware tool

<img src="https://github.com/pedro-javierf/dsaver/raw/master/dsaverHW.jpg" width="250" height="300" />

## Description
_dsaver_ is a hardware solution to dump and modify Nintendo DS cartridges savegames. It is based on the popular and extended Arduino board; it works perfectly on clone and unofficial boards.

## Hardware required
to build a _dsaver_ device compatible with the code provided in the ARDUINO folder you will need:
- [ ] Arduino board, official or clone (ie: freaduino)
- [ ] Nintendo DS slot 1 header
- [ ] Jumper Wires
- [ ] (optional)(recommennded) i2c oled display

## Wiring

| Arduino       | Slot 1        | Description   |
| ------------- | ------------- | ------------- |
| PIN 13 (ICSP) | 2             | CLK           |
| PIN 10 (ICSP) | 6             | CS (or SS)    |
| Any VCC 3.3v  | 8             | VCC 3.3v      |
| PIN 11 MOSI   | 15 (MISO)     | ***           |
| PIN 12 MISO   | 16 (MOSI)     | ***           |
| Any GND       | 17            | GND           |

- Arduino PIN 11 is MOSI and should be connected to pin 15 of the slot1 (MISO)
- Arduino PIN 12 is MISO and should be connected to pin 15 of the slot1 (MOSI)

If you don't understand why it is this way check the [SPI Protocol](https://es.wikipedia.org/wiki/Serial_Peripheral_Interface) or the [dsaver writeup](https://pedro-javierf.github.io)

## software
Provided in this repository you will find all software required to build a _dsaver_ device:
- [x] ARDUINO   : C code to be flashed on the board. `.ino` `.h` and `.cpp` files.
- [x] PC        : File `dsaver.py` contains python3 data receiver for the PC side.

### Requeriments
dsaver.py requires PySerial:
- pip3 install pyserial

dsaver.ino requires U8g2lib and U8x8lib for OLED support
-  #include <U8g2lib.h>
-  #include <U8x8lib.h>
