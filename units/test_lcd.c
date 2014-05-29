#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#include <assert.c>

#include <avr/pgmspace.h>

#define TEST_KEY_ARR_SIZE 128

#include "lcd.c"
#include "kbd.c"
#include "ep_store.c"
#include "i2c.c"
//#include "menu.c"

int
main(void)
{
  LCD_init();
  printw("Press F1 to exit");
  LCD_WR_LINE(0, 0, "0123456789012345");
  LCD_WR_LINE(1, 0, "0123456789012345");
  LCD_WR_LINE(2, 0, "0123456789012345");
  LCD_WR_LINE(3, 0, "0123456789012345");
  LCD_WR_LINE(4, 0, "0123456789012345");
  LCD_WR_LINE(5, 0, "0123456789012345");
  LCD_WR_LINE(6, 0, "0123456789012345");
  LCD_WR_LINE(7, 0, "0123456789012345");
  LCD_WR_LINE(8, 0, "0123456789012345");
  LCD_WR_LINE(9, 0, "0123456789012345");
  LCD_WR_LINE(10, 0, "0123456789012345");
  LCD_WR_LINE(11, 0, "0123456789012345");
  LCD_WR_LINE(12, 0, "0123456789012345");
  LCD_WR_LINE(13, 0, "0123456789012345");
  LCD_WR_LINE(14, 0, "0123456789012345");
  LCD_WR_LINE(15, 0, "0123456789012345");
  LCD_refresh();
  getch();
  LCD_end();
}
