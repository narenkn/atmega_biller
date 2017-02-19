#include <stdint.h>
#include <avr/io.h>
#include "spi.h"

/* SPI initialize for SD card */
/* clock rate: 125Khz */
void
spiInit(void)
{
  DDRB |=  (1<<0) | (1<<1) | (1<<2);
  DDRB &=  ~_BV(3);
  PORTB |=  (1<<0);
  PORTB &= ~((1<<1) | (1<<2));

  /* setup SPI: Master mode, MSB first,
     SCK phase low, SCK idle low */
  SPCR = (1<<SPE)|(1<<MSTR);
  SPCR |= (1<<SPR1)|(1<<SPR0);
  SPSR = 0x00;

#if NVFLASH_EN
  /* */
  PORTF |= 0xF;
  DDRF |= 0xF;
#endif
}

uint8_t
spiTransmit(uint8_t data)
{
  /* Start transmission */
  SPDR = data;

  /* Wait for transmission complete */
  while(!(SPSR & (1<<SPIF)));

  return SPDR;
}
