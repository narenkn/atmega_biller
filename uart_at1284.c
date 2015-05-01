#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "uart.h"

float uartWeight, uartInDecimal;

//**************************************************
// UART initialize
// baud rate: 19200  (for controller clock = 32MHz)
// char size: 8 bit
// parity: Disabled
//   UBRRn = (f_Osc/(16*BAUD)) - 1
//         = ((32*10^6)/(16*19200)) - 1
//         = 103
//**************************************************
void
uartInit(void)
{
  /* Set baud rate */
  uint16_t baud = 103;
  UBRR0H = (unsigned char)(baud>>8);
  UBRR0L = (unsigned char)baud;
  /* Enable receiver and transmitter */
  UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0);
  /* Set frame format: 8data, 2stop bit */
  UCSR0C = (1<<USBS0)|(3<<UCSZ00);

  /* Enable device */
  PRR0 &= ~(1<<PRUSART0);

  /* For UART select */
  DDRD |= (3<<5);
  PORTD &= ~(3<<5);

  /* init */
  uartInDecimal = 0;
  uartWeight = 0;
}

void
uartSelect(uint8_t uid)
{
  PORTD |= (uid & 3) << 5;
}

ISR(USART0_RX_vect)
{
  uint8_t ReceivedByte = UDR0;

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

//**************************************************
//Function to receive a single byte
//*************************************************
uint8_t
uartReceiveByte( void )
{
  uint8_t data, status;

  /* Wait for data to be received */
  while ( !(UCSR0A & (1<<RXC0)) )
    ;
  /* Get and return received data from buffer */
  status = UCSR0A;
  data = UDR0;

  return data;
}

//***************************************************
//Function to transmit a single byte
//***************************************************
void
uartTransmitByte( uint8_t data )
{
  /* Wait for empty transmit buffer */
  while ( !( UCSR0A & (1<<UDRE0)) )
    ;
  /* Put data into buffer, sends the data */
  UDR0 = data;
}
