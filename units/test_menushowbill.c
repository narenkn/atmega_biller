
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
main(int argc, char *argv[])
{
  uint16_t ui16_1, ui16_2, ui16_3, ui16_4;
  uint16_t arr[2];
  struct sale sl;

  srand(time(NULL));
  KbdInit();

  uint32_t ui1;
  if ((argc == 1) || (0 == argv[1]))
    ui1 = time(NULL);
  else
    ui1 = atoi(argv[1]);
  //ui1 = 1487091282;
  printf("seed : %d\n", ui1);
  srand(ui1);

  /* time */
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  date_t d = {tm.tm_mday, tm.tm_mon+1, 1900+tm.tm_year};
  timerDateSet(d);
  arg1.valid = MENU_ITEM_DATE;
  timerDateGet(arg1.value.date);

  /* when can't find a bill should error out */
  eeprom_update_word((uint16_t *)(offsetof(struct ep_store_layout, unused_todayStartAddr)), NVF_SALE_START_ADDR);
  menuViewOldBill(0);
  assert(0 == strncmp(lcd_buf[0], "No Bills", 8));

  /* */
  ui16_3 = rand() % NVF_SALE_MAX_BILLS;
  ui16_3 = NVF_BILL_ADDR(ui16_3);
  ui16_3 = 0;

  /* create valid # bills */
  for (ui16_2=0; ui16_2<MAX_VALID_BILLS; ui16_2++, ui16_3=NVF_NEXT_SALE_RECORD(ui16_3)) {
    sl.crc = rand();
    sl.crc_invert = ~(sl.crc);
    sl.info.id = ui16_2+1;
    sl.info.date_dd = arg1.value.date.day;
    sl.info.date_mm = arg1.value.date.month;
    sl.info.date_yy = arg1.value.date.year;
    sl.total = rand() % 99999999;
    bill_write_bytes(ui16_3, (uint8_t *)&sl, SIZEOF_SALE_EXCEP_ITEMS);
    vbill_idx[ui16_2] = ui16_3;
    idx[ui16_2] = ui16_2+1;
  }
  eeprom_update_word((uint16_t *)(offsetof(struct ep_store_layout, unused_nextBillAddr)), ui16_3);
  /* save bills to disk */
  menuSdSaveBillDat(NVF_SALE_START_ADDR);
  /* read back, check */
  uint32_t ret_val;
  menuMemset(&FS, 0, sizeof(FS));
  menuMemset(&Fil, 0, sizeof(Fil));
  f_mount(&FS, ".", 1);
  date_t date = arg1.value.date;
  sprintf_P((char *)bufSS, PSTR("%02d-%02d-%04d.dat"), date.day, date.month, date.year);
  if ((FR_OK != f_chdir("billdat")) || (FR_OK != f_open(&Fil, (char *)bufSS, FA_READ|FA_OPEN_EXISTING))) {
    LCD_ALERT(PSTR("No Bills"));
    f_mount(NULL, "", 0);
    return 0;
  }
  for (ui16_2=0; ui16_2<MAX_VALID_BILLS; ui16_2++) {
    f_read(&Fil, (void *)&sl, MAX_SIZEOF_1BILL, &ret_val);
    if (MAX_SIZEOF_1BILL != ret_val) break;
    assert (0xFFFF == (sl.crc_invert ^ sl.crc));
    assert((1+ui16_2) == sl.info.id);
    //printf("sl.info.id:%d\n", sl.info.id);
  }
  f_close(&Fil);
  f_chdir("..");
  f_mount(NULL, "", 0);

  /* don't let kbd.c introduce errors */
  KbdInTypeErrors = 0;

  /* Keep finding the first bill */
  arg2.valid = MENU_ITEM_NONE;
  for (ui16_1=0; ui16_1<10; ui16_1++) {
    RESET_TEST_KEYS;
    inp2[0] = ASCII_ESCAPE;
    inp2[1] = 0;
    INIT_TEST_KEYS(inp2);
    menuViewOldBill(0);

    sscanf(lcd_buf[0]+1, "%u", &ui1);
    ui16_3 = ui1;
    assert(ui16_3 > 0);
    assert(ui16_3 <= MAX_VALID_BILLS);
    assert(idx[ui16_3-1]);
    assert(1 == ui16_3);
  }

  /* Find consecutive next bills */
  for (ui16_1=0; ui16_1<5; ui16_1++) {
    RESET_TEST_KEYS;
    for (ui1=0; ui1<ui16_1; ui1++)
      inp2[ui1] = ASCII_ENTER;
    inp2[ui1] = ASCII_ESCAPE;
    inp2[ui1+1] = 0;
    INIT_TEST_KEYS(inp2);
    menuViewOldBill(0);

    sscanf(lcd_buf[0]+1, "%u", &ui1);
    ui16_3 = ui1;
    assert(ui16_3 > 0);
    assert(ui16_3 <= MAX_VALID_BILLS);
    assert(idx[ui16_3-1]);
    assert((1+ui16_1) == ui16_3);
  }

  /* Find consecutive prev bills */
  for (ui16_1=0; ui16_1<MAX_VALID_BILLS-1; ui16_1++) {
    RESET_TEST_KEYS;
    for (ui1=0; ui1<MAX_VALID_BILLS-1; ui1++)
      inp2[ui1] = ASCII_ENTER;
    inp2[ui1++] = ASCII_LEFT;
    for (ui16_2=0; ui16_2<ui16_1; ui16_2++)
      inp2[ui1++] = ASCII_ENTER;
    inp2[ui1++] = ASCII_ESCAPE;
    inp2[ui1++] = 0;
    INIT_TEST_KEYS(inp2);
    menuViewOldBill(0);

    sscanf(lcd_buf[0]+1, "%u", &ui1);
    ui16_3 = ui1;
    assert(ui16_3 > 0);
    assert(ui16_3 <= MAX_VALID_BILLS);
    assert(idx[ui16_3-1]);
    //printf("lcd_buf:%s ui16_3:%d ui16_1:%d\n", lcd_buf[0], ui16_3, ui16_1);
    assert((MAX_VALID_BILLS-1-ui16_1) == ui16_3);
  }

  return 0;
}
