#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "uart.h"

float uartWeight, uartInDecimal;

//**************************************************
//UART initialize
//baud rate: 19200  (for controller clock = 8MHz)
//char size: 8 bit
//parity: Disabled
//**************************************************
void
uartInit(void)
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

  /* Enable the USART Recieve Complete interrupt (USART_RXC) */
  UCSRB |= (1 << RXCIE);

  /* init */
  uartInDecimal = 0;
  uartWeight = 0;
}

void
uartSelect(uint8_t uid)
{
  PORTD |= (uid & 3) << 5;
}

ISR(USART_RXC_vect)
{
  uint8_t ReceivedByte;
  ReceivedByte = UDR;
  if ( ('\n' == ReceivedByte) || ('\r' == ReceivedByte) ) {
    uartWeight = 0;
    uartInDecimal = 0;
  } else if ( ('0' <= ReceivedByte) && ('9' >= ReceivedByte) ) {
    if (0 == uartInDecimal) {
      uartWeight *= 10;
      uartWeight += ReceivedByte-'0';
    } else {
      uartWeight += uartInDecimal * (ReceivedByte-'0');
      uartInDecimal /= 10;
    }
  } else if ('.' == ReceivedByte)
    uartInDecimal = 0.1;
}

////**************************************************
////Function to receive a single byte
////*************************************************
//uint8_t
//uartReceiveByte( void )
//{
//  uint8_t data, status;
//	
//  while(!(UCSRA & (1<<RXC))); 	// Wait for incomming data
//	
//  status = UCSRA;
//  data = UDR;
//	
//  return(data);
//}

//***************************************************
//Function to transmit a single byte
//***************************************************
void
uartTransmitByte( uint8_t data )
{
  while ( !(UCSRA & (1<<UDRE)) )
    ; 			                /* Wait for empty transmit buffer */
  UDR = data; 			        /* Start transmition */
}
