#include "common_incl.c"

/* test menuBilling */

typedef struct {
  uint32_t                  discount;
  uint32_t                      cost;
  uint8_t                 prod_code[ITEM_PROD_CODE_BYTEL];
  uint8_t                      name[ITEM_NAME_BYTEL];
} shopItems_t;

shopItems_t shopItems[] = {
  {0, 15.5, "", "milk"},
  {0, 25.1, "", "soap"},
  {0, 75.0, "", "colgate tooth"},
  {0, 40.0, "", "Powder"},
};
#define TEST_NUM_ITEMS (sizeof(shopItems)/sizeof(shopItems_t))

/* place to store items */
struct item all_items[ITEM_MAX];

uint8_t inp5[6][TEST_KEY_ARR_SIZE];

void
make_item(struct item *ri1, uint8_t rand_save)
{
  uint32_t ui1, ui2, ui3, ui4;
  uint8_t save;

  RESET_TEST_KEYS;

  static uint8_t itemIdx = 0;

  /* Construct a random item name */
  for (ui1=0; ui1<ITEM_NAME_BYTEL; ui1++) {
    ri1->name[ui1] = shopItems[itemIdx].name[ui1];
    if (0 == shopItems[itemIdx].name[ui1])
      break;
  }
  inp5[0][ui1] = 0;
  for (ui1=0;ui1<ITEM_NAME_BYTEL;ui1++) {
    ri1->name[ui1] = toupper(ri1->name[ui1]);
  }
  for (ui1=ui3; ui1<ITEM_NAME_BYTEL; ui1++) {
    ri1->name[ui1] = ' ';
  }
  INIT_TEST_KEYS(inp5[0]);

  /* don't provide id */
  inp5[4][0] = 0;
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
    ri1->prod_code[ui1] = inp[ui1] = '!' + (rand() % ('~'-'!'));
  }
  inp[ui2] = 0;
  for (ui1=0;ui1<ITEM_PROD_CODE_BYTEL;ui1++) {
    ri1->prod_code[ui1] = toupper(ri1->prod_code[ui1]);
  }
  for (ui1=ui2; ui1<ITEM_PROD_CODE_BYTEL; ui1++) {
    ri1->prod_code[ui1] = ' ';
  }
  INIT_TEST_KEYS(inp);

  /* cost, discount */
  ri1->cost = shopItems[itemIdx].cost;
  ri1->discount = shopItems[itemIdx].discount;
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
  ui2 &= 0xFE; /* doesn't have weiging mc */
  for (ui1=0; ui1<ui2; ui1++) {
    inp5[4][ui1+(5*LCD_MAX_COL)] = (rand()&1) ? ASCII_RIGHT : ASCII_LEFT;
  }
  inp5[4][ui2+(5*LCD_MAX_COL)] = 0;
  ri1->has_weighing_mc = (ui2 & 1);
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
  ui2 = rand_save ? rand()&0xF : rand()&0xE;
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
  arg2.value.str.sptr = bufSS+LCD_MAX_COL+2;
  arg2.value.str.len = ITEM_NAME_BYTEL;
  menuGetOpt(menu_str1+(MENU_STR1_IDX_ITEM*MENU_PROMPT_LEN), &arg2, MENU_ITEM_ID|MENU_ITEM_OPTIONAL, NULL);
  //PRINT_TEST_KEYS(ui1);
  menuAddItem(MENU_MNORMAL);
  //printf("test_key_arr_idx:%d\n", test_key_arr_idx);

  /* find index that it would use to store */
  for (ui1=0; ui1<ITEM_MAX; ui1++) {
    if (0xFF !=
	(all_items[ui1].unused_crc ^ all_items[ui1].unused_crc_invert))
      break;
  }

  /* no space left */
  if (ui1 >= ITEM_MAX) {
    assert(0 == strncmp(lcd_buf[0], "Items Mem Full..", LCD_MAX_COL));
    return;
  }
  itemIdx++;
  if (!save) {
    return;
  }

  /* should have saved */
  item_read_bytes(itemAddr(ui1+1), (void *)ri1, ITEM_SIZEOF);
  assert(ri1->id == (ui1+1));
  assert((uint16_t)-1 == test_key_idx);
  assert(0 == strncmp(lcd_buf[0], "Success!        ", LCD_MAX_COL));
}

uint16_t
uint8ncmp(uint8_t *s1, uint8_t *s2, uint16_t size)
{
  while (size) {
    if (s1[size] != s1[size])
      return size;
    size--;
  }
  return 0;
}

