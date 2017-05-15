#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/crc16.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <util/twi.h>

#include "ep_ds.h"
#include "version.h"
#include "assert.h"
#include "lcd.h"
#include "kbd.h"
#include "ep_store.h"
#include "billing.h"
#include "i2c.h"
#include "uart.h"
#include "a1micro2mm.h"
#include "main.h"
#include "menu.h"

//************************************************************
//    ******** FUNCTIONS FOR I2C COMMUNICATION  *******
//************************************************************
//Controller		: ATmega32 (Clock: 8 Mhz-internal)
//Compiler		: AVR-GCC (winAVR with AVRStudio-4)
//Project Version	: DL_1.0
//Author		: CC Dharmani, Chennai (India)
//			  www.dharmanitech.com
//Date			: 10 May 2011
//************************************************************

#include "i2c.h"

void tmr_init(void);

//************************************************
// TWI initialize
// bit rate:18 (freq: 100Khz @16MHz)
//************************************************
void
i2c_init(void)
{
#if 0
#if F_CPU <= 1000000UL
  /*
   * Note [4]
   * Slow system clock, double Baud rate to improve rate error.
   */
  UCSRA = _BV(U2X);
  UBRR = (F_CPU / (8 * 9600UL)) - 1; /* 9600 Bd */
#else
  UBRR = (F_CPU / (16 * 9600UL)) - 1; /* 9600 Bd */
#endif
  UCSRB = _BV(TXEN);		/* tx enable */

  /* initialize TWI clock: 100 kHz clock, TWPS = 0 => prescaler = 1 */
#if defined(TWPS0)
  /* has prescaler (mega128 & newer) */
  TWSR = 0;
#endif

#if F_CPU < 3600000UL
  TWBR = 10;			/* smallest TWBR value, see note [5] */
#else
  TWBR = (F_CPU / 100000UL - 16) / 2;
#endif

#else

  TWCR= 0x00; //disable twi
  TWBR= 0x12; //set bit rate
  TWSR= 0x00; //set prescale
  //TWCR= 0x44; //enable twi
#endif

#if !DS1307
  tmr_init();
#endif
}

#if DS1307

//*************************************************
//Function to start i2c communication
//*************************************************
uint8_t
i2c_start(void)
{
  TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN); 	//Send START condition
	
  while (!(TWCR & (1<<TWINT)));   		//Wait for TWINT flag set. This indicates that the
  //START condition has been transmitted
  if ((TWSR & 0xF8) == START)			//Check value of TWI Status Register
    return(0);
  else
    return(1);
}

//*************************************************
//Function for repeat start condition
//*************************************************
uint8_t
i2c_repeatStart(void)
{
  TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN); 		//Send START condition
  while (!(TWCR & (1<<TWINT)));   		//Wait for TWINT flag set. This indicates that the
  //START condition has been transmitted
  if ((TWSR & 0xF8) == REPEAT_START)			//Check value of TWI Status Register
    return(0);
  else
    return(1);
}

//**************************************************
//Function to transmit address of the slave
//*************************************************
uint8_t
i2c_sendAddress(uint8_t address)
{
  uint8_t STATUS;

  if((address & 0x01) == 0) 
    STATUS = MT_SLA_ACK;
  else
    STATUS = MR_SLA_ACK; 

  TWDR = address; 
  TWCR = (1<<TWINT)|(1<<TWEN);	   //Load SLA_W into TWDR Register. Clear TWINT bit
  //in TWCR to start transmission of address
  while (!(TWCR & (1<<TWINT)));	   //Wait for TWINT flag set. This indicates that the
  //SLA+W has been transmitted, and
  //ACK/NACK has been received.
  if ((TWSR & 0xF8) == STATUS)	   //Check value of TWI Status Register
    return(0);
  else 
    return(1);
}

//**************************************************
//Function to transmit a data byte
//*************************************************
uint8_t
i2c_sendData(uint8_t data)
{
  TWDR = data; 
  TWCR = (1<<TWINT) |(1<<TWEN);	   //Load SLA_W into TWDR Register. Clear TWINT bit
  //in TWCR to start transmission of data
  while (!(TWCR & (1<<TWINT)));	   //Wait for TWINT flag set. This indicates that the
  //data has been transmitted, and
  //ACK/NACK has been received.
  if ((TWSR & 0xF8) != MT_DATA_ACK)   //Check value of TWI Status Register
    return(1);
  else
    return(0);
}

//*****************************************************
//Function to receive a data byte and send ACKnowledge
//*****************************************************
uint8_t
i2c_receiveData_ACK(void)
{
  uint8_t data;
  
  TWCR = (1<<TWEA)|(1<<TWINT)|(1<<TWEN);
  
  while (!(TWCR & (1<<TWINT)));	   	   //Wait for TWINT flag set. This indicates that the
  //data has been received
  if ((TWSR & 0xF8) != MR_DATA_ACK)    //Check value of TWI Status Register
    return(ERROR_CODE);
  
  data = TWDR;
  return(data);
}

