#ifndef DSSPI_H
#define DSSPI_H

#define DATAOUT 11     //MOSI
#define DATAIN 12      //MISO
#define SPICLOCK 13    //sck
#define SLAVESELECT 10 //ss

//Command's Opcodes
#define WREN  6
#define WRDI  4
#define RDSR  5
#define WRSR  1
#define READ  3
#define WRITE 2

#include "Arduino.h"

inline char spi_transfer(volatile char data)
{
  SPDR = data;                    // Start the transmission
  while (!(SPSR & (1<<SPIF)))     // Wait the end of the transmission
  {
  };
  return SPDR;                    // return the received byte
}

inline void releaseChip()
{
  digitalWrite(SLAVESELECT,HIGH); //release chip, signal end transfer
}

#endif
