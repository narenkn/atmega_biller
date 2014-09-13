#include <stdint.h>

#define UNIT_TEST_MENU_1 menu_handler
void menu_handler(uint8_t ui);

#include "test_common.c"

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
