
#define SD_ITEM_FILE "test_data/items_1.dat"

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

  eeprom_update_block((const void *)"Sri Ganapathy Stores",
		      (void *)(offsetof(struct ep_store_layout, shop_name)) , SHOP_NAME_SZ_MAX);
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
  uint8_t ui8_1;
  uint16_t ui16_1, ui16_2;
  struct item *it = (void *)bufSS;

  LCD_init();
  ep_store_init();
  KbdInit();
  test_init1();
  menuInit();
  test_init2();
  printerInit();

  move(0, 0);
  printw("Press F2 to exit");
  menuSDLoadItem(MENU_MSUPER);
  test_init3();

//  for (ui16_1=0; ui16_1 < (ITEM_MAX_ADDR>>EEPROM_MAX_DEVICES_LOGN2);
//       ui16_1+=(ITEM_SIZEOF>>EEPROM_MAX_DEVICES_LOGN2)) {
//    ui16_2 = ee24xx_read_bytes(ui16_1, bufSS, ITEM_SIZEOF);
//    assert(ITEM_SIZEOF == ui16_2);
//    if (0 == it->id) continue;
//
//    printf("addr:%x loop : ", ui16_1);
//    for (ui16_2=0; ui16_2<ITEM_SIZEOF; ui16_2++) {
//      printf("%x ", bufSS[ui16_2]);
//    }
//    printf("\n");
//    it->name[ITEM_NAME_BYTEL-1] = 0;
//    printf("item sizeof:%d -2:%d -1:%d\n", ITEM_SIZEOF, bufSS[ITEM_SIZEOF-2], bufSS[ITEM_SIZEOF-1]);
//    printf("item name:%s\n", it->name);
//    printf("item discount:%d\n", it->discount);
//    printf("item cost:%d\n", it->cost);
//    printf("item id:%d\n", it->id);
//    //    printf("item prod_code:%s\n", it->prod_code);
//    printf("item vat_sel:%d\n", it->vat_sel);
//    printf("item has_common_discount:%d\n", it->has_common_discount);
//    printf("item has_weighing_mc:%d\n", it->has_weighing_mc);
//    printf("item name_in_unicode:%d\n", it->name_in_unicode);
//    //    printf("item is_biller_item:%d\n", it->is_biller_item);
//    printf("item has_serv_tax:%d\n", it->has_serv_tax);
//    printf("item is_disabled:%d\n", it->is_disabled);
//    printf("item unused_5:%d\n", it->unused_5);
//    printf("item unused_4:%d\n", it->unused_4);
//    printf("item unused_3:%d\n", it->unused_3);
//    printf("item unused_2:%d\n", it->unused_2);
//    printf("item unused_1:%d\n", it->unused_1);
//  }

  menuMain();

  /* Prepare to exit */
  getch();
  LCD_end();
}
