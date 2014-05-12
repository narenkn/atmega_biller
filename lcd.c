#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

#include "lcd.h"

uint8_t lcd_buf[LCD_MAX_ROW][LCD_MAX_COL];
uint8_t lcd_buf_prop;
uint8_t *lcd_buf_p;

void
LCD_init(void)
{
  /* Set IO directions */
  DDRD |= 0x10;
  DDRA |= 0x0F;
  DDRC |= 0xC0;

#if 4 == LCD_DPORT_SIZE
  _delay_ms(100);
  LCD_wrnib(3);
  _delay_ms(100);
  LCD_wrnib(3);
  _delay_ms(10);
  LCD_wrnib(3);
  _delay_ms(10);
  LCD_wrnib(2);
  _delay_ms(10);
#endif

  /*  Function set: 2 Line, 8-bit, 5x7 dots */
  LCD_cmd(LCD_CMD_2LINE_5x7);
  LCD_cmd(LCD_CMD_2LINE_5x7);
  LCD_cmd(LCD_CMD_2LINE_5x7);
  LCD_cmd(LCD_CMD_2LINE_5x7);
  LCD_cmd(LCD_CMD_2LINE_5x7);
  _delay_ms(100);

  /* Display on, Curson blinking command */
  LCD_cmd(LCD_CMD_DISON_CURON_BLINKON);
  _delay_ms(100);

  /* Clear LCD */
  LCD_cmd(LCD_CMD_CLRSCR);
  LCD_cmd(LCD_CMD_CLRSCR);
  LCD_cmd(LCD_CMD_CLRSCR);
  LCD_CLRSCR;
  _delay_ms(100);

  /* Entry mode, auto increment with no shift */
  LCD_cmd(LCD_CMD_INC_CUR);
  LCD_cmd(LCD_CMD_INC_CUR);
  _delay_ms(100);

  lcd_buf_prop = 0;
}

void
LCD_refresh(void)
{
  uint8_t ui2, *ui3_p;

  if (0 == (lcd_buf_prop&LCD_PROP_DIRTY)) {
    return;
  }
  lcd_buf_prop &= ~LCD_PROP_DIRTY;

  /**************************************************/
  /*****************   LCD   ************************/
  /**************************************************/
  /* Display on LCD */
  LCD_cmd(LCD_CMD_CUR_10);
  ui3_p = (uint8_t*) lcd_buf;
  for (ui2=0; ui2<LCD_MAX_COL; ui2++) {
    LCD_wrchar(ui3_p[0]);
    ui3_p++;
  }

  LCD_cmd(LCD_CMD_CUR_20);
  for (ui2=0; ui2<LCD_MAX_COL; ui2++) {
    if ((lcd_buf_prop & LCD_PROP_NOECHO_L2) && (' ' != ui3_p[0])) {
      LCD_wrchar('*');
    } else {
      LCD_wrchar(ui3_p[0]);
    }
    ui3_p++;
  }
}
