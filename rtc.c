#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "i2c.h"
#include "rtc.h"

uint8_t time[9]; 	//xx:xx:xx;
uint8_t date[11];	//xx/xx/xxxx;
uint8_t day;
uint8_t rtc_register[7];

//***************************************************************************
//Function to set initial address of the RTC for subsequent reading / writing
//***************************************************************************
uint8_t
RTC_setStartAddress(void)
{
  if (i2c_start()) {
    i2c_stop();
    return 1;
  } 
   
  if (i2c_sendAddress(DS1307_W)) {
    i2c_stop();
    return 1;
  } 
   
  if (i2c_sendData(0x00)) {
    i2c_stop();
    return 1;
  } 

  i2c_stop();
  return 0;
}

//***********************************************************************
//Function to read RTC registers and store them in buffer rtc_register[]
//***********************************************************************    
uint8_t
RTC_read(void)
{
  uint8_t errorStatus, i, data;
  
  errorStatus = i2c_start();
  if(errorStatus == 1)
    {
      //transmitString_F(PSTR("RTC start1 failed.."));
      i2c_stop();
      return 1;
    } 
   
  errorStatus = i2c_sendAddress(DS1307_W);
   
  if(errorStatus == 1)
    {
      //transmitString_F(PSTR("RTC sendAddress1 failed.."));
      i2c_stop();
      return 1;
    } 
   
  errorStatus = i2c_sendData(0x00);
  if(errorStatus == 1)
    {
      //transmitString_F(PSTR("RTC write-1 failed.."));
      i2c_stop();
      return 1;
    } 

  errorStatus = i2c_repeatStart();
  if(errorStatus == 1)
    {
      //transmitString_F(PSTR("RTC repeat start failed.."));
      i2c_stop();
      return 1;
    } 
   
  errorStatus = i2c_sendAddress(DS1307_R);
   
  if(errorStatus == 1)
    {
      //transmitString_F(PSTR("RTC sendAddress2 failed.."));
      i2c_stop();
      return 1;
    } 
 
  for(i=0;i<7;i++)
    {
      if(i == 6)  	 //no Acknowledge after receiving the last byte
	data = i2c_receiveData_NACK();
      else
	data = i2c_receiveData_ACK();
		  
      if(data == ERROR_CODE)
	{
	  //transmitString_F(PSTR("RTC receive failed.."));
	  i2c_stop();
	  return 1;
	}
	  
      rtc_register[i] = data;
    }
	
  i2c_stop();
  return 0;
}	  

//******************************************************************
//Function to form time string for sending it to LCD & UART
//****************************************************************** 
uint8_t RTC_getTime(void)
{
  uint8_t error;
  error = RTC_read();
  if(error) return 1;

  RTC_read();
  time[8] = 0x00;	  //NULL
  time[7] = (SECONDS & 0x0f) | 0x30;		 //seconds(1's)
  time[6] = ((SECONDS & 0x70) >> 4) | 0x30; //seconds(10's)
  time[5] = ':';
   
  time[4] = (MINUTES & 0x0f) | 0x30;
  time[3] = ((MINUTES & 0x70) >> 4) | 0x30;
  time[2] = ':'; 
   
  time[1] = (HOURS & 0x0f) | 0x30;	
  time[0] = ((HOURS & 0x30) >> 4) | 0x30;

  return 0;
}

//******************************************************************
//Function to form date string for sending it to LCD & UART
//****************************************************************** 
uint8_t RTC_getDate(void)
{
  uint8_t error;
  error = RTC_read();
  if(error) return 1;

  date[10] = 0x00;  //NULL
  date[9] = (YEAR & 0x0f) | 0x30;
  date[8] = ((YEAR & 0xf0) >> 4) | 0x30;
  date[7] = '0';
  date[6] = '2';
  date[5] = '/';
  date[4] = (MONTH & 0x0f) | 0x30;
  date[3] = ((MONTH & 0x10) >> 4) | 0x30;
  date[2] = '/';
  date[1] = (DATE & 0x0f) | 0x30;
  date[0] = ((DATE & 0x30) >> 4) | 0x30;
  return 0;
}  
  
