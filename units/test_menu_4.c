#include "test_common.c"

/* test item add/ modify/ save */

/* place to store items */
struct item all_items[ITEM_MAX];

uint8_t inp5[6][TEST_KEY_ARR_SIZE];

void
make_item(struct item *ri1, uint8_t rand_save)
{
  uint32_t ui1, ui2, ui3, ui4;
  uint8_t save;

  RESET_TEST_KEYS;

  /* Construct a random item name */
  for (ui1=0; ui1<ITEM_NAME_BYTEL; ui1++) {
    ri1->name[ui1] = ' ';
  }
  ui3 = (rand() % (ITEM_NAME_BYTEL-1)) + 1;
  for (ui1=0; ui1<ui3; ui1++) {
    inp5[0][ui1] = ' ' + (rand()%('~'-' '));
    ri1->name[ui1] = inp5[0][ui1];
  }
  inp5[0][ui1] = 0;
  for (ui2=0;ui2<ITEM_NAME_BYTEL;ui2++) {
    ri1->name[ui2] = toupper(ri1->name[ui2]);
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
  for (ui1=0; ui1<ITEM_PROD_CODE_BYTEL; ui1++) {
    ri1->prod_code[ui1] = ' ';
  }
  ui2 = (rand() % (ITEM_PROD_CODE_BYTEL-1)) + 1;
  for (ui1=0; ui1<ui2; ui1++) {
    ri1->prod_code[ui1] = inp[ui1] = '!' + (rand() % ('~'-'!'));
  }
  inp[ui2] = 0;
  for (ui2=0;ui2<ITEM_PROD_CODE_BYTEL;ui2++) {
    ri1->prod_code[ui2] = toupper(ri1->prod_code[ui2]);
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

  /* vat */
  ri1->vat_sel = rand() % EPS_MAX_VAT_CHOICE;
  for (ui2=0; ui2<ri1->vat_sel; ui2++) {
    inp3[ui2] = ASCII_RIGHT;
  }
  inp3[ui2] = 0;
  INIT_TEST_KEYS(inp3);

  /* serv tax */
  ri1->has_serv_tax = rand();
  ui2 = 0;
  for (ui2=0; ui2<ri1->has_serv_tax; ui2++) {
    inp4[ui2] = ASCII_RIGHT;
  }
  inp4[ui2] = 0;
  INIT_TEST_KEYS(inp4);

  /* confirm save of item */
  ui2 = rand_save ? rand()&16 : rand()&0xE;
  for (ui1=0; ui1<ui2; ui1++) {
    inp5[3][ui1] = (rand()&1) ? ASCII_RIGHT : ASCII_LEFT;
  }
  inp5[3][ui2] = 0;
  save = (ui2 & 1) ? 0 : 1;
  INIT_TEST_KEYS(inp5[3]);

  arg1.valid = MENU_ITEM_NONE;
  arg1.value.str.sptr = bufSS;
  arg1.value.str.len = ITEM_NAME_BYTEL;
  //printf("test_key[0]:'%s'\n", test_key[0]);
  menuGetOpt(menu_str1+(MENU_STR1_IDX_NAME*MENU_PROMPT_LEN), &arg1, MENU_ITEM_STR, NULL);
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

  if (!save) {
    return;
  }

  /* should have saved */
  ee24xx_read_bytes(itemAddr(ui1+1), (void *)ri1, ITEM_SIZEOF);
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
  assert(ITEM_SIZEOF == ee24xx_read_bytes(ee24x_addr, bufTT, ITEM_SIZEOF));

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
  if (ri->vat_sel != rif.vat_sel) {
    printf("ERROR: vat_sel mismatch Exp:'%d' Obt:'%d'\n", ri->vat_sel, rif.vat_sel);
  }
  if (ri->has_serv_tax != rif.has_serv_tax) {
    printf("ERROR: serv_tax mismatch Exp:'%d' Obt:'%d'\n", ri->has_serv_tax, rif.has_serv_tax);
  }

  /* */
  uint8_t cn, cpc, cn3;
  for (ui1=0, cn=0; ui1<ITEM_NAME_BYTEL; ui1++) {
    assert(ri->name[ui1] == rif.name[ui1]);
    //    printf("ri->c:%c rif.c:%c\n", ri->name[ui1], rif.name[ui1]);
    assert(ri->name[ui1] >= ' ');
    assert(ri->name[ui1] < '~');
    cn = _crc_ibutton_update(cn, ri->name[ui1]);
    if (2 == ui1)
      cn3 = cn;
  }
  for (ui1=0, cpc=0; ui1<ITEM_PROD_CODE_BYTEL; ui1++) {
    cpc = _crc_ibutton_update(cpc, ri->prod_code[ui1]);
  }
  assert(0 != ri->id);
  assert(ri->id <= ITEM_MAX);
  assert(itIdxs[ri->id-1].crc_prod_code == cpc);
  assert(itIdxs[ri->id-1].crc_name3 == cn3);
  assert(cn == eeprom_read_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_itIdxName))+ri->id-1) );
  //  printf("ee24xaddr:%x cpc:%x cn:%x\n", ee24x_addr, cpc, cn);
}

