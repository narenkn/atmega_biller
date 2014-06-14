#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#include <assert.c>

#include <avr/pgmspace.h>

#define TEST_KEY_ARR_SIZE 128

#define UNIT_TEST_MENU_1 menu_handler


#include "lcd.c"
#include "kbd_ncurses.c"
#include "menu.c"

int
main(void)
{
  LCD_init();
  KbdInit();

  printw("Press F1 to exit");
  menu_main();
  
  getch();
  LCD_end();
}