//******************************************************************
//Function to update buffer rtc_register[] for next writing to RTC
//****************************************************************** 
void
RTC_updateRegisters(void)
{
  SECONDS = ((time[6] & 0x07) << 4) | (time[7] & 0x0f);
  MINUTES = ((time[3] & 0x07) << 4) | (time[4] & 0x0f);
  HOURS = ((time[0] & 0x03) << 4) | (time[1] & 0x0f);  
  DAY = date[10];
  DATE = ((date[0] & 0x03) << 4) | (date[1] & 0x0f);
  MONTH = ((date[3] & 0x01) << 4) | (date[4] & 0x0f);
  YEAR = ((date[8] & 0x0f) << 4) | (date[9] & 0x0f);
}  

//******************************************************************
//Function to write new time in the RTC 
//******************************************************************   
uint8_t
RTC_writeTime(void)
{
  uint8_t errorStatus, i;
  
  errorStatus = i2c_start();
  if(errorStatus == 1)
    {
      //transmitString_F(PSTR("RTC start1 failed.."));
      i2c_stop();
      return(1);
    } 
   
  errorStatus = i2c_sendAddress(DS1307_W);
   
  if(errorStatus == 1)
    {
      //transmitString_F(PSTR("RTC sendAddress1 failed.."));
      i2c_stop();
      return(1);
    } 
   
  errorStatus = i2c_sendData(0x00);
  if(errorStatus == 1)
    {
      //transmitString_F(PSTR("RTC write-1 failed.."));
      i2c_stop();
      return(1);
    } 

  for(i=0;i<3;i++)
    {
      errorStatus = i2c_sendData(rtc_register[i]);  
      if(errorStatus == 1)
	{
	  //transmitString_F(PSTR("RTC write time failed.."));
	  i2c_stop();
	  return(1);
	}
    }
	
  i2c_stop();
  return(0);
}


//******************************************************************
//Function to write new date in the RTC
//******************************************************************   
uint8_t
RTC_writeDate(void)
{
  uint8_t errorStatus, i;
  
  errorStatus = i2c_start();
  if(errorStatus == 1)
    {
      //transmitString_F(PSTR("RTC start1 failed.."));
      i2c_stop();
      return(1);
    } 
   
  errorStatus = i2c_sendAddress(DS1307_W);
   
  if(errorStatus == 1)
    {
      //transmitString_F(PSTR("RTC sendAddress1 failed.."));
      i2c_stop();
      return(1);
    } 
   
  errorStatus = i2c_sendData(0x03);
  if(errorStatus == 1)
    {
      //transmitString_F(PSTR("RTC write-1 failed.."));
      i2c_stop();
      return(1);
    } 

  for(i=3;i<7;i++)
    {
      errorStatus = i2c_sendData(rtc_register[i]);  
      if(errorStatus == 1)
	{
	  //transmitString_F(PSTR("RTC write date failed.."));
	  i2c_stop();
	  return(1);
	}
    }
	
  i2c_stop();
  return(0);
}
  
//******************************************************************
//Function to get RTC date & time in FAT32 format
//******************************************************************   
uint32_t
get_fattime (void)
{
  uint8_t ui1;
  uint32_t dtFat;

  /* init with 1/1/1981 */
  dtFat = 0x02108000;

  if (RTC_read()) /* may not be the latest time */
    return dtFat;

  dtFat = (YEAR & 0xf0) >> 4;
  dtFat = (dtFat * 10) + (YEAR & 0x0f);
  dtFat = dtFat+20;

  ui1 = (MONTH & 0xf0) >> 4;
  ui1 = (ui1 * 10) + (MONTH & 0x0f);
  dtFat <<= 4;
  dtFat |= ui1;

  ui1 = (DATE & 0xf0) >> 4;
  ui1 = (ui1 * 10) + (DATE & 0x0f);
  dtFat <<= 5;
  dtFat |= ui1;

  ui1 = (HOURS & 0xf0) >> 4;
  ui1 = (ui1 * 10) + (HOURS & 0x0f);
  dtFat <<= 5;
  dtFat |= ui1;

  ui1 = (MINUTES & 0xf0) >> 4;
  ui1 = (ui1 * 10) + (MINUTES & 0x0f);
  dtFat <<= 6;
  dtFat |= ui1;

  /* FAT32 fromat accepts dates with 2sec resolution
     (e.g. value 5 => 10sec) */
  ui1 = (SECONDS & 0xf0) >> 4;
  ui1 = (ui1 * 10) + (SECONDS & 0x0f);
  ui1 >>= 1;
  dtFat <<= 5;
  dtFat |= ui1;

  return dtFat;
}
