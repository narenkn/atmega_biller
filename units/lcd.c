#include "lcd.h"
#include <ncurses.h>

static uint8_t lcd_x, lcd_y;
static uint8_t lcd_buf[LCD_MAX_ROW][LCD_MAX_COL];
static uint8_t *lcd_buf_p;
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

  lcd_x = 0;
  lcd_y = 0;

  /* */
  uint8_t ui1;
  for (ui1=1; ui1<LCD_MAX_ROW; ui1++) {
    LCD_CLRLINE(ui1);
  }
  LCD_CLRLINE(0);
}

void
LCD_end(void)
{
  delwin(win);
  endwin();
}

void
lcd_clrscr()
{
  uint8_t ui1, ui2;
  refresh();

  for (ui1=0; ui1<LCD_MAX_ROW; ui1++) {
    for (ui2=0; ui2<LCD_MAX_COL; ui2++) {
      mvaddch(ui1+LCD_WIN_STARTX+1, ui2+LCD_WIN_STARTY+1, ' ');
    }
  }
  wrefresh(win);

  lcd_x = 0;
  lcd_y = 0;

  for (ui1=1; ui1<LCD_MAX_ROW; ui1++) {
    LCD_CLRLINE(ui1);
  }
  LCD_CLRLINE(0);
}

void
LCD_CLRLINE(uint8_t line)
{
  uint8_t ui2;
  refresh();

  assert(line < LCD_MAX_ROW);
  for (ui2=0; ui2<LCD_MAX_COL; ui2++) {
    mvaddch(line+LCD_WIN_STARTX+1, ui2+LCD_WIN_STARTY+1, ' ');
  }
  wrefresh(win);

  lcd_x = line;
  lcd_y = 0;

  for (ui2=0; ui2<LCD_MAX_COL; ui2++) {
    lcd_buf[line][ui2] = ' ';
  }
  lcd_buf_p = lcd_buf[line];
}

void
LCD_WR(char *str)
{
  uint8_t ui1_t;

  refresh();
  for (ui1_t=0; 0 != str[ui1_t]; ui1_t++) {
    mvaddch(lcd_x+LCD_WIN_STARTX+1, ui1_t+lcd_y+LCD_WIN_STARTY+1, str[ui1_t]);
    lcd_buf_p[0] = str[ui1_t]; lcd_buf_p++;
  }
  wrefresh(win);
  lcd_y += ui1_t;
  assert(lcd_y <= LCD_MAX_COL);
}

void
LCD_WR_N(uint8_t *str, uint8_t len)
{
  uint8_t ui1_t;

  refresh();
  assert(len <= LCD_MAX_COL);
  for (ui1_t=0; ui1_t<len; ui1_t++) {
    if (0 == str[ui1_t]) break;
    mvaddch(lcd_x+LCD_WIN_STARTX+1, ui1_t+lcd_y+LCD_WIN_STARTY+1, str[ui1_t]);
    lcd_buf_p[0] = str[ui1_t]; lcd_buf_p++;
  }
  wrefresh(win);
  lcd_y += ui1_t;
  assert(lcd_y <= LCD_MAX_COL);
}

void
LCD_WR_P(const char *str)
{
  uint8_t ui1_t;
  refresh();

  for (ui1_t=0; 0 != str[ui1_t]; ui1_t++) {
    mvaddch(lcd_x+LCD_WIN_STARTX+1, ui1_t+lcd_y+LCD_WIN_STARTY+1, str[ui1_t]);
    lcd_buf_p[0] = str[ui1_t]; lcd_buf_p++;
  }
  wrefresh(win);
  lcd_y += ui1_t;
  assert(lcd_y <= LCD_MAX_COL);
}

void
LCD_WR_NP(const char *str, uint8_t len)
{
  uint8_t ui1_t;

  refresh();

  assert(len <= LCD_MAX_COL);
  for (ui1_t=0; ui1_t<len; ui1_t++) {
    if (0 == str[ui1_t]) break;
    mvaddch(lcd_x+LCD_WIN_STARTX+1, ui1_t+lcd_y+LCD_WIN_STARTY+1, str[ui1_t]);
    lcd_buf_p[0] = str[ui1_t]; lcd_buf_p++;
  }
  wrefresh(win);
  lcd_y += ui1_t;
  assert(lcd_y <= LCD_MAX_COL);
}

