
/* All Header overrides */
#define SD_SETTINGS_FILE "test_data/settings_2.dat"

#include "test_common.c"

int
main(void)
{
  uint16_t ui16_1, ui16_2;
  struct item *it = (void *)bufSS;

  /* FIXME: corrupt a byte and find crc fails */

  /* Load the items, FIXME: make it selfcheck */
  menuInit();
  menuSDLoadSettings(MENU_MSUPER);
  printf("lcd_buf:%s\n", lcd_buf[0]);

  /* save item */
  menuSDSaveSettings(MENU_MSUPER);
  printf("lcd_buf:%s\n", lcd_buf[0]);

  /* print settings */
  eeprom_read_block((void *)bufSS, (const void *)0, BUFSS_SIZE);
  struct ep_store_layout *eps = (void *) bufSS;
  printf("Vat0:%d Vat1:%d Vat2:%d Vat3:%d\n", eps->Vat[0], eps->Vat[1], eps->Vat[2], eps->Vat[3]);

  return 0;
}
