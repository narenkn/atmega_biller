#define TEST_KEY_ARR_SIZE    1024

#include "test_common.c"

/* test setting set routines */

/* place to store items */
struct item all_items[ITEM_MAX];

int
main()
{
  uint32_t errors;
  uint8_t  ui8_1, ui8_2, ui8_3, ui8_4, ui8_5;
  uint32_t loop, ui32_1, ui32_2;
  uint16_t ui16_1, ui16_2, ui16_3;
  uint8_t quest[LCD_MAX_COL];

  assert_init();
  KbdInit();


  return errors;
}
