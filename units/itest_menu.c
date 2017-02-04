#include <stdint.h>
#include <time.h>

#define __UNITS_KBD_C
#define INCL_UNITS_KBD_NCURSES_C
#include "common_incl.c"

void
test_init1()
{
  uint16_t ui32_1, ui32_2;

  eeprom_update_byte((uint8_t *)0, 0xFA);
  eeprom_update_byte((uint8_t *)1, 0xC7);
  eeprom_update_byte((uint8_t *)2, 0x05);
  eeprom_update_byte((uint8_t *)3, 0x1A);
  
  for (ui32_1=0, ui32_2=0; ui32_2<(SERIAL_NO_MAX-3); ui32_2++) {
    ui32_1 = _crc16_update(ui32_1, 'a'+ui32_2);
    eeprom_update_byte((uint8_t *)ui32_2+4, 'a'+ui32_2);
  }
  ui32_1 = _crc16_update(ui32_1, '1');
  eeprom_update_byte((uint8_t *)(SERIAL_NO_MAX-3)+4, '1');
  eeprom_update_byte((uint8_t *)(SERIAL_NO_MAX-2)+4, (ui32_1>>8)&0xFF);
  eeprom_update_byte((uint8_t *)(SERIAL_NO_MAX-1)+4, (ui32_1>>0)&0xFF);
}

void
test_init2()
{
  uint8_t ui8_1;
  uint16_t ui16_1;

  eeprom_update_block((const void *)"Sri Ganapathy Stores",
		      (void *)(offsetof(struct ep_store_layout, ShopName)) , SHOP_NAME_SZ_MAX);
}

void
test_init3()
{
  /* login */
  MenuMode = MENU_MSUPER;

  /* time */
  time_t t = time(NULL);
  struct tm tm = *localtime(&t); /* returns YYYY-1900 */
  date_t d = {tm.tm_mday, (tm.tm_mon+1), (tm.tm_year+1900)};
  timerDateSet(d);
  s_time_t st = {tm.tm_hour, tm.tm_min, 0};
  timerTimeSet(st);
}

volatile uint8_t keypadMultiKeyModeOff;

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
