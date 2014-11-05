#include "test_common.c"

int
main(void)
{
  uint8_t ui1;

  LCD_init();

  uartInit();
  uartSelect(0);
  printerInit();

  MenuMode = MENU_MSUPER;
  menuFactorySettings(MENU_MSUPER);

  LCD_WR_LINE_N(0, 0, "Bill Sample: ", 13);
  _delay_ms(10);

  struct sale *sl = (void *) (bufSS+LCD_MAX_COL+LCD_MAX_COL+4);
  assert(sizeof(struct sale)+LCD_MAX_COL+LCD_MAX_COL+4 <= BUFSS_SIZE);
  sl->info.n_items = 1;
  sl->info.prop    = 0;
  strncpy_P(sl->info.user, PSTR("naren   "), 8);
  sl->info.date_yy = 44;
  sl->info.date_mm = 9;
  sl->info.date_dd = 4;
  sl->items[0].ep_item_ptr = 0;
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
  sl->it[0].id = 23;
  sl->it[0].cost = 5500;
  sl->it[0].discount = 1155;
  strncpy_P(sl->it[0].name, PSTR("milk n cream"), 12);
  strncpy_P(sl->it[0].prod_code, PSTR("98798sdfaiuy988a"), 16);
  sl->it[0].has_serv_tax = 1;
  sl->it[0].has_common_discount = 0;
  sl->it[0].has_weighing_mc = 1;
  sl->it[0].vat_sel = 0;
  sl->it[0].name_in_unicode = 0;
  sl->it[0].has_vat = 1;
  sl->it[0].is_disabled = 0;

  menuPrnBill(sl);

  LCD_end();

  return 0;
}
