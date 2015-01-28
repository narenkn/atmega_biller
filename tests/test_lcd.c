#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/crc16.h>
#include <avr/eeprom.h>
#include <util/twi.h>

#include "lcd.c"

int
main()
{
  LCD_init();

  DDRD |= 0x10 ; LCD_bl_on;

  LCD_CLRSCR;
  LCD_WR_NP(PSTR("Hello World 7"), 13);
  _delay_ms(1000);
  LCD_CLRLINE(1);
  LCD_WR_NP(PSTR("Hello World 8"), 13);
  _delay_ms(1000);

  while (1) {}

  return 0;
}
