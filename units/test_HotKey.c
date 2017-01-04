#include "common_incl.c"

/* place to store items */
struct item all_items[ITEM_MAX];

uint8_t inp5[6][TEST_KEY_ARR_SIZE];

void
make_item(struct item *ri1, uint16_t idx)
{
  uint32_t ui1, ui2, ui3, ui4;
  uint8_t save;

  RESET_TEST_KEYS;

  /* Construct a random item name */
  ui3 = (rand() % (ITEM_NAME_BYTEL>>1)) + (ITEM_NAME_BYTEL>>1);
  for (ui1=0; ui1<ui3; ui1++) {
    inp5[0][ui1] = ' ' + (rand()%('~'-' '));
    if (',' == inp5[0][ui1]) {
      inp5[0][ui1] = '_';
    }
    ri1->name[ui1] = inp5[0][ui1];
  }
  inp5[0][ui1] = 0;
  for (ui1=0;ui1<ITEM_NAME_BYTEL;ui1++) {
    ri1->name[ui1] = toupper(ri1->name[ui1]);
  }
  for (ui1=ui3; ui1<ITEM_NAME_BYTEL; ui1++) {
    ri1->name[ui1] = ' ';
  }
  INIT_TEST_KEYS(inp5[0]);

  /* provide id */
  sprintf(inp5[4], "%d", idx);
  INIT_TEST_KEYS(inp5[4]);

  /* confirm replace of item */
  ui2 = rand()&0xE;
  for (ui1=0; ui1<ui2; ui1++) {
    inp5[5][ui1] = (rand()&1) ? ASCII_RIGHT : ASCII_LEFT;
  }
  inp5[5][ui2] = 0;
  INIT_TEST_KEYS(inp5[5]);

  /* Prod code */
  ui2 = (rand() % (ITEM_PROD_CODE_BYTEL>>1)) + (ITEM_PROD_CODE_BYTEL>>1);
  for (ui1=0; ui1<ui2; ui1++) {
    inp[ui1] = '!' + (rand() % ('~'-'!'));
    if (',' == inp[ui1]) {
      inp[ui1] = '_';
    }
    ri1->prod_code[ui1] = inp[ui1];
  }
  inp[ui2] = 0;
  if ('#' == inp[0]) { /* comment line */
    inp[0] = '$';
  }
  for (ui1=0;ui1<ITEM_PROD_CODE_BYTEL;ui1++) {
    ri1->prod_code[ui1] = toupper(ri1->prod_code[ui1]);
  }
  for (ui1=ui2; ui1<ITEM_PROD_CODE_BYTEL; ui1++) {
    ri1->prod_code[ui1] = ' ';
  }
  INIT_TEST_KEYS(inp);

  /* cost, discount */
  ri1->cost = (rand() % (1<<14))+1;
  ri1->discount = rand() % ri1->cost;
  ui2 = 0;
  int2str(inp5[1], ri1->cost, &ui2);
  inp5[1][ui2] = 0;
  INIT_TEST_KEYS(inp5[1]);
  ui2 = 0;
  int2str(inp5[2], ri1->discount, &ui2);
  inp5[2][ui2] = 0;
  INIT_TEST_KEYS(inp5[2]);

  /* tax'es */
  ri1->Vat = rand() % 1000;
  sprintf(inp3, "%d.%d", ri1->Vat/100, ri1->Vat%100);
  INIT_TEST_KEYS(inp3);
  ri1->Tax1 = rand() % 10000;
  sprintf(inp4, "%d.%d", ri1->Tax1/100, ri1->Tax1%100);
  INIT_TEST_KEYS(inp4);
  ri1->Tax2 = rand() % 10000;
  sprintf(inp4+LCD_MAX_COL, "%d.%d", ri1->Tax2/100, ri1->Tax2%100);
  INIT_TEST_KEYS(inp4+LCD_MAX_COL);
  ri1->Tax3 = rand() % 10000;
  sprintf(inp4+LCD_MAX_COL+LCD_MAX_COL, "%d.%d", ri1->Tax3/100, ri1->Tax3%100);
  INIT_TEST_KEYS(inp4+LCD_MAX_COL+LCD_MAX_COL);

  /* Provide choices */
  assert(TEST_KEY_ARR_SIZE > (8*LCD_MAX_COL));
  ui2 = rand() % (LCD_MAX_COL-1);
  for (ui1=0; ui1<ui2; ui1++) {
    inp5[4][ui1+(1*LCD_MAX_COL)] = (rand()&1) ? ASCII_RIGHT : ASCII_LEFT;
  }
  inp5[4][ui2+(1*LCD_MAX_COL)] = 0;
  ri1->has_vat = (ui2 & 1) ? 0 : 1;
  INIT_TEST_KEYS(((uint8_t*)&(inp5[4][0]))+(1*LCD_MAX_COL)); /* VAT */
  ui2 = rand() % (LCD_MAX_COL-1);
  for (ui1=0; ui1<ui2; ui1++) {
    inp5[4][ui1+(2*LCD_MAX_COL)] = (rand()&1) ? ASCII_RIGHT : ASCII_LEFT;
  }
  inp5[4][ui2+(2*LCD_MAX_COL)] = 0;
  ri1->has_tax1 = (ui2 & 1) ? 0 : 1;
  INIT_TEST_KEYS(((uint8_t*)&(inp5[4][0]))+(2*LCD_MAX_COL)); /* Tax1 */
  ui2 = rand() % (LCD_MAX_COL-1);
  for (ui1=0; ui1<ui2; ui1++) {
    inp5[4][ui1+(3*LCD_MAX_COL)] = (rand()&1) ? ASCII_RIGHT : ASCII_LEFT;
  }
  inp5[4][ui2+(3*LCD_MAX_COL)] = 0;
  ri1->has_tax2 = (ui2 & 1) ? 0 : 1;
  INIT_TEST_KEYS(((uint8_t*)&(inp5[4][0]))+(3*LCD_MAX_COL)); /* Tax2 */
  ui2 = rand() % (LCD_MAX_COL-1);
  for (ui1=0; ui1<ui2; ui1++) {
    inp5[4][ui1+(4*LCD_MAX_COL)] = (rand()&1) ? ASCII_RIGHT : ASCII_LEFT;
  }
  inp5[4][ui2+(4*LCD_MAX_COL)] = 0;
  ri1->has_tax3 = (ui2 & 1) ? 0 : 1;
  INIT_TEST_KEYS(((uint8_t*)&(inp5[4][0]))+(4*LCD_MAX_COL)); /* Tax3 */
  ui2 = rand() % (LCD_MAX_COL-1);
  for (ui1=0; ui1<ui2; ui1++) {
    inp5[4][ui1+(5*LCD_MAX_COL)] = (rand()&1) ? ASCII_RIGHT : ASCII_LEFT;
  }
  inp5[4][ui2+(5*LCD_MAX_COL)] = 0;
  ri1->has_weighing_mc = (ui2 & 1) ? 0 : 1;
  INIT_TEST_KEYS(((uint8_t*)&(inp5[4][0]))+(5*LCD_MAX_COL)); /* has_weighing_mc */
  ui2 = rand() % (LCD_MAX_COL-1);
  for (ui1=0; ui1<ui2; ui1++) {
    inp5[4][ui1+(6*LCD_MAX_COL)] = (rand()&1) ? ASCII_RIGHT : ASCII_LEFT;
  }
  inp5[4][ui2+(6*LCD_MAX_COL)] = 0;
  ri1->is_reverse_tax = (ui2 & 1) ? 0 : 1;
  INIT_TEST_KEYS(((uint8_t*)&(inp5[4][0]))+(6*LCD_MAX_COL)); /* is_reverse_tax */
  ui2 = rand() % (LCD_MAX_COL-1);
  for (ui1=0; ui1<ui2; ui1++) {
    inp5[4][ui1+(7*LCD_MAX_COL)] = (rand()&1) ? ASCII_RIGHT : ASCII_LEFT;
  }
  inp5[4][ui2+(7*LCD_MAX_COL)] = 0;
  ri1->has_common_discount = (ui2 & 1) ? 0 : 1;
  INIT_TEST_KEYS(((uint8_t*)&(inp5[4][0]))+(7*LCD_MAX_COL)); /* has_common_discount */

  /* confirm save of item */
  ui2 = rand()&0xE;
  for (ui1=0; ui1<ui2; ui1++) {
    inp5[3][ui1] = (rand()&1) ? ASCII_RIGHT : ASCII_LEFT;
  }
  inp5[3][ui2] = 0;
  save = (ui2 & 1) ? 0 : 1;
  INIT_TEST_KEYS(inp5[3]);

  arg1.valid = MENU_ITEM_NONE;
  arg1.value.str.sptr = bufSS;
  arg1.value.str.len = ITEM_NAME_BYTEL;
  //  printf("test_key[0]:'%s'\n", test_key[0]);
  menuGetOpt(menu_str1+(MENU_STR1_IDX_NAME*MENU_PROMPT_LEN), &arg1, MENU_ITEM_STR, NULL);
  //  printf("test_key[0]:'%s'\n", arg1.value.str.sptr);
  arg2.valid = MENU_ITEM_NONE;
  arg2.value.str.sptr = bufSS+LCD_MAX_COL;
  arg2.value.str.len = ITEM_NAME_BYTEL;
  menuGetOpt(menu_str1+(MENU_STR1_IDX_ITEM*MENU_PROMPT_LEN), &arg2, MENU_ITEM_ID|MENU_ITEM_OPTIONAL, NULL);
  //PRINT_TEST_KEYS(ui1);
  menuAddItem(MENU_MNORMAL);
  //printf("test_key_arr_idx:%d\n", test_key_arr_idx);

  if (!save) {
    return;
  }

  /* should have saved */
  item_read_bytes(itemAddr(idx), (void *)ri1, ITEM_SIZEOF);
  assert(ri1->id == idx);
  assert((uint16_t)-1 == test_key_idx);
  assert(0 == strncmp(lcd_buf[0], "Success!        ", LCD_MAX_COL));
}

