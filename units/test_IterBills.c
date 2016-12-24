#include "common_incl.c"

/* place to store items */
struct item all_items[ITEM_MAX];
struct sale all_sales[NVF_SALE_MAX_BILLS];

uint8_t inp5[6][TEST_KEY_ARR_SIZE];

void
make_item(struct item *ri1, uint8_t rand_save)
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
  arg2.value.str.sptr = bufSS+LCD_MAX_COL;
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

  /* both can be invalid items */

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
make_bill(struct sale *sl, uint8_t rand_save)
{
  uint16_t ui16_1, ui16_2, ui16_3;

  /* check if bill addr is free */
  ui16_2 = eeprom_read_word((uint16_t *)(offsetof(struct ep_store_layout, unused_nextBillAddr)));
  bill_read_bytes(ui16_2, (void *)sl, offsetof(struct sale, info));
  if (0xFFFF == (sl->crc ^ sl->crc_invert)) {
    assert(0 == __LINE__);
    return;
  }

  /* randomize everything */
  sl->info.n_items = (rand() % MAX_ITEMS_IN_BILL) + 1;
  strcpy(sl->info.user, "naren");

  /* */
  sl->info.id = eeprom_read_word((uint16_t *)(offsetof(struct ep_store_layout, unused_LastBillId))) + 1;
  eeprom_update_word((uint16_t *)(offsetof(struct ep_store_layout, unused_LastBillId)), sl->info.id);
  sl->info.is_void = (0 == (rand() % 5));
  date_t date; s_time_t time;
  timerDateGet(date);
  timerTimeGet(time);
  sl->info.date_yy = date.year; sl->info.date_mm = date.month; sl->info.date_dd = date.day;
  sl->info.time_hh = time.hour; sl->info.time_mm = time.min; sl->info.time_ss = time.sec;
  sl->tableNo = rand();
  for (uint16_t ui16_1 = sl->info.n_items; ui16_1; ui16_1--) {
    uint16_t itNo = (rand() % ITEM_MAX)+1;
    sl->items[ui16_1].ep_item_ptr = itemAddr(itNo);
    sl->items[ui16_1].quantity = (rand() % 100000)+1;
    sl->items[ui16_1].cost = (rand() % 9999) + 1;
    sl->items[ui16_1].discount = (rand() % sl->items[ui16_1].cost) + 1;
    sl->items[ui16_1].has_common_discount = rand() & 1;
    sl->items[ui16_1].has_vat = rand() & 1;
    sl->items[ui16_1].has_tax1 = rand() & 1;
    sl->items[ui16_1].has_tax2 = rand() & 1;
    sl->items[ui16_1].has_tax3 = rand() & 1;
    sl->items[ui16_1].is_reverse_tax = rand() & 1;
  }

  /* Calculate bill and tax */
  uint32_t ui32_1=0;
  sl->t_tax1 = sl->t_tax2 = sl->t_tax3 = sl->t_vat = 0;
  sl->t_discount = 0, sl->total = 0;
  uint32_t ui32_2 = 0;
  for (uint8_t ui8_3 = sl->info.n_items; ui8_3; ui8_3--) {
    // Start original source
    if ( (0 != sl->items[ui8_3].discount) && (sl->items[ui8_3].cost >= sl->items[ui8_3].discount) ) {
      ui32_1 = (sl->items[ui8_3].cost - sl->items[ui8_3].discount);
      sl->t_discount += sl->items[ui8_3].discount * sl->items[ui8_3].quantity / 10;
    } else if (sl->items[ui8_3].has_common_discount) {
      ui32_1 = eeprom_read_word((uint16_t *)offsetof(struct ep_store_layout, CommonDisc));
      if (ui32_1 <= 10000) {
	ui32_1 = (10000 - ui32_1) * sl->items[ui8_3].cost;
	ui32_1 /= 100;
	sl->t_discount += (sl->items[ui8_3].cost - ui32_1) * sl->items[ui8_3].quantity / 10;
      } else {
	ui32_1 = sl->items[ui8_3].cost;
	LCD_ALERT(PSTR("Err: ComnDis > 100%"));
      }
    } else
      ui32_1 = sl->items[ui8_3].cost;

    if (sl->items[ui8_3].has_vat) {
      ui32_2 = sl->it[0].Vat;
      ui32_2 *= ui32_1;
      ui32_2 /= 100;
      sl->t_vat += ui32_2;
      if (!(sl->items[ui8_3].is_reverse_tax))
	sl->total += ui32_2;
    }
    if (sl->items[ui8_3].has_tax1) {
      ui32_2 = sl->it[0].Tax1;
      ui32_2 *= ui32_1;
      ui32_2 /= 100;
      sl->t_tax1 += ui32_2;
      if (!(sl->items[ui8_3].is_reverse_tax))
	sl->total += ui32_2;
    }
    if (sl->items[ui8_3].has_tax2) {
      ui32_2 = sl->it[0].Tax2;
      ui32_2 *= ui32_1;
      ui32_2 /= 100;
      sl->t_tax2 += ui32_2;
      if (!(sl->items[ui8_3].is_reverse_tax))
	sl->total += ui32_2;
    }
    if (sl->items[ui8_3].has_tax3) {
      ui32_2 = sl->it[0].Tax3;
      ui32_2 *= ui32_1;
      ui32_2 /= 100;
      sl->t_tax3 += ui32_2;
      if (!(sl->items[ui8_3].is_reverse_tax))
	sl->total += ui32_2;
    }

    ui32_1 *= sl->items[ui8_3].quantity / 10;
    sl->total += ui32_1;
    // End original source
  }

  /* now save the data */
  for (ui16_1=offsetof(struct sale, info), ui16_3=0; ui16_1<SIZEOF_SALE_EXCEP_ITEMS; ui16_1++)
    ui16_3 = _crc16_update(ui16_3, ((uint8_t *)sl)[ui16_1]);
  sl->crc = ui16_3;
  sl->crc_invert = ~ui16_3;
  //printf("sl->crc:%x sl->crc_invert:%x\n", sl->crc, sl->crc_invert);
  ui16_2 = eeprom_read_word((uint16_t *)(offsetof(struct ep_store_layout, unused_nextBillAddr)));
  bill_write_bytes(ui16_2, (uint8_t *)sl, SIZEOF_SALE_EXCEP_ITEMS);
  ui16_2 = NVF_NEXT_SALE_RECORD(ui16_2);
  eeprom_update_word((uint16_t *)(offsetof(struct ep_store_layout, unused_nextBillAddr)), ui16_2);
}

