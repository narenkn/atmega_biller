#include <stdint.h>
#include <avr/io.h>
#include "spi.h"

/* SPI initialize for SD card */
/* clock rate: 125Khz */
void
spi_init(void)
{
  DDRB |=  (1<<0) | (1<<1) | (1<<3) | (1<<4) | (1<<5) | (1<<7);
  DDRB &=  ~(1<<6);

  /* setup SPI: Master mode, MSB first,
     SCK phase low, SCK idle low */
  SPCR = 0x50;
  SPSR = 0x00;
}

uint8_t
SPI_transmit(uint8_t data)
{
  /* Start transmission */
  SPDR = data;

  /* Wait for transmission complete */
  while(!(SPSR & (1<<SPIF)));
  data = SPDR;

  return(data);
}

uint8_t
SPI_receive(void)
{
  uint8_t data;
  /* Wait for reception complete */

  SPDR = 0xff;
  while(!(SPSR & (1<<SPIF)));
  data = SPDR;

  /* Return data register */
  return data;
}