//******************************************************************
//Function to receive the last data byte (no acknowledge from master
//******************************************************************
uint8_t
i2c_receiveData_NACK(void)
{
  uint8_t data;
  
  TWCR = (1<<TWINT)|(1<<TWEN);
  
  while (!(TWCR & (1<<TWINT)));	   	   //Wait for TWINT flag set. This indicates that the
  //data has been received
  if ((TWSR & 0xF8) != MR_DATA_NACK)    //Check value of TWI Status Register
    return(ERROR_CODE);
  
  data = TWDR;
  return(data);
}

//**************************************************
//Function to end the i2c communication
//*************************************************   	
void
i2c_stop(void)
{
  TWCR =  (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);	  //Transmit STOP condition
}  
#endif

//******************************************************************
//Function to get RTC date & time in FAT32 format
//  Return format : Year[31:25], Month[24:21], Date[20:16]
//                  Hour[15:11], Min[10:5], Sec[4:0]
//******************************************************************  
uint32_t
get_fattime (void)
{
  date_t   date;
  s_time_t   time;
  uint32_t dtFat;

  /* Process date */
  timerDateGet(date);
  dtFat = (date.year-2000);
  dtFat <<= 4;
  dtFat |= date.month;
  dtFat <<= 5;
  dtFat |= date.day;

  /* Process time */
  timerTimeGet(time);
  dtFat <<= 5;
  dtFat |= time.hour;
  dtFat <<= 6;
  dtFat |= time.min;
  /* FAT32 fromat accepts dates with 2sec resolution
     (e.g. value 5 => 10sec) */
  dtFat <<= 5;
  dtFat |= time.sec>>1;

  return dtFat;
}

// Global time
volatile uint8_t rtc_sec;
volatile uint8_t rtc_min;
volatile uint8_t rtc_hour;
volatile uint8_t rtc_date;
volatile uint8_t rtc_month;
volatile uint16_t rtc_year;

// Interrupt handler on compare match (TCNT0 is cleared automatically)
ISR(TIMER0_COMP_vect)
{
  static  uint8_t idleDelay = 0;
  uint8_t max_days_in_month;

  /* Wait for 5 secs to disable timer permanently
     assume we need to get call next */
  if ( (0 == keyHitData.hbCnt) && (0 == keyHitData.count) && (0 == keyHitData.KbdDataAvail) )
    idleDelay++;
  else idleDelay = 0;
  if (idleDelay > 4) {
    LCD_bl_off;
    TIMSK &= ~(1 << TOIE1); /* disable Timer1 overflow */
  } else {
    LCD_bl_on;
  }
  
  // Increment time
  if (++rtc_sec >= 60) {
    rtc_sec = 0;
    if (++rtc_min >= 60) {
      rtc_min = 0;
      if (++rtc_hour >= 24) {
	rtc_hour = 0;
	rtc_date++;

	/* date rollover */
	soft_reset();

	/* find max time, keep date */
	if (2 == rtc_month) {
	  max_days_in_month = ((0 == (rtc_year & 0x3)) && (0 != (rtc_year%100)))? 29 : 28;
	} else if ( (1 == rtc_month) || (3 == rtc_month) ||
		    (5 == rtc_month) || (7 == rtc_month) ||
		    (8 == rtc_month) || (10 == rtc_month) ||
		    (12 == rtc_month) )
	  max_days_in_month = 31;
	else
	  max_days_in_month = 30;
	if (rtc_date > max_days_in_month) {
	  rtc_date = 1;
	  if (++rtc_month > 12) {
	    rtc_month = 1;
	    rtc_year++;
	  }
	}
      }
      
    }
  }
}

void
tmr_init(void)
{
  /* Start timer 0 with clock prescaler CLK/1024 and CTC Mode ("Clear Timer on Compare")*/
  /* Resolution is 32.25 ms */
  TCCR0 = (0<<FOC0)|(0<<WGM00)|(0<<COM01)|(0<<COM00)|(1<<WGM01)|(1<<CS02)|(1<<CS01)|(1<<CS00);

  // Reset time
  TCNT0 = 0;

  // Calculate and set period
  OCR0  = (uint16_t)(((RTC_F/1024)*RTC_PERIOD_MS)/1000) - 1;

  // Enable interrupt on compare match
  TIMSK |= (1<<OCIE0);

  // Select asynchronous timer 0 operation to use external 32.768 kHz crystal
  ASSR |= (1<<AS0);
}
