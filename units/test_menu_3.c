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

#include "lcd.h"
#include "kbd.h"
#include "i2c.h"
#include "ep_store.h"
#include "menu.h"

#include "lcd.c"
#include "kbd.c"
#include "i2c.c"
#include "ep_store.c"
#include "menu.c"

uint8_t inp[TEST_KEY_ARR_SIZE], inp2[TEST_KEY_ARR_SIZE];
uint8_t inp3[TEST_KEY_ARR_SIZE];

int
main(void)
{
  uint32_t loop;
  uint8_t ui1, ui2, ui3, ui4;

  srand(time(NULL));

  /* */
  assert_init();
  menuInit();
  i2c_init();
  ep_store_init();

  /* menuSetPasswd */
  for (loop=0; loop<0; loop++) {
    uint16_t passwd_size = ( rand() % (LCD_MAX_COL-1) ) + 1;
    for (ui1=0; ui1<passwd_size; ui1++) {
      if (0 == (rand() % 3))
	inp[ui1] = 'A' + (rand()%26);
      else
	inp[ui1] = 'a' + (rand()%26);
    }
    INIT_TEST_KEYS(inp);
    KBD_RESET_KEY;
    arg2.value.sptr = bufSS+LCD_MAX_COL;
    menuGetOpt("Prompt 1", &arg2, MENU_ITEM_STR);

    LCD_CLRSCR;
    ui2 = LoginUserId = (rand() % EPS_MAX_USERS)+1;
    ui3 = MenuMode = (rand() & 1) ? MENU_MSUPER : MENU_MNORMAL;
    menuSetPasswd(MENU_MRESET);
    assert(ui3 == MenuMode);
    assert(0 == strncmp("Passwd Updated  ", lcd_buf[0], LCD_MAX_COL));

    /* sometimes corrupt the password */
    uint8_t corrupted = rand() % 2;
    if (corrupted) {
      ui4 = rand()%passwd_size;
      inp[ui4]++;
      if (inp[ui4] > '~') inp[ui4] -= 2;
    }

    INIT_TEST_KEYS(inp);
    KBD_RESET_KEY;
    arg1.value.sptr = bufSS;
    menuGetOpt("Prompt 1", &arg1, MENU_ITEM_STR);
    menuSetPasswd(ui3|MENU_MVALIDATE);

    if (corrupted) {
      assert(0 == strncmp("Passwd Wrong!!  ", lcd_buf[0], LCD_MAX_COL));
    } else {
      assert(0 == strncmp("Passwd Updated  ", lcd_buf[0], LCD_MAX_COL));
    }
    assert(MenuMode == ui3);
    assert(LoginUserId == ui2);
  }

  /* menuSetUserPasswd */
  for (loop=0; loop<10; loop++) {
    uint16_t passwd_size = ( rand() % (LCD_MAX_COL-1) ) + 1;
    for (ui1=0; ui1<passwd_size; ui1++) {
      if (0 == (rand() % 3))
	inp[ui1] = 'A' + (rand()%26);
      else
	inp[ui1] = 'a' + (rand()%26);
    }
    INIT_TEST_KEYS(inp);
    KBD_RESET_KEY;
    arg2.value.sptr = bufSS+LCD_MAX_COL;
    menuGetOpt("Prompt 1", &arg2, MENU_ITEM_STR);

    for (ui1=0; ui1<EPS_MAX_UNAME; ui1++) {
      if (0 == (rand() % 3))
	inp2[ui1] = 'A' + (rand()%26);
      else
	inp2[ui1] = 'a' + (rand()%26);
    }
    if (0 == (rand()%5))
      inp2[0] = ' ';
    INIT_TEST_KEYS(inp2);
    KBD_RESET_KEY;
    arg1.value.sptr = bufSS;
    menuGetOpt("Prompt 2", &arg1, MENU_ITEM_STR);

    LCD_CLRSCR;
    MenuMode = MENU_MSUPER;
    inp[0] = ASCII_ENTER;
    INIT_TEST_KEYS(inp);
    menuSetUserPasswd(MENU_MSUPER);
    if (' ' == inp2[0]) {
      assert(0 == strncmp("Invalid User    ", lcd_buf[0], LCD_MAX_COL));
    } else {
      assert(MENU_MSUPER == MenuMode);
      assert(0 == strncmp("Passwd Updated  ", lcd_buf[0], LCD_MAX_COL));
      printf("lcd_buf:%s\n", lcd_buf[0]);
      assert(MenuMode == MENU_MSUPER);
    }
  }

#if 0
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
    menuGetOpt("sdlkfjlaksfklas", &arg1, MENU_ITEM_FLOAT);

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
    menuGetOpt("Prompt 1", &arg1, MENU_ITEM_STR);
    assert(0 == strncmp("Promp ?         ", lcd_buf[0], LCD_MAX_COL));

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
#endif

  return 0;
}