void
test_init1()
{
  uint32_t ui32_1, ui32_2;

  for (ui32_1=0; ui32_1<AVR_EEPROM_SIZE; ui32_1++)
    AVR_EEPROM_DIRECT_ASSIGN(ui32_1, rand());
  KBD_RESET_KEY;
  memset(all_items, 0xFF, ITEM_SIZEOF*ITEM_MAX);

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
		      (void *)(offsetof(struct ep_store_layout, ShopName)), SHOP_NAME_SZ_MAX);
}

int
main(int argc, char *argv[])
{
  uint32_t ui1, ui2, ui3;

  if ((argc == 1) || (0 == argv[1]))
    ui1 = time(NULL);
  else
    ui1 = atoi(argv[1]);
  printf("seed : %d\n", ui1);
  srand(ui1);

  /* */
  common_init();
  assert_init();
  ep_store_init();
  KbdInit();
  test_init1();
  menuInit();
  test_init2();
  nvfInit();

  /* Create random items */
  for (ui1=0, ui2=0; ui2<(ITEM_MAX>>1); ui1 += (rand()&1)+1, ui2++) {
    make_item(all_items+ui1, ui1+1);
  }

  /* */
  for (uint32_t loop=0; loop<1000; loop++) {
    RESET_TEST_KEYS;

    /* select valid item */
    uint8_t *ptr = inp;
    do {
      ui2 = rand() % ITEM_MAX;

      /* */
      sprintf(ptr, "%d", ui2+1);
      INIT_TEST_KEYS(ptr);

      /* found valid item */
      if (0xFF == (all_items[ui2].unused_crc ^ all_items[ui2].unused_crc_invert))
	break;

      /* incr */
      ptr += LCD_MAX_COL;
    } while (1);

    /* select key combination */
    ptr += LCD_MAX_COL;
    ptr[0] = rand() % 16;
    ui1 = rand() & 3;
    ptr[1] = (0 == ui1) ? 0 : (1 == ui1) ? ASCII_ALT :
      (2 == ui1) ? ASCII_LGUI : ASCII_LGUI | ASCII_ALT;
    ptr[1] |= kbdHit;
    ptr[2] = 0;
    INIT_TEST_KEYS(ptr);

    menuSetHotKey(0);

    /* verify that the shortcut was created */
    ui3 = ((uint16_t)(ptr[1] & (kbdWinHit|kbdAltHit))) * 16 * sizeof(uint16_t);
    ui3 += ((uint16_t)ptr[0]) * sizeof(uint16_t);
    uint16_t idx = offsetof(struct ep_store_layout, unused_HotKey) + ui3;
    ui1 = eeprom_read_word((uint16_t *)idx);
    assert(ui1 == (ui2+1));
  }

  return MENU_RET_NOERROR;
}