void
compare_bills(struct sale *osl, struct sale *sl)
{
  if ((osl->crc != sl->crc) || (osl->crc_invert != sl->crc_invert)) {
    //printf("compare_bills:crc mismatch\n");
    assert(0);
  }
  if ((osl->info.n_items != sl->info.n_items) ||
      (0 != strncmp(osl->info.user, sl->info.user, EPS_MAX_UNAME)) ||
      (osl->info.id != sl->info.id) ||
      (osl->info.is_deleted != sl->info.is_deleted) ||
      (osl->info.dup_bill_issued != sl->info.dup_bill_issued) ||
      (osl->info.is_void != sl->info.is_void) ||
      (osl->info.date_yy != sl->info.date_yy) ||
      (osl->info.date_mm != sl->info.date_mm) ||
      (osl->info.date_dd != sl->info.date_dd) ||
      (osl->info.time_hh != sl->info.time_hh) ||
      (osl->info.time_mm != sl->info.time_mm) ||
      (osl->info.time_ss != sl->info.time_ss)) {
    //printf("compare_bills:sale_info mismatch\n");
    assert(0);
  }
  if ((osl->tableNo != sl->tableNo) ||
      (osl->t_tax1 != sl->t_tax1) ||
      (osl->t_tax2 != sl->t_tax2) ||
      (osl->t_tax3 != sl->t_tax3) ||
      (osl->t_vat != sl->t_vat) ||
      (osl->t_discount != sl->t_discount) ||
      (osl->total != sl->total) ||
      (osl->t_cash_pay != sl->t_cash_pay) ||
      (osl->t_other_pay != sl->t_other_pay)) {
    //printf("compare_bills:total mismatch\n");
    assert(0);
  }
  for (uint16_t ui1=osl->info.n_items; ui1; ) {
    ui1--;
    if ((osl->items[ui1].quantity != sl->items[ui1].quantity) ||
	(osl->items[ui1].cost != sl->items[ui1].cost) ||
	(osl->items[ui1].discount != sl->items[ui1].discount) ||
	(osl->items[ui1].ep_item_ptr != sl->items[ui1].ep_item_ptr) ||
	(osl->items[ui1].has_common_discount != sl->items[ui1].has_common_discount) ||
	(osl->items[ui1].has_vat != sl->items[ui1].has_vat) ||
	(osl->items[ui1].has_tax1 != sl->items[ui1].has_tax1) ||
	(osl->items[ui1].has_tax2 != sl->items[ui1].has_tax2) ||
	(osl->items[ui1].has_tax3 != sl->items[ui1].has_tax3) ||
	(osl->items[ui1].is_reverse_tax != sl->items[ui1].is_reverse_tax) ||
	(osl->items[ui1].has_weighing_mc != sl->items[ui1].has_weighing_mc)) {
      //printf("compare_bills:%d: items mismatch\n", ui1);
      assert(0);
    }
  }
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

int
main(int argc, char *argv[])
{
  struct item ri, ri1;
  uint32_t ui1, ui2;
  uint16_t ui16_1, ui16_2, ui16_3;

  if ((argc == 1) || (0 == argv[1]))
    ui1 = time(NULL);
  else
    ui1 = atoi(argv[1]);
  printf("seed : %d\n", ui1);
  srand(ui1);

  /* */
  common_init();
  assert_init();
  i2c_init();
  ep_store_init();
  KbdInit();
  test_init1();
  menuInit();
  test_init2();
  nvfInit();

  /* test init */
#define NUM_ITEMS2TEST  ITEM_MAX
  /* Test to check the number of items that could be stored */
  for (ui1=0; ui1<NUM_ITEMS2TEST; ui1++) {
    make_item(all_items+ui1, 0);
  }

  /* Check all items as per expectations */
  for (ui1=0; ui1<NUM_ITEMS2TEST; ui1++) {
    compare_item(all_items+ui1, itemAddr(ui1+1));
  }

  #define TEST_LOOP 10
  /* Test for all bills saved & retrieved */
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  date_t date;
  date.day = tm.tm_mday, date.month=tm.tm_mon, date.year = 1900+tm.tm_year;
  for (uint32_t loop=0; loop<TEST_LOOP; loop++) {
    timerDateSet(date);
    for (ui1=0; ui1<NVF_SALE_MAX_BILLS; ui1++) {
      make_bill(all_sales+ui1, 0);
    }

    /* */
    struct sale sl;
    add_expect_assert("0 == __LINE__" ", " __FILE__);
    make_bill(&sl, 0);
    add_expect_assert("0 == __LINE__" ", " __FILE__);
    make_bill(&sl, 0);
    /* Save to file */
    menuSdSaveBillDat(0);
    /* test all bills are deleted */
    for (ui1=0, ui16_2=0; ui1<NVF_SALE_MAX_BILLS;
	 ui1++, ui16_2 = NVF_NEXT_SALE_RECORD(ui16_2)) {
      bill_read_bytes(ui16_2, (void *)&sl, offsetof(struct sale, info));
      assert (0xFFFF != (sl.crc ^ sl.crc_invert));
      //printf("ui1:%d crc:%x crc_invert:%x\n", ui1, sl.crc, sl.crc_invert);
    }

    /* check data in file */
    sprintf(inp, "billdat/%02d-%02d-%04d.dat", date.day, date.month, date.year);
    FILE *inf = fopen(inp, "r");
    //printf("opening file '%s' : %d inf:%p\n", inp, errno, inf);
    ssize_t ret;
    for (ui1=0; ui1<NVF_SALE_MAX_BILLS; ui1++) {
      ret = fread ((void *)&sl, 1, SIZEOF_SALE_EXCEP_ITEMS, inf);
      if (0 == ret) break;
      assert(SIZEOF_SALE_EXCEP_ITEMS == ret);
      assert(0xFFFF == (sl.crc ^ sl.crc_invert));
      compare_bills(all_sales+ui1, &sl);
      /* load bills */
      ui16_2 = eeprom_read_word((uint16_t *)(offsetof(struct ep_store_layout, unused_nextBillAddr)));
      bill_write_bytes(ui16_2, (uint8_t *)&sl, SIZEOF_SALE_EXCEP_ITEMS);
      ui16_2 = NVF_NEXT_SALE_RECORD(ui16_2);
      eeprom_update_word((uint16_t *)(offsetof(struct ep_store_layout, unused_nextBillAddr)), ui16_2);
      /* */
      ret = fread ((void *)&sl, 1, MAX_SIZEOF_1BILL-SIZEOF_SALE_EXCEP_ITEMS, inf);
      if (0 == ret) break;
      assert((MAX_SIZEOF_1BILL-SIZEOF_SALE_EXCEP_ITEMS) == ret);
    }
    ret = fread ((void *)&sl, MAX_SIZEOF_1BILL, 1, inf);
    assert(0 == ret);
    fclose(inf);

    /* delete bills */
    for (ui1=0, ui16_2=0; ui1<NVF_SALE_MAX_BILLS;
	 ui1++, ui16_2 = NVF_NEXT_SALE_RECORD(ui16_2)) {
      bill_read_bytes(ui16_2, (void *)&sl, offsetof(struct sale, info));
      assert (0xFFFF == (sl.crc_invert ^ sl.crc));
      sl.crc = (0 != sl.crc_invert) ? 0xFFFF : 0x0;
      bill_write_bytes(ui16_2, (void *)&sl, offsetof(struct sale, info));
      all_sales[ui1].crc = all_sales[ui1].crc_invert = 0;
      //printf("ui1:%d crc:%x crc_invert:%x\n", ui1, sl.crc, sl.crc_invert);
    }

    /* goto next date */
    nextDate(&date);
  }

  /* */
  char fn[32];
  FILE *inf;

  if (TEST_LOOP < 10) return MENU_RET_NOERROR;
  date.day = tm.tm_mday, date.month=tm.tm_mon, date.year = 1900+tm.tm_year;
  /* choose a random range to delete */
  uint8_t ui8_1 = rand() & 0x3;
  uint8_t ui8_2 = ui8_1 + (rand() & 0x3);

  RESET_TEST_KEYS;
  for (uint32_t loop=0; loop<TEST_LOOP; loop++) {
    if (loop == ui8_1) {
      sprintf(inp, "%02d%02d%04d", date.day, date.month, date.year);
      INIT_TEST_KEYS(inp);
    }
    if (loop == ui8_2) {
      sprintf(inp+LCD_MAX_COL, "%02d%02d%04d", date.day, date.month, date.year);
      INIT_TEST_KEYS(inp+LCD_MAX_COL);
    }
    nextDate(&date);
  }
  MENU_GET_OPT(menu_str1+(MENU_STR1_IDX_DAY*MENU_PROMPT_LEN), &arg1, MENU_ITEM_DATE, NULL);
  MENU_GET_OPT(menu_str1+(MENU_STR1_IDX_DAY*MENU_PROMPT_LEN), &arg2, MENU_ITEM_DATE, NULL);
  menuDelAllBill(MENU_NOCONFIRM);
  /* now check if the files are deleted */
  date.day = tm.tm_mday, date.month=tm.tm_mon, date.year = 1900+tm.tm_year;
  for (uint32_t loop=0; loop<TEST_LOOP; loop++) {
    sprintf(fn, "billdat/%02d-%02d-%04d.dat", date.day, date.month, date.year);
    //printf("File %s : ", fn);
    inf = fopen(fn, "r");
    if ((ui8_1 <= loop) && (loop <= ui8_2)) {
      //printf("Deleted : %p\n", inf);
      assert(NULL == inf);
    } else {
      //printf("exists : %p\n", inf);
      assert(NULL != inf);
    }
    fclose(inf);
    nextDate(&date);
  }
  
  return MENU_RET_NOERROR;
}
