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
  spi_init();
  _delay_ms(1000);
  TftInit();
  _delay_ms(1000);
  TftClear(RED);
  BACK_COLOR=BLACK;
  POINT_COLOR=WHITE;

  while(1) {
    TftInit();
    TftClear(RED);
    TftShowString(0, 0, "Hello World");
    _delay_ms(1000);
    TftClear(GREEN);
    TftShowString(0, 0, "Hello World");
    _delay_ms(1000);
    TftClear(BLUE);
    TftShowString(0, 0, "Hello World");
    _delay_ms(1000);
  }

}
