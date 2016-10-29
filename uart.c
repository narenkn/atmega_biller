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

static volatile uint8_t uart0TxBusy;
static uint8_t uart0TxBitsLeft;
static uint16_t uart0TxBuffer;
volatile uint32_t uartWeight;
volatile uint8_t  uartDecimalPlace;

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

  UCSR0B = 0x00;
  UCSR0A = 0x00;
  UCSR1B = 0x00;
  UCSR1A = 0x00;
#if F_CPU <= 1000000UL
  /*
   * Note [4]
   * Slow system clock, double Baud rate to improve rate error.
   */
  UCSR0A = _BV(U2X0);
  UCSR1A = _BV(U2X1);
  ui1 = (F_CPU / (8 * 9600UL)) - 1; /* 9600 Bd */
#else
  ui1 = (F_CPU / (16 * 9600UL)) - 1; /* 9600 Bd */
#endif
  /* Printer uart 0 */
  UCSR0C = (1 << UMSEL0) | 0x06 | _BV(USBS0);
  UBRR0L = ui1;
  UBRR0H = ui1 >> 8;
  UCSR0B = _BV(TXEN0) | _BV(RXEN0); /* rx, tx enable */
  /* Weighing mc uart 1 */
  UCSR1C = (1 << UMSEL1) | 0x06 | _BV(USBS0);
  UBRR1L = ui1;
  UBRR1H = ui1 >> 8;
  UCSR1B = _BV(RXEN1); /* rx enable */
#endif

  /* For UART0 select */
  DDRE &= ~_BV(PE0); DDRE |= _BV(PE1);
  PORTE &= ~(_BV(PE0)|_BV(PE1));
  /* For UART1 select */
  DDRD &= ~_BV(PD2); DDRA |= _BV(PA4);
  PORTD &= ~_BV(PD2); PORTA &= ~_BV(PA4);

  /* Enable the USART Recieve Complete interrupt (USART_RXC) */
  UCSR0B |= _BV(RXCIE0);
  UCSR1B |= _BV(RXCIE1);

  /* init */
  uartDecimalPlace = 0;
  uartWeight = 0;
}

ISR(USART1_RX_vect)
{
  uint8_t ReceivedByte;

  if (0 != uartDecimalPlace)
    uartDecimalPlace ++;

  ReceivedByte = UDR1;

  /* */
  if ( ('\n' == ReceivedByte) || ('\r' == ReceivedByte) ) {
    uartWeight = 0;
    uartDecimalPlace = 0;
  }

  if ( ('0' <= ReceivedByte) && ('9' >= ReceivedByte) ) {
    uartWeight *= 10;
    uartWeight += ReceivedByte-'0';
    uartDecimalPlace = (0 == uartDecimalPlace) ? 0 : uartDecimalPlace+1;
  } else if ('.' == ReceivedByte) {
    uartDecimalPlace = 1;
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
  while ( !(UCSR1A & _BV(UDRE1)) )
    ; 			                /* Wait for empty transmit buffer */
  UDR1 = data; 			        /* Start transmition */
}
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
