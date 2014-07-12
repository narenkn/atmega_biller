#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#include <assert.c>

#include <avr/pgmspace.h>

#define TEST_KEY_ARR_SIZE 128

#include "lcd.h"
#include "kbd.h"
#include "ep_store.h"
#define UNIT_TEST_MENU_1 menu_handler
void menu_handler(uint8_t ui);
#include "menu.h"

#include "lcd.c"
#include "kbd_ncurses.c"
#include "ep_store.c"
#include "menu.c"

void
menu_handler(uint8_t ui)
{
  move(0, 0);
  printw("menu_handler called:%d", ui);
}

void
test_init()
{
  eeprom_update_word((uint16_t *)(offsetof(struct ep_store_layout, shop_name_len)), sizeof("My Shop")-1);
  eeprom_update_block("My Shop", (uint16_t *)(offsetof(struct ep_store_layout, shop_name)), sizeof("My Shop")-1);
}

int
main(void)
{
  LCD_init();
  KbdInit();
  ep_store_init();
  test_init();
  menuInit();

  printw("Press F2 to exit");
  menuMain();

  /* Prepare to exit */
  LCD_end();
}
