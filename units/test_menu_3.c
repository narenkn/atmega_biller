#include "test_common.c"

/* tests user login/logout features */
int
main(void)
{
  uint32_t loop;
  uint8_t ui1, ui2, ui3, ui4;

  srand(time(NULL));
  common_init();

  /* */
  assert_init();
  menuInit();
  i2c_init();
  ep_store_init();

  /* menuSetPasswd */
  for (loop=0; loop<1000; loop++) {
    RESET_TEST_KEYS;
 
    uint16_t passwd_size = ( rand() % (LCD_MAX_COL-1) ) + 1;
    for (ui1=0; ui1<passwd_size; ui1++) {
      if (0 == (rand() % 3))
	inp[ui1] = 'A' + (rand()%26);
      else
	inp[ui1] = 'a' + (rand()%26);
    }
    inp[ui1] = 0;
    INIT_TEST_KEYS(inp);
    KBD_RESET_KEY;
    arg2.value.str.sptr = bufSS+LCD_MAX_COL+2;
    arg2.value.str.len = passwd_size;
    menuGetOpt("Prompt 1", &arg2, MENU_ITEM_STR, NULL);

    LCD_CLRSCR;
    ui2 = LoginUserId = (rand() % EPS_MAX_USERS)+1;
    ui3 = MenuMode = (rand() & 1) ? MENU_MSUPER : MENU_MNORMAL;
    menuSetPasswd(MENU_MRESET);
    assert(ui3 == MenuMode);
    assert(0 == strncmp("Passwd Updated  ", lcd_buf[0], LCD_MAX_COL));

    /* sometimes corrupt the password */
    uint8_t corrupted = rand() % 2;
    //printf("before:'%s'\n", inp);
    if (corrupted) {
      ui4 = rand()%passwd_size;
      inp[ui4]++;
      if (inp[ui4] > '~') inp[ui4] -= 2;
    }
    //printf("after:'%s'\n", inp);

    INIT_TEST_KEYS(inp);
    KBD_RESET_KEY;
    arg1.value.str.sptr = bufSS;
    arg1.value.str.len = passwd_size;
    menuGetOpt("Prompt 1", &arg1, MENU_ITEM_STR, NULL);
    menuSetPasswd(ui3|MENU_MVALIDATE);

    if (corrupted) {
      assert(0 == strncmp("Passwd Wrong!!  ", lcd_buf[0], LCD_MAX_COL));
    } else {
      assert(0 == strncmp("Passwd Updated  ", lcd_buf[0], LCD_MAX_COL));
    }
    //printf("%s\n", lcd_buf[0]);
    assert(MenuMode == ui3);
    assert(LoginUserId == ui2);
  }

  /* menuSetUserPasswd */
  for (loop=0; loop<0; loop++) {
    RESET_TEST_KEYS;

    uint16_t passwd_size = ( rand() % (LCD_MAX_COL-1) ) + 1;
    for (ui1=0; ui1<passwd_size; ui1++) {
      if (0 == (rand() % 3))
	inp[ui1] = 'A' + (rand()%26);
      else
	inp[ui1] = 'a' + (rand()%26);
    }
    inp[ui1] = 0;
    INIT_TEST_KEYS(inp);
    inp[passwd_size] = 0;
    KBD_RESET_KEY;
    arg2.value.str.sptr = bufSS+LCD_MAX_COL+2;
    arg2.value.str.len = passwd_size;
    menuGetOpt("Prompt 1", &arg2, MENU_ITEM_STR, NULL);
    //    printf("passwd:'%s'", inp);
    //    printf("test_key_idx:%d test_key_arr_idx:%d\n", test_key_idx, test_key_arr_idx);

    for (ui1=0; ui1<EPS_MAX_UNAME; ui1++) {
      if (0 == (rand() % 3))
	inp2[ui1] = 'A' + (rand()%26);
      else
	inp2[ui1] = 'a' + (rand()%26);
    }
    if (0 == (rand()%5))
      inp2[0] = ' ';
    inp2[EPS_MAX_UNAME] = 0;
    INIT_TEST_KEYS(inp2);
    KBD_RESET_KEY;
    arg1.value.str.sptr = bufSS;
    arg1.value.str.len = EPS_MAX_UNAME;
    menuGetOpt("Prompt 2", &arg1, MENU_ITEM_STR, NULL);
    //    printf("test_key_idx:%d test_key_arr_idx:%d\n", test_key_idx, test_key_arr_idx);

    LCD_CLRSCR;
    MenuMode = MENU_MSUPER;
    ui2 = rand() % (EPS_MAX_USERS+1);
    inp3[ui2] = 0;
    for (; ui2; ui2--) {
      inp3[ui2] = ASCII_RIGHT;
    }
    INIT_TEST_KEYS(inp3);
    inp4[0] = 0;
    INIT_TEST_KEYS(inp4);
    menuSetUserPasswd(MENU_MSUPER);
    if (' ' == inp2[0]) {
      assert(0 == strncmp("Invalid User    ", lcd_buf[0], LCD_MAX_COL));
    } else if (ASCII_RIGHT == inp3[0]) {
      assert(0 == strncmp("Aborting!       ", lcd_buf[0], LCD_MAX_COL));
    } else {
      assert(MENU_MSUPER == MenuMode);
      assert(0 == strncmp("Passwd Updated  ", lcd_buf[0], LCD_MAX_COL));
      assert(MenuMode == MENU_MSUPER);
    }
    //    printf("test_key_idx:%d test_key_arr_idx:%d\n", test_key_idx, test_key_arr_idx);

    /* check login for only valid user */
    if ((' ' == inp2[0]) || (ASCII_RIGHT == inp[0]))
      continue;

    ui3 = rand() & 1; /* corrupt user name */
    ui4 = rand() & 1; /* corrupt passwd */
    RESET_TEST_KEYS;
    MenuMode = MENU_MRESET;
    INIT_TEST_KEYS(inp);
    KBD_RESET_KEY;
    menuGetOpt("Prompt 1", &arg2, MENU_ITEM_STR, NULL);
    arg2.value.str.sptr[0] ^= ui4;
    INIT_TEST_KEYS(inp2);
    KBD_RESET_KEY;
    menuGetOpt("Prompt 2", &arg1, MENU_ITEM_STR, NULL);
    arg1.value.str.sptr[0] ^= ui3;
    //    printf("inp2:'%s' arg1:'%s'\n", inp2, arg1.value.str.sptr);
    menuUserLogin(MENU_MRESET);
    if (ui3) {
      assert(0 == strncmp("No user         ", lcd_buf[0], LCD_MAX_COL));
      assert(MENU_MRESET == MenuMode);
      assert(0 == LoginUserId);
    } else if (ui4) {
      assert(0 == strncmp("Wrong Passwd    ", lcd_buf[0], LCD_MAX_COL));
      assert(MENU_MRESET == MenuMode);
      assert(0 == LoginUserId);
    } else {
      assert(((0 == ui2) ? MENU_MSUPER : MENU_MNORMAL) == MenuMode);
      assert(ui2+1 == LoginUserId);
      //printf("ui2:%d LoginUserId:%d maxusers:%d\n", ui2, LoginUserId, EPS_MAX_USERS);
      //printf("lcd:%s\n", lcd_buf[0]);
    }

    /* check logout */
    if (0 != LoginUserId) {
      //    ui1 = (rand() % (TEST_KEY_ARR_SIZE-2)) + 1;
      ui1 = (rand() % (5)) + 1;
      for (ui2=0; ui2<ui1; ui2++)
	inp[ui2] = (rand()&1) ? ASCII_LEFT : ASCII_RIGHT;
      inp[ui1] = 0;
      INIT_TEST_KEYS(inp);
      menuUserLogout(MENU_MNORMAL);
      if (ui1&1) { /* logout should be unsuccessful */
	assert(0 != LoginUserId);
	inp[ui1] = (rand()&1) ? ASCII_LEFT : ASCII_RIGHT;
	inp[ui1+1] = 0;
	INIT_TEST_KEYS(inp);
	menuUserLogout(MENU_MNORMAL);
      }
      assert(0 == LoginUserId);
      assert(MENU_MRESET == MenuMode);
    }
  }

  return 0;
}
