#include <stdint.h>
#include <avr/io.h>
#include "spi.h"

/* SPI initialize for SD card */
/* clock rate: 125Khz */
void
spiInit(void)
{
  DDRB |=  (1<<0) | (1<<1) | (1<<2);
  DDRB &=  ~(_BV(3) | _BV(4));
  PORTB |=  (1<<0) | (1<<1) | (1<<2);
  PORTB &=  ~(_BV(3) | _BV(4));

  /* setup SPI: Master mode, MSB first,
     SCK phase low, SCK idle low, 2xSPI */
  SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR1);
  SPSR = 0x0;

  /* */
  PORTF |= 0xF;
  DDRF |= 0xF;
}

#if 1
uint8_t
spiTransmit(uint8_t data)
{
  /* Start transmission */
  SPDR = data;

  /* Wait for transmission complete */
  while(!(SPSR & (1<<SPIF)));

  return ~SPDR;
}
#else

#define SCK   1
#define MOSI  2
#define MISO  3
uint8_t
spiTransmit(uint8_t dat)
{
    uint8_t cnt = 8;
    while (cnt--) {
       if (dat & 0x80) PORTB |= (1<<MOSI);
       else PORTB &= ~(1<<MOSI);
       PORTB |= (1<<SCK);
       dat <<= 1;
       if (PINB & (1<<MISO)) dat++;
       PORTB &= ~(1<<SCK);
    }
    return ~dat;
}
#endif
