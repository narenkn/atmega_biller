
#define  assert(X)

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "lcd.h"
#include "i2c.h"

#include "lcd.c"
#include "i2c.c"

int
main(void)
{
  uint8_t ui1;
  uint8_t ymd[3], hms[3], ui2;

  _delay_ms(1000);
  LCD_init();
  i2c_init();
  LCD_bl_on;

  _delay_ms(1000);
  LCD_WR_LINE(0, 0, "DS1307 Tests!!!");
  LCD_refresh();

  timerDateSet(0x5, 0x5, 0x14);
  timerTimeSet(0x1, 0x53);

  for (ui1=0; ; ui1++) {
    LCD_POS(0, 15);
    LCD_PUTCH('a'+(ui1&0xF));
    timerDateGet(ymd);
    LCD_POS(1, 0);
    LCD_PUTCH('0'+((ymd[0]>>4)&0xF));
    LCD_PUTCH('0'+(ymd[0]&0xF));
    LCD_PUTCH('/');
    LCD_PUTCH('0'+((ymd[1]>>4)&0xF));
    LCD_PUTCH('0'+(ymd[1]&0xF));
    LCD_PUTCH('/');
    LCD_PUTCH('0'+((ymd[2]>>4)&0xF));
    LCD_PUTCH('0'+(ymd[2]&0xF));
    LCD_refresh();
    timerTimeGet(hms);
    LCD_PUTCH('0'+((hms[2]>>4)&0xF));
    LCD_PUTCH('0'+(hms[2]&0xF));
    LCD_PUTCH(':');
    LCD_PUTCH('0'+((hms[1]>>4)&0xF));
    LCD_PUTCH('0'+(hms[1]&0xF));
    LCD_PUTCH(':');
    LCD_PUTCH('0'+((hms[2]>>4)&0xF));
    LCD_PUTCH('0'+(hms[2]&0xF));
    _delay_ms(1000);
  }

  return 0;
}
