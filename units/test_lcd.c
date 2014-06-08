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

int
main(void)
{
  LCD_init();
  printw("Press F1 to exit");
  LCD_WR_LINE(0, 0, "0123456789012345");
  LCD_WR_LINE(1, 0, "1234567890123456");
  LCD_WR_LINE(2, 0, "2345678901234567");
  LCD_WR_LINE(3, 0, "3456789012345678");
  LCD_WR_LINE(4, 0, "4567890123456789");
  LCD_WR_LINE(5, 0, "5678901234567890");
  LCD_WR_LINE(6, 0, "6789012345678901");
  LCD_WR_LINE(7, 0, "7890123456789012");
  LCD_WR_LINE(8, 0, "8901234567890123");
  LCD_WR_LINE(9, 0, "9012345678901234");
  LCD_WR_LINE(10, 0, "0123456789012345");
  LCD_WR_LINE(11, 0, "1234567890123456");
  LCD_WR_LINE(12, 0, "2345678901234567");
  LCD_WR_LINE(13, 0, "3456789012345678");
  LCD_WR_LINE(14, 0, "4567890123456789");
  LCD_WR_LINE(15, 0, "5678901234567890");
  LCD_refresh();
  getch();
  LCD_end();
}