void
LCD_PUT_UINT8X(uint8_t ch)
{
  uint8_t ui1_t;

  refresh();

  ui1_t = (ch>>4) & 0xF;
  ui1_t = ((ui1_t>9) ? 'A'-10 : '0') + ui1_t;
  mvaddch(lcd_x+LCD_WIN_STARTX+1, lcd_y+LCD_WIN_STARTY+1, ui1_t);
  lcd_y ++;
  lcd_buf_p[0] = ui1_t; lcd_buf_p++;

  ui1_t = ch & 0xF;
  ui1_t = ((ui1_t>9) ? 'A'-10 : '0') + ui1_t;
  mvaddch(lcd_x+LCD_WIN_STARTX+1, lcd_y+LCD_WIN_STARTY+1, ui1_t);
  lcd_y ++;
  lcd_buf_p[0] = ui1_t; lcd_buf_p++;

  wrefresh(win);
  assert(lcd_y <= LCD_MAX_COL);
}

void
LCD_PUT_UINT16X(uint16_t ch)
{
  uint8_t ui1_t;
  refresh();

  ui1_t = (ch>>12) & 0xF;
  ui1_t = ((ui1_t>9) ? 'A'-10 : '0') + ui1_t;
  mvaddch(lcd_x+LCD_WIN_STARTX+1, lcd_y+LCD_WIN_STARTY+1, ui1_t);
  lcd_y ++;
  lcd_buf_p[0] = ui1_t; lcd_buf_p++;

  ui1_t = (ch>>8) & 0xF;
  ui1_t = ((ui1_t>9) ? 'A'-10 : '0') + ui1_t;
  mvaddch(lcd_x+LCD_WIN_STARTX+1, lcd_y+LCD_WIN_STARTY+1, ui1_t);
  lcd_y ++;
  lcd_buf_p[0] = ui1_t; lcd_buf_p++;

  ui1_t = (ch>>4) & 0xF;
  ui1_t = ((ui1_t>9) ? 'A'-10 : '0') + ui1_t;
  mvaddch(lcd_x+LCD_WIN_STARTX+1, lcd_y+LCD_WIN_STARTY+1, ui1_t);
  lcd_y ++;
  lcd_buf_p[0] = ui1_t; lcd_buf_p++;

  ui1_t = ch & 0xF;
  ui1_t = ((ui1_t>9) ? 'A'-10 : '0') + ui1_t;
  mvaddch(lcd_x+LCD_WIN_STARTX+1, lcd_y+LCD_WIN_STARTY+1, ui1_t);
  lcd_y ++;
  lcd_buf_p[0] = ui1_t; lcd_buf_p++;

  wrefresh(win);
  assert(lcd_y <= LCD_MAX_COL);
}

void
LCD_wrchar(uint8_t ch)
{
  refresh();
  assert(isprint(ch));
  mvaddch(lcd_x+LCD_WIN_STARTX+1, lcd_y+LCD_WIN_STARTY+1, ch);
  lcd_y++;
  lcd_buf_p[0] = ch; lcd_buf_p++;

  assert(lcd_y <= LCD_MAX_COL);
  wrefresh(win);
}

void
LCD_PUTCH(uint8_t var)
{
  LCD_wrchar(var);
}

void
LCD_PUT_UINT(uint32_t val)
{
  uint8_t ui8_1;

  ui8_1 = val % 10;
  ui8_1 += '0';

  val /= 10;
  if (val)
    LCD_PUT_UINT(val);

  LCD_PUTCH(ui8_1);
}

void
lcd_alert(const char *str)
{
  LCD_CLRLINE(LCD_MAX_ROW-1);
  for (uint8_t ui1_t=0; str; ui1_t++) {
    uint8_t ui2_t = str[ui1_t];
    if (0 == ui2_t) break;
    LCD_PUTCH(ui2_t);
  }
}

void
lcd_alert_n(const char *str, uint32_t n)
{
  LCD_CLRLINE(LCD_MAX_ROW-1);
  for (uint8_t ui1_t=0; (str) && (ui1_t < 10); ui1_t++) {
    uint8_t ui2_t = str[ui1_t];
    if (0 == ui2_t) break;
    LCD_PUTCH(ui2_t);
  }
  lcdD(n);
}

void
lcdD(uint32_t var)
{
  if (var>9)
    lcdD(var/10);
  LCD_PUTCH(('0'+(var%10)));
}

void
lcdFd(uint32_t var)
{
  lcdD(var/100);
  LCD_PUTCH('.');
  lcdD(var%100);
}
