#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#include <assert.c>

#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/crc16.h>

#define TEST_KEY_ARR_SIZE 128

#include "version.h"
#include "lcd.h"
#include "kbd.h"
#include "ep_store.h"
#define UNIT_TEST_MENU_1 menu_handler
void menu_handler(uint8_t ui);
#include "billing.h"
#include "i2c.h"
#include "uart.h"
#include "a1micro2mm.h"
#include "menu.h"
#include "main.h"

#include "lcd.c"
#include "kbd_ncurses.c"
#include "ep_store.c"
#include "i2c.c"
#include "uart.c"
#include "ff.c"
#include "a1micro2mm.c"
#include "menu.c"

#include <time.h>
//******************************************************************
//Function to get RTC date & time in FAT32 format
//  Return format : Year[31:25], Month[24:21], Date[20:16]
//                  Hour[15:11], Min[10:5], Sec[4:0]
//******************************************************************
uint32_t
get_fattime (void)
{
  time_t now_t = time(NULL);
  struct tm *now = localtime(&now_t);

  return (((now->tm_year-80)&0x7F)<<25) |
    ((now->tm_mon&0xF)<<21) |
    ((now->tm_mday&0x1F)<<16) |
    ((now->tm_hour&0x1F)<<11) |
    ((now->tm_min&0x3F)<<5) |
    (now->tm_sec&0x1F);
}

void
menu_handler(uint8_t ui)
{
  move(0, 0);
  printw("menu_handler called:%d", ui);
}

void
test_init()
{
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
  menuGetYesNo(menu_str1+(MENU_STR1_IDX_S_TAX*MENU_PROMPT_LEN), MENU_PROMPT_LEN);

  /* Prepare to exit */
  LCD_end();
}
