#include "test_common.c"

/* test item add/ modify/ save
 */

/* place to store items */
struct item all_items[ITEM_MAX];

uint8_t inp5[5][TEST_KEY_ARR_SIZE];

void
make_item(struct item *ri1, uint8_t rand_save, uint8_t check_success)
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
  //PRINT_TEST_KEYS(ui1);
  menuAddItem(MENU_MNORMAL);
  //printf("test_key_arr_idx:%d\n", test_key_arr_idx);

  /* find index that it would use to store */
  for (ui1=0; ui1<ITEM_MAX; ui1++) {
    if (0 == all_items[ui1].id)
      break;
  }

  if (ui1 < ITEM_MAX) {
    struct item it;
    ee24xx_read_bytes(menuItemAddr(ui1), (void *)&it, ITEM_SIZEOF);
    ri1->id = save ? ui1+1 : 0;
    //printf("ui1:%d ri1->id:%d it.id:%d\n", ui1, ri1->id, it.id);
    //printf("lcd_buf:%s\n", lcd_buf[0]);
    assert(ri1->id == it.id);
  }

  /* */
  assert((uint16_t)-1 == test_key_idx);
  if (check_success) {
    assert(0 == strncmp(lcd_buf[0], "Success!        ", LCD_MAX_COL));
  }
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
//  for (ui1=0; ui1<ITEM_SIZEOF;) {
//    printf("%02x ", bufTT[ui1]);
//    ui1++;
//    if (0 == (ui1%16)) printf("\n");
//  }
//  printf("\n");

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
  ui1 = uint8ncmp(ri->name_unicode, rif.name_unicode, ITEM_NAME_UNI_BYTEL);
  if (0 != ui1) {
    printf("ERROR: name-unicode mismatch byte:%d\n", ui1);
  }
  if (ri->vat_sel != rif.vat_sel) {
    printf("ERROR: vat_sel mismatch Exp:'%d' Obt:'%d'\n", ri->vat_sel, rif.vat_sel);
  }
  if (ri->has_serv_tax != rif.has_serv_tax) {
    printf("ERROR: serv_tax mismatch Exp:'%d' Obt:'%d'\n", ri->has_serv_tax, rif.has_serv_tax);
  }

  /* */
  uint32_t ui32_1;
  uint16_t ui16_1;
  itemIdxs_t itIdx;
  for (ui1=0, ui16_1=0; ui1<ITEM_NAME_BYTEL; ui1++) {
    assert(ri->name[ui1] == rif.name[ui1]);
    //printf("rif.name:%s ri->c:0x%x rif.c:0x%x\n", rif.name, ri->name[ui1], rif.name[ui1]);
    assert(ri->name[ui1] >= ' ');
    assert(ri->name[ui1] < '~');
    ui16_1 = _crc16_update(ui16_1, ri->name[ui1]);
  }
  assert(0 != ri->id);
  ui32_1 = (uint32_t)((ITEM_MAX_ADDR + (sizeof(itemIdxs_t) * (ri->id-1))) >> EEPROM_MAX_DEVICES_LOGN2);
  ee24xx_read_bytes(ui32_1, (uint8_t *)&itIdx, sizeof(itemIdxs_t));
  assert(ui16_1 == itIdx[1]);
  //printf("ui16_1:0x%0x itIdx[1]:0x%0x\n", ui16_1, itIdx[1]);
}

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
main(int argc, char *argv[])
{
  struct item ri, ri1;
  uint32_t ui1, ui2, ui3, ui4;

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
    I2C_EEPROM_DIRECT_ASSIGN(ui1, 0);
  for (ui1=0; ui1<EEPROM_MAX_ADDRESS; ui1++)
    EEPROM_DIRECT_ASSIGN(ui1, 0);
  for (ui1=0; ui1<ITEM_MAX; ui1++) {
    all_items[ui1].id = 0;
  }
  KBD_RESET_KEY;

#define NUM_ITEMS2TEST  ITEM_MAX
  /* Test to check the number of items that could be stored */
  for (ui1=0; ui1<NUM_ITEMS2TEST; ui1++) {
    make_item(all_items+ui1, 0, 1);
  }
  /* adding one more item should hit an assertion */
  make_item(&ri, 0, 0);
  assert(0 == strncmp(lcd_buf[0], "Items full      ", LCD_MAX_COL));
  RESET_TEST_KEYS;

  /* Check all items as per expectations */
  for (ui1=0; ui1<NUM_ITEMS2TEST; ui1++) {
    //printf("compare_item: 0:%p 1:%x\n", all_items+ui1, menuItemAddr(ui1+1));
    compare_item(all_items+ui1, menuItemAddr(ui1));
  }

  for (ui2=0; ui2<300; ui2++) {
    /* Now delete an item and check if the item was replaced */
    ui1 = rand() % ITEM_MAX;
    ui1++;
    arg1.valid = MENU_ITEM_NONE;
    ui3 = 0;
    int2str(inp, ui1, &ui3);
    INIT_TEST_KEYS(inp);
    menuGetOpt(menu_str1+(MENU_STR1_IDX_ITEM*MENU_PROMPT_LEN), &arg1, MENU_ITEM_ID, NULL);
    menuDelItem(MENU_MNORMAL);
    ee24xx_read_bytes(menuItemAddr((ui1-1)), (void *)&ri, ITEM_SIZEOF);
    assert(0 == ri.id);
    /* check indexing */
    itemIdxs_t itIdx;
    ui3 = (uint32_t)((ITEM_MAX_ADDR + (sizeof(itemIdxs_t) * (ui1-1))) >> EEPROM_MAX_DEVICES_LOGN2);
    ee24xx_read_bytes(ui3, (void *)&itIdx, sizeof(itemIdxs_t));
    assert(0 == itIdx[0]);
    assert(0 == itIdx[1]);
    assert(0 == itIdx[2]);
    assert(0 == itIdx[3]);

    /* Now add item and check for validness*/
    make_item(all_items+ui1-1, 0, 1);
    compare_item(all_items+ui1-1, menuItemAddr((ui1-1)));
  }

  /* find some items */
  for (ui2=0; ui2<300; ui2++) {
    ui1 = rand() % ITEM_MAX;
    ui1++;
    ee24xx_read_bytes(menuItemAddr((ui1-1)), (void *)&ri, ITEM_SIZEOF);
    assert(ui1 == ri.id);
    ui4 = rand() & 3;
    if (3 == ui4) {
      ui3 = menuItemFind(ri.name, NULL, &ri1, 0);
    } else if (2 == ui4) {
      ui3 = menuItemFind(NULL, ri.prod_code, &ri1, 0);
    } else {
      ui3 = menuItemFind(ri.name, ri.prod_code, &ri1, 0);
    }
    assert(-1 != ui3);
    if (ui1 != ui3) {
      ee24xx_read_bytes(menuItemAddr((ui3-1)), (void *)&ri1, ITEM_SIZEOF);
      if (3 == ui4) {
	//	printf("not equal %d:'%s', %d:'%s'\n", ri.id, ri.name, ri1.id, ri1.name);
	assert(0 == strncmp(ri.name, ri1.name, ITEM_NAME_BYTEL));
      } else if (2 == ui4) {
	//	printf("not equal prod_code %d:'%s', %d:'%s'\n", ri.id, ri.prod_code, ri1.id, ri1.prod_code);
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
