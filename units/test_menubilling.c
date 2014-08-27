#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <assert.c>

#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/crc16.h>

#define TEST_KEY_ARR_SIZE 128
#define NO_MAIN

#include "version.h"
#include "lcd.h"
#include "kbd.h"
#include "ep_store.h"
#include "billing.h"
#include "i2c.h"
#include "uart.h"
#include "a1micro2mm.h"
#include "menu.h"
#include "main.h"

/* All Header overrides */
#undef  SD_ITEM_FILE
#define SD_ITEM_FILE "test_data/items_1.dat"
#define LCD_ALERT(str)				\
  LCD_WR_LINE(0, 0, str);			\
  KBD_RESET_KEY; KBD_GETCH

#define LCD_ALERT_16N(str, n)			\
  LCD_WR_LINE(0, 0, str);			\
  LCD_PUT_UINT16X(n);				\
  KBD_RESET_KEY; KBD_GETCH

#include "lcd.c"
#include "kbd_ncurses.c"
#include "ep_store.c"
#include "i2c.c"
#include "uart.c"
#include "ff.c"
#include "a1micro2mm.c"
#include "menu.c"
#include "main.c"

void
test_init()
{
  eeprom_update_block("My Shop", (uint16_t *)(offsetof(struct ep_store_layout, shop_name)), sizeof("My Shop")-1);
}

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

int
main(void)
{
  uint8_t ui8_1;
  uint16_t ui16_1, ui16_2;
  struct item *it = (void *)bufSS;

  printerInit();

  menuSDLoadItem(MENU_MSUPER);
//  printf("lcd_buf:%s\n", lcd_buf[0]);
//  for (ui16_1=0; (EEPROM_MAX_ADDRESS-ui16_1+1)>=(ITEM_SIZEOF>>EEPROM_MAX_DEVICES_LOGN2);
//       ui16_1+=(ITEM_SIZEOF>>EEPROM_MAX_DEVICES_LOGN2)) {
//    ui16_2 = ee24xx_read_bytes(ui16_1, bufSS, ITEM_SIZEOF);
//    assert(ITEM_SIZEOF == ui16_2);
//    if (0 == it->id) continue;
//
//    printf("addr:%x loop : ", ui16_1);
//    for (ui16_2=0; ui16_2<ITEM_SIZEOF; ui16_2++) {
//      printf("%x ", bufSS[ui16_2]);
//    }
//    printf("\n");
//    it->name[ITEM_NAME_BYTEL-1] = 0;
//    printf("item sizeof:%d -2:%d -1:%d\n", ITEM_SIZEOF, bufSS[ITEM_SIZEOF-2], bufSS[ITEM_SIZEOF-1]);
//    printf("item name:%s\n", it->name);
//    printf("item discount:%d\n", it->discount);
//    printf("item cost:%d\n", it->cost);
//    printf("item id:%d\n", it->id);
//    //    printf("item prod_code:%s\n", it->prod_code);
//    printf("item vat_sel:%d\n", it->vat_sel);
//    printf("item has_common_discount:%d\n", it->has_common_discount);
//    printf("item has_weighing_mc:%d\n", it->has_weighing_mc);
//    printf("item name_in_unicode:%d\n", it->name_in_unicode);
//    //    printf("item is_biller_item:%d\n", it->is_biller_item);
//    printf("item has_serv_tax:%d\n", it->has_serv_tax);
//    printf("item is_disabled:%d\n", it->is_disabled);
//    printf("item unused_5:%d\n", it->unused_5);
//    printf("item unused_4:%d\n", it->unused_4);
//    printf("item unused_3:%d\n", it->unused_3);
//    printf("item unused_2:%d\n", it->unused_2);
//    printf("item unused_1:%d\n", it->unused_1);
//  }

  LCD_init();
  KbdInit();
  ep_store_init();
  test_init();
  menuInit();

  arg1.valid = MENU_ITEM_NONE;
  arg1.value.sptr = bufSS;
  menuGetOpt(menu_str1+(MENU_STR1_IDX_NAME*MENU_PROMPT_LEN), &arg1, MENU_ITEM_STR);
  arg2.valid = MENU_ITEM_NONE;
  arg1.value.sptr = bufSS+LCD_MAX_COL+2;
//  menuGetOpt(menu_str1+(MENU_STR1_IDX_ITEM*MENU_PROMPT_LEN), &arg2, MENU_ITEM_STR);
  menuBilling(MENU_MSUPER);

  /* Prepare to exit */
  LCD_end();
}
