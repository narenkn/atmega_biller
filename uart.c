#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "uart.h"

//**************************************************
//UART initialize
//baud rate: 19200  (for controller clock = 8MHz)
//char size: 8 bit
//parity: Disabled
//**************************************************
void
uart_init(void)
{
#if 0
  UCSRB = 0x00; //disable while setting baud rate
  UCSRA = 0x00;
  UCSRC = (1 << URSEL) | 0x06;
  UBRRL = 0x19; //set baud rate lo
  UBRRH = 0x00; //set baud rate hi
  UCSRB = 0x18;
#endif

#if 1
  uint16_t ui1;

  UCSRB = 0x00;
  UCSRA = 0x00;
#if F_CPU <= 1000000UL
  /*
   * Note [4]
   * Slow system clock, double Baud rate to improve rate error.
   */
  UCSRA = _BV(U2X);
  ui1 = (F_CPU / (8 * 9600UL)) - 1; /* 9600 Bd */
#else
  ui1 = (F_CPU / (16 * 9600UL)) - 1; /* 9600 Bd */
#endif
  UCSRC = (1 << URSEL) | 0x06;
  UBRRL = ui1;
  UBRRH = ui1 >> 8;
  UCSRB = _BV(TXEN);		/* tx enable */
#endif

  /* For UART select */
  DDRD |= (3<<5);
  PORTD &= ~(3<<5);
}

void
uart_select(uint8_t uid)
{
  PORTD |= (uid & 3) << 5;
}

//**************************************************
//Function to receive a single byte
//*************************************************
uint8_t
uart_receiveByte( void )
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
void
uart_transmitByte( uint8_t data )
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
void
uart_transmitHex( uint8_t dataType, unsigned long data )
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

  uart_transmitString (dataString);
}

//***************************************************
//Function to transmit a string in Flash
//***************************************************
void
uart_transmitString_F(uint8_t* string)
{
  while (pgm_read_byte(&(*string)))
  uart_transmitByte(pgm_read_byte(&(*string++)));
}

//***************************************************
//Function to transmit a string in RAM
//***************************************************
void
uart_transmitString(uint8_t* string)
{
  while (*string)
  uart_transmitByte(*string++);
}

//************ END ***** www.dharmanitech.com *******
