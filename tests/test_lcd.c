#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/crc16.h>
#include <avr/eeprom.h>
#include <util/twi.h>

#include "lcd.c"
#include "i2c.c"

int
main()
{
  LCD_init();

  DDRD |= 0x10 ; LCD_bl_on;
  LCD_WR_LINE_NP(0, 0, PSTR("Hello World 7"), 13);
  LCD_refresh();
  _delay_ms(1000);
  LCD_WR_LINE_NP(1, 0, PSTR("Hello World 8"), 13);
  LCD_refresh();
  _delay_ms(1000);

  _delay_ms(4000);
  while (1) {}

  return 0;
}
