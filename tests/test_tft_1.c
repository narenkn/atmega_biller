#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "spi.c"
#include "tft.c"

int
main()
{
  _delay_ms(1000);
  TFT_BL_ON;
  _delay_ms(1000);
  spi_init();
  _delay_ms(1000);
  TftInit();
  TFT_BL_OFF;
  _delay_ms(1000);
  TFT_BL_ON;
  TftClear(RED);
  BACK_COLOR=BLACK;
  POINT_COLOR=WHITE;

  while(1) {		
    TftInit();
    TftClear(RED);
    _delay_ms(1000);
    TftClear(GREEN);
    _delay_ms(1000);
    TftClear(BLUE);
    _delay_ms(1000);
  }

}
