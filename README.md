# dsaver
Arduino based Nintendo DS savegame hardware tool

## description
_dsaver_ is a hardware solution to dump and modify Nintendo DS cartridges savegames. It is based on the popular and extended Arduino board; it works perfectly on clone and unofficial boards.

## hardware required
to build a _dsaver_ device compatible with the code provided in the ARDUINO folder you will need:
- [ ] Arduino board, official or clone (ie: freaduino)
- [ ] Nintendo DS slot 1 header
- [ ] Jumper Wires
- [ ] (optional)(recommennded) i2c oled display

## software
Provided in this repository you will find all software required to build a _dsaver_ device:
- [ ] ARDUINO   : .ino C code to be flashed on the board
- [ ] dsaver.py : python3 data receiver for the PC side

### requeriments
dsaver.py requires PySerial

pip3 install pyserial
