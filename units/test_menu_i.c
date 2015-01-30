#include <stdint.h>

#define SD_ITEM_FILE "test_data/items_1.dat"

#define __UNITS_KBD_C
#define INCL_UNITS_KBD_NCURSES_C
#include "test_common.c"

void
test_init1()
{
  uint16_t ui16_1, ui16_2;

  for (ui16_1=0, ui16_2=0; ui16_2<13; ui16_2++) {
    ui16_1 = _crc16_update(ui16_1, 'a'+ui16_2);
    eeprom_update_byte((uint8_t *)ui16_2, 'a'+ui16_2);
  }
  ui16_1 = _crc16_update(ui16_1, '1');
  eeprom_update_byte((uint8_t *)13, '1');
  eeprom_update_byte((uint8_t *)14, (ui16_1>>8)&0xFF);
  eeprom_update_byte((uint8_t *)15, (ui16_1>>0)&0xFF);
}

void
test_init2()
{
  eeprom_update_block((const void *)"Sri Ganapathy Stores",
		      (void *)(offsetof(struct ep_store_layout, shop_name)) , SHOP_NAME_SZ_MAX);
}

int
main(void)
{
  LCD_init();
  ep_store_init();
  KbdInit();
  test_init1();
  menuInit();
  //  test_init2();
  printerInit();

  move(0, 0);
  printw("Press F2 to exit");
  menuSDLoadItem(0);
  menuMain();
  getch();

  /* Prepare to exit */
  LCD_end();
}
