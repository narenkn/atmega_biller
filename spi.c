//**************************************************************
// ****** FUNCTIONS FOR SPI COMMUNICATION *******
//**************************************************************
//Controller		: ATmega32 (Clock: 8 Mhz-internal)
//Compiler			: AVR-GCC (winAVR with AVRStudio-4)
//Project Version	: DL_1.0
//Author			: CC Dharmani, Chennai (India)
//			  		  www.dharmanitech.com
//Date				: 10 May 2011
//**************************************************************

#include <stdint.h>
#include <avr/io.h>
#include "spi.h"

/* SPI initialize for SD card */
/* clock rate: 125Khz */
void
spi_init(void)
{
  SPCR = 0x52; //setup SPI: Master mode, MSB first, SCK phase low, SCK idle low
  SPSR = 0x00;
}

uint8_t
SPI_transmit(uint8_t data)
{
  // Start transmission
  SPDR = data;

  // Wait for transmission complete
  while(!(SPSR & (1<<SPIF)));
  data = SPDR;

  return(data);
}

uint8_t
SPI_receive(void)
{
  uint8_t data;
  // Wait for reception complete

  SPDR = 0xff;
  while(!(SPSR & (1<<SPIF)));
  data = SPDR;

  // Return data register
  return data;
}
