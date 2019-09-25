#ifndef EEPROM_H
#define EEPROM_H

#include "dsSPI.h"
#include "Arduino.h"

void send_read_eeprom(int EEPROM_address);
void write_enable();

//Inline function MUST be declared on .h files
//See: https://isocpp.org/wiki/faq/inline-functions#inline-nonmember-fns
inline byte read_eeprom()
{
  //READ EEPROM
  int data;
  data = spi_transfer(0xFF); //get data byte
  return data;
}

inline void write_eeprom(byte (&page)[32], int EEPROM_address)
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
}


#endif
