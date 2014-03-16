//**************************************************************
//******** FUNCTIONS FOR SERIAL COMMUNICATION USING UART *******
//**************************************************************
//Controller		: ATmega32 (Clock: 8 Mhz-internal)
//Compiler			: AVR-GCC (winAVR with AVRStudio-4)
//Project Version	: DL_1.0
//Author			: CC Dharmani, Chennai (India)
//			  		  www.dharmanitech.com
//Date				: 10 May 2011
//**************************************************************

#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "uart.h"

//**************************************************
//UART0 initialize
//baud rate: 19200  (for controller clock = 8MHz)
//char size: 8 bit
//parity: Disabled
//**************************************************
void uart0_init(void)
{
  UCSRB = 0x00; //disable while setting baud rate
  UCSRA = 0x00;
  UCSRC = (1 << URSEL) | 0x06;
  UBRRL = 0x19; //set baud rate lo
  UBRRH = 0x00; //set baud rate hi
  UCSRB = 0x18;
}

//**************************************************
//Function to receive a single byte
//*************************************************
uint8_t receiveByte( void )
{
  uint8_t data, status;
	
  while(!(UCSRA & (1<<RXC))); 	// Wait for incomming data
	
  status = UCSRA;
  data = UDR;
	
  return(data);
}

//***************************************************
//Function to transmit a single byte
//***************************************************
void transmitByte( uint8_t data )
{
  while ( !(UCSRA & (1<<UDRE)) )
    ; 			                /* Wait for empty transmit buffer */
  UDR = data; 			        /* Start transmition */
}


//***************************************************
//Function to transmit hex format data
//first argument indicates type: CHAR, INT or LONG
//Second argument is the data to be displayed
//***************************************************
void transmitHex( uint8_t dataType, unsigned long data )
{
  uint8_t count, i, temp;
  uint8_t dataString[] = "0x        ";

  if (dataType == CHAR) count = 2;
  if (dataType == INT) count = 4;
  if (dataType == LONG) count = 8;

  for(i=count; i>0; i--)
    {
      temp = data % 16;
      if((temp>=0) && (temp<10)) dataString [i+1] = temp + 0x30;
      else dataString [i+1] = (temp - 10) + 0x41;

      data = data/16;
    }

  transmitString (dataString);
}

//***************************************************
//Function to transmit a string in Flash
//***************************************************
void transmitString_F(uint8_t* string)
{
  while (pgm_read_byte(&(*string)))
    transmitByte(pgm_read_byte(&(*string++)));
}

//***************************************************
//Function to transmit a string in RAM
//***************************************************
void transmitString(uint8_t* string)
{
  while (*string)
    transmitByte(*string++);
}

//************ END ***** www.dharmanitech.com *******
