#include <stdint.h>

#define UNIT_TEST_MENU_1 menu_handler
void menu_handler(uint8_t ui);

#include "common_incl.c"

void
menu_handler(uint8_t ui)
{
  LCD_CLRLINE(1);
  LCD_WR_NP(PSTR("menu_handler:"), 13);
  LCD_PUT_UINT8X(ui);
  LCD_refresh();
}

int
main(int argc, char *argv[])
{
  uint32_t loop, size, ui32_1;
  uint32_t ui1;

  if ((argc == 1) || (0 == argv[1]))
    ui32_1 = time(NULL);
  else
    ui32_1 = atoi(argv[1]);
  printf("seed : %d\n", ui32_1);
  srand(ui32_1);

  KbdInit();

#define DEBUG 1

  /* test menuGetOpt::MENU_ITEM_STR */
  for (loop=0; loop<1000; loop++) {
    size = (rand() % (TEST_KEY_ARR_SIZE-1)) + 1;
    for (ui1=0; ui1<size; ui1++) {
      if (0 == (rand() % 3))
	inp[ui1] = 'A' + (rand()%26);
      else
	inp[ui1] = 'a' + (rand()%26);
    }
    inp[size] = 0;
    INIT_TEST_KEYS(inp);
    KBD_RESET_KEY;
    arg1.value.str.sptr = bufSS;
    arg1.value.str.len = size;
    menuGetOpt("Prompt 1", &arg1, MENU_ITEM_STR, NULL);
    assert(0 == strncmp("Prompt 1?", &(lcd_buf[1][0]), 9));
    if (0 != strncmp("Prompt 1?", &(lcd_buf[1][0]), 9)) {
#if DEBUG
      for (ui1=0; ui1<LCD_MAX_COL; ui1++)
	putchar(lcd_buf[0][ui1]);
      putchar('\n');
      for (ui1=0; ui1<LCD_MAX_COL; ui1++)
	putchar(lcd_buf[LCD_MAX_ROW-1][ui1]);
      putchar('\n');
#endif
    }
    if (size <= (LCD_MAX_COL-9)) {
      assert(0 == strncmp(inp, lcd_buf[LCD_MAX_ROW-1]+9, size));
#if DEBUG
      if (0 != strncmp(inp, lcd_buf[LCD_MAX_ROW-1]+9, size)) {
	printf("size:%d lcd_buf:", size);
	for (ui1=0; ui1<LCD_MAX_COL; ui1++)
	  printf("%c ", lcd_buf[LCD_MAX_ROW-1][ui1]);
	printf("\ninp    :");
	for (ui1=0; ui1<LCD_MAX_COL; ui1++)
	  printf("%c ", inp[ui1]);
	printf("\n");
      }
#endif
    } else {
      assert(0 == strncmp(inp+size-(LCD_MAX_COL-9), lcd_buf[LCD_MAX_ROW-1]+9, LCD_MAX_COL-9));
      assert(0 == strncmp(inp, arg1.value.str.sptr, size));
      //printf("lcd_buf:%s\n", lcd_buf);
    }
  }

  /* test menuGetOpt::MENU_ITEM_STR : 0 size
     If user just escapes or hits-enter with 0 len
     then it has to be marked MENU_ITEM_NONE
   */
  for (loop=0; loop<1000; loop++) {
    RESET_TEST_KEYS;
    size = (rand() % (TEST_KEY_ARR_SIZE-1)) + 1;
    for (ui1=0; ui1<size; ui1++) {
      if (0 == (rand() % 3))
	inp[ui1] = ASCII_ENTER;
      else
	inp[ui1] = ASCII_ESCAPE;
    }
    inp[size] = 0;
    INIT_TEST_KEYS(inp);
    KBD_RESET_KEY;
    arg1.value.str.sptr = bufSS;
    arg1.value.str.len = size;
    menuGetOpt("Prompt 1", &arg1, MENU_ITEM_STR, NULL);
    assert(0 == strncmp("Prompt 1?", &(lcd_buf[1][0]), 9));
    if (0 != strncmp("Prompt 1?", &(lcd_buf[1][0]), 9)) {
#ifdef DEBUG
      for (ui1=0; ui1<LCD_MAX_COL; ui1++)
	putchar(lcd_buf[0][ui1]);
      putchar('\n');
      for (ui1=0; ui1<LCD_MAX_COL; ui1++)
	putchar(lcd_buf[LCD_MAX_ROW-1][ui1]);
      putchar('\n');
#endif
    }
    assert(MENU_ITEM_NONE == arg1.valid);
  }

  /* test menuGetOpt::MENU_ITEM_ID */
  for (loop=0; loop<1000; loop++) {
    RESET_TEST_KEYS;
    //    printf("loop:%d\n", loop);
    uint32_t r1 = rand() & 0xFFFFFF, r2;
    uint8_t  s[16], ui2;
    r2 = r1;
    for (ui1=0; r1; ui1++) {
      s[ui1] = '0' + r1%10;
      r1 /= 10;
    }
    inp[ui1] = 0;
    for (ui2=0;ui1;ui1--, ui2++) {
      inp[ui1-1] = s[ui2];
    }
    INIT_TEST_KEYS(inp);
    KBD_RESET_KEY;
    menuGetOpt("lsjdflkjf", &arg1, MENU_ITEM_ID, NULL);
    r1 = arg1.value.integer.i32;
    assert(r2 == r1);
    if (r2 != r1) {
      printf("r2:0x%x r1:0x%x\n", r2, r1);
    }
  }

  /* test menuGetOpt::MENU_ITEM_FLOAT */
  for (loop=0; loop<1000; loop++) {
    RESET_TEST_KEYS;
    //    printf("loop:%d\n", loop);
    uint32_t r1 = rand() & 0xFFFFF, r2;
    uint8_t  r3 = rand() % 100;
    uint8_t  s[16], ui2;
    r2 = r1;
    sprintf(s, "%d.%d", r1, r3);
    strcpy(inp, s);
    INIT_TEST_KEYS(inp);
    KBD_RESET_KEY;
    menuGetOpt("lsjdflkjf", &arg1, MENU_ITEM_FLOAT, NULL);
    r2 = arg1.value.integer.i32;
    r1 *= 100;
    r1 += (r3<10) ? r3*10 : r3;
    assert(r2 == r1);
    if (r2 != r1) {
      printf("s:%s r2:%u r1:%d\n", s, r2, r1);
    }
  }

  /* test menuGetOpt::MENU_ITEM_DATE */
  RESET_TEST_KEYS;
  for (loop=0; loop<1000; loop++) {
    uint8_t date, month;
    uint16_t year = 2000;
    date = 1 + (rand() % 28);
    month = 1 + (rand() % 12);
    year += rand() % 100;
    sprintf(inp, "%02d%02d%04d", date, month, year);
    INIT_TEST_KEYS(inp);
    KBD_RESET_KEY;
    menuGetOpt("dflkjf", &arg1, MENU_ITEM_DATE, NULL);
    assert(MENU_ITEM_DATE == arg1.valid);
    if ( (date != arg1.value.date.day) ||
	 (month != arg1.value.date.month) ||
	 (year != arg1.value.date.year) ) {
      printf("string:%s\n", inp);
      printf("date:%x org:%d\n", arg1.value.date.day, date);
      printf("month:%x org:%d\n", arg1.value.date.month, month);
      printf("year:%x org:%d\n", arg1.value.date.year, year);
    }
    assert(date == arg1.value.date.day);
    assert(month == arg1.value.date.month);
    assert(year == arg1.value.date.year);
  }

  /* test menuGetOpt::MENU_ITEM_MONTH */
  for (loop=0; loop<1000; loop++) {
    uint8_t month;
    uint16_t year = 2000;
    month = 1 + (rand() % 12);
    year += rand() % 100;
    sprintf(inp, "%02d%04d", month, year);
    INIT_TEST_KEYS(inp);
    KBD_RESET_KEY;
    menuGetOpt("kjslfjklsdlff", &arg1, MENU_ITEM_MONTH, NULL);
    if ( (month != arg1.value.date.month) || (year != arg1.value.date.year) ) {
      printf("string:%s\n", inp);
      printf("month:%d org:%d\n", arg1.value.date.month, month);
      printf("year:%d org:%d\n", arg1.value.date.year, year);
    }
    assert(month == arg1.value.date.month);
    assert(year == arg1.value.date.year);
  }

  /* test menuGetOpt::MENU_ITEM_TIME */
  for (loop=0; loop<1000; loop++) {
    uint8_t hour, min, sec;
    hour = rand() % 24;
    min = rand() % 60;
    sec = rand() % 60;
    sprintf(inp, "%02d%02d%02d", hour, min, sec);
    INIT_TEST_KEYS(inp);
    KBD_RESET_KEY;
    menuGetOpt("dflkjf", &arg1, MENU_ITEM_TIME, NULL);
    if ( (hour != arg1.value.time.hour) || (min != arg1.value.time.min) ) {
      printf("string:%s\n", inp);
      printf("hour:%d org:%d\n", arg1.value.time.hour, hour);
      printf("min:%d org:%d\n", arg1.value.time.min, min);
      printf("sec:%d org:%d\n", arg1.value.time.sec, sec);
      printf("%s\n", lcd_buf[0]);
    }
    assert(0 == strncmp("dflkjf?", lcd_buf[1], 7));
    assert(hour == arg1.value.time.hour);
    assert(min == arg1.value.time.min);
  }

  /* test menuGetChoice */
  KBD_RESET_KEY;
  for (loop=0; loop<1000; loop++) {
    //    printf("loop:%d\n", (uint32_t)loop);
    uint8_t ui1, ui2;
    uint16_t ui3;
    ui1 = rand() % MENU_STR1_IDX_NUM_ITEMS; /* choosen one */
    ui2 = 0; ui3 = 0;
    while (ui1 != ui2) {
      if (rand()%2) {
	inp[ui3] = ASCII_LEFT;
	ui3++;
	ui2 = (0==ui2) ? MENU_STR1_IDX_NUM_ITEMS-1 : ui2-1;
      } else {
	inp[ui3] = ASCII_RIGHT;
	ui3++;
	ui2 = (MENU_STR1_IDX_NUM_ITEMS-1==ui2) ? 0 : ui2+1;
      }
    }
    inp[ui3] = 0;
    if (ui3 >= TEST_KEY_ARR_SIZE)
      continue;
    INIT_TEST_KEYS(inp);
    ui3 = menuGetChoice(PSTR("what crap?"), (uint8_t *)menu_str1, MENU_PROMPT_LEN, MENU_STR1_IDX_NUM_ITEMS);
#if 0
    assert(0 == strncmp("what : Name     ", lcd_buf, LCD_MAX_COL));
    if (0 != strncmp("what : Name     ", lcd_buf, LCD_MAX_COL)) {
#ifdef DEBUG
      for (ui1=0; ui1<LCD_MAX_COL; ui1++)
	putchar(lcd_buf[0][ui1]);
      putchar('\n');
      for (ui1=0; ui1<LCD_MAX_COL; ui1++)
	putchar(lcd_buf[LCD_MAX_ROW-1][ui1]);
      putchar('\n');
#endif
    }
#endif
    if (ui3 != ui1) {
      for (ui2=0; 0 != inp[ui2]; ui2++) {
	printf("0x%x ", inp[ui2]);
      }
      printf("\nExp:0x%x Act:0x%x\n", ui1, ui3);
    }
    assert(ui3 == ui1);
  }

  /* test menuGetYesNo */
  KBD_RESET_KEY;
  for (loop=0; loop<1000; loop++) {
    uint8_t ui1, ui2, ui3;

    /* decide yes or no */
    ui1 = rand() % (TEST_KEY_ARR_SIZE-1); /* space for \0 */
    for (ui2=0; ui2<ui1; ui2++) {
      inp[ui2] = (rand()&1) ? ASCII_RIGHT : ASCII_LEFT;
    }
    inp[ui1] = 0;
    INIT_TEST_KEYS(inp);

    /* prompt */
    ui2 = (rand() % 9)+1;
    for (ui3=0; ui3<ui2; ui3++) {
      inp2[ui3] = (rand() & 1) ? 'a' + (rand()%26) : 'A' + (rand()%26);
    }
    inp2[ui2] = 0;
    assert( (ui1 & 1) == (1 & menuGetYesNo(inp2, ui2)) );
    ui2 += (ui1 & 1) ? sprintf(inp2+ui2, ":No ?") :
      sprintf(inp2+ui2, ":Yes?");
    assert(0 == strncmp(inp2, lcd_buf[LCD_MAX_ROW-1], ui2));
    //printf("ui2:%d inp2:'%s' lcd_buf:'%s'\n", ui2, inp2, lcd_buf[LCD_MAX_ROW-1]);
  }

  return 0;
}
