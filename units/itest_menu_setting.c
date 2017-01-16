#include <stdint.h>

#define  __UNITS_KBD_C
#define  INCL_UNITS_KBD_NCURSES_C
#include "common_incl.c"

volatile uint8_t keypadMultiKeyModeOff;

void
test_init()
{
  eeprom_update_block("My Shop", (uint16_t *)(offsetof(struct ep_store_layout, ShopName)), sizeof("My Shop")-1);
}

int
main(void)
{
  LCD_init();
  ep_store_init();
  test_init();
  menuInit();
  KbdInit();

  menuSettingSet(MENU_MSUPER);

  /* Prepare to exit */
  KBD_GETCH;
  LCD_end();
}
