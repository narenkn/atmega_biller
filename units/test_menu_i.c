#include <stdint.h>

#define SD_ITEM_FILE "test_data/items_1.dat"

#define __UNITS_KBD_C
#define INCL_UNITS_KBD_NCURSES_C
#include "test_common.c"

void
test_init()
{
  uint16_t ui16_1, ui16_2;
  eeprom_update_block("abcdefghijklmn", 0, 14);
  ui16_1 = 0;
  for (ui16_2=0; ui16_2<14; ui16_2++)
    ui16_1 = _crc16_update(ui16_1, 'a'+ui16_2);
  eeprom_update_byte(14, (ui16_1>>8)&0xFF);
  eeprom_update_byte(15, (ui16_1>>0)&0xFF);
}

int
main(void)
{
  LCD_init();
  ep_store_init();
  KbdInit();
  test_init();
  menuInit();
  printerInit();

  printw("Press F2 to exit");
  menuSDLoadItem(0);
  menuMain();
  getch();

  /* Prepare to exit */
  LCD_end();
}
