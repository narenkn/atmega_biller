#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "uart.h"

#define UART_DEV_PRINTER 3

static uint8_t uart_device;

void
uartInit(void)
{
}

void
uartSelect(uint8_t uid)
{
  uart_device = uid;
}

uint8_t
uartReceiveByte( void )
{
  assert(UART_DEV_PRINTER != uart_device);
  assert(0);
  return 0;
}

void
uartTransmitByte( uint8_t data )
{
  assert(0);
}

void
uartTransmitHex( uint8_t dataType, unsigned long data )
{
  assert(0);
}

//***************************************************
//Function to transmit a string in Flash
//***************************************************
void
uartTransmitString_P(uint8_t* string)
{
  uint8_t ui8_1;
  while (ui8_1 = pgm_read_byte(&(string[0]))) {
    uartTransmitByte(ui8_1);
    string++;
  }
}

//***************************************************
//Function to transmit a string in RAM
//***************************************************
void
uartTransmitString(uint8_t* string)
{
  while (*string)
    uartTransmitByte(*string++);
}
