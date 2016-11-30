
#include "common_incl.c"

#define MAX_VALID_BILLS 100
uint16_t vbill_idx[MAX_VALID_BILLS];
uint16_t idx[MAX_VALID_BILLS];

void
sort(uint16_t *arr, uint16_t *idx)
{
  uint16_t a, i, j;

  for (i = 0; i < MAX_VALID_BILLS; ++i) {
    for (j = i + 1; j < MAX_VALID_BILLS; ++j) {
      if (arr[i] > arr[j]) {
	a =  arr[i];
	arr[i] = arr[j];
	arr[j] = a;
	/* rearrange idx */
	a = idx[i];
	idx[i] = idx[j];
	idx[j] = a;
      }
    }
  }
}

int
main()
{
  uint16_t ui16_1, ui16_2, ui16_3, ui16_4;
  uint16_t arr[2];
  struct sale sl;

  srand(time(NULL));
  KbdInit();

  /* when can't find a bill should error out */
  eeprom_update_word((uint16_t *)(offsetof(struct ep_store_layout, unused_next_billaddr)), EEPROM_SALE_START_ADDR);
  menuShowBill(0);
  assert(0 == strncmp(lcd_buf[0], "Bill Not Found", 14));

  /* */
  ui16_3 = rand() % EEPROM_SALE_MAX_BILLS;
  ui16_3 = EEPROM_BILL_ADDR(ui16_3);

  /* create valid # bills */
  for (ui16_2=0; ui16_2<MAX_VALID_BILLS; ui16_3=EEPROM_NEXT_SALE_RECORD(ui16_3)) {
    ee24xx_read_bytes(ui16_3, (uint8_t *)arr, 4);
    if ( (rand() & 0x1) && (0xFFFF != (arr[0] ^ arr[1])) ) {
      sl.crc = rand();
      sl.crc_invert = ~(sl.crc);
      sl.info.bill_id = ui16_2+1;
      sl.total = rand() % 99999999;
      ee24xx_write_bytes(ui16_3, (uint8_t *)&sl, SIZEOF_SALE_EXCEP_ITEMS);
      eeprom_update_word((uint16_t *)(offsetof(struct ep_store_layout, unused_next_billaddr)), ui16_3);
      vbill_idx[ui16_2] = ui16_3;
      idx[ui16_2] = ui16_2+1;
      ui16_2++;
    }
  }

  uint ui1;

  /* Keep finding the same bill */
  ui16_2 = (rand() % MAX_VALID_BILLS)+1;
  eeprom_update_word((uint16_t *)(offsetof(struct ep_store_layout, unused_next_billaddr)), EEPROM_NEXT_SALE_RECORD(vbill_idx[ui16_2-1]));
  arg1.valid = arg2.valid = MENU_ITEM_NONE;
  KbdInTypeErrors = 0; /* don't let kbd.c introduce errors */
  for (ui16_1=0; ui16_1<1000; ui16_1++) {
    RESET_TEST_KEYS;
    inp2[0] = ASCII_ESCAPE;
    inp2[1] = 0;
    INIT_TEST_KEYS(inp2);
    menuShowBill(0);

    sscanf(lcd_buf[0]+1, "%u", &ui1);
    ui16_3 = ui1;
    assert(ui16_3 < MAX_VALID_BILLS);
    assert(vbill_idx[ui16_3-1]);
    assert(ui16_3 == ui16_2);
  }

  /* Find the prev one*/
  sort(vbill_idx, idx);
  ui16_2 = (rand() % MAX_VALID_BILLS);
  eeprom_update_word((uint16_t *)(offsetof(struct ep_store_layout, unused_next_billaddr)), vbill_idx[ui16_2]);
  arg1.valid = arg2.valid = MENU_ITEM_NONE;
  KbdInTypeErrors = 0; /* don't let kbd.c introduce errors */
  for (ui16_1=0; ui16_1<1000; ui16_1++) {
    ui16_3 = rand() % 10; /* find next this bill */
    RESET_TEST_KEYS;
    for (ui1=0; ui1<ui16_3; ui1++) {
      inp2[ui1] = ASCII_ENTER;
    }
    inp2[ui1] = ASCII_ESCAPE;
    inp2[ui1+1] = 0;
    INIT_TEST_KEYS(inp2);
    menuShowBill(0);

    /* it found something */
    ui16_4 = (ui16_2 < (ui16_3+1)) ? (MAX_VALID_BILLS+ui16_2-ui16_3-1) :
      (ui16_2-ui16_3-1);
    sscanf(lcd_buf[0]+1, "%u", &ui1);
    assert(ui16_4 < MAX_VALID_BILLS);
    assert(ui1 == idx[ui16_4]);
    //    printf("ui16_2:%u ui16_3:%u ui16_4:%u ui1:%u idx[ui16_4]:%u\n",
    //	   ui16_2, ui16_3, ui16_4, ui1, idx[ui16_4]);
  }

  /* Find the next one */
  sort(vbill_idx, idx);
  ui16_2 = (rand() % MAX_VALID_BILLS);
  eeprom_update_word((uint16_t *)(offsetof(struct ep_store_layout, unused_next_billaddr)), vbill_idx[ui16_2]);
  arg1.valid = arg2.valid = MENU_ITEM_NONE;
  KbdInTypeErrors = 0; /* don't let kbd.c introduce errors */
  for (ui16_1=0; ui16_1<1000; ui16_1++) {
    ui16_3 = rand() % 10; /* find next this bill */
    RESET_TEST_KEYS;
    inp2[0] = ASCII_RIGHT;
    for (ui1=1; ui1<(ui16_3+2); ui1++) {
      inp2[ui1] = ASCII_ENTER;
    }
    inp2[ui1] = ASCII_ESCAPE;
    inp2[ui1+1] = 0;
    INIT_TEST_KEYS(inp2);
    menuShowBill(0);

    /* it found something */
    ui16_4 = ((ui16_3+1) >= (MAX_VALID_BILLS-ui16_2)) ?
      (ui16_3-(MAX_VALID_BILLS-ui16_2)) :
      (ui16_2+ui16_3+1);
    sscanf(lcd_buf[0]+1, "%u", &ui1);
    assert(ui16_4 < MAX_VALID_BILLS);
    assert(ui1 == idx[ui16_4]);
    //    printf("ui16_2:%u ui16_3:%u ui16_4:%u ui1:%u idx[ui16_4]:%u\n",
    //	   ui16_2, ui16_3, ui16_4, ui1, idx[ui16_4]);
  }

  return 0;
}
