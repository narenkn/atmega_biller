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
    for (ui1=0; ui1<LCD_MAX_ROW; ui1++)
      for (ui2=0; ui2<LCD_MAX_COL; ui2++) {
	mvaddch(ui1+LCD_WIN_STARTX+1, ui2+LCD_WIN_STARTY+1, lcd_buf[ui1][ui2]);
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