void
compare_item(struct item *ri, uint16_t ee24x_addr)
{
  uint16_t ui1;
  struct item rif;
  char *bufTT = (void *) &(rif);

  /* load item */
  assert(ITEM_SIZEOF == item_read_bytes(ee24x_addr, bufTT, ITEM_SIZEOF));

  //printf("rif.name:'%s'\n", rif.name);
  if (ri->id != rif.id) {
    printf("ERROR: id mismatch Exp:'%d' Obt:'%d'\n", ri->id, rif.id);
  }
  if (0 != strncmp(&(ri->name[0]), &(rif.name[0]), ITEM_NAME_BYTEL)) {
    printf("ERROR: Name mismatch Exp:'%s' Obt:'%s'\n", ri->name, rif.name);
  }
  if (ri->cost != rif.cost) {
    printf("ERROR: cost mismatch Exp:'%d' Obt:'%d'\n", ri->cost, rif.cost);
  }
  if (ri->discount != rif.discount) {
    printf("ERROR: discount mismatch Exp:'%d' Obt:'%d'\n", ri->discount, rif.discount);
  }
  ui1 = uint8ncmp(ri->prod_code, rif.prod_code, ITEM_PROD_CODE_BYTEL);
  if (0 != ui1) {
    printf("ERROR: prod_code mismatch byte:%d\n", ui1);
  }
  if (ri->Vat != rif.Vat) {
    printf("ERROR: Vat mismatch Exp:'%d' Obt:'%d'\n", ri->Vat, rif.Vat);
  }
  if (ri->Tax1 != rif.Tax1) {
    printf("ERROR: Tax1 mismatch Exp:'%d' Obt:'%d'\n", ri->Tax1, rif.Tax1);
  }
  if (ri->Tax2 != rif.Tax2) {
    printf("ERROR: Tax2 mismatch Exp:'%d' Obt:'%d'\n", ri->Tax2, rif.Tax2);
  }
  if (ri->Tax3 != rif.Tax3) {
    printf("ERROR: Tax3 mismatch Exp:'%d' Obt:'%d'\n", ri->Tax3, rif.Tax3);
  }
  if (ri->has_vat != rif.has_vat) {
    printf("ERROR: has_vat mismatch Exp:'%d' Obt:'%d'\n", ri->has_vat, rif.has_vat);
  }
  if (ri->has_tax1 != rif.has_tax1) {
    printf("ERROR: has_tax1 mismatch Exp:'%d' Obt:'%d'\n", ri->has_tax1, rif.has_tax1);
  }
  if (ri->has_tax2 != rif.has_tax2) {
    printf("ERROR: has_tax2 mismatch Exp:'%d' Obt:'%d'\n", ri->has_tax2, rif.has_tax2);
  }
  if (ri->has_tax3 != rif.has_tax3) {
    printf("ERROR: has_tax3 mismatch Exp:'%d' Obt:'%d'\n", ri->has_tax3, rif.has_tax3);
  }
  if (ri->is_reverse_tax != rif.is_reverse_tax) {
    printf("ERROR: is_reverse_tax mismatch Exp:'%d' Obt:'%d'\n", ri->is_reverse_tax, rif.is_reverse_tax);
  }
  if (ri->has_common_discount != rif.has_common_discount) {
    printf("ERROR: has_common_discount mismatch Exp:'%d' Obt:'%d'\n", ri->has_common_discount, rif.has_common_discount);
  }

  /* */
  uint8_t cn, cpc, cn3;
  for (ui1=0, cn=0; ui1<ITEM_NAME_BYTEL; ui1++) {
    assert(ri->name[ui1] == rif.name[ui1]);
    if ((ri->name[ui1] < ' '))
    assert((ri->name[ui1] >= ' '));
    assert(ri->name[ui1] < '~');
    //printf("%d:%d\n", ui1, ri->name[ui1]);
    cn = _crc_ibutton_update(cn, ri->name[ui1]);
    if (2 == ui1)
      cn3 = cn;
  }
  for (ui1=0, cpc=0; ui1<ITEM_PROD_CODE_BYTEL; ui1++) {
    cpc = _crc_ibutton_update(cpc, ri->prod_code[ui1]);
  }
  assert(0 != ri->id);
  assert(ri->id <= ITEM_MAX);
  assert(itIdxs[ri->id-1].crc_name3 == cn3);
  assert(cn == eeprom_read_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_itIdxName))+(ri->id)-1) );
  assert(cpc == eeprom_read_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_crc_prod_code))+(ri->id)-1) );
  //  printf("ee24xaddr:%x cpc:%x cn:%x\n", ee24x_addr, cpc, cn);
}

void
delete_item(uint16_t id)
{
  all_items[id-1].unused_crc = all_items[id-1].unused_crc_invert = 0xFF;
}

