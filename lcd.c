#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/crc16.h>
#include <avr/eeprom.h>

#include "ep_ds.h"
#include "version.h"
#include "assert.h"
#include "lcd.h"
#include "kbd.h"
#include "ep_store.h"
#include "billing.h"
#include "i2c.h"
#include "uart.h"
#include "a1micro2mm.h"
#include "main.h"
#include "menu.h"

#ifdef UNIT_TEST
uint8_t    lcd_col_idx;
# define   UNIT_TEST_INIT			\
  lcd_col_idx = 0
# define   UNIT_TEST_COL_INC(a)			\
  lcd_col_idx += a
# define   UNIT_TEST_ASSERT			\
  assert(lcd_col_idx <= LCD_MAX_COL)
#else
# define   UNIT_TEST_INIT
# define   UNIT_TEST_COL_INC(a)
# define   UNIT_TEST_ASSERT
#endif

void
LCD_init(void)
{
  uint8_t ui8_1;

  /* Set IO directions */
  DDRG |= _BV(2);
  DDRD |= 0xF0;
  DDRA |= 0x0F;
  DDRB |= 0xE0;

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
  for (ui8_1=1; ui8_1<5; ui8_1++) {
    LCD_cmd(LCD_CMD_2LINE_5x7);
    _delay_us(100);
  }
  _delay_ms(100);

  /* Display on, Curson blinking command */
  LCD_cmd(LCD_CMD_DISON_CURON_BLINKON);
  _delay_ms(100);

  /* Clear LCD */
  for (ui8_1=1; ui8_1<3; ui8_1++) {
    LCD_cmd(LCD_CMD_CLRSCR);
  }
  _delay_ms(100);

  /* Entry mode, auto increment with no shift */
  for (ui8_1=1; ui8_1<3; ui8_1++) {
    LCD_cmd(LCD_CMD_INC_CUR);
  }
  _delay_ms(100);
}

void
LCD_PUTCH(uint8_t var)
{
  LCD_wrchar(var);
}

void
LCD_CLRLINE(uint8_t n)
{
  uint8_t ui8_1;

  /* which line */
  n = (n&1) ? LCD_CMD_CUR_20 : LCD_CMD_CUR_10;
  LCD_cmd(n);
  for (ui8_1=0; ui8_1<LCD_MAX_COL; ui8_1++) {
    LCD_PUTCH(' ');
  }
  LCD_cmd(n);
}

void
LCD_WR(char *str)
{
  uint8_t ui1_t;

  for (ui1_t=0; (0 != str[ui1_t]); ui1_t++) {
    LCD_PUTCH(str[ui1_t]);
  }
}

void
LCD_WR_N(uint8_t *str, uint8_t len)
{
  uint8_t ui1_t;
  len %= (LCD_MAX_COL+1);
  for (ui1_t=0; (0 != str[ui1_t]) && (ui1_t<len); ui1_t++) {
    LCD_PUTCH(str[ui1_t]);
  }
  for (; (ui1_t<len); ui1_t++) {
    LCD_PUTCH(' ');
  }
}

void
LCD_WR_P(const char *str)
{
  uint8_t ui1_t, ui2_t;
  for (ui1_t=0; str; ui1_t++) {
    ui2_t = pgm_read_byte(str+ui1_t);
    if (0 == ui2_t) break;
    LCD_PUTCH(ui2_t);
  }
}

void
LCD_WR_NP(const char *str, uint8_t len)
{
  uint8_t ui1_t, ui2_t;
  len %= (LCD_MAX_COL+1);
  for (ui1_t=0; (ui1_t<len); ui1_t++) {
    ui2_t = pgm_read_byte(str+ui1_t);
    if (0 == ui2_t) break;
    LCD_PUTCH(ui2_t);
  }
  for (; (ui1_t<len); ui1_t++) {
    LCD_PUTCH(' ');
  }
}

void
LCD_BUSY(void)
{
  static uint8_t busySign[] = "|/-\\", busyBit=0;
  LCD_cmd((LCD_CMD_CUR_10|0xF));
  LCD_PUTCH(busySign[busyBit]);
  busyBit++;
  busyBit &= ~0x3;
}

void
LCD_PUT_UINT8X(uint8_t ch)
{
  uint8_t ui2_a = (ch>>4) & 0xF;

  ui2_a = ((ui2_a>9) ? 'A'-10 : '0') + ui2_a;
  LCD_PUTCH(ui2_a);

  ui2_a = ch & 0xF;
  ui2_a = ((ui2_a>9) ? 'A'-10 : '0') + ui2_a;
  LCD_PUTCH(ui2_a);
}

void
LCD_PUT_UINT16X(uint16_t ch)
{
  uint8_t ui2_a = (ch>>12) & 0xF;

  ui2_a = ((ui2_a>9) ? 'A'-10 : '0') + ui2_a;
  LCD_PUTCH(ui2_a);

  ui2_a = (ch>>8) & 0xF;
  ui2_a = ((ui2_a>9) ? 'A'-10 : '0') + ui2_a;
  LCD_PUTCH(ui2_a);

  ui2_a = (ch>>4) & 0xF;
  ui2_a = ((ui2_a>9) ? 'A'-10 : '0') + ui2_a;
  LCD_PUTCH(ui2_a);

  ui2_a = ch & 0xF;
  ui2_a = ((ui2_a>9) ? 'A'-10 : '0') + ui2_a;
  LCD_PUTCH(ui2_a);
}

void
LCD_PUT_UINT(uint32_t val)
{
  if (val>9)
    LCD_PUT_UINT(val/10);
  LCD_PUTCH(('0'+(val%10)));
}

void
LCD_PUT_FLOAT(uint32_t val)
{
  LCD_PUT_UINT(val / 100);
  LCD_PUTCH('.');
  val %= 100;
  if (val < 10) LCD_PUTCH('0');
  LCD_PUT_UINT(val % 100);
}

void
lcd_alert(const char *str)
{
  LCD_CLRLINE(LCD_MAX_ROW-1);
  for (uint8_t ui1_t=0; str; ui1_t++) {
    uint8_t ui2_t = pgm_read_byte(str+ui1_t);
    if (0 == ui2_t) break;
    LCD_PUTCH(ui2_t);
  }
  LCD_refresh();
}

void
lcd_alert_n(const char *str, uint32_t n)
{
  LCD_CLRLINE(LCD_MAX_ROW-1);
  for (uint8_t ui1_t=0; (str) && (ui1_t < 10); ui1_t++) {
    uint8_t ui2_t = pgm_read_byte(str+ui1_t);
    if (0 == ui2_t) break;
    LCD_PUTCH(ui2_t);
  }
  LCD_PUT_UINT(n);
  LCD_refresh();
}
