
#define SD_ITEM_FILE "test_data/items_1.dat"

#define  __UNITS_KBD_C
#define  INCL_UNITS_KBD_NCURSES_C
#include "test_common.c"

void
test_init()
{
  eeprom_update_block("My Shop", (uint16_t *)(offsetof(struct ep_store_layout, shop_name)), sizeof("My Shop")-1);
}

int
main(void)
{
  uint8_t ui8_1;
  uint16_t ui16_1, ui16_2;
  struct item *it = (void *)bufSS;

  LCD_init();
  ep_store_init();
  test_init();
  menuInit();
  KbdInit();
  printerInit();

  menuSDLoadItem(MENU_MSUPER);
  for (ui16_1=0; ui16_1 < (ITEM_MAX_ADDR>>EEPROM_MAX_DEVICES_LOGN2);
       ui16_1+=(ITEM_SIZEOF>>EEPROM_MAX_DEVICES_LOGN2)) {
    ui16_2 = ee24xx_read_bytes(ui16_1, bufSS, ITEM_SIZEOF);
    assert(ITEM_SIZEOF == ui16_2);
    if (0 == it->id) continue;

    printf("addr:%x loop : ", ui16_1);
    for (ui16_2=0; ui16_2<ITEM_SIZEOF; ui16_2++) {
      printf("%x ", bufSS[ui16_2]);
    }
    printf("\n");
    it->name[ITEM_NAME_BYTEL-1] = 0;
    printf("item sizeof:%d -2:%d -1:%d\n", ITEM_SIZEOF, bufSS[ITEM_SIZEOF-2], bufSS[ITEM_SIZEOF-1]);
    printf("item name:%s\n", it->name);
    printf("item discount:%d\n", it->discount);
    printf("item cost:%d\n", it->cost);
    printf("item id:%d\n", it->id);
    //    printf("item prod_code:%s\n", it->prod_code);
    printf("item vat_sel:%d\n", it->vat_sel);
    printf("item has_common_discount:%d\n", it->has_common_discount);
    printf("item has_weighing_mc:%d\n", it->has_weighing_mc);
    printf("item name_in_unicode:%d\n", it->name_in_unicode);
    //    printf("item is_biller_item:%d\n", it->is_biller_item);
    printf("item has_serv_tax:%d\n", it->has_serv_tax);
    printf("item is_disabled:%d\n", it->is_disabled);
    printf("item unused_5:%d\n", it->unused_5);
    printf("item unused_4:%d\n", it->unused_4);
    printf("item unused_3:%d\n", it->unused_3);
    printf("item unused_2:%d\n", it->unused_2);
    printf("item unused_1:%d\n", it->unused_1);
  }

  arg1.valid = MENU_ITEM_NONE;
  arg1.value.str.sptr = bufSS;
  menuGetOpt(menu_str1+(MENU_STR1_IDX_NAME*MENU_PROMPT_LEN), &arg1, MENU_ITEM_STR, NULL);
  arg2.valid = MENU_ITEM_NONE;
  arg2.value.str.sptr = bufSS+LCD_MAX_COL+2;
  menuGetOpt(menu_str1+(MENU_STR1_IDX_ITEM*MENU_PROMPT_LEN), &arg2, MENU_ITEM_ID|MENU_ITEM_OPTIONAL, NULL);
  menuBilling(MENU_MSUPER);

  /* Prepare to exit */
  LCD_end();
}
