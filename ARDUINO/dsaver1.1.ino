/*
	dsaver 1.1
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

#include <U8g2lib.h>
#include <U8x8lib.h>
#include <math.h> //for round

#include "dsSPI.H"
#include "eeprom.h"

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0); 

//flags
bool mode;
bool done = false;
bool oled = true;

byte eeprom_output_data;
byte eeprom_input_data=0;
byte clr;
//Using a 32byte page size to overcome arduino's 63kb UART ring
byte mempage[32];

uint64_t address=(uint64_t)0;
uint64_t page_counter = (uint64_t)0; //also serves as an address variable for the memory page
bool write_complete = false;

//////////////////////////////////////////OLED////////////////////////////////////
void update_lcd_percent(uint64_t num)
{
    u8g2.clearBuffer();          // clear the internal memory
    u8g2.setFont(u8g2_font_logisoso28_tr);  // choose a suitable font at https://github.com/olikraus/u8g2/wiki/fntlistall

    char str[8];
    sprintf(str, "%d%%", num);
  
    u8g2.drawStr(8,29,str);  // write something to the internal memory
    u8g2.sendBuffer();         // transfer internal memory to the display
}

void update_lcd_text(char *txt)
{

    u8g2.clearBuffer();          // clear the internal memory
    u8g2.setFont(u8g2_font_logisoso28_tr);  // choose a suitable font at https://github.com/olikraus/u8g2/wiki/fntlistall
    u8g2.drawStr(8,29,txt);  // write something to the internal memory
    u8g2.sendBuffer();         // transfer internal memory to the display
}

void setup() {

  byte incomingByte = 0;
  
  //oled
  u8g2.begin();
  //u8g2.clearBuffer();          		  // clear the internal memory
  u8g2.setFont(u8g2_font_logisoso28_tr);  // choose a suitable font at https://github.com/olikraus/u8g2/wiki/fntlistall
  u8g2.drawStr(8,29,"dsaver1"); 		  // write something to the internal memory
  u8g2.sendBuffer();         			  // transfer internal memory to the display
  
  // Send identification string to PC
  Serial.begin(115200);
  Serial.println("dsaver");
  delay(5000);

  pinMode(DATAOUT, OUTPUT);
  pinMode(DATAIN, INPUT);
  pinMode(SPICLOCK,OUTPUT);
  pinMode(SLAVESELECT,OUTPUT);
  digitalWrite(SLAVESELECT,HIGH); //disable device

  /*
  Mode 0 (the default) - clock is normally low (CPOL = 0), and the data is sampled on the transition from low to high (leading edge) (CPHA = 0)
  Mode 1 - clock is normally low (CPOL = 0), and the data is sampled on the transition from high to low (trailing edge) (CPHA = 1)
  Mode 2 - clock is normally high (CPOL = 1), and the data is sampled on the transition from high to low (leading edge) (CPHA = 0)
  Mode 3 - clock is normally high (CPOL = 1), and the data is sampled on the transition from low to high (trailing edge) (CPHA = 1)

  SET THE SPI PINS, SPECIALLY CLOCK TO 4MHz
  SPCR = 01010000
  interrupt disabled,spi enabled,msb 1st,master,clk low when idle,
  sample on leading edge of clk,system clock/4 rate (fastest)
  */
  SPCR = (1<<SPE)|(1<<MSTR);
  clr=SPSR;
  clr=SPDR;

  //Read desired mode sent by the PC
  while(incomingByte == 0)
  {
    if(Serial.available())
    {
      incomingByte = Serial.read();
    }
  }
  
  
  if(incomingByte == 0x45)     
  { 
	//dump
    mode = true;
  }
  else if(incomingByte == 0x49)
  {
	//inject
    mode = false;
  }
  else
  {
    u8g2.clearBuffer(); 
    u8g2.setFont(u8g2_font_logisoso28_tr);  
    u8g2.drawStr(8,29,"ERROR!");  
    u8g2.sendBuffer();         
    while(1);
  }

 
 //Starts read at 0x0000
 if(mode)
 {send_read_eeprom(0x0);}
 else
 {u8g2.clearBuffer();}
}

void loop() {

  if(mode) //dump savegame
  {
    if(!done)
    {
      //Read next byte
      eeprom_output_data = read_eeprom();
  
      //Send RAW byte
      Serial.write(eeprom_output_data);
  
      address++;
      if ((uint64_t)address == (uint64_t)65536)
      {//address = 0;
        done = true;
      }
    }
    else
    {
      releaseChip();
      update_lcd_text("DONE.");
      while(1);
    }
  
    if(oled && (address%100==0))
    {
      float percent = ((float)address /(float)65535)*100;
      update_lcd_percent(round(percent));
    }
  }

  /*
  Type   Total Size  Page Size  Chip/Example      Game/Example
  EEPROM 0.5K bytes   16 bytes  ST M95040-W       (eg. Metroid Demo)
  EEPROM   8K bytes   32 bytes  ST M95640-W       (eg. Super Mario DS)
  EEPROM  64K bytes  128 bytes  ST M95512-W       (eg. Downhill Jam)
  */
  

  else //inject savegame
  { 
      if(page_counter==(uint64_t)2048)
      {write_complete = true;}

      if(!write_complete)
      {
        if(oled)
          {update_lcd_percent(page_counter);}
          
        //receive page data from PC
		    // Wait: until there are 32 Bytes waiting
        while (Serial.available()<32){} 
        
		    // Then: Get them.
        for(int n=0; n<32; n++){
          mempage[n] = Serial.read();  
        }
        
        //Enable the write
        write_enable();
  
        //write page
        //ENSURE THIS CALL IS CORRECT
        write_eeprom(mempage,(page_counter*(uint64_t)32));
        page_counter++;
        
      }
      else
      {
        releaseChip();
        update_lcd_text("DONE.");
        while(1);
      }
  }
}
