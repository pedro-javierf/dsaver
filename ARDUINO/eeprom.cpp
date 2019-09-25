/*
	eeprom.cpp
    Copyright (C) 2019  pedro-javierf
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "eeprom.h"
#include "Arduino.h"

void send_read_eeprom(int EEPROM_address)
{
  //READ EEPROM
  digitalWrite(SLAVESELECT,LOW);
  spi_transfer(READ); //transmit read opcode
  spi_transfer((char)(EEPROM_address>>8));   //send MSByte address first
  spi_transfer((char)(EEPROM_address));      //send LSByte address
  //data = spi_transfer(0xFF); //get data byte
  //digitalWrite(SLAVESELECT,HIGH); //release chip, signal end transfer
}

/*
inline byte read_eeprom()
{
  //READ EEPROM
  int data;
  data = spi_transfer(0xFF); //get data byte
  return data;
}*/

void write_enable()
{
  //Assert Slave
  digitalWrite(SLAVESELECT,LOW);
  spi_transfer(WREN); //transmit read enable opcode
  delay(1);
  releaseChip();
}

/*
inline void write_eeprom(byte (&page)[32], int EEPROM_address) //arduino int -> 16 bits
{
  //Assert Slave
  digitalWrite(SLAVESELECT,LOW);
  
  //WRITE EEPROM
  spi_transfer(WRITE);                       //transmit write opcode
  spi_transfer((char)(EEPROM_address>>8));   //send MSByte address first
  spi_transfer((char)(EEPROM_address));      //send LSByte address

  for(int x = 0; x < 32; x++)
  {
    spi_transfer(page[x]); //send byte
    //delay(1); //delay 0.001s to give time the eeprom to write  
  }
  releaseChip(); 
}*/
