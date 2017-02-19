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
  LCD_bl_on;
  LCD_CLRSCR;
  LCD_WR_NP(PSTR("Hello World!"), 13);
  _delay_ms(1000);

  uint8_t buf[LCD_MAX_COL];
  while (1) {
    for (uint8_t c=' '; c<='~'; c++) {
      for (uint8_t idx=0; idx<(LCD_MAX_COL-1); idx++)
	buf[idx] = buf[idx+1];
      buf[LCD_MAX_COL-1] = c;
      _delay_ms(500);
      LCD_CLRLINE(1);
      LCD_WR_N(buf, LCD_MAX_COL);
    }
  }

  return 0;
}