void
delete_item(uint16_t id)
{
  all_items[id-1].unused_crc = all_items[id-1].unused_crc_invert = 0xFF;
}

int
main(int argc, char *argv[])
{
  struct item ri, ri1;
  uint32_t ui1, ui2, ui3, ui4, ui5, ui6;

  if ((argc == 1) || (0 == argv[1]))
    ui1 = time(NULL);
  else
    ui1 = atoi(argv[1]);
  printf("seed : %d\n", ui1);
  srand(ui1);

  /* */
  common_init();
  assert_init();
  menuInit();
  i2c_init();
  ep_store_init();
  KbdInit();

  /* test init */
  for (ui1=0; ui1<EEPROM_SIZE; ui1++)
    I2C_EEPROM_DIRECT_ASSIGN(ui1, rand());
  for (ui1=0; ui1<AVR_EEPROM_SIZE; ui1++)
    AVR_EEPROM_DIRECT_ASSIGN(ui1, rand());
  KBD_RESET_KEY;
  memset(all_items, 0xFF, ITEM_SIZEOF*ITEM_MAX);

#define NUM_ITEMS2TEST  ITEM_MAX
  /* Test to check the number of items that could be stored */
  for (ui1=0; ui1<NUM_ITEMS2TEST; ui1++) {
    make_item(all_items+ui1, 0);
  }
  /* adding one more item should hit an assertion */
  make_item(&ri, 0);
  assert(0 == strncmp(lcd_buf[0], "Items Mem Full..", LCD_MAX_COL));

  /* Check all items as per expectations */
  for (ui1=0; ui1<NUM_ITEMS2TEST; ui1++) {
    compare_item(all_items+ui1, itemAddr(ui1+1));
  }

  for (ui2=0; ui2<NUM_ITEMS2TEST; ui2++) {
    /* Now delete an item and check if the item was replaced */
    RESET_TEST_KEYS;
    ui1 = rand() % ITEM_MAX; ui1++;
    arg1.valid = MENU_ITEM_NONE;
    ui3 = 0;
    int2str(inp, ui1, &ui3);
    INIT_TEST_KEYS(inp);
    menuGetOpt(menu_str1+(MENU_STR1_IDX_ITEM*MENU_PROMPT_LEN), &arg1, MENU_ITEM_ID, NULL);
    ui5 = numValidItems;
    menuDelItem(MENU_MNORMAL);
    delete_item(ui1);
    assert((ui5-1) == numValidItems);

    /* check crc */
    ee24xx_read_bytes(itemAddr(ui1), (void *)&ri, ITEM_SIZEOF);
    assert(0xFF != (ri.unused_crc ^ ri.unused_crc_invert));

    /* check indexing */
    assert (0xFF == itIdxs[ui1-1].crc_name3);
    assert (0xFF == itIdxs[ui1-1].crc_prod_code);
    assert (0xFF == eeprom_read_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_itIdxName))+ui1-1));

    /* Now add item and check for validness*/
    make_item(all_items+(ui1-1), 0);
    compare_item(all_items+(ui1-1), itemAddr(ui1));

    assert(numValidItems == ITEM_MAX);
  }

  /* find some items */
  for (ui2=0; ui2<6000; ui2++) {
    ui1 = rand() % ITEM_MAX;
    ui1++;
    ee24xx_read_bytes(itemAddr(ui1), (void *)&ri, ITEM_SIZEOF);
    assert(ui1 == ri.id);
    ui4 = rand() & 3;
    if (3 == ui4) {
      ui3 = menuItemFind(ri.name, NULL, &ri1, 0);
    } else if (2 == ui4) {
      ui3 = menuItemFind(NULL, ri.prod_code, &ri1, 0);
    } else {
      ui3 = menuItemFind(ri.name, ri.prod_code, &ri1, 0);
    }
    assert(((uint16_t)-1) != ui3);
    if (ui1 != ui3) {
      ee24xx_read_bytes(itemAddr(ui3), (void *)&ri1, ITEM_SIZEOF);
      if (3 == ui4) {
	if (0 != strncmp(ri.name, ri1.name, ITEM_NAME_BYTEL)) {

	  printf("not equal %d:'%s', %d:'%s'\n", ri.id, ri.name, ri1.id, ri1.name);
	}
	assert(0 == strncmp(ri.name, ri1.name, ITEM_NAME_BYTEL));
      } else if (2 == ui4) {
	if (0 != strncmp(ri.prod_code, ri1.prod_code, ITEM_PROD_CODE_BYTEL)) {
	  printf("not equal prod_code %d:'%s', %d:'%s'\n", ri.id, ri.prod_code, ri1.id, ri1.prod_code);
	}
	assert(0 == strncmp(ri.prod_code, ri1.prod_code, ITEM_PROD_CODE_BYTEL));
      } else {
	printf("not equal %d:'%s', %d:'%s'\n", ri.id, ri.name, ri1.id, ri1.name);
	printf("not equal prod_code %d:'%s', %d:'%s'\n", ri.id, ri.prod_code, ri1.id, ri1.prod_code);
	printf("ui1:%d found ui3:%d\n", ui1, ui3);
	assert(0);
      }
    }
    //    else printf("equal\n");
  }

}