void
test_init1()
{
  uint32_t ui32_1, ui32_2;

  for (ui32_1=0; ui32_1<EEPROM_SIZE; ui32_1++)
    I2C_EEPROM_DIRECT_ASSIGN(ui32_1, 0xFF);
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

/* shared variables between routines */
uint8_t randNumItems=0;
uint32_t ui1, ui2, ui3, ui4, ui5, ui6;
uint16_t ui16_1, ui16_2, ui16_3;

void
make_valid_bill(bool canBeKot)
{
    /* */
    RESET_TEST_KEYS;
    arg1.valid = MENU_ITEM_NONE;
    arg2.valid = MENU_ITEM_NONE;

    /* case for just enter */
    inp[LCD_MAX_COL] = 0;

    uint8_t randItem = (rand()%TEST_NUM_ITEMS)+1;
    ui1 = 0;
    int2str(inp, randItem, &ui1);
    INIT_TEST_KEYS(inp); /* item id */

    randNumItems = rand()%50;
    inp[(LCD_MAX_COL*2)] = inp[(LCD_MAX_COL*2)+1] =
      inp[(LCD_MAX_COL*2)+2] = inp[(LCD_MAX_COL*2)+3] =
      inp[(LCD_MAX_COL*2)+4] = ASCII_DEL;
    ui1 = 0;
    int2str(inp+(LCD_MAX_COL*2)+5, randNumItems, &ui1);
    INIT_TEST_KEYS(inp+(LCD_MAX_COL*2)); /* num items */

    INIT_TEST_KEYS(inp+LCD_MAX_COL); /* confirm item */

    INIT_TEST_KEYS(inp+LCD_MAX_COL); /* goto finalize */

    bool makeKotBill = canBeKot && (rand()&1) && (randNumItems>0);
    if (makeKotBill) {
      inp[LCD_MAX_COL*3] = ASCII_RIGHT;
      inp[(LCD_MAX_COL*3)+1] = 0;
      INIT_TEST_KEYS(inp+(LCD_MAX_COL*3)); /* finalize */
    } else {
      INIT_TEST_KEYS(inp+LCD_MAX_COL); /* finalize */

      INIT_TEST_KEYS(inp+LCD_MAX_COL); /* cash pay */

      INIT_TEST_KEYS(inp+LCD_MAX_COL); /* cash pay */
    }

    INIT_TEST_KEYS(inp+LCD_MAX_COL); /* confirm bill */

    menuBilling(0);

    struct sale *sl = (void *)(bufSS+LCD_MAX_COL+LCD_MAX_COL);
    if (makeKotBill) {
      assert(1 == sl->info.is_void);
    } else {
      assert(0 == sl->info.is_void);
    }
}

void
addto_void_bill(struct sale *sl)
{
    /* */
    RESET_TEST_KEYS;
    arg1.valid = MENU_ITEM_NONE;
    arg2.valid = MENU_ITEM_NONE;

    /* case for just enter */
    inp[LCD_MAX_COL] = 0;

    for (ui1=0; ui1<sl->info.n_items; ui1++)
      INIT_TEST_KEYS(inp+LCD_MAX_COL); /* confirm old items */

    /* */
    uint8_t randItem = (rand()%TEST_NUM_ITEMS)+1;
    ui1 = 0;
    int2str(inp, randItem, &ui1);
    INIT_TEST_KEYS(inp); /* item id */

    randNumItems = rand()%50;
    inp[(LCD_MAX_COL*2)] = inp[(LCD_MAX_COL*2)+1] =
      inp[(LCD_MAX_COL*2)+2] = inp[(LCD_MAX_COL*2)+3] =
      inp[(LCD_MAX_COL*2)+4] = ASCII_DEL;
    ui1 = 0;
    int2str(inp+(LCD_MAX_COL*2)+5, randNumItems, &ui1);
    INIT_TEST_KEYS(inp+(LCD_MAX_COL*2)); /* num items */

    INIT_TEST_KEYS(inp+LCD_MAX_COL); /* confirm item */

    INIT_TEST_KEYS(inp+LCD_MAX_COL); /* goto finalize */

    INIT_TEST_KEYS(inp+LCD_MAX_COL); /* finalize */

    INIT_TEST_KEYS(inp+LCD_MAX_COL); /* cash pay */

    INIT_TEST_KEYS(inp+LCD_MAX_COL); /* cash pay */

    INIT_TEST_KEYS(inp+LCD_MAX_COL); /* confirm bill */

    arg2.valid = MENU_ITEM_ID;
    arg2.value.integer.i16 = sl->info.id;
    menuBilling(MENU_VOIDBILL);
}

int
main(int argc, char *argv[])
{
  struct item ri, ri1;

  if ((argc == 1) || (0 == argv[1]))
    ui1 = time(NULL);
  else
    ui1 = atoi(argv[1]);
  //ui1 = 1486157840;
  printf("seed : %d\n", ui1);
  srand(ui1);

  /* test init */
  for (ui1=0; ui1<EEPROM_SIZE; ui1++)
    I2C_EEPROM_DIRECT_ASSIGN(ui1, 0xFF);
  for (ui1=0; ui1<AVR_EEPROM_SIZE; ui1++)
    AVR_EEPROM_DIRECT_ASSIGN(ui1, rand());
  KBD_RESET_KEY;
  memset(all_items, 0xFF, ITEM_SIZEOF*ITEM_MAX);

  /* */
  common_init();
  assert_init();
  i2c_init();
  ep_store_init();
  KbdInit();
  test_init1();
  test_init2();
  nvfInit();
  printerInit();
  menuInit();

  /* */
  struct sale *sl = (void *)inp4;
  uint32_t loop;

  /* Add 1 item, bill only that */
  for (ui1=0; ui1<TEST_NUM_ITEMS; ui1++) {
    make_item(all_items+ui1, 0);
  }

  /* Check all items as per expectations */
  for (ui1=0; ui1<TEST_NUM_ITEMS; ui1++) {
    compare_item(all_items+ui1, itemAddr(ui1+1));
  }

  /* check after power-on-reset */
  ui16_3 = ui16_1 = NVF_SALE_START_ADDR;

  /* Create Mixed of void bill & valid bills */
  for (loop=0; loop<NVF_SALE_MAX_BILLS; loop=randNumItems?loop+1:loop, ui16_3=randNumItems?NVF_NEXT_SALE_RECORD(ui16_3):ui16_3) {
    /* */
    ui16_2 = eeprom_read_word((uint16_t *)(offsetof(struct ep_store_layout, unused_todayStartAddr)));
    assert(ui16_1 == ui16_2);
    if (ui16_1 != ui16_2) {
      printf("lcd_buf:%s ui16_1:%x ui16_2:%x\n", lcd_buf[0], ui16_1, ui16_2);
    }
    ui16_2 = eeprom_read_word((uint16_t *)(offsetof(struct ep_store_layout, unused_nextBillAddr)));
    assert(ui16_3 == ui16_2);
    if (ui16_3 != ui16_2) {
      printf("lcd_buf:%s ui16_3:%x ui16_2:%x\n", lcd_buf[0], ui16_3, ui16_2);
    }

    make_valid_bill(true);

    if (loop<NVF_SALE_MAX_BILLS) {
      assert(0 != strncmp("Bill Memory Full", lcd_buf[0], 16));
    } else {
      assert(0 == strncmp("Bill Memory Full", lcd_buf[0], 16));
    }
  }

  /* Take few void bills & make them valid bills */
  loop = rand() % (NVF_SALE_MAX_BILLS/2);
  ui16_2 = eeprom_read_word((uint16_t *)(offsetof(struct ep_store_layout, unused_todayStartAddr)));
  for (; loop; loop--) {
    for (ui3=0; ui3<NVF_SALE_MAX_BILLS; ui3++) { /* find bill to modify */
      ui2 = rand() % 5;
      ui16_2 = NVF_NEXT_SALE_RECORD(ui16_2);
      bill_read_bytes(ui16_2, (void *)sl, offsetof(struct sale, items));
      if ((0xFFFF == (sl->crc ^ sl->crc_invert)) && (sl->info.is_void))
	break;
    }
    if (ui3 >= NVF_SALE_MAX_BILLS) {
      //printf("No more void bills !!\n");
      break; /* No more void bills */
    }

    /* */
    //printf("Modify @%d\n", sl->info.id);
    assert(1 == sl->info.is_void);
    assert(0xFFFF == (sl->crc ^ sl->crc_invert));
    uint16_t prev_n_items = sl->info.n_items;
    addto_void_bill(sl);

    /* */
    bill_read_bytes(ui16_2, (void *)sl, offsetof(struct sale, items));
    assert(0 == sl->info.is_void);
    if (randNumItems) {
      assert((prev_n_items+1) == sl->info.n_items);
      //printf("lcd_buf:%s\n", lcd_buf[0]);
      //printf("prev_n_items:%d, sl->info.n_items:%d\n", prev_n_items, sl->info.n_items);
    } else {
      assert(prev_n_items == sl->info.n_items);
    }
  }

  return 0;
}
