#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#include <assert.c>

#include <avr/pgmspace.h>

#define TEST_KEY_ARR_SIZE 128

#include "i2c.c"
#include "lcd.c"

int
main(void)
{
  LCD_init();
  printw("Press F1 to exit");
  LCD_WR("0123456789012345");
  LCD_CLRLINE(1);
  LCD_WR("1234567890123456");
  LCD_refresh();
  getch();
  LCD_end();
}
