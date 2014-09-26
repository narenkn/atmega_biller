#include "test_common.c"

/* test item add/ modify/ save
 */

/* place to store items */
struct item all_items[ITEM_MAX];

uint8_t inp5[4][TEST_KEY_ARR_SIZE];

void
make_item(struct item *ri1, uint8_t rand_save)
{
  uint32_t ui1, ui2, ui3, ui4;
  uint8_t save;

  /* Construct a random item */
  ui3 = (rand() % (ITEM_NAME_BYTEL-1)) + 1;
  for (ui1=0; ui1<ui3; ui1++) {
    inp5[0][ui1] = 'a' + (rand()%26);
    ri1->name[ui1] = inp5[0][ui1];
  }
  inp5[0][ui1] = 0;
  ri1->name[ui1] = inp5[0][ui1];
  INIT_TEST_KEYS(inp5[0]);

  /* cost, discount */
  ri1->cost = rand() % (1<<14);
  ri1->discount = rand() % ri1->cost;
  ui2 = 0;
  int2str(inp5[1], ri1->cost, &ui2);
  inp5[1][ui1] = 0;
  INIT_TEST_KEYS(inp5[1]);
  ui2 = 0;
  int2str(inp5[2], ri1->discount, &ui2);
  inp5[2][ui1] = 0;
  INIT_TEST_KEYS(inp5[2]);

  /* Prod code */
  ui2 = (rand() % (ITEM_PROD_CODE_BYTEL-1)) + 1;
  for (ui1=0; ui1<ui2; ui1++) {
    ri1->prod_code[ui1] = inp[ui1] = '!' + (rand() % ('~'-'!'));
  }
  inp[ui2] = 0;
  for (;ui2<ITEM_PROD_CODE_BYTEL;ui2++) {
    ri1->prod_code[ui2] = ' ';
  }
  INIT_TEST_KEYS(inp);

  /* Uni code */
  ui2 = (rand() % (ITEM_NAME_UNI_BYTEL-1)) + 1;
  for (ui1=0; ui1<ui2; ui1++) {
    ui2 = rand() % 0x10;
    ri1->name_unicode[ui1] = inp2[ui1] = (ui2<10) ? '0' + ui2 : 'a' + (ui2-10);
  }
  for (; ui2<ITEM_NAME_UNI_BYTEL; ui2++) {
    ri1->name_unicode[ui2] = ' ';
  }
  INIT_TEST_KEYS(inp2);
  
  /* vat */
  ri1->vat_sel = rand() % EPS_MAX_VAT_CHOICE;
  ui2 = 0;
  int2str(inp3, ri1->vat_sel, &ui2);
  inp3[ui1] = 0;
  INIT_TEST_KEYS(inp3);

  /* serv tax */
  ri1->has_serv_tax = rand();
  ui2 = 0;
  int2str(inp4, ri1->vat_sel, &ui2);
  inp4[ui1] = 0;
  INIT_TEST_KEYS(inp4);

  /* confirm save of item */
  ui2 = rand_save ? rand()&16 : rand()&0xE;
  for (ui1=0; ui1<ui2; ui1++) {
    inp5[3][ui1] = (rand()&1) ? ASCII_RIGHT : ASCII_LEFT;
  }
  inp5[3][ui2] = 0;
  save = (ui2 & 1) ? 0 : 1;

  arg1.valid = MENU_ITEM_NONE;
  arg1.value.sptr = bufSS;
  menuGetOpt(menu_str1+(MENU_STR1_IDX_NAME*MENU_PROMPT_LEN), &arg1, MENU_ITEM_STR);
  menuAddItem(MENU_MNORMAL);
  printf("test_key_arr_idx:%d\n", test_key_arr_idx);

  /* find index that it would use to store */
  for (ui1=0; ui1<ITEM_MAX; ui1++) {
    if (0 == all_items[ui1].id)
      break;
  }
  assert (ui1 < ITEM_MAX);

  struct item it;
  char *b = (char *)&it;
  ui4 = menuItemAddr(ui1);
  for (ui2=0; ui2<ITEM_SIZEOF; ui2++, ui4++) {
    b[ui2] = eeprom_read_byte((uint8_t *)ui4);
  }
  ri1->id = save ? ui1 : 0;
}

