#include <stdint.h>
#include <time.h>

#define SD_ITEM_FILE "test_data/items_1.dat"

#define __UNITS_KBD_C
#define INCL_UNITS_KBD_NCURSES_C
#include "test_common.c"

void
test_init1()
{
  uint16_t ui16_1, ui16_2;

  eeprom_update_byte((uint8_t *)0, 0xFA);
  eeprom_update_byte((uint8_t *)1, 0xC7);
  eeprom_update_byte((uint8_t *)2, 0x05);
  eeprom_update_byte((uint8_t *)3, 0x1A);
  
  for (ui16_1=0, ui16_2=0; ui16_2<13; ui16_2++) {
    ui16_1 = _crc16_update(ui16_1, 'a'+ui16_2);
    eeprom_update_byte((uint8_t *)ui16_2+4, 'a'+ui16_2);
  }
  ui16_1 = _crc16_update(ui16_1, '1');
  eeprom_update_byte((uint8_t *)13+4, '1');
  eeprom_update_byte((uint8_t *)14+4, (ui16_1>>8)&0xFF);
  eeprom_update_byte((uint8_t *)15+4, (ui16_1>>0)&0xFF);
}

void
test_init2()
{
  uint8_t ui8_1;
  uint16_t ui16_1;

  eeprom_update_block((const void *)"Sri Ganapathy Stores",
		      (void *)(offsetof(struct ep_store_layout, shop_name)) , SHOP_NAME_SZ_MAX);
  for (ui8_1=0, ui16_1=700; ui8_1<EPS_MAX_VAT_CHOICE; ui8_1++, ui16_1 += 110) {
    eeprom_update_word((uint16_t *)(offsetof(struct ep_store_layout, Vat) + (sizeof(uint16_t)*ui8_1)), ui16_1);
  }
}

void
test_init3()
{
  /* login */
  MenuMode = MENU_MSUPER;
  devStatus ^= DS_DEV_INVALID;

  /* time */
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  timerDateSet(tm.tm_year-80, tm.tm_mon+1, tm.tm_mday);
  timerTimeSet(tm.tm_hour, tm.tm_min);
}

int
main(void)
{
  LCD_init();
  ep_store_init();
  KbdInit();
  test_init1();
  menuInit();
  test_init2();
  printerInit();

  move(0, 0);
  printw("Press F2 to exit");
  //  menuSDLoadItem(MENU_MSUPER);
  test_init3();
  menuMain();
  getch();

  /* Prepare to exit */
  LCD_end();
}
