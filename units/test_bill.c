
#define  __UNITS_KBD_C
#define  INCL_UNITS_KBD_NCURSES_C
#include "test_common.c"

void
test_init1()
{
  uint16_t ui16_1, ui16_2;

  for (ui16_1=0, ui16_2=0; ui16_2<13; ui16_2++) {
    ui16_1 = _crc16_update(ui16_1, 'a'+ui16_2);
    eeprom_update_byte((uint8_t *)ui16_2, 'a'+ui16_2);
  }
  ui16_1 = _crc16_update(ui16_1, '1');
  eeprom_update_byte((uint8_t *)13, '1');
  eeprom_update_byte((uint8_t *)14, (ui16_1>>8)&0xFF);
  eeprom_update_byte((uint8_t *)15, (ui16_1>>0)&0xFF);
}

void
test_init2()
{
  uint8_t ui8_1;
  uint16_t ui16_1;

#if 0
  eeprom_update_block((const void *)"Sri Ganapathy Stores            ",
		      (void *)(offsetof(struct ep_store_layout, shop_name)) , SHOP_NAME_SZ_MAX);
#endif
  for (ui8_1=0, ui16_1=700; ui8_1<EPS_MAX_VAT_CHOICE; ui8_1++, ui16_1 += 110) {
    eeprom_update_word((uint16_t *)(offsetof(struct ep_store_layout, Vat) + (sizeof(uint16_t)*ui8_1)), ui16_1);
  }
}

void
test_init3()
{
  MenuMode = MENU_MSUPER;
  LoginUserId = 1;
}

int
main(void)
{
  uint8_t ui1;

  LCD_init();

  ep_store_init();
  KbdInit();
  test_init1();

  menuInit();
  //  test_init2();

  uartInit();
  uartSelect(0);
  printerInit();

  move(0, 0);
  printw("Press F2 to exit");
  menuSDLoadItem(MENU_MSUPER);
  test_init3();

  LCD_CLRLINE(0);
  LCD_WR_N("Bill Sample: ", 13);
  _delay_ms(10);

  struct sale *sl = (void *) (bufSS+LCD_MAX_COL+LCD_MAX_COL+4);
  assert(sizeof(struct sale)+LCD_MAX_COL+LCD_MAX_COL+4 <= BUFSS_SIZE);
  sl->info.n_items = 1;
  sl->info.property = 0;
  strncpy_P(sl->info.user, PSTR("naren   "), 8);
  sl->info.date_yy = 44;
  sl->info.date_mm = 9;
  sl->info.date_dd = 4;
  sl->items[0].ep_item_ptr = menuItemAddr(22-1);
  sl->items[0].quantity = 5;
  sl->items[0].cost = 55;
  sl->items[0].discount = 13;
  sl->items[0].has_common_discount = 0;
  sl->items[0].has_vat = 0;
  sl->items[0].has_serv_tax = 0;
  sl->items[0].vat_sel = 0;
  sl->t_stax = 233;
  sl->t_discount = 144;
  sl->t_vat = 33;
  sl->total = 16734;

  menuPrnBill(sl, menuPrnBillEE24xxHelper);

  getch();
  LCD_end();

  return 0;
}