//void
//compare_item(item *ri, uint16_t flash_item)
//{
//  uint16_t ui1;
//  item rif;
//  char *bufTT = (void *) &(rif);
//
//  for (ui1=0; ui1<ITEM_SIZEOF; ui1++)
//    bufTT[ui1] = FlashReadByte(flash_item+ui1);
//
//  if (0 != strncmp(&(ri->name[0]), &(rif.name[0]))) {
//    fprintf(stderr, "ERROR: Name mismatch Exp:'%s' Obt:'%s'\n", ri->name, rif.name);
//  }
//  if (ri->cost != rif.cost) {
//    fprintf(stderr, "ERROR: cost mismatch Exp:'%s' Obt:'%s'\n", ri->cost, rif.cost);
//  }
//  if (ri->discount != rif.discount) {
//    fprintf(stderr, "ERROR: discount mismatch Exp:'%s' Obt:'%s'\n", ri->discount, rif.discount);
//  }
//  if (ri->vat_sel != rif.vat_sel) {
//    fprintf(stderr, "ERROR: vat_sel mismatch Exp:'%s' Obt:'%s'\n", ri->vat_sel, rif.vat_sel);
//  }
//  if (ri->has_serv_tax != rif.has_serv_tax) {
//    fprintf(stderr, "ERROR: cost mismatch Exp:'%s' Obt:'%s'\n", ri->has_serv_tax, rif.has_serv_tax);
//  }
//}
//
//void
//delete_item(item *ri, uint16_t id)
//{
//  uint16_t ui1;
//  uint8_t  ui2;
//
//  //  ui2 = 0;
//  //  int2str(id, &ui2);
//  INIT_TEST_KEYS(inp[4]);
//  menu_DelItem(MENU_MSUPER);
//
//  char *bufTT = (void *)ri;
//  for (ui1=0; ui1<ITEM_SIZEOF; ui1++) {
//    bufTT[ui1] = 0;
//  }
//}

int
main(void)
{
  uint32_t ui1, ui2, ui3;

  srand(time(NULL));

  /* */
  assert_init();
  menuInit();
  i2c_init();
  ep_store_init();

  /* test init */
  for (ui1=0; ui1<EEPROM_SIZE; ui1++)
    I2C_EEPROM_DIRECT_ASSIGN(ui1, 0);
  for (ui1=0; ui1<EEPROM_MAX_ADDRESS; ui1++)
    EEPROM_DIRECT_ASSIGN(ui1, 0);
  for (ui1=0; ui1<ITEM_MAX; ui1++) {
   all_items[ui1].id = 0;
  }
  KBD_RESET_KEY;

  /* Test to check the number of items that could be stored */
  for (ui1=0; ui1<1; ui1++) {
    make_item(&(all_items[ui1]), 0);
  }
  { /* adding one more item should hit an assertion */
    struct item ri;
    //    add_expect_assert("Items Exceeded");
    //    make_item(&ri, 0);
  }
  printf("lcd_buf:'%s'\n", lcd_buf[0]);

//  /* Check all items as per expectations */
//  for (ui1=0; ui1<1; ui1++) {
//    compare_item(all_items+ui1, flash_item_find(ui1));
//  }
//
//  /* Now delete an item and check if the item was replaced */
//  ui1 = rand() % ITEM_MAX;
//  delete_item(all_items+ui1, ui1);
//  compare_item(all_items+ui1, flash_item_find(ui1));
//
//  /* Now add item and check for validness*/
//  add_item(all_items+ui1);
//  for (ui1=0; ui1<1; ui1++) {
//    compare_item(all_items+ui1, flash_item_find(ui1));
//  }
}
