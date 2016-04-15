#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "uart.h"

uint32_t uartWeight;
uint8_t  uartDecimalPlace;
static uint8_t uart0_func;
#define PCPASS_SIZE ((((SERIAL_NO_MAX-2)/3)*4)+2)
uint8_t pcPassword[PCPASS_SIZE];
uint8_t pcPassIdx = 0;

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
}

void
uartSelect(uint8_t uid)
{
  PORTD |= (uid & 3) << 5;
}

inline void
uart0_sel_func(uint8_t func)
{
  if (0 == uart0_func)
    uart0_func = func;
}

#if 1
volatile uint8_t ReceivedByte;
ISR(USART_RXC_vect)
{
  ReceivedByte = UDR;
}
#else
ISR(USART_RXC_vect)
{
  uint8_t ReceivedByte;

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
  } else {
    pcPassword[pcPassIdx] = ReceivedByte;
    ReceivedByte++;
    if (ReceivedByte>=PCPASS_SIZE) {
      uint16_t ui16_1 = 0;
      ReceivedByte = 0;
      for (uint8_t ui1=0; ui1<(PCPASS_SIZE-2); ui1++) {
	ui16_1 = _crc16_update(ui16_1, pcPassword[ui1]);
      }
      if ( ((ui16_1 & OxFF) == pcPassword[PCPASS_SIZE-1]) &&
	   ( ((ui16_1>>8)&0xFF) == pcpass_size[PCPASS_SIZE-2] ) )
    }
  }

}
#endif

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
