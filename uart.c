#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/crc16.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>
#include <util/twi.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#include "ep_ds.h"
#include "version.h"
#include "assert.h"
#include "lcd.h"
#include "i2c.h"
#include "uart.h"

volatile uint32_t uartWeight;

//**************************************************
//UART initialize
//baud rate: 19200  (for controller clock = 8MHz)
//char size: 8 bit
//parity: Disabled
//**************************************************
void
uartInit(void)
{
  uint16_t ui0, ui1;

  UCSR0B = 0x00;  UCSR0A = 0x00;
  UCSR1B = 0x00;  UCSR1A = 0x00;
  /*
   * Note [4]
   * Slow system clock, double Baud rate to improve rate error.
   */
  UCSR0A = _BV(U2X0);
  UCSR1A = _BV(U2X1);
  ui0 = (F_CPU / (8 * UART0_BAUD));
  ui1 = (F_CPU / (8 * UART1_BAUD)) - 1;
  /* Printer uart 0 */
  UCSR0C = 0x06 | _BV(USBS0);
  UBRR0L = ui0;
  UBRR0H = ui0 >> 8;
  UCSR0B = _BV(TXEN0) | _BV(RXEN0); /* rx, tx enable */
  /* Weighing mc uart 1 */
  UCSR1C = 0x06 | _BV(USBS0);
  UBRR1L = ui1;
  UBRR1H = ui1 >> 8;
  UCSR1B = _BV(RXEN1); /* rx enable */

  /* For UART0 select */
  DDRE &= ~_BV(PE0); DDRE |= _BV(PE1);
  PORTE &= ~_BV(PE0); PORTE |= _BV(PE1);
  /* For UART1 select */
  DDRD &= ~_BV(PD2); DDRA |= _BV(PA4);
  PORTD &= ~_BV(PD2); PORTA |= _BV(PA4);

  /* Enable the USART Recieve Complete interrupt (USART_RXC) */
  UCSR0B |= _BV(RXCIE0);
  UCSR1B |= _BV(RXCIE1);

  /* init */
  uartWeight = 0;
}

ISR(USART1_RX_vect)
{
  uint8_t ReceivedByte;
  static uint8_t _decimal = 0;
  static uint32_t _weight = 0;

  ReceivedByte = UDR1;

  if ( ('0' <= ReceivedByte) && ('9' >= ReceivedByte) && (_decimal < 4) ) {
    _weight *= 10;
    _weight += ReceivedByte-'0';
    _decimal = (0 == _decimal) ? 0 : _decimal+1;
  } else if ('.' == ReceivedByte) {
    _decimal = 1;
  } else {
    for (; _decimal<4; _decimal++) {
      _weight *= 10;
    }
    uartWeight = _weight;
    _weight = 0;
    _decimal = 0;
  }
}

//**************************************************
//Function to receive a single byte
//*************************************************
uint8_t
uart0ReceiveByte( void )
{
  uint8_t data, status;

  // Wait for incomming data
  while(!(UCSR0A & _BV(RXC0)));

  status = UCSR0A;
  data = UDR0;

  return(data);
}
#if 0
uint8_t
uart1ReceiveByte( void )
{
  uint8_t data, status;

  // Wait for incomming data
  while(!(UCSR1A & _BV(RXC1)));

  status = UCSR1A;
  data = UDR1;

  return(data);
}
#endif

//***************************************************
//Function to transmit a single byte
//***************************************************
void
uart0TransmitByte( uint8_t data )
{
  while ( !(UCSR0A & _BV(UDRE0)) )
    ; 			                /* Wait for empty transmit buffer */
  UDR0 = data; 			        /* Start transmition */
}
#if 0
static uint8_t uart0TxBitsLeft;
static uint16_t uart0TxBuffer;

void
uart1TransmitByte( uint8_t data )
{
  while ( uart0TxBitsLeft ) {
    ; // wait for transmitter ready
    // add watchdog-reset here if needed;
  }

  /* 1 start + 8 data + 2 stop */
  uart0TxBitsLeft = 8+3;
  uart0TxBuffer = ((uint16_t)data << 1) | ((uint16_t)3<<9);
}

/* FIXME: Needs to be called every baud rate */
void
uart1TransmitBit(void)
{
  if (0 == uart0TxBitsLeft)
    return;

  uart0TxBitsLeft--;
  (uart0TxBuffer & 1) ? (PORTA |= _BV(4)) : (PORTA &= ~_BV(4));
  uart0TxBuffer >>= 1;
}
#else
void
uart1TransmitByte( uint8_t data )
{
  cli();

  /* 1 start + 8 data + 2 stop */
  uint16_t uart1TxBuffer = ((uint16_t)data << 1) | ((uint16_t)3<<9);
  for (uint8_t ui1=11; ui1; ui1--) {
    (uart1TxBuffer & 1) ? (PORTA |= _BV(PA4)) : (PORTA &= ~_BV(PA4));
    uart1TxBuffer >>= 1;
    _delay_us(104); /* 9600 bits per second */
  }

  PORTA |= _BV(PA4);
  sei();
}
#endif
