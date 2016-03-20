
#include "test_common.c"

int
main()
{
  uint16_t ui16_1, ui16_2, ui16_3, ui16_4;
  uint16_t arr[2];

  srand(time(NULL));
  KbdInit();

  for (ui16_4=0; ui16_4<1000; ui16_4++) {
    /* */
    ui16_1 = rand() % EEPROM_SALE_MAX_BILLS;
    ui16_3 = rand() % EEPROM_SALE_MAX_BILLS;
    ui16_3 = EEPROM_BILL_ADDR(ui16_3);

    /* create valid # bills */
    for (ui16_2=0; ui16_2<ui16_1; ui16_3=EEPROM_NEXT_SALE_RECORD(ui16_3)) {
      ee24xx_read_bytes(ui16_3, (uint8_t *)arr, 4);
      if ( (rand() & 0x1) && (0xFFFF != (arr[0] ^ arr[1])) ) {
	arr[0] = rand();
	arr[1] = ~arr[0];
	ee24xx_write_bytes(ui16_3, (uint8_t *)arr, 4);
	eeprom_update_word((uint16_t *)(offsetof(struct ep_store_layout, unused_next_billaddr)), ui16_3);
	ui16_2++;
      }
    }

    /* */
    arg2.valid = arg1.valid = MENU_ITEM_NONE;

    uint16_t ui16_5 = 0, ui16_6;
    inp[1] = inp[0] = 0;
    INIT_TEST_KEYS(inp);
    ui16_6 = rand() & 1;
    if (0 == ui16_6)
      inp[0] = ASCII_RIGHT;
    menuDelAllBill(0);
    sprintf(inp2, "# Bills : %d          ", ui16_2);
    assert (0 == strncmp(lcd_buf[0], inp2, LCD_MAX_COL));

    /* count # valid bills */
    ui16_3 = rand() % EEPROM_SALE_MAX_BILLS;
    ui16_3 = EEPROM_BILL_ADDR(ui16_3);
    for (ui16_2=0; ui16_2<EEPROM_SALE_MAX_BILLS; ui16_3=EEPROM_NEXT_SALE_RECORD(ui16_3), ui16_2++) {
      ee24xx_read_bytes(ui16_3, (uint8_t *)arr, 4);
      if ( (0xFFFF == (arr[0] ^ arr[1])) )
	ui16_5++;
    }
    assert (ui16_5 == ((ui16_6) ? 0 : ui16_1));

    /* Delete all bills anyway */
    inp[1] = inp[0] = 0;
    INIT_TEST_KEYS(inp);
    menuDelAllBill(0);
  }

  return 0;
}
