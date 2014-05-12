#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>

#define __code
#define __idata
#define __pdata
#define __sbit  uint8_t

#define ERROR(msg) fprintf(stderr, msg)

#include "assert.h"
#include "billing.h"
#include "crc.h"
#include "lcd.h"
#include "kbd.h"
#include "i2c.h"
#include "uart.h"
#include "flash.h"
#include "ep_store.h"
#include "printer.h"
#include "menu.h"

#include "assert.c"
#include "crc.c"
#include "lcd.c"
#include "kbd.c"
#include "i2c.c"
#include "uart.c"
#include "flash.c"
#include "ep_store.c"
#include "printer.c"
#include "menu.c"

uint8_t inp[FLASH_SECTOR_SIZE], inp2[FLASH_SECTOR_SIZE];

int
main(void)
{
  uint32_t loop;
  uint8_t ui1, ui2, ui3, ui4;

  srand(time(NULL));

  /* */
  assert_init();
  menu_Init();
  i2cInit();
  ep_store_init();

  /* Passwd */
  for (loop=0; loop<1000; loop++) {
    uint16_t passwd_size = ( rand() % (LCD_MAX_COL-1) ) + 1;
    for (ui1=0; ui1<passwd_size; ui1++) {
      if (0 == (rand() % 3))
	inp[ui1] = 'A' + (rand()%26);
      else
	inp[ui1] = 'a' + (rand()%26);
    }
    INIT_TEST_KEYS(inp);
    KBD_RESET_KEY;
    menu_getopt("Prompt 1", &arg1, MENU_ITEM_STR);
 
    MenuMode = MENU_MNORMAL;
    menu_SetPasswd(MENU_MRESET);
    assert(MENU_MNORMAL == MenuMode);

    /* sometimes corrupt the password */
    uint8_t corrupted = rand() % 2;
    //    printf("Before corruption inp:'%s'\n", inp);
    if (corrupted) {
      inp[rand()%passwd_size]++;
    }
    //    printf("After corruption inp:'%s'\n", inp);

    INIT_TEST_KEYS(inp);
    KBD_RESET_KEY;
    menu_getopt("Prompt 1", &arg1, MENU_ITEM_STR);
    menu_SetPasswd(MENU_MNORMAL|MENU_MVALIDATE);

    //    printf("Corrupted:%d\n", (uint32_t) corrupted);
    if (corrupted) {
      //      printf("MenuMode:0x%x\n", (uint32_t)MenuMode);
      assert(MENU_MNORMAL == MenuMode);
    } else {
      assert(MENU_MSUPER == MenuMode);
    }
  }

  /* modvat */
  for (loop=0; loop<1000; loop++) {
    uint32_t vat = (rand()%100)*100 + (rand()%100);
    uint32_t vat1 = vat;
    LCD_CLRSCR;
    for (ui1=0; ui1<LCD_MAX_COL; ui1++)
      inp[ui1] = ' ';
    for (ui1=0; ui1<4; ui1++) {
      uint16_t ui2 = vat % 10;
      vat /= 10;
      inp[3-ui1] = '0' + ui2;
    }
    inp[4] = 0;
    KBD_RESET_KEY;
    INIT_TEST_KEYS(inp);
    //printf("inp is %s\n", inp);

    /* */
    arg1.valid = MENU_ITEM_NONE;
    LCD_CLRSCR;
    menu_getopt("sdlkfjlaksfklas", &arg1, MENU_ITEM_FLOAT);

    /* select input */
    uint16_t sel = rand() % 4;
    uint16_t ui2 = rand()%2;
    if (0 != ui2) {
      ui2 = sel + 4*(rand()%8);
      for (ui1=0; ui1<ui2; ui1++)
	inp[ui1] = ASCII_RIGHT;
      inp[ui1] = 0;
    } else {
      ui2 = (4-sel) + 4*(rand()%8);
      for (ui1=0; ui1<ui2; ui1++)
	inp[ui1] = ASCII_LEFT;
      inp[ui1] = 0;
    }

    KBD_RESET_KEY;
    INIT_TEST_KEYS(inp);
    menu_ModVat(MENU_MNORMAL);
    EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.vat[sel]), (uint8_t *)&ui2, sizeof(uint16_t));
    //printf("sel:%0d ui2:%0d vat1:%0d\n", (uint32_t) sel, (uint32_t) ui2, (uint32_t) vat1);
    assert(ui2 == vat1);
  }

  /* menu_Header */
  for (loop=0; loop<1; loop++) {
    for (ui1=0; ui1<LCD_MAX_COL/2; ui1++) {
      if (0 == (rand() % 3))
	inp[ui1] = 'A' + (rand()%26);
      else
	inp[ui1] = 'a' + (rand()%26);
    }
    INIT_TEST_KEYS(inp);
    KBD_RESET_KEY;
    menu_getopt("Prompt 1", &arg1, MENU_ITEM_STR);
    assert(0 == strncmp("Promp ?         ", lcd_buf, LCD_MAX_COL));

    /* Generate random string for Header */
    ui2 = rand() % HEADER_MAX_SZ;
    for (ui1=0; ui1<ui2; ui1++) {
      if (0 == (rand() % 3))
	inp2[ui1] = 'A' + (rand()%26);
      else
	inp2[ui1] = 'a' + (rand()%26);
    }
    INIT_TEST_KEYS(inp2);
    KBD_RESET_KEY;
    menu_Header(MENU_MSUPER);

    /* Validate */
    for (ui1=0; ui1<LCD_MAX_COL/2; ui1++) {
      if (0 == inp[ui1]) break;
      EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.shop_name[ui1]), (uint8_t *)&ui3, sizeof(uint8_t));
      assert(ui3 == inp[ui1]);
      if (ui3 != inp[ui1]) {
	printf("Header char mismatch ui3:%0d inp[ui1]:%0d\n", ui3, inp[ui1]);
      }
    }
    for (ui1=0; ui1<ui2; ui1++) {
      if (0 == inp[ui1]) break;
      EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.prn_header[ui1]), (uint8_t *)&ui3, sizeof(uint8_t));
      assert(ui3 == inp[ui1]);
      if (ui3 != inp[ui1]) {
	printf("Header char mismatch ui3:%0d inp[ui1]:%0d\n", ui3, inp[ui1]);
      }
    }

  }


  return 0;
}
