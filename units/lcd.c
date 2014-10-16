#include "lcd.h"
#include <ncurses/ncurses.h>

uint8_t lcd_buf[LCD_MAX_ROW][LCD_MAX_COL];
uint8_t lcd_buf_prop;
uint8_t *lcd_buf_p;

WINDOW *win;

#define LCD_WIN_STARTX 2
#define LCD_WIN_STARTY 1

void
LCD_init(void)
{
  initscr();
  raw();			/* Line buffering disabled	*/
  keypad(stdscr, TRUE);		/* We get F1, F2 etc..		*/
  noecho();			/* Don't echo() while we do getch */

  win = newwin(LCD_MAX_ROW+2, LCD_MAX_COL+2, LCD_WIN_STARTX, LCD_WIN_STARTY);
  box(win, 0, 0);

  LCD_CLRSCR;
}

void
LCD_refresh(void)
{
  uint8_t ui1, ui2;
  
  refresh();
  if (lcd_buf_prop & LCD_PROP_DIRTY) {
    for (ui1=0; ui1<(LCD_MAX_ROW-1); ui1++)
      for (ui2=0; ui2<LCD_MAX_COL; ui2++) {
	mvaddch(ui1+LCD_WIN_STARTX+1, ui2+LCD_WIN_STARTY+1, lcd_buf[ui1][ui2]);
      }
    ui1 = LCD_MAX_ROW-1;
    for (ui2=0; ui2<LCD_MAX_COL; ui2++) {
      mvaddch(ui1+LCD_WIN_STARTX+1, ui2+LCD_WIN_STARTY+1, ((lcd_buf_prop&LCD_PROP_NOECHO_L2)&&(' '!=lcd_buf[ui1][ui2]))?'*':lcd_buf[ui1][ui2]);
    }
    wrefresh(win);
  }
}

void
LCD_end(void)
{
  delwin(win);
  endwin();
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
LCD_WR_LINE_P(uint8_t x, uint8_t y, uint32_t str)
{
  uint8_t ui1_t, ui2_t;
  lcd_buf_p = &(lcd_buf[x][y]);
  for (ui1_t=0, ui2_t=0; (ui1_t<LCD_MAX_COL); ui1_t++) {
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
    lcd_buf_p[0] = (str+ui1_t)[0];
    lcd_buf_p++;
    if ((ui1_t+1)<len)
      assert(0 != ((lcd_buf_p-(uint8_t*)lcd_buf)%LCD_MAX_COL));
  }
  for (; 0 != (ui1_t<LCD_MAX_COL); ui1_t++) {
    lcd_buf_p[0] = ' ';
    lcd_buf_p++;
  }
  lcd_buf_prop |= LCD_PROP_DIRTY;
}

void
LCD_WR_LINE_NP(uint8_t x, uint8_t y, uint32_t str, uint8_t len)
{
  uint8_t ui1_t;
  lcd_buf_p = &(lcd_buf[x][y]);
  for (ui1_t=0; (ui1_t<len); ui1_t++) {
    lcd_buf_p[0] = pgm_read_byte(str+ui1_t);
    if (0 == lcd_buf_p[0]) break;
    lcd_buf_p++;
    if ((ui1_t+1)<len)
      assert(0 != ((lcd_buf_p-(uint8_t*)lcd_buf)%LCD_MAX_COL));
  }
  for (; ui1_t < LCD_MAX_COL; ui1_t++) {
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
    lcd_buf_p[0] = str[ui1_t];
    lcd_buf_p++;
    if ((ui1_t+1)<len)
      assert(0 != ((lcd_buf_p-(uint8_t*)lcd_buf)%LCD_MAX_COL));
  }
  for (; (ui1_t<len); ui1_t++) {
    lcd_buf_p[0] = ' ';
    lcd_buf_p++;
    if ((ui1_t+1)<len)
      assert(0 != ((lcd_buf_p-(uint8_t*)lcd_buf)%LCD_MAX_COL));
  }
  lcd_buf_prop |= LCD_PROP_DIRTY;
}

void
LCD_WR_P(uint32_t str)
{
  uint8_t ui1_t;
  for (ui1_t=0; 0 != pgm_read_byte(str+ui1_t); ui1_t++) {
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
    lcd_buf_p[0] = ' ';
    lcd_buf_p++;
  }
  lcd_buf_prop |= LCD_PROP_DIRTY;
}

#endif
