
/* All Header overrides */
#define SD_SETTINGS_FILE "test_data/settings_1.dat"

#include "test_common.c"

int
main(void)
{
  uint16_t ui16_1, ui16_2;
  struct item *it = (void *)bufSS;

  /* FIXME: corrupt a byte and find crc fails */

  /* Load the items, FIXME: make it selfcheck */
  menuSDLoadSettings(MENU_MSUPER);

  /* save item */
  //  menuSDSaveSettings(MENU_MSUPER);

  return 0;
}
