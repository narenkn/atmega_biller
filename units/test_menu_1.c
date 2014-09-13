#include <stdint.h>

#define UNIT_TEST_MENU_1 menu_handler
void menu_handler(uint8_t ui);

#include "test_common.c"

void
menu_handler(uint8_t ui)
{
  LCD_WR_LINE_NP(1, 0, "menu_handler:", 13);
  LCD_PUT_UINT8X(ui);
  LCD_refresh();
}

uint8_t inp[TEST_KEY_ARR_SIZE];

/* Test Item related routines */

int
main(void)
{
  uint32_t loop;
  uint8_t ui1;

  srand(time(NULL));

  menuAddItem(0);

}
