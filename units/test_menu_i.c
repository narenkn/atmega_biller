#include <stdint.h>

#define SD_ITEM_FILE "test_data/items_1.dat"

#define __UNITS_KBD_C
#define INCL_UNITS_KBD_NCURSES_C
#include "test_common.c"

void
test_init()
{
  eeprom_update_block("My Shop", (uint16_t *)(offsetof(struct ep_store_layout, shop_name)), sizeof("My Shop")-1);
}

int
main(void)
{
  LCD_init();
  ep_store_init();
  test_init();
  menuInit();
  KbdInit();

  printw("Press F2 to exit");
  menuSDLoadItem(0);
  menuMain();

  /* Prepare to exit */
  LCD_end();
}
