#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "lcd.c"
#include "i2c.c"

int
main(void)
{
  uint8_t ui1;
  s_time_t time, time1;
  date_t date, date1;

  LCD_init();
  i2c_init();
  LCD_bl_on;

  _delay_ms(1000);
  date = (date_t){27, 2, 2017};
  time = (s_time_t){23, 59, 30};

  timerDateSet(date);
  timerTimeSet(time);

  sei();
  LCD_CLRLINE(0);
  LCD_WR_P(PSTR("RTC Test!!!"));

  for (ui1=0; ; ui1++) {
    LCD_cmd((LCD_CMD_CUR_10|0xE));
    LCD_PUTCH(('a'+(ui1&0xF)));
    LCD_CLRLINE(1);

    timerDateGet(date1);
    LCD_PUT_UINT(date1.day);
    LCD_PUTCH('/');
    LCD_PUT_UINT(date1.month);
    //LCD_PUTCH('/');
    //LCD_PUT_UINT(date1.year);
    LCD_PUTCH(' ');

    timerTimeGet(time1);
    LCD_PUT_UINT(time1.hour);
    LCD_PUTCH(':');
    LCD_PUT_UINT(time1.min);
    LCD_PUTCH(':');
    LCD_PUT_UINT(time1.sec);

    LCD_refresh();
    _delay_ms(500);
  }

  return 0;
}
