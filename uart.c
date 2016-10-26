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

uint32_t uartWeight;
uint8_t  uartDecimalPlace;
uint8_t uart0_func;
uint8_t pcPassword[PCPASS_SIZE];
uint8_t pcPassIdx;

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
  UCSRC = (1 << URSEL) | 0x06 | _BV(USBS);
  UBRRL = ui1;
  UBRRH = ui1 >> 8;
  UCSRB = _BV(TXEN) | _BV(RXEN);		/* rx, tx enable */
#endif

  /* For UART select */
  DDRD |= (3<<5);
  PORTD &= ~(3<<5);

  /* Enable the USART Recieve Complete interrupt (USART_RXC) */
  UCSRB |= (1 << RXCIE);

  /* init */
  uartDecimalPlace = 0;
  uartWeight = 0;
  uart0_func = UART0_NONE;
  pcPassIdx = 0;

  assert(0 == (PCPASS_SIZE&0x3)); /* used to read values from 24cXX */
}

void
uartSelect(uint8_t uid)
{
  PORTD |= (uid & 3) << 5;
}

/*
** Translation Table to decode (created by author)
*/
static const uint8_t cd64[] PROGMEM =
  "|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

/*
** decodeblock
**
** decode 4 '6-bit' characters into 3 8-bit binary bytes
*/
static void
b64decode( uint8_t *in, uint8_t *out )
{   
  out[ 0 ] = (in[0] << 2 | in[1] >> 4);
  out[ 1 ] = (in[1] << 4 | in[2] >> 2);
  out[ 2 ] = (((in[2] << 6) & 0xc0) | in[3]);
}

/*
** decode
**
** decode a base64 encoded stream discarding padding, line breaks and noise
*/
static void
decode( uint8_t *in, uint8_t *out )
{
  uint8_t len, in_idx, out_idx;
  uint8_t ui1, v;

  for (len=0, in_idx=out_idx=0; len<12;
       len+=3, in_idx+=4, out_idx+=3) {
    for (ui1=0; ui1<4; ui1++) {
      v = in[in_idx+ui1];
      v = (v < 43 || v > 122) ? (uint8_t) 0 :
	pgm_read_byte (cd64+v-43);
      if (v != 0) {
	v = ((v == (uint8_t)'$') ? 0 : v - 61);
	in[in_idx+ui1] = (uint8_t) (v - 1);
      } else {
	in[in_idx+ui1] = (uint8_t) 0;
      }
    }

    b64decode (in+in_idx, out+out_idx);
  }
}

ISR(USART_RXC_vect)
{
  uint8_t ReceivedByte, ui8_1;

  if (0 != uartDecimalPlace)
    uartDecimalPlace ++;

  ReceivedByte = UDR;

  /* */
  if ( ('\n' == ReceivedByte) || ('\r' == ReceivedByte) ) {
    pcPassIdx = 0;
    uartWeight = 0;
    uartDecimalPlace = 0;
  }

  if (UART0_WEIGHMC == uart0_func) {
    if ( ('0' <= ReceivedByte) && ('9' >= ReceivedByte) ) {
      uartWeight *= 10;
      uartWeight += ReceivedByte-'0';
      uartDecimalPlace = (0 == uartDecimalPlace) ? 0 : uartDecimalPlace+1;
    } else if ('.' == ReceivedByte) {
      uartDecimalPlace = 1;
    }
  } else if (UART0_NONE == uart0_func) {
    pcPassword[pcPassIdx++] = ReceivedByte;
    if ('\n' == ReceivedByte) pcPassIdx = 0;
    if (pcPassIdx>=PCPASS_SIZE) {
      pcPassIdx = 0;
      decode(pcPassword, pcPassword);
      for (ui8_1=0; ui8_1<SERIAL_NO_MAX; ui8_1++)
	if (pcPassword[ui8_1] !=
	    eeprom_read_byte((uint8_t *)offsetof(struct ep_store_layout, unused_serial_no)+ui8_1))
	  break;
      if (ui8_1 == SERIAL_NO_MAX)
	uart0_func = UART0_PC;
    }
  } else if (UART0_PC == uart0_func) {
    pcPassword[pcPassIdx++] = ReceivedByte;
    if (('\n' == ReceivedByte) || (pcPassIdx>=PCPASS_SIZE))
      pcPassIdx = 0;
    else if (('\r' == ReceivedByte) && pcPassIdx)
      uart0_func = UART0_PC_CMD;
  }
}

//**************************************************
//Function to receive a single byte
//*************************************************
uint8_t
uartReceiveByte( void )
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
uartTransmitByte( uint8_t data )
{
  while ( !(UCSRA & (1<<UDRE)) )
    ; 			                /* Wait for empty transmit buffer */
  UDR = data; 			        /* Start transmition */
}
