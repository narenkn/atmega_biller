#include "lcd.h"
#include <ncurses/ncurses.h>

static uint8_t lcd_x, lcd_y;
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

  lcd_x = LCD_WIN_STARTX;
  lcd_y = LCD_WIN_STARTY;
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

  for (ui1=0; ui1<(LCD_MAX_ROW-1); ui1++)
    for (ui2=0; ui2<LCD_MAX_COL; ui2++) {
      mvaddch(ui1+LCD_WIN_STARTX+1, ui2+LCD_WIN_STARTY+1, ' ');
    }
  ui1 = LCD_MAX_ROW-1;
  for (ui2=0; ui2<LCD_MAX_COL; ui2++) {
    mvaddch(ui1+LCD_WIN_STARTX+1, ui2+LCD_WIN_STARTY+1, ' ');
  }
  wrefresh(win);

  lcd_x = LCD_WIN_STARTX;
  lcd_y = LCD_WIN_STARTY;
}

void
LCD_CLRLINE(uint8_t line)
{
  uint8_t ui2;
  refresh();

  for (ui2=0; ui2<LCD_MAX_COL; ui2++) {
    mvaddch(line+LCD_WIN_STARTX+1, ui2+LCD_WIN_STARTY+1, ' ');
  }
  wrefresh(win);

  lcd_x = line;
}

void
LCD_WR(char *str)
{
  uint8_t ui1_t;

  for (ui1_t=0; 0 != str[ui1_t]; ui1_t++) {
    mvaddch(lcd_x+1, ui1_t+lcd_y+1, str[ui1_t]);
  }
  wrefresh(win);
  refresh();
  lcd_y += ui1_t;
  assert(lcd_y <= LCD_MAX_COL);
}

void
LCD_WR_N(char *str, uint8_t len)
{
  uint8_t ui1_t;

  assert(len <= LCD_MAX_COL);
  for (ui1_t=0; ui1_t<len; ui1_t++) {
    if (0 == str[ui1_t]) break;
    mvaddch(lcd_x+1, ui1_t+lcd_y+1, str[ui1_t]);
  }
  wrefresh(win);
  refresh();
  lcd_y += ui1_t;
  assert(lcd_y <= LCD_MAX_COL);
}

void
LCD_WR_P(const uint8_t *str)
{
  uint8_t ui1_t;

  for (ui1_t=0; 0 != str[ui1_t]; ui1_t++) {
    mvaddch(lcd_x+1, ui1_t+lcd_y+1, str[ui1_t]);
  }
  wrefresh(win);
  refresh();
  lcd_y += ui1_t;
  assert(lcd_y <= LCD_MAX_COL);
}

void
LCD_WR_NP(const uint8_t *str, uint8_t len)
{
  uint8_t ui1_t;

  assert(len <= LCD_MAX_COL);
  for (ui1_t=0; ui1_t<len; ui1_t++) {
    if (0 == str[ui1_t]) break;
    mvaddch(lcd_x+1, ui1_t+lcd_y+1, str[ui1_t]);
  }
  wrefresh(win);
  refresh();
  lcd_y += ui1_t;
  assert(lcd_y <= LCD_MAX_COL);
}

void
LCD_PUT_UINT8X(uint8_t ch)
{
  uint8_t ui1_t;

  ui1_t = (ch>>4) & 0xF;
  ui1_t = ((ui1_t>9) ? 'A'-10 : '0') + ui1_t;
  mvaddch(lcd_x+1, ui1_t+lcd_y+1, ui1_t);
  lcd_y ++;

  ui1_t = ch & 0xF;
  ui1_t = ((ui1_t>9) ? 'A'-10 : '0') + ui1_t;
  mvaddch(lcd_x+1, ui1_t+lcd_y+1, ui1_t);
  lcd_y ++;

  wrefresh(win);
  refresh();
  assert(lcd_y <= LCD_MAX_COL);
}

void
LCD_PUT_UINT16X(uint16_t ch)
{
  uint8_t ui1_t;

  ui1_t = (ch>>12) & 0xF;
  ui1_t = ((ui1_t>9) ? 'A'-10 : '0') + ui1_t;
  mvaddch(lcd_x+1, ui1_t+lcd_y+1, ui1_t);
  lcd_y ++;

  ui1_t = (ch>>8) & 0xF;
  ui1_t = ((ui1_t>9) ? 'A'-10 : '0') + ui1_t;
  mvaddch(lcd_x+1, ui1_t+lcd_y+1, ui1_t);
  lcd_y ++;

  ui1_t = (ch>>4) & 0xF;
  ui1_t = ((ui1_t>9) ? 'A'-10 : '0') + ui1_t;
  mvaddch(lcd_x+1, ui1_t+lcd_y+1, ui1_t);
  lcd_y ++;

  ui1_t = ch & 0xF;
  ui1_t = ((ui1_t>9) ? 'A'-10 : '0') + ui1_t;
  mvaddch(lcd_x+1, ui1_t+lcd_y+1, ui1_t);
  lcd_y ++;

  wrefresh(win);
  refresh();
  assert(lcd_y <= LCD_MAX_COL);
}

void
LCD_wrchar(uint8_t ch)
{
  assert(isgraph(ch));
  mvaddch(lcd_x+1, lcd_y+1, ch);
  lcd_y++;

  wrefresh(win);
  refresh();
  assert(lcd_y <= LCD_MAX_COL);
}
