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
#include "menu.h"
#include "main.h"

uint8_t lcd_buf[LCD_MAX_ROW][LCD_MAX_COL];
uint8_t lcd_buf_prop;
uint8_t *lcd_buf_p;

void
LCD_init(void)
{
  uint8_t ui8_1;

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
  for (ui8_1=1; ui8_1; ui8_1++)
    LCD_cmd(LCD_CMD_2LINE_5x7);
  _delay_ms(100);

  /* Display on, Curson blinking command */
  LCD_cmd(LCD_CMD_DISON_CURON_BLINKON);
  _delay_ms(100);

  /* Clear LCD */
  for (ui8_1=1; ui8_1; ui8_1++)
    LCD_cmd(LCD_CMD_CLRSCR);
  LCD_CLRSCR;
  _delay_ms(100);

  /* Entry mode, auto increment with no shift */
  for (ui8_1=1; ui8_1; ui8_1++)
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

#if LCD_USE_FUNCTIONS

void
lcd_clrscr()
{
  uint16_t ui1_t;
  lcd_buf_p = (uint8_t *)lcd_buf;
  for (ui1_t=0; ui1_t<(LCD_MAX_COL*LCD_MAX_ROW); ui1_t++) {
    lcd_buf_p[0] = ' ';
    lcd_buf_p++;
  }
  lcd_buf_p = (uint8_t *)lcd_buf;
  lcd_buf_prop |= LCD_PROP_DIRTY;
}

void
LCD_WR_LINE(x, y, str)
{
  uint8_t ui1_t, ui2_t;
  lcd_buf_p = &(lcd_buf[x][y]);
  for (ui1_t=0, ui2_t=0; (ui1_t<LCD_MAX_COL); ui1_t++) {
    if ( 0 == ((lcd_buf_p-lcd_buf[0])%LCD_MAX_COL) )
      break;
    if (0 == ((char *)str)[ui2_t]) {
      lcd_buf_p[0] = ' ';
    } else {
      lcd_buf_p[0] = ((char *)str)[ui2_t];
      ui2_t++;
    }
    lcd_buf_p++;
    if ((ui1_t+1)<LCD_MAX_COL)
      assert(0 != ((lcd_buf_p-(uint8_t*)lcd_buf)%LCD_MAX_COL));
  }
  lcd_buf_prop |= LCD_PROP_DIRTY;
}

void
LCD_WR_LINE_P(uint8_t x, uint8_t y, uint16_t str)
{
  uint8_t ui1_t, ui2_t;
  lcd_buf_p = &(lcd_buf[x][y]);
  for (ui1_t=0, ui2_t=0; (ui1_t<LCD_MAX_COL); ui1_t++) {
    if ( 0 == ((lcd_buf_p-lcd_buf[0])%LCD_MAX_COL) )
      break;
    if (0 == pgm_read_byte(str+ui2_t)) {
      lcd_buf_p[0] = ' ';
    } else {
      lcd_buf_p[0] = pgm_read_byte(str+ui2_t);
      ui2_t++;
    }
    lcd_buf_p++;
    if ((ui1_t+1)<LCD_MAX_COL)
      assert(0 != ((lcd_buf_p-(uint8_t*)lcd_buf)%LCD_MAX_COL));
  }
  lcd_buf_prop |= LCD_PROP_DIRTY;
}

void
LCD_WR_LINE_N(uint8_t x, uint8_t y, uint8_t *str, uint8_t len)
{
  uint8_t ui1_t;
  lcd_buf_p = &(lcd_buf[x][y]);
  for (ui1_t=0; (0 != (str+ui1_t)[0]) && (ui1_t<len) && (ui1_t < LCD_MAX_COL); ui1_t++) {
    if ( 0 == ((lcd_buf_p-lcd_buf[0])%LCD_MAX_COL) )
      break;
    lcd_buf_p[0] = (str+ui1_t)[0];
    lcd_buf_p++;
    if ((ui1_t+1)<len)
      assert(0 != ((lcd_buf_p-(uint8_t*)lcd_buf)%LCD_MAX_COL));
  }
  for (; 0 != (ui1_t<LCD_MAX_COL); ui1_t++) {
    if ( 0 == ((lcd_buf_p-lcd_buf[0])%LCD_MAX_COL) )
      break;
    lcd_buf_p[0] = ' ';
    lcd_buf_p++;
  }
  lcd_buf_prop |= LCD_PROP_DIRTY;
}

void
LCD_WR_LINE_NP(uint8_t x, uint8_t y, uint8_t *str, uint8_t len)
{
  uint8_t ui1_t;
  lcd_buf_p = &(lcd_buf[x][y]);
  for (ui1_t=0; (ui1_t<len); ui1_t++) {
    if ( 0 == ((lcd_buf_p-lcd_buf[0])%LCD_MAX_COL) )
      break;
    lcd_buf_p[0] = pgm_read_byte(str+ui1_t);
    if (0 == lcd_buf_p[0]) break;
    lcd_buf_p++;
    if ((ui1_t+1)<len)
      assert(0 != ((lcd_buf_p-(uint8_t*)lcd_buf)%LCD_MAX_COL));
  }
  for (; ui1_t < LCD_MAX_COL; ui1_t++) {
    if ( 0 == ((lcd_buf_p-lcd_buf[0])%LCD_MAX_COL) )
      break;
    lcd_buf_p[0] = ' ';
    lcd_buf_p++;
  }
  lcd_buf_prop |= LCD_PROP_DIRTY;
}

void
LCD_WR_N(uint8_t *str, uint8_t len)
{
  uint8_t ui1_t;
  for (ui1_t=0; (0 != str[ui1_t]) && (ui1_t<len); ui1_t++) {
    if ( 0 == ((lcd_buf_p-lcd_buf[0])%LCD_MAX_COL) )
      break;
    lcd_buf_p[0] = str[ui1_t];
    lcd_buf_p++;
    if ((ui1_t+1)<len)
      assert(0 != ((lcd_buf_p-(uint8_t*)lcd_buf)%LCD_MAX_COL));
  }
  for (; (ui1_t<len); ui1_t++) {
    if ( 0 == ((lcd_buf_p-lcd_buf[0])%LCD_MAX_COL) )
      break;
    lcd_buf_p[0] = ' ';
    lcd_buf_p++;
    if ((ui1_t+1)<len)
      assert(0 != ((lcd_buf_p-(uint8_t*)lcd_buf)%LCD_MAX_COL));
  }
  lcd_buf_prop |= LCD_PROP_DIRTY;
}

void
LCD_WR_P(uint16_t str)
{
  uint8_t ui1_t;
  for (ui1_t=0; 0 != pgm_read_byte(str+ui1_t); ui1_t++) {
    if ( 0 == ((lcd_buf_p-lcd_buf[0])%LCD_MAX_COL) )
      break;
    lcd_buf_p[0] = pgm_read_byte(str+ui1_t);
    lcd_buf_p++;
    assert(0 != ((lcd_buf_p-(uint8_t*)lcd_buf)%LCD_MAX_COL));
  }
  lcd_buf_prop |= LCD_PROP_DIRTY;
}

void
LCD_PUT_UINT8X(uint8_t ch)
{
  uint8_t ui2_a = (ch>>4) & 0xF;
  lcd_buf_p[0] = ((ui2_a>9) ? 'A'-10 : '0') + ui2_a;
  lcd_buf_p++;
  assert(0 != ((lcd_buf_p-(uint8_t*)lcd_buf)%LCD_MAX_COL));
  ui2_a = ch & 0xF;
  lcd_buf_p[0] = ((ui2_a>9) ? 'A'-10 : '0') + ui2_a;
  lcd_buf_p++;
  lcd_buf_prop |= LCD_PROP_DIRTY;
  assert(0 != ((lcd_buf_p-(uint8_t*)lcd_buf)%LCD_MAX_COL));
}

void
LCD_PUT_UINT16X(uint16_t ch)
{
  uint8_t ui2_a = (ch>>12) & 0xF;
  lcd_buf_p[0] = ((ui2_a>9) ? 'A'-10 : '0') + ui2_a;
  lcd_buf_p++;
  assert(0 != ((lcd_buf_p-(uint8_t*)lcd_buf)%LCD_MAX_COL));
  ui2_a = (ch>>8) & 0xF;
  lcd_buf_p[0] = ((ui2_a>9) ? 'A'-10 : '0') + ui2_a;
  lcd_buf_p++;
  assert(0 != ((lcd_buf_p-(uint8_t*)lcd_buf)%LCD_MAX_COL));
  ui2_a = (ch>>4) & 0xF;
  lcd_buf_p[0] = ((ui2_a>9) ? 'A'-10 : '0') + ui2_a;
  lcd_buf_p++;
  assert(0 != ((lcd_buf_p-(uint8_t*)lcd_buf)%LCD_MAX_COL));
  ui2_a = ch & 0xF;
  lcd_buf_p[0] = ((ui2_a>9) ? 'A'-10 : '0') + ui2_a;
  lcd_buf_p++;
  lcd_buf_prop |= LCD_PROP_DIRTY;
  assert(0 != ((lcd_buf_p-(uint8_t*)lcd_buf)%LCD_MAX_COL));
}

void
LCD_PUTCH(uint8_t ch)
{
  lcd_buf_p[0] = ch;
  lcd_buf_p++;
  lcd_buf_prop |= LCD_PROP_DIRTY;
  assert(0 != ((lcd_buf_p-(uint8_t*)lcd_buf)%LCD_MAX_COL));
}

void
LCD_WR_SPRINTF(uint8_t x, uint8_t y, uint8_t *BUF, uint8_t *FMT, uint8_t N)
{
  uint8_t ui8_1;
  sprintf(BUF, FMT, N);
  lcd_buf_p = &(lcd_buf[x][y]);
  for (ui8_1=0; 0!=BUF[ui8_1]; ui8_1++)  {
    if ( 0 == ((lcd_buf_p-lcd_buf[0])%LCD_MAX_COL) )
      break;
    lcd_buf_p[0] = BUF[ui8_1];
    lcd_buf_p++;
  }
}

void
LCD_WR_LINE_N_EE24XX(uint8_t x, uint8_t y, uint16_t str, uint8_t len)
{
  uint8_t ui1_t;
  lcd_buf_p = &(lcd_buf[x][y]);
  ee24xx_read_bytes((uint16_t)(str), lcd_buf_p, len);
  for (ui1_t = len, lcd_buf_p+=len; ui1_t < LCD_MAX_COL; ui1_t++) {
    if ( 0 == ((lcd_buf_p-lcd_buf[0])%LCD_MAX_COL) )
      break;
    lcd_buf_p[0] = ' ';
    lcd_buf_p++;
  }
  lcd_buf_prop |= LCD_PROP_DIRTY;
}

#endif
