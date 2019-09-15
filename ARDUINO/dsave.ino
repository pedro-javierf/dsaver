#include <U8g2lib.h>
#include <U8x8lib.h>
#include <math.h> //for round
//#include <SPI.h>

#define DATAOUT 11     //MOSI
#define DATAIN 12      //MISO
#define SPICLOCK 13    //sck
#define SLAVESELECT 10 //ss

//opcodes
#define WREN  6
#define WRDI  4
#define RDSR  5
#define WRSR  1
#define READ  3
#define WRITE 2

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0); 

//flags
bool mode;
bool done = false;
bool oled = true;
byte eeprom_output_data;
byte eeprom_input_data=0;
byte clr;
uint64_t address=(uint64_t)0;
byte mempage[64];
bool write_complete = false;
uint64_t page_counter = (uint64_t)0; //also serves as an address variable for the memory page


//////////////////////////////////////////OLED////////////////////////////////////
void update_lcd_percent(uint64_t num)
{
  if(num>=0 && num<=10000)
  {
    u8g2.clearBuffer();          // clear the internal memory
    u8g2.setFont(u8g2_font_logisoso28_tr);  // choose a suitable font at https://github.com/olikraus/u8g2/wiki/fntlistall

    char str[8];
    sprintf(str, "%d%%", num);
  
    u8g2.drawStr(8,29,str);  // write something to the internal memory
    u8g2.sendBuffer();         // transfer internal memory to the display
  }
}

void update_lcd_text(char *txt)
{

    u8g2.clearBuffer();          // clear the internal memory
    u8g2.setFont(u8g2_font_logisoso28_tr);  // choose a suitable font at https://github.com/olikraus/u8g2/wiki/fntlistall
    u8g2.drawStr(8,29,txt);  // write something to the internal memory
    u8g2.sendBuffer();         // transfer internal memory to the display
}

/*
void printHex(int num, int precision) {
     char tmp[16];
     char format[128];
     sprintf(format, "%%.%dX", precision);
     sprintf(tmp, format, num);
     Serial.println(tmp);
}*/

////////////////////////////////////DUMP FUNCTIONS//////////////////////////////////////////
inline char spi_transfer(volatile char data)
{
  SPDR = data;                    // Start the transmission
  while (!(SPSR & (1<<SPIF)))     // Wait the end of the transmission
  {
  };
  return SPDR;                    // return the received byte
}

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

inline byte read_eeprom()
{
  //READ EEPROM
  int data;
  data = spi_transfer(0xFF); //get data byte
  return data;
}

inline void releaseChip()
{
  digitalWrite(SLAVESELECT,HIGH); //release chip, signal end transfer
}

/*
inline byte read_eeprom(int EEPROM_address)
{
  //READ EEPROM
  int data;
  digitalWrite(SLAVESELECT,LOW);
  spi_transfer(READ); //transmit read opcode
  spi_transfer((char)(EEPROM_address>>8));   //send MSByte address first
  spi_transfer((char)(EEPROM_address));      //send LSByte address
  data = spi_transfer(0xFF); //get data byte
  digitalWrite(SLAVESELECT,HIGH); //release chip, signal end transfer
  return data;
}
*/


////////////////////////////////INJECT FUNCTIONS///////////////////////////////
void write_enable()
{
  //Assert Slave
  digitalWrite(SLAVESELECT,LOW);
  spi_transfer(WREN); //transmit read enable opcode
  releaseChip();
}

inline void write_eeprom(byte (&page)[64], int EEPROM_address) //arduino int -> 16 bits
{
  //WRITE EEPROM
  spi_transfer(WRITE);                       //transmit read opcode
  spi_transfer((char)(EEPROM_address>>8));   //send MSByte address first
  spi_transfer((char)(EEPROM_address));      //send LSByte address

  for(int x = 0; x < 64; x++)
  {
    spi_transfer(page[x]); //send byte
    delay(100); //delay 0.1s to give time the eeprom to write  
}
  releaseChip(); 
}


void setup() {

  byte incomingByte = 0;
  
  //oled
  u8g2.begin();

  //show startup
  //u8g2.clearBuffer();          // clear the internal memory
  u8g2.setFont(u8g2_font_logisoso28_tr);  // choose a suitable font at https://github.com/olikraus/u8g2/wiki/fntlistall
  u8g2.drawStr(8,29,"dsaver1");  // write something to the internal memory
  u8g2.sendBuffer();         // transfer internal memory to the display
  
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("dsaver");
  delay(5000);

  pinMode(DATAOUT, OUTPUT);
  pinMode(DATAIN, INPUT);
  pinMode(SPICLOCK,OUTPUT);
  pinMode(SLAVESELECT,OUTPUT);
  digitalWrite(SLAVESELECT,HIGH); //disable device

  //SET THE SPI PINS, SPECIALLY CLOCK TO 4MHz
  //SPCR = 01010000
  //interrupt disabled,spi enabled,msb 1st,master,clk low when idle,
  //sample on leading edge of clk,system clock/4 rate (fastest)
  
  SPCR = (1<<SPE)|(1<<MSTR);
  clr=SPSR;
  clr=SPDR;

  //Read desired mode
  while(incomingByte == 0)
  {
    if(Serial.available())
    {
      incomingByte = Serial.read();
    }
  }
  if(incomingByte == 0x45)//dump
  {
    
    mode = true;
  }
  else if(incomingByte == 0x49)//inject
  {
    mode = false;
  }
  else
  {
    u8g2.clearBuffer();          // clear the internal memory
    u8g2.setFont(u8g2_font_logisoso28_tr);  // choose a suitable font at https://github.com/olikraus/u8g2/wiki/fntlistall
    u8g2.drawStr(8,29,"ERROR!");  // write something to the internal memory
    u8g2.sendBuffer();         // transfer internal memory to the display 
    while(1);
  }

  /*
  Mode 0 (the default) - clock is normally low (CPOL = 0), and the data is sampled on the transition from low to high (leading edge) (CPHA = 0)
  Mode 1 - clock is normally low (CPOL = 0), and the data is sampled on the transition from high to low (trailing edge) (CPHA = 1)
  Mode 2 - clock is normally high (CPOL = 1), and the data is sampled on the transition from high to low (leading edge) (CPHA = 0)
  Mode 3 - clock is normally high (CPOL = 1), and the data is sampled on the transition from low to high (trailing edge) (CPHA = 1)
  */

 //Starts read at 0x0000
 if(mode)
 {send_read_eeprom(0x0);}
 else
 {
      u8g2.clearBuffer();          // clear the internal memory
      u8g2.setFont(u8g2_font_logisoso28_tr);  // choose a suitable font at https://github.com/olikraus/u8g2/wiki/fntlistall
      u8g2.drawStr(8,29,"INJECT");  // write something to the internal memory
      u8g2.sendBuffer();         // transfer internal memory to the display 
  }
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

  //use a 32byte page size to overcome arduino's 63kb UART ring

  else //inject savegame
  { 
      if(page_counter==(uint64_t)2048)
      {write_complete = true;}

      if(!write_complete)
      {
        update_lcd_percent(page_counter);
        //receive page data from PC
        while (Serial.available()<32){} // Wait 'till there are 32 Bytes waiting
        for(int n=0; n<32; n++){
          mempage[n] = Serial.read(); // Then: Get them.
        }
        
        //Enable the write
        write_enable();
  
        //write page
        //ENSURE THIS CALL IS CORRECT
        write_eeprom(mempage,(page_counter*(uint64_t)64));
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
