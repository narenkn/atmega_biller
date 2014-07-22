#include <stdint.h>
#include <stdlib.h>
#include <avr/pgmspace.h>

#include "menu.h"

#define ROW_JOIN
#define COL_JOIN
#define MENU_HIER(A)
#define MENU_MODE(A)
#define MENU_NAME(A) A
#define MENU_FUNC(A)
#define ARG1(A, B)
#define ARG2(A, B)
const uint8_t *menu_names PROGMEM = MENU_ITEMS;
#undef  ARG2
#undef  ARG1
#undef  MENU_FUNC
#undef  MENU_NAME
#undef  MENU_MODE
#undef  MENU_HIER
#undef  ROW_JOIN
#undef  COL_JOIN

typedef void (*menu_func_t)(uint8_t mode);

#define ROW_JOIN ,
#define COL_JOIN
#define MENU_HIER(A)
#define MENU_MODE(A)
#define MENU_NAME(A)
#define MENU_FUNC(A)
#define ARG1(A, B) B,
#define ARG2(A, B) B
const uint8_t menu_args[] PROGMEM = {
  MENU_ITEMS
};
#undef  ARG2
#undef  ARG1
#undef  MENU_FUNC
#undef  MENU_NAME
#undef  MENU_MODE
#undef  MENU_HIER
#undef  ROW_JOIN
#undef  COL_JOIN

#define ROW_JOIN ,
#define COL_JOIN
#define MENU_HIER(A)
#define MENU_MODE(A)
#define MENU_NAME(A)
#define MENU_FUNC(A)
#define ARG1(A, B) A,
#define ARG2(A, B) A
const uint8_t menu_prompts[] PROGMEM = {
  MENU_ITEMS
};
#undef  ARG2
#undef  ARG1
#undef  MENU_FUNC
#undef  MENU_NAME
#undef  MENU_MODE
#undef  MENU_HIER
#undef  ROW_JOIN
#undef  COL_JOIN

#define ROW_JOIN ,
#define COL_JOIN
#define MENU_HIER(A)
#define MENU_MODE(A) A
#define MENU_NAME(A)
#define MENU_FUNC(A)
#define ARG1(A, B)
#define ARG2(A, B)
const uint8_t menu_mode[] PROGMEM = {
  MENU_ITEMS
};
#undef  ARG2
#undef  ARG1
#undef  MENU_FUNC
#undef  MENU_NAME
#undef  MENU_MODE
#undef  MENU_HIER
#undef  ROW_JOIN
#undef  COL_JOIN

#define ROW_JOIN +
#define COL_JOIN
#define MENU_HIER(A)
#define MENU_MODE(A)
#define MENU_NAME(A)
#define MENU_FUNC(A) 1
#define ARG1(A, B)
#define ARG2(A, B)
const uint8_t MENU_MAX PROGMEM = MENU_ITEMS;
#undef  ARG2
#undef  ARG1
#undef  MENU_FUNC
#undef  MENU_NAME
#undef  MENU_MODE
#undef  MENU_HIER
#undef  ROW_JOIN
#undef  COL_JOIN

static uint8_t    menu_prompt_str[] PROGMEM = MENU_PROMPTS;
static menu_arg_t arg1, arg2;
uint8_t bufSS[BUFSS_SIZE];
uint8_t    menu_error;

#define MENU_STR1_IDX_PRICE 0
#define MENU_STR1_IDX_DISCO 1
#define MENU_STR1_IDX_S_TAX 2
#define MENU_STR1_IDX_YesNo 3
#define MENU_STR1_IDX_VAT   5
#define MENU_STR1_IDX_CONFI 6
#define MENU_STR1_IDX_ITEM  7
#define MENU_STR1_IDX_MANY  8
#define MENU_STR1_IDX_FINAL 9
#define MENU_STR1_IDX_PRINT 10
#define MENU_STR1_IDX_SAVE  11
#define MENU_STR1_IDX_DADAT 12
#define MENU_STR1_IDX_NAME  13
#define MENU_STR1_IDX_REPLA 14
#define MENU_STR1_IDX_DAY   15
#define MENU_STR1_IDX_MONTH 16
#define MENU_STR1_IDX_YEAR  17
#define MENU_STR1_IDX_NUM_ITEMS 18
uint8_t menu_str1[] PROGMEM =
  "Price   " /* 0 */
  "Discount" /* 1 */
  "Serv.Tax" /* 2 */
  "Yes     " /* 3 */
  "No      " /* 4 */
  "Vat     " /* 5 */
  "Confirm?" /* 6 */
  "Items # " /* 7 */
  "Many #  " /* 8 */
  "Final?  " /* 9 */
  "Print?  " /*10 */
  "Save?   " /*11 */
  "Delete? " /*12 */
  "Name    " /*13 */
  "Replace?" /*14 */
  "Date    " /*15 */
  "Month   " /*16 */
  "Year    " /*17 */
  ;

/* */
static uint8_t MenuMode = MENU_MRESET;
static uint8_t LoginUserId = 0; /* 0 is invalid */

/* Helper routine to obtain input from user */
void
menuGetOpt(uint8_t *prompt, menu_arg_t *arg, uint8_t opt)
{
  uint8_t col_id;
  uint8_t item_type = (opt & MENU_ITEM_TYPE_MASK);
  uint8_t *lbp = (uint8_t *) lcd_buf[LCD_MAX_ROW-1], *lp;
  uint32_t val = 0;
  uint8_t ui1;

  if (MENU_ITEM_NONE == opt) return;

  /* */
  if (MENU_ITEM_CONFIRM == opt) {
    ui1 = menuGetChoice(menu_str1+(MENU_STR1_IDX_CONFI*MENU_PROMPT_LEN), menu_str1+(MENU_STR1_IDX_YesNo*MENU_PROMPT_LEN), MENU_PROMPT_LEN, 2);
    arg->value.integer.i8  = ui1;
    return;
  }

  /* init */
  LCD_CLRSCR;
  lcd_buf_prop = (opt & MENU_ITEM_PASSWD) ? LCD_PROP_NOECHO_L2 : 0;

  /* Ask a question */
  LCD_POS(0, 0);
  LCD_WR_N(prompt, MENU_PROMPT_LEN);
  LCD_WR(" ?");
  LCD_refresh();

  /* Set the prompt */
  col_id = 0;
  lp = &(lcd_buf[LCD_MAX_ROW-1][0]);

  /* Get a string */
  do {
    /* FIXME: Need to added cursor functionality */
    KBD_GETCH;

    /* Don't overflow buffer */
    if (col_id > 15) col_id = 15;

    switch (KbdData) {
    case ASCII_BACKSPACE:
    case ASCII_LEFT:
    case ASCII_UP:
      if (col_id == 0) {
	break;
      }
      lp[0] = ' ';
      col_id--; lp--;
      break;
    case ASCII_LF:
    case ASCII_ENTER:
      col_id++;
      break;
    case ASCII_RIGHT:
    case ASCII_DOWN:
    case ASCII_PRNSCRN:
    case ASCII_NUMLK:
    case ASCII_UNDEF:
    case ASCII_DEFINED:
    case ASCII_F2:
      break;
    default:
      lp[0] = KbdData;
      lp++; col_id++;
    }
    if (ASCII_ENTER != KbdData) {
      KBD_RESET_KEY;
    }
    LCD_refresh();
  } while (KbdData != ASCII_ENTER);

  menu_error = 1;
  if ((MENU_ITEM_ID == item_type) || (MENU_ITEM_FLOAT == item_type)) {
    for (ui1=0; ui1<col_id; ui1++) {
      if ((lbp[0] >= '0') && (lbp[0] <= '9')) {
	val *= 10;
	val += lbp[0] - '0';
	menu_error = 0;
      } else {
	break;
      }
      lbp++;
    }
    arg->value.integer.i8  = val>>16;
    arg->value.integer.i16 = val;
  } else if ((MENU_ITEM_DATE == item_type) || (MENU_ITEM_MONTH == item_type)) {
    /* format DDMMYYYY || format MMYYYY */
    menu_error = 0;
    for (ui1=0; ui1<(item_type+2); ui1++) { /* cleaverly spaced item_type apart */
      if ((lbp[ui1] < '0') || (lbp[ui1] > '9'))
	menu_error++;
    }
    if (0 == menu_error) {
      /* Date */
      if (MENU_ITEM_DATE == item_type) {
	for (ui1=0; ui1<2; ui1++) {
	  val *= 10;
	  val += lbp[0] - '0';
	  lbp++;
	}
	if ((0 == val) || (val > 31)) menu_error++;
	arg->value.date.day = val;
      }
      /* Month */
      val = 0;
      for (ui1=0; ui1<2; ui1++) {
	val *= 10;
	val += lbp[0] - '0';
	lbp++;
      }
      if ((0 == val) || (val > 12)) menu_error++;
      arg->value.date.month = val;
      /* Year */
      val = 0;
      for (ui1=0; ui1<4; ui1++) {
	val *= 10;
	val += lbp[0] - '0';
	lbp++;
      }
      if ((val < 2000) || (val > 2100)) menu_error++;
      arg->value.date.year = val-2000;
    }
  } else if (MENU_ITEM_TIME == item_type) {
    /* format HHMM */
    uint8_t ui1;
    menu_error = 0;
    for (ui1=0; ui1<4; ui1++) {
      if ((lbp[ui1] < '0') || (lbp[ui1] > '9'))
	menu_error++;
    }
    if (0 == menu_error) {
      /* Hour */
      for (ui1=0; ui1<2; ui1++) {
	val *= 10;
	val += lbp[0] - '0';
	lbp++;
      }
      if (val>23) menu_error++;
      arg->value.time.hour = val;
      /* Mins */
      val = 0;
      for (ui1=0; ui1<2; ui1++) {
	val *= 10;
	val += lbp[0] - '0';
	lbp++;
      }
      if (val > 59) menu_error++;
      arg->value.time.min = val;
    }
  } else if (MENU_ITEM_STR == item_type) {
    menu_error = 0;
    strncpy(arg->value.sptr, lcd_buf[LCD_MAX_ROW-1], LCD_MAX_COL);
  } else assert(0);

  /* */
  if (0 == menu_error) {
    arg->valid = item_type;
  } else if (opt & MENU_ITEM_OPTIONAL) {
  } else if ((opt & MENU_ITEM_DONTCARE_ON_PREV) && (arg == &arg2)) {
  } else
    assert(0);

  lcd_buf_prop = 0;
}

/* Helper routine to obtain choice from user */
uint8_t
menuGetChoice(uint8_t *quest, uint8_t *opt_arr, uint8_t choice_len, uint8_t max_idx)
{
  uint8_t ret = 0;

  do {
    assert(ret < max_idx);

    LCD_WR_LINE_N(1, 0, quest, choice_len);
    LCD_WR(": ");
    LCD_WR_N((opt_arr+(ret*choice_len)), choice_len);

    KBD_GETCH;

    if ((ASCII_RIGHT == KbdData) || (ASCII_DOWN == KbdData)) {
      ret = ((max_idx-1)==ret) ? 0 : ret+1;
    } else if ((ASCII_LEFT == KbdData) || (ASCII_UP == KbdData)) {
      ret = (0==ret) ? max_idx-1 : ret-1;
    } else if (ASCII_ENTER == KbdData) {
      KBD_RESET_KEY;
      return ret;
    }
    KBD_RESET_KEY;
  } while (1);
  assert (0);
}

// Not unit tested
/* Load in the factory settings */
/* Todo:
   1. admin passwd needs to be reset
   2. users : make default user names
 */
void
menuFactorySettings(uint8_t mode)
{
  assert(MENU_MSUPER == MenuMode);
}

/* FIXME: Needs to be removed */
void
menu_unimplemented(uint32_t idx)
{
  LCD_WR_LINE_N(LCD_MAX_ROW-1, 0, "unimplemented ", 14);
  LCD_PUT_UINT8X(idx);
  LCD_refresh();
}

/* Set others passwd : only admin can do this
   arg1 : user name
   arg2 : passwd
 */
void
menuSetUserPasswd(uint8_t mode)
{
  uint8_t ui2, ui3;
  uint16_t ui1;
  assert(MENU_MSUPER == MenuMode);

  /* Choose the user to replace it with */
  assert(MENU_ITEM_STR == arg1.valid);
  assert(MENU_ITEM_STR == arg2.valid);
  for (ui2=0; ui2<EPS_MAX_UNAME; ui2++) {
    ui3 = arg1.value.sptr[ui2];
    /* check alnum? */
    if ((!isgraph(ui3)) && (ui2 > 0))
      break;
    else if (!isalnum(ui3)) {
      LCD_ALERT("Invalid User");
      return;
    }
  }
  for (
       ui2=0, ui1=offsetof(struct ep_store_layout, users)+EPS_MAX_UNAME;
       ui2<(EPS_MAX_USERS*EPS_MAX_UNAME); ui2++ ) {
    bufSS[(LCD_MAX_COL*3)+ui2] = eeprom_read_byte((uint8_t *)ui1);
  }
  ui2 = menuGetChoice("Replace at?", bufSS+(LCD_MAX_COL*3), EPS_MAX_UNAME, EPS_MAX_USERS-1) + 1;
  if (0 != menuGetChoice(menu_str1+(MENU_STR1_IDX_CONFI*MENU_PROMPT_LEN), menu_str1+(MENU_STR1_IDX_YesNo*MENU_PROMPT_LEN), MENU_PROMPT_LEN, 2)) {
    LCD_ALERT("Aborting!");
    return;
  }

  /* modify at will */
  ui3 = LoginUserId;
  LoginUserId = ui2;
  menuSetPasswd(mode & ~MENU_MVALIDATE);
  LoginUserId = ui3;
  for (ui3=0; ui3<EPS_MAX_UNAME; ui3++) {
    eeprom_update_byte((uint8_t *)(offsetof(struct ep_store_layout, users)+((ui2-1)*EPS_MAX_UNAME)+ui3), arg1.value.sptr[ui3]);
  }
}

/* Set my password, arg1 is old passwd, arg2 is new passwd */
void
menuSetPasswd(uint8_t mode)
{
  uint16_t crc_old = 0, crc_new = 0;
  uint8_t ui2, ui3, ui4;

  /* */
  assert(0 != LoginUserId);
  assert(MENU_MRESET != MenuMode);

  /* Compute CRC on old password, check */
  if (0 != (mode & (~MENU_MODEMASK) & MENU_MVALIDATE)) {
    assert(MENU_ITEM_STR == arg1.valid);
    for (ui4=0; ui4<LCD_MAX_COL; ui4++) {
      ui2 = arg1.value.sptr[ui4];
      /* check isprintable? */
      for (ui3=0; (isgraph(ui2)) && (ui3<(KCHAR_COLS*KCHAR_ROWS)); ui3++) {
	if (ui2 == keyChars[ui3])
	  break;
      }
      if (ui3 < (KCHAR_ROWS*KCHAR_COLS))
	crc_old = _crc16_update(crc_old, ui2);
    }

    if (eeprom_read_word((uint16_t *)offsetof(struct ep_store_layout, passwds[(LoginUserId-1)])) != crc_old) {
      LCD_ALERT("Passwd Wrong!!");
      return;
    }
  }

  /* update mine only */
  assert(MENU_ITEM_STR == arg2.valid);
  for (ui4=0; ui4<LCD_MAX_COL; ui4++) {
    ui2 = arg2.value.sptr[ui4];
    /* check isprintable? */
    for (ui3=0; (isgraph(ui2)) && (ui3<(KCHAR_COLS*KCHAR_ROWS)); ui3++) {
      if (ui2 == keyChars[ui3])
	break;
    }
    if (ui3 < (KCHAR_ROWS*KCHAR_COLS))
      crc_new = _crc16_update(crc_new, ui2);
  }

  eeprom_update_word((uint16_t *)(offsetof(struct ep_store_layout, passwds)+((LoginUserId-1)*sizeof(uint16_t))), crc_new);
  LCD_ALERT("Passwd Updated");
}

/* Logout an user */
void
menuUserLogout(uint8_t mode)
{
  LoginUserId = 0;
  MenuMode = MENU_MRESET;
}

/* Login an user */
void
menuUserLogin(uint8_t mode)
{
  uint16_t crc = 0;
  uint8_t ui2, ui3, ui4, ui5;

  assert(MENU_ITEM_STR == arg1.valid);
  assert(MENU_ITEM_STR == arg2.valid);

  for ( ui2=0; ui2<(EPS_MAX_USERS+1); ui2++ ) {
    for ( ui3=0; ui3<EPS_MAX_UNAME; ui3++ ) {
      ui4 = eeprom_read_byte((uint8_t *)(offsetof(struct ep_store_layout, users)+(ui2*EPS_MAX_UNAME)+ui3));
      if (ui4 != arg1.value.sptr[ui3]) {
	break;
      } else if ((0 == ui3) && (0 == ui2) && (!isgraph(ui4)))
	break;
    }
    if (EPS_MAX_UNAME == ui3)
      goto menuUserLogin_found;
  }
  LCD_ALERT("No user");
  return;

 menuUserLogin_found:
  assert(ui2 < (EPS_MAX_USERS+1));
  for (ui4=0; ui4<LCD_MAX_COL; ui4++) {
    ui3 = arg2.value.sptr[ui4];
    /* check isprintable? */
    for (ui5=0; isgraph(ui3) && (ui5<(KCHAR_COLS*KCHAR_ROWS)); ui5++) {
      if (ui3 == keyChars[ui5])
	break;
    }
    if (ui5 < (KCHAR_ROWS*KCHAR_COLS))
      crc = _crc16_update(crc, ui3);
  }

  if (eeprom_read_word((uint16_t *)(offsetof(struct ep_store_layout, passwds) + (ui2*sizeof(uint16_t)))) != crc) {
    LCD_ALERT("Wrong Passwd");
    return;
  }

  /* */
  MenuMode = (0 == ui2) ? MENU_MSUPER : MENU_MNORMAL;
  LoginUserId = ui2+1;
}

void
menuInit(void)
{
  MenuMode = MENU_MRESET;

  if (ITEM_SIZEOF > BUFSS_SIZE)
    ERROR("item should be smaller");
  
  assert(1 == sizeof(uint8_t));
}

// Not unit tested
void
menuBilling(uint8_t mode)
{
//  uint8_t ui2, ui3, ui4, ui5;
//  uint16_t item_addr;
//
//  billing *bi = (void *) bufSS;
//  for (ui2=0; ui2<sizeof(billing); ui2++) {
//    bufSS[ui2] = 0;
//  }
//
//  arg2.valid = MENU_ITEM_NONE;
//  menu_getopt(menu_str1+(MENU_STR1_IDX_ITEM*MENU_PROMPT_LEN), &arg2, MENU_ITEM_ID);
//
//get_more_items:
//  for (ui5=0; ui5 < MAX_ITEMS_IN_BILL; ui5++) {
//    item_addr = flash_item_find(arg2.value.integer.i16);
//    if ((MENU_ITEM_NONE == arg2.valid) || (FLASH_ADDR_INVALID == item_addr))
//      break;
//
//    /* take the item record */
//    ui3 = (uint8_t)&(((billing *)0)->temp);
//    for (ui4=0; ui4<ITEM_SIZEOF; ui3++, ui4++) {
//      bufSS[ui3] = FlashReadByte(item_addr+ui4);
//    }
//    bi->bi[ui2].vat_sel = bi->temp.vat_sel;
//    bi->bi[ui2].has_serv_tax = bi->temp.has_serv_tax;
//
//    /* The item could be invalid as well */
//    if ((0 == bi->temp.cost) && (0 == bi->temp.discount) && (0 == bi->temp.id))
//      break;
//
//    /* */
//    ui2 = bi->info.n_items;
//    bi->items[ui2].item_id = arg2.value.integer.i16;
//
//    do {
//      /* # items */
//      arg2.valid = MENU_ITEM_NONE;
//      menu_getopt(menu_str1+(MENU_STR1_IDX_MANY*MENU_PROMPT_LEN), &arg2, MENU_ITEM_ID);
//    } while (MENU_ITEM_NONE == arg2.valid);
//
//    /* */
//    bi->items[ui2].num_sale  = (uint8_t) arg2.value.integer.i16;
//  }
//  bi->info.n_items = ui5;
//
//  /* */
//  if (0 != menu_getchoice(menu_str1+(MENU_STR1_IDX_CONFI*MENU_PROMPT_LEN), menu_str1+(MENU_STR1_IDX_YesNo*MENU_PROMPT_LEN), MENU_PROMPT_LEN, 2))
//    goto get_more_items;
//
//  /* */
//  if (0 != menu_getchoice(menu_str1+(MENU_STR1_IDX_SAVE*MENU_PROMPT_LEN), menu_str1+(MENU_STR1_IDX_YesNo*MENU_PROMPT_LEN), MENU_PROMPT_LEN, 2))
//    return;
//
//  /* time */
//  {
//    uint8_t buf[3];
//    timerDateGet(buf);
//    bi->info.date_dd = ((buf[0]>>4)&0x3)*10+(buf[0]&0xF);
//    bi->info.date_mm = ((buf[0]>>4)&0x1)*10+(buf[0]&0xF);
//
//    timerTimeGet(buf);
//    bi->info.time_hh = ((buf[0]>>4)&0x3)*10+(buf[0]&0xF);
//    bi->info.time_mm = ((buf[0]>>4)&0x7)*10+(buf[0]&0xF);
//  }
//
//  /* billing ... */
//  EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.print_it ), (uint8_t *)&ui2, sizeof(uint8_t));
//  if (0 == ui2) { /* enabled */
//    if (0 == menu_getchoice(menu_str1+(MENU_STR1_IDX_PRINT*MENU_PROMPT_LEN), menu_str1+(MENU_STR1_IDX_YesNo*MENU_PROMPT_LEN), MENU_PROMPT_LEN, 2)) {
//      menu_PrnFullBill(bi);
//    }
//  }
//
//  menu_sale_add(bufSS);
  menu_unimplemented(__LINE__);
}

// Not unit tested
void
menuShowBill(uint8_t mode)
{
//  uint8_t ui2, ui3, ui4;
//  uint16_t sale_info, ui1;
//  billing *bp;
//
//  if (MENU_PR_NONE == arg1.valid)
//    return;
//
//  /* by default, select the first bill */
//  if (MENU_PR_NONE == arg2.valid)
//    arg2.value.integer.i16 = 1;
//
//  sale_info = menu_sale_find(&(arg1.value.date.date), arg2.value.integer.i16);
//  if (FLASH_ADDR_INVALID == sale_info) {
//    ERROR("Not Found");
//    return;
//  }
//
//  /* Keep reacting to user inputs */
//  bp = (billing *)bufSS;
//  do {
//    /* retrieve & display bill */
//    KBD_GETCH;
//
//    if (ASCII_PRNSCRN == KbdData) {
//      mode &= MENU_MODEMASK;
//      mode |= MENU_MPRINT;
//      break;
//    } else if ((ASCII_LEFT == KbdData) || (ASCII_UP == KbdData)) {
//      arg2.value.integer.i16 --;
//      sale_info = menu_sale_find(&(arg1.value.date.date), arg2.value.integer.i16);
//      if (FLASH_ADDR_INVALID == sale_info) {
//	arg2.value.integer.i16 ++;
//      }
//    } else if ((ASCII_RIGHT == KbdData) || (ASCII_DOWN == KbdData)) {
//      arg2.value.integer.i16 ++;
//      sale_info = menu_sale_find(&(arg1.value.date.date), arg2.value.integer.i16);
//      if (FLASH_ADDR_INVALID == sale_info) {
//	arg2.value.integer.i16 --;
//      }
//    } else if ( (ASCII_ENTER == KbdData) || (ASCII_UNDEF == KbdData) ) {
//      break;
//    }
//
//    /* retrieve this bill data, exit if invalid condition */
//    for (ui4=0; ui4<SALE_INFO_SIZEOF; ui4++) {
//      bufSS[ui4] = FlashReadByte((uint16_t)(bufSS+ui4));
//    }
//    for (ui3=0; ui3<(SALE_SIZEOF*(bp->info.n_items)); ui3++, ui4++) {
//      bufSS[ui4] = FlashReadByte((uint16_t)(bufSS+ui4));
//    }
//    ui4 = (uint8_t)&(((billing *)0)->temp);
//    for (ui3=0; ui3<bp->info.n_items; ui3++, ui4++) {
//      ui1 = flash_item_find(bp->items[ui3].item_id);
//      for (ui2=0; ui2<ITEM_SIZEOF; ui4++, ui2++) {
//	bufSS[ui4] = FlashReadByte(ui1+ui2);
//      }
//      bp->bi[ui3].vat_sel = bp->temp.vat_sel;
//      bp->bi[ui3].has_serv_tax = bp->temp.has_serv_tax;
//    }
//
//#if 0
//    /* FIXME: The item could have been deleted as well */
//    if ((0 == bi->temp.cost) && (0 == bi->temp.discount) && (0 == bi->temp.id))
//      break;
//#endif
//
//    /* FIXME: display */
//
//    KBD_RESET_KEY;
//  } while (1);
//  KBD_RESET_KEY;
//
//  if ((mode&(~MENU_MODEMASK)) == MENU_MPRINT) {
//    menu_PrnFullBill(bp);
//  } else if ((mode&(~MENU_MODEMASK)) == MENU_MDELETE) {
//    ui3 = FlashReadByte(sale_info+offsetof(struct _sale_info, property));
//    if ( FLASH_RESET_DATA_VALUE == ui3 ) {
//      FlashWriteByte(sale_info+offsetof(struct _sale_info, property), SALE_INFO_DELETED);
//    }
//  }
  menu_unimplemented(__LINE__);
}

// Not unit tested
/*
  Menu item : Name,     cost, discount, serv-tax, vat-id, id
              15bytes, 13bit,    13bit,     1bit,   2bit, 9bit
 */
void
menuAddItem(uint8_t mode)
{
//  uint8_t choice[MENU_PROMPT_LEN*4], ui2, ui3, ui4, vat;
//  uint16_t ui1, ui5;
//  menu_arg_t cost;
//  item *it;
//  uint8_t s_tax;
//
//  /* Check if space is available */
//  EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.item_count), (uint8_t *)&ui5, sizeof(uint16_t));
//  if (ui5 >= ITEM_MAX) {
//    ERROR("Items Exceeded");
//    return;
//  }
//
//  /* save name */
//  it = (void *) bufSS;
//  for (ui4=0; ui4<ITEM_SIZEOF; ui4++) {
//    bufSS[ui4] = 0;
//  }
//  for (ui4=0; ui4<15; ui4++) {
//    it->name[ui4] = lcd_buf[LCD_MAX_ROW-1][ui4];
//  }
//
//  /* Cost, discount */
//  cost.valid = MENU_ITEM_NONE;
//  menu_getopt(menu_str1+(MENU_STR1_IDX_PRICE*MENU_PROMPT_LEN), &cost, MENU_ITEM_FLOAT);
//  arg2.valid = MENU_ITEM_NONE;
//  menu_getopt(menu_str1+(MENU_STR1_IDX_DISCO*MENU_PROMPT_LEN), &arg2, MENU_ITEM_FLOAT);
//
//  /* vat */
//  for (ui2=0; ui2<4; ui2++) {
//    EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.vat[ui2]), (uint8_t *)&ui1, sizeof(uint16_t));
//    for (ui3=0; ui3<4; ui3++) {
//      (choice+(ui2*MENU_PROMPT_LEN)+MENU_PROMPT_LEN-ui3)[0] = '0' + ui1%10;
//      ui1 /= 10;
//    }
//  }
//  vat = menu_getchoice(menu_str1+(MENU_STR1_IDX_VAT*MENU_PROMPT_LEN), choice, MENU_PROMPT_LEN, 4);
//
//  /* serv-tax */
//  s_tax = menu_getchoice(menu_str1+(MENU_STR1_IDX_S_TAX*MENU_PROMPT_LEN), menu_str1+(MENU_STR1_IDX_YesNo*MENU_PROMPT_LEN), MENU_PROMPT_LEN, 2) ? 0 : 1;
//
//  /* Confirm */
//  if ( (MENU_ITEM_NONE == arg1.valid) || (MENU_ITEM_NONE == arg2.valid) ||
//       (MENU_ITEM_NONE == cost.valid) ) {
//    ERROR("Invalid Option");
//    return;
//  }
//  if (0 != menu_getchoice(menu_str1+(MENU_STR1_IDX_CONFI*MENU_PROMPT_LEN), menu_str1+(MENU_STR1_IDX_YesNo*MENU_PROMPT_LEN), MENU_PROMPT_LEN, 2))
//    return;
//
//  /* Pack the value */
//  it->vat_sel = vat;
//  it->has_serv_tax = s_tax;
//  it->id = ui1;
//  it->cost = cost.value.integer.i16;
//  it->discount = arg2.value.integer.i16;
//
//  /* init */
//  EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.item_last_modified), (uint8_t *)&ui5, sizeof(uint16_t));
//
//  /* Find empty space */
//  for (ui1 = menu_item_find(ui5);;) {
//    for (ui2=0; ui2<ITEM_SIZEOF; ui2++) {
//      ITEM_READ_BYTE(ui1+ui2, ui3);
//      if (0 != ui3)
//	break;
//    }
//    if (ui2==ITEM_SIZEOF) break; /* Found empty space */
//    /* Next addr to check */
//    ui5++;
//    ui1 += ITEM_SIZEOF;
//    if (ui5 >= ITEM_MAX) {
//      ui5 = 0;
//      ui1 = ITEM_DATA_START;
//    }
//  }
//
//  /* */
//  for (ui2=0; ui2<ITEM_SIZEOF; ui2++, ui1++) {
//    ITEM_READ_BYTE(ui1, ui3);
//    if (ui3 != byte_arr[ui2])
//      ITEM_WRITE_BYTE(ui1, byte_arr[ui2]);
//  }
//
//  /* Finally update pointers accordingly */
//  EEPROM_STORE_WRITE_NoSig((uint16_t)&(EEPROM_DATA.item_last_modified), (uint8_t *)&ui5, sizeof(uint16_t));
//  EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.item_count), (uint8_t *)&ui1, sizeof(uint16_t));
//  ui1++;
//  EEPROM_STORE_WRITE((uint16_t)&(EEPROM_DATA.item_count), (uint8_t *)&ui1, sizeof(uint16_t));
  menu_unimplemented(__LINE__);
}

// Not unit tested
void
menuDelItem(uint8_t mode)
{
//  uint16_t ui1, addr, id;
//  uint8_t ui2, ui3;
//  uint8_t *block;
//
//  /* check validity of argument */
//  if ((MENU_ITEM_NONE == arg1.valid) || (MENU_ITEM_ID != arg1.valid)) {
//    ERROR("Invalid Option");
//    return;
//  }
//  if (arg1.value.integer.i16 >= ITEM_MAX) {
//    ERROR("Too Large Opt");
//    return;
//  }
//  id = arg1.value.integer.i16;
//
//  /* input is id, compute addr & do nothing if already clear */
//  ui1 = menu_item_find(id);
//  for (ui2=0; ui2<ITEM_SIZEOF; ui2++) {
//    ITEM_READ_BYTE(ui1+ui2, ui3);
//    if (FLASH_RESET_DATA_VALUE != ui3)
//      break;
//  }
//  if (ui2>=ITEM_SIZEOF) {
//    return;
//  }
//
//  /* Save the item to EEPROM */
//  EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.item_mod_his_ptr), (uint8_t *)&ui3, sizeof(uint8_t));
//  ui1 = (uint16_t)&(EEPROM_DATA.item_mod_his[ui3]);
//  ui3++;
//  EEPROM_STORE_WRITE_NoSig((uint16_t)&(EEPROM_DATA.item_mod_his_ptr), (uint8_t *)&ui3, sizeof(uint8_t));
//  for (ui2=0; ui2<ITEM_SIZEOF; ui1++, ui2++) {
//    ITEM_READ_BYTE(addr+ui2, ui3);
//    EEPROM_STORE_WRITE_NoSig(ui1, (uint8_t *)&ui3, sizeof(uint8_t));
//  }
//
//  /* Modify lookups accordingly */
//  {
//    uint16_t sale_start, sale_end;
//    uint8_t  n_bytes;
//    billing *bi = (void *)bufSS;
//    EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.sale_start), (uint8_t *)&sale_start, sizeof(uint16_t));
//    EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.sale_end), (uint8_t *)&sale_end, sizeof(uint16_t));
//    for (; sale_start!=sale_end;) {
//      for (ui2=0; ui2<SALE_INFO_SIZEOF; ui2++)
//	ITEM_READ_BYTE(sale_start+ui2, bufSS[ui2]);
//      n_bytes = SALE_INFO_SIZEOF + (bi->info.n_items * (uint16_t)SALE_SIZEOF);
//      if (0 == (bi->info.property & SALE_INFO_DELETED)) {
//	for (ui2=SALE_INFO_SIZEOF; ui2<n_bytes; ui2++)
//	  ITEM_READ_BYTE(sale_start+ui2, bufSS[ui2]);
//	for (ui2=0; ui2<bi->info.n_items; ui2++) {
//	  if (bi->items[ui2].item_id == id) {
//	    /* it has modified items */
//	    ITEM_WRITE_BYTE(sale_start+(uint16_t)(&(SALE_INFO.property)), SALE_INFO_MODIFIED);
//	  }
//	}
//      }
//      sale_start += n_bytes;
//      if (sale_start >= ITEM_DATA_END) {
//	sale_start = ITEM_DATA_START + (sale_start - ITEM_DATA_END);
//      }
//    }
//  }
//
//#if ITEM_STORED_IN_FLASH
//  /* save start block */
//  block = (uint8_t *) (addr & ~((uint16_t)(FLASH_SECTOR_SIZE-1)));
//  for (ui1=0; ui1<FLASH_SECTOR_SIZE; ui1++) {
//    if ( ((block+ui1) < (uint8_t *)addr) || ((block+ui1) >= (uint8_t *)(addr+ITEM_SIZEOF)) )
//      bufSS[ui1] = block[ui1];
//    else
//      bufSS[ui1] = FLASH_RESET_DATA_VALUE;
//  }
//  FlashEraseSector((uint16_t)block);
//  for (ui1=0; ui1<FLASH_SECTOR_SIZE; ui1++) {
//    if (FLASH_RESET_DATA_VALUE != bufSS[ui1])
//      FlashWriteByte((uint16_t)(block+ui1), bufSS[ui1]);
//  }
//
//  /* end block */
//  ui1 = (uint16_t) block;
//  block = (uint8_t *) ((addr+ITEM_SIZEOF-1) & ~((uint16_t)(FLASH_SECTOR_SIZE-1)));
//  if ((uint8_t*)ui1 != block) {
//    for (ui1=0; ui1<FLASH_SECTOR_SIZE; ui1++) {
//      if ( (block+ui1) >= (uint8_t *)(addr+ITEM_SIZEOF) )
//	bufSS[ui1] = block[ui1];
//      else
//	bufSS[ui1] = FLASH_RESET_DATA_VALUE;
//    }
//    FlashEraseSector((uint16_t)block);
//    for (ui1=0; ui1<FLASH_SECTOR_SIZE; ui1++) {
//      if (FLASH_RESET_DATA_VALUE != bufSS[ui1])
//	FlashWriteByte((uint16_t)(block+ui1), bufSS[ui1]);
//    }
//  }
//#endif
//
//  /* upate # */
//  EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.item_count), (uint8_t *)&ui1, sizeof(uint16_t));
//  ui1--;
//  EEPROM_STORE_WRITE((uint16_t)&(EEPROM_DATA.item_count), (uint8_t *)&ui1, sizeof(uint16_t));
  menu_unimplemented(__LINE__);
}

// Not unit tested
void
menuModItem(uint8_t mode)
{
  menu_unimplemented(__LINE__);
}

// Not unit tested
void
menuBillReports(uint8_t mode)
{
//  uint16_t ui1, next_record;
//  uint8_t ui3, ui4, ui5;
//  billing *bp = (void *)bufSS;
//
//  uint16_t start_record = menu_sale_find((uint8_t *)&(arg1.value.date), 1);
//
//  /* Only process valid records... */
//  if (FLASH_ADDR_INVALID == start_record)
//    return;
//
//  /* */
//  menu_PrnHeader();
//  bp->ui1 = bp->ui3 = bp->ui5 = bp->ui7 = 0;
//  bp->ui2 = bp->ui4 = bp->ui6 = bp->flags = 0;
//  bp->ui9 = bp->ui11 = bp->ui13 = bp->ui15 = 0;
//
//  next_record = start_record;
//  while (1) {
//    /* Only process valid records... */
//    if (FLASH_ADDR_INVALID == start_record)
//      break;
//
//    /* retrieve this bill data, exit if invalid condition */
//    for (ui4=0; ui4<SALE_INFO_SIZEOF; ui4++) {
//      bufSS[ui4] = FlashReadByte(next_record+ui4);
//    }
//    ui3 = mode & ~MENU_MODEMASK;
//    if ( (ui3 >= MENU_MDAYITEM) && (ui3 <= MENU_MMONTAX) && (bp->info.date_mm != arg1.value.date.month) ) {
//      assert(start_record != next_record);
//      break;
//    }
//
//    if ( (ui3 >= MENU_MDAYITEM) && (ui3 <= MENU_MDAYTAX) && (bp->info.date_dd != arg1.value.date.date) ) {
//      assert(start_record != next_record);
//      break;
//    }
//    for (ui3=0; ui3<(ITEM_SIZEOF*(bp->info.n_items)); ui3++, ui4++) {
//      bufSS[ui4] = FlashReadByte(next_record+ui4);
//    }
//    /* populate pointers */
//    for (ui3=0; ui3<(bp->info.n_items); ui3++) {
//      ui4 = (uint8_t)&(((billing *)0)->temp);
//      ui1 = flash_item_find(bp->items[ui3].item_id);
//      for (ui5=0; ui5<ITEM_SIZEOF; ui4++, ui5++) {
//	bufSS[ui4] = FlashReadByte(ui1+ui5);
//      }
//      bp->bi[ui3].vat_sel = bp->temp.vat_sel;
//      bp->bi[ui3].has_serv_tax = bp->temp.has_serv_tax;
//    }
//
//    /* print after skipping deleted record */
//    if (0 == bp->info.property) {
//      ui3 = mode & ~MENU_MODEMASK;
//      if ((MENU_MDAYITEM == ui3) || (MENU_MMONITEM == ui3) || (MENU_MALLITEM == ui3))
//	menu_PrnItemBill(bp);
//      else if ((MENU_MDAYFULL == ui3) || (MENU_MMONFULL == ui3) || (MENU_MALLFULL == ui3))
//	menu_PrnFullBill(bp);
//      else if ((MENU_MDAYTAX == ui3) || (MENU_MMONTAX == ui3))
//	menu_PrnTaxReport(bp);
//    }
//
//    /* atleast one record, move past this */
//    assert(start_record != next_record);
//    next_record += ui4;
//  }
//
//  ui3 = mode & ~MENU_MODEMASK;
//  if ((MENU_MDAYITEM == ui3) || (MENU_MMONITEM == ui3) || (MENU_MALLITEM == ui3))
//    menu_PrnItemBillFooter();
//  else if ((MENU_MDAYFULL == ui3) || (MENU_MMONFULL == ui3) || (MENU_MALLFULL == ui3))
//    menu_PrnFooter();
//  else if ((MENU_MDAYTAX == ui3) || (MENU_MMONTAX == ui3))
//    menu_PrnTaxReportFooter(bp);
  menu_unimplemented(__LINE__);
}
//
// Not unit tested
//void
//menuPrnHeader(void)
//{
//  uint16_t ui1;
//  uint8_t ui2, ui3;
//
//  PRINTER_ONLINE;
//
//  /* Shop name */
//  ui1 = (uint16_t) &(EEPROM_DATA.shop_name);
//  for (ui2=0; ui2<SHOP_NAME_SZ; ui2++) {
//    EEPROM_STORE_READ(ui1, (uint8_t *)&ui3, sizeof(uint8_t));
//    PRINTER_PRINT(ui3);
//    ui1++;
//  }
//
//  /* Header */
//  ui1 = (uint16_t) &(EEPROM_DATA.prn_header);
//  for (ui2=0; ui2<HEADER_MAX_SZ; ui2++) {
//    EEPROM_STORE_READ(ui1, (uint8_t *)&ui3, sizeof(uint8_t));
//    PRINTER_PRINT(ui3);
//    ui1++;
//  }
//}
//
// Not unit tested
//void
//menuPrnFooter(void)
//{
//  uint16_t ui1;
//  uint8_t ui2, ui3;
//
//  ui1 = (uint16_t) &(EEPROM_DATA.prn_footer);
//  for (ui2=0; ui2<HEADER_MAX_SZ; ui2++) {
//    EEPROM_STORE_READ(ui1, (uint8_t *)&ui3, sizeof(uint8_t));
//    PRINTER_PRINT(ui3);
//    ui1++;
//  }
//}
//
// Not unit tested
//void
//menuPrnItemBill(billing *bp)
//{
//  menu_unimplemented(__LINE__);
//}
//
// Not unit tested
//void
//menuPrnItemBillFooter(void)
//{
//  menu_unimplemented(__LINE__);
//}
//
// Not unit tested
//void
//menuPrnFullBill(billing *bp)
//{
//  assert(0 == bp->info.property);
//
//  /* #, Date */
//  bp->ui1++;
//  if (bp->flags & BILLING_PRINT) {
//    uint8_t ymd[3];
//
//    printer_prn_str("Bill No:");
//    printer_prn_uint16(bp->ui1);
//    PRINTER_PRINT(' ');
//
//    ymd[0] = bp->info.date_dd;
//    ymd[1] = bp->info.date_mm;
//    //    ymd[2] = bp->info.date_yy;
//    printer_prn_date(ymd);
//  }
//
//  /* Item */
//  {
//    uint16_t tot_tax = 0, tot_bill = 0, serv_tax = 0, ui1, ui1_2;
//    uint8_t ui2, ui3, ui4, ui5;
//    ui3 = bp->info.n_items;
//    for (ui2=0; ui2<ui3; ui2++) {
//      printer_prn_uint16(bp->items[ui2].item_id);
//      if (bp->flags & BILLING_PRINT)
//	PRINTER_PRINT(' ');
//      ui1 = flash_item_find(bp->items[ui2].item_id);
//      for (ui4=0; ui4<ITEM_NAME_BYTEL; ui4++, ui1++) {
//	ui5 = FlashReadByte(ui1);
//	if (0 == ui5) break;
//	if (bp->flags & BILLING_PRINT)
//	  PRINTER_PRINT(ui5);
//      }
//
//      ui1 = flash_item_find(bp->items[ui2].item_id);
//      ui5 = (uint8_t)&(((billing *)0)->temp);
//      for (ui4=0; ui4<ITEM_SIZEOF; ui5++, ui4++) {
//	bufSS[ui5] = FlashReadByte(ui1+ui4);
//      }
//      if (bp->flags & BILLING_PRINT) {
//	PRINTER_PRINT(' ');
//	printer_prn_uint16(bp->temp.cost);
//	PRINTER_PRINT(' ');
//	printer_prn_uint16(bp->temp.discount);
//	PRINTER_PRINT(' ');
//	printer_prn_uint16(bp->items[ui2].num_sale);
//	PRINTER_PRINT(' ');
//      }
//      assert(bp->temp.cost > bp->temp.discount);
//      ui1 = (bp->temp.cost - bp->temp.discount) * bp->items[ui2].num_sale;
//      if (bp->flags & BILLING_PRINT)
//	printer_prn_uint16(ui1);
//      /* */
//      tot_bill += ui1;
//      if (bp->bi[ui2].has_serv_tax) {
//	serv_tax += ui1;
//      }
//      EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.vat[bp->bi[ui2].vat_sel]), (uint8_t *)&ui1_2, sizeof(uint16_t));
//      tot_tax += ui1 * ui1_2;
//    }
//    /* */
//    EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.service_tax), (uint8_t *)&ui1, sizeof(uint16_t));
//    serv_tax *= ui1;
//
//    /* Total */
//    if (bp->flags & BILLING_PRINT)
//      printer_prn_uint16(tot_tax);
//    if (bp->flags & BILLING_PRINT)
//      printer_prn_uint16(serv_tax);
//    if (bp->flags & BILLING_PRINT)
//      printer_prn_uint16(tot_bill);
//    bp->ui3 = tot_bill;
//    bp->ui5 = serv_tax;
//    bp->ui7 = tot_tax;
//  }
//}
//
// Not unit tested
//void
//menuPrnTaxReport(billing *bp)
//{
//  bp->flags &= ~BILLING_PRINT;
//  menu_PrnFullBill(bp);
//  bp->flags |= BILLING_PRINT;
//
//  /* Bill No, Date */
//  {
//    uint8_t ymd[3];
//
//    printer_prn_str("Bill No:");
//    printer_prn_uint16(bp->ui1);
//    PRINTER_PRINT(' ');
//
//    ymd[0] = bp->info.date_dd;
//    ymd[1] = bp->info.date_mm;
//    //    ymd[2] = bp->info.date_yy;
//    printer_prn_date(ymd);
//  }
//
//  /* This bill */
//  printer_prn_uint16(bp->ui3); /* tot_bill */
//  printer_prn_uint16(bp->ui5); /* serv_tax */
//  printer_prn_uint16(bp->ui7); /* tot_tax */
//
//  /* Total */
//  bp->ui9 += bp->ui3;
//  bp->ui11 += bp->ui5;
//  bp->ui13 += bp->ui7;
//}
//
// Not unit tested
//void
//menu_PrnTaxReportFooter(billing *bp)
//{
//  printer_prn_uint16(bp->ui9);
//  printer_prn_uint16(bp->ui11);
//  printer_prn_uint16(bp->ui13);
//}

// Not unit tested
void
menuModVat(uint8_t mode)
{
//  uint16_t ui1; /* FIXME: ui1 should be of 32 bits */
//  uint8_t  ui2, ui3, choice[MENU_PROMPT_LEN*4];
//
//  if (MENU_ITEM_NONE == arg1.valid)
//    return;
//
//  for (ui2=0; ui2<4; ui2++) {
//    EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.vat[ui2]), (uint8_t *)&ui1, sizeof(uint16_t));
//    for (ui3=0; ui3<MENU_PROMPT_LEN; ui3++) {
//      *(choice+(ui2*MENU_PROMPT_LEN)+MENU_PROMPT_LEN-1-ui3) = '0' + ui1%10;
//      ui1 /= 10;
//    }
//  }
//  ui3 = menu_getchoice(menu_str1+(MENU_STR1_IDX_REPLA*MENU_PROMPT_LEN), choice, MENU_PROMPT_LEN, 4);
//  assert(ui3 < 4);
//
//  ui1 = arg1.value.integer.i8;
//  ui1 <<= 16;
//  ui1 |= arg1.value.integer.i16;
//  EEPROM_STORE_WRITE((uint16_t)&(EEPROM_DATA.vat[ui3]), (uint8_t *)&ui1, sizeof(uint16_t));
  menu_unimplemented(__LINE__);
}

// Not unit tested
void
menuHeader(uint8_t mode)
{
//  uint8_t chars = 0, ui2, ui3;
//  uint8_t mode_max = ((mode&(~MENU_MODEMASK)) == MENU_MFOOTER) ? FOOTER_MAX_SZ : HEADER_MAX_SZ;
//
//  /* Enters with arg1 as valid */
//  do {
//    if (MENU_ITEM_NONE == arg1.valid)
//      break;
//
//    /* check for valid chars */
//    for (ui2=0, ui3=0; (ui2<LCD_MAX_COL) && (0==ui3); ui2++) {
//      ui3 = (isgraph(lcd_buf[LCD_MAX_ROW-1][ui2])) ? 1 : 0;
//    }
//    if (ui3) {
//      for (ui2=0; (ui2<LCD_MAX_COL) && (chars<mode_max); ui2++) {
//	bufSS[ui2] = lcd_buf[LCD_MAX_ROW-1][ui2];
//	chars++;
//      }
//    } else break;
//
//    /* store */
//    if (((mode&(~MENU_MODEMASK)) != MENU_MFOOTER) && (MENU_ITEM_NONE != arg1.valid) && (0 != chars)) {
//      bufSS[chars] = 0;
//      EEPROM_STORE_WRITE((uint16_t)&(EEPROM_DATA.shop_name[0]), (uint8_t *)bufSS, sizeof(uint8_t)*chars);
//      chars = 0;
//      arg1.valid = MENU_ITEM_NONE;
//    }
//
//    arg2.valid = MENU_ITEM_NONE;
//    menu_getopt(menu_str1+(MENU_STR1_IDX_ITEM*MENU_PROMPT_LEN), &arg2, MENU_ITEM_STR);
//  } while (chars < mode_max);
//  bufSS[chars] = 0;
//
//  if (chars) {
//    if ((mode&(~MENU_MODEMASK)) == MENU_MFOOTER) {
//      EEPROM_STORE_WRITE((uint16_t)&(EEPROM_DATA.prn_footer[0]), (uint8_t *)bufSS, sizeof(uint8_t)*chars);
//    } else {
//      EEPROM_STORE_WRITE((uint16_t)&(EEPROM_DATA.prn_header[0]), (uint8_t *)bufSS, sizeof(uint8_t)*chars);
//    }
//  }
  menu_unimplemented(__LINE__);
}

// Not unit tested
void
menuDelAllBill(uint8_t mode)
{
//  uint16_t ui1 = 0;
//  uint8_t  ui2;
//
//  /* Remove all data when date is changed */
//  LCD_ALERT("Delete AllData");
//  if (0 != menu_getchoice(menu_str1+(MENU_STR1_IDX_DADAT*MENU_PROMPT_LEN), menu_str1+(MENU_STR1_IDX_YesNo*MENU_PROMPT_LEN), MENU_PROMPT_LEN, 2))
//    return;
//
//  /* */
//  timerDateSet(arg1.value.date.year, arg1.value.date.month, arg1.value.date.date);
//  for (ui2=0; ui2<(13*4); ui2++) {
//    EEPROM_STORE_WRITE((uint16_t)&(EEPROM_DATA.sale_date_ptr[ui2]), (uint8_t *)&ui1, sizeof(uint16_t));
//  }
//  ui1 = FLASH_DATA_START;
//  EEPROM_STORE_WRITE((uint16_t)&(EEPROM_DATA.sale_start), (uint8_t *)&ui1, sizeof(uint16_t));
//  EEPROM_STORE_WRITE((uint16_t)&(EEPROM_DATA.sale_end), (uint8_t *)&ui1, sizeof(uint16_t));
//  FlashEraseSector(ui1);
  menu_unimplemented(__LINE__);
}

// Not unit tested
void
menuSetServTax(uint8_t mode)
{
//  uint16_t ui1; /* FIXME: ui1 should be of 32 bits */
//
//  if (MENU_ITEM_NONE == arg1.valid)
//    return;
//
//  ui1 = arg1.value.integer.i8;
//  ui1 <<= 16;
//  ui1 |= arg1.value.integer.i16;
//  EEPROM_STORE_WRITE((uint16_t)&(EEPROM_DATA.service_tax), (uint8_t *)&ui1, sizeof(uint16_t));
}

// Not unit tested
void
menuSetDateTime(uint8_t mode)
{
//  uint16_t ui1 = 0;
//  uint8_t ymd[3];
//
//  if ((MENU_ITEM_NONE == arg1.valid) || (MENU_ITEM_NONE == arg2.valid))
//    return;
//
//  timerTimeSet(arg1.value.time.hour, arg1.value.time.min);
//
//  /* if no change in date */
//  timerDateGet(ymd);
//  if ( (ymd[2] == arg1.value.date.year) && (ymd[1] == arg1.value.date.month) && (ymd[0] == arg1.value.date.date) )
//    return;
//
//  menu_DelAllBill(mode);
  menu_unimplemented(__LINE__);
}

// Not unit tested
void
menuRunDiag(uint8_t mode)
{
//  uint8_t  ui2, ui3, ui4;
//
//  /* Run LCD
//     Run slowly and make user see if there is any error
//   */
//  /* Run Printer : Print test page
//   */
//  /* Verify unused EEPROM spacesd : Write/readback
//   */
//  /* FIXME: randomize ui3 */
//  /*
//  for (ui2=0; ui2<NUM_TESTING_BYTES; ui2++) {
//    EEPROM_STORE_WRITE((uint16_t)&(EEPROM_DATA.testing[ui2]), &ui3, sizeof(uint8_t)*LCD_MAX_COL);
//    EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.testing[ui2]), &ui3, sizeof(uint8_t)*LCD_MAX_COL);
//  }
//  */
//  /* Verify Keypad : Ask user to press a key and display it
//   */
  menu_unimplemented(__LINE__);
}

#define ROW_JOIN ,
#define COL_JOIN
#define MENU_HIER(A)
#define MENU_MODE(A)
#define MENU_NAME(A)
#define MENU_FUNC(A) A
#define ARG1(A, B)
#define ARG2(A, B)
menu_func_t menu_handlers[] PROGMEM = {
  MENU_ITEMS
};
#undef  ARG2
#undef  ARG1
#undef  MENU_FUNC
#undef  MENU_NAME
#undef  MENU_MODE
#undef  MENU_HIER
#undef  ROW_JOIN
#undef  COL_JOIN

#define ROW_JOIN ,
#define COL_JOIN
#define MENU_HIER(A) A
#define MENU_MODE(A)
#define MENU_NAME(A)
#define MENU_FUNC(A)
#define ARG1(A, B)
#define ARG2(A, B)
const uint8_t menu_hier[] PROGMEM = {
  MENU_ITEMS
};
#undef  ARG2
#undef  ARG1
#undef  MENU_FUNC
#undef  MENU_NAME
#undef  MENU_MODE
#undef  MENU_HIER
#undef  ROW_JOIN
#undef  COL_JOIN

const uint8_t menu_hier_names[] PROGMEM = MENU_HIER_NAMES;

void
menuMain(void)
{
  uint8_t menu_selected, menu_selhier, ui8_1;

  /* initialize
     0==menu_selhier : hier not selected
   */
  menu_selected = 0;
  menu_selhier = 0;
  assert(KBD_NOT_HIT); /* ensures kbd for user-inputs */

menu_main_start:
  /* Clear known key press */
  KBD_RESET_KEY;

    eeprom_read_block(bufSS, (uint8_t *)offsetof(struct ep_store_layout, shop_name), SHOP_NAME_SZ_MAX);
    bufSS[SHOP_NAME_SZ_MAX] = 0;

  /* First select a Menu */
  if (0 == menu_selhier) {
    /* Display shop name */
    LCD_WR_LINE_N(0, 0, bufSS, LCD_MAX_COL);
    LCD_WR_LINE_NP(LCD_MAX_ROW-1, 0, (menu_hier_names+(menu_selected*MENU_HIER_NAME_SIZE)), MENU_HIER_NAME_SIZE);
  } else {
    /* Shop name (8 chars) */
    LCD_WR_LINE_N(0, 0, bufSS, ((SHOP_NAME_SZ_MAX<LCD_MAX_COL)?SHOP_NAME_SZ_MAX:LCD_MAX_COL));
    LCD_POS(0, (LCD_MAX_COL>>1)&0xF);
    LCD_PUTCH('>');
    LCD_WR_LINE_NP(0, ((LCD_MAX_COL>>1)+1)&0xF, (menu_hier_names+((menu_selhier-1)*MENU_HIER_NAME_SIZE)), MENU_HIER_NAME_SIZE);
    LCD_WR_LINE_NP(LCD_MAX_ROW-1, 0, (menu_names+(menu_selected*MENU_NAMES_LEN)), MENU_NAMES_LEN);
  }
  LCD_refresh();

  /* Wait until get command from user */
  KBD_GETCH;

  if ((ASCII_ENTER == KbdData) && (0 == menu_selhier)) {
    menu_selhier = menu_selected + 1;
    for (ui8_1=0; ui8_1<MENU_MAX; ui8_1++) {
      if ( ((menu_hier[ui8_1]) == menu_selhier) /* menu appropriate */ &&
	   (0 != (MenuMode & (menu_mode[ui8_1] & MENU_MODEMASK))) /* mode appropriate */
	   ) {
	menu_selected = ui8_1;
	break;
      }
    }
    /* No valid menu items, go back */
    if (MENU_MAX == ui8_1) {
      menu_selhier = 0; /* menu_selected remains the same */
    }
  } else if ((ASCII_ENTER == KbdData) && (0 != menu_selhier)) {
    /* */
    LCD_WR_LINE_NP(0, 0, (menu_hier_names+((menu_selhier-1)*MENU_HIER_NAME_SIZE)), MENU_HIER_NAME_SIZE);
    LCD_POS(0, (LCD_MAX_COL>>1)&0xF);
    LCD_PUTCH('>');
    LCD_WR_LINE_NP(0, ((LCD_MAX_COL>>1)+1)&0xF, (menu_names+(menu_selected*MENU_NAMES_LEN)), ((MENU_NAMES_LEN>6)?6:MENU_NAMES_LEN));

    /* Get choices before menu function is called */
    KBD_RESET_KEY;
    arg1.valid = MENU_ITEM_NONE;
    arg1.value.sptr = bufSS;
    LCD_CLRSCR;
    menuGetOpt(menu_prompt_str+((menu_prompts[menu_selected<<1])*MENU_PROMPT_LEN), &arg1, menu_args[(menu_selected<<1)]);
    assert (KBD_HIT);
    KBD_RESET_KEY;
    arg2.valid = MENU_ITEM_NONE;
    arg2.value.sptr = bufSS+LCD_MAX_COL+2;
    LCD_CLRSCR;
    menuGetOpt(menu_prompt_str+((menu_prompts[(menu_selected<<1)+1])*MENU_PROMPT_LEN), &arg2, menu_args[((menu_selected<<1)+1)]);
#ifdef UNIT_TEST_MENU_1
    UNIT_TEST_MENU_1(menu_selected);
#else
    (menu_handlers[menu_selected])(menu_mode[menu_selected]);
#endif
  } else if ((ASCII_LEFT == KbdData) || (ASCII_UP == KbdData)) {
    if (0 == menu_selhier) {
      /* selection of menu */
      menu_selected = (0 == menu_selected) ? MENU_HIER_MAX-1 : menu_selected-1;
    } else { /* (0 != menu_selhier) && (0 != menu_selected) */
      for (ui8_1=0; ui8_1<MENU_MAX; ui8_1++) {
	menu_selected--;
	if (0 == menu_selected) {
	  menu_selhier = 0;
	  break;
	} else if ( ((menu_hier[menu_selected]) == menu_selhier) /* menu appropriate */ &&
		   (0 != (MenuMode & (menu_mode[menu_selected] & MENU_MODEMASK))) /* mode appropriate */
		   ) {
	  break;
	}
      }
      /* No valid menu items, go back */
      if (MENU_MAX == ui8_1) {
	menu_selected = menu_selhier = 0;
      }
    }
  } else if ((ASCII_RIGHT == KbdData) || (ASCII_DOWN == KbdData)) {
    if (0 == menu_selhier) {
      menu_selected = (menu_selected >= (MENU_HIER_MAX-1)) ? 0 : menu_selected+1;
    } else {
      for (ui8_1=0; ui8_1<MENU_MAX; ui8_1++) {
	menu_selected = (MENU_MAX < (1+menu_selected)) ? menu_selected+1 : 0;
	if ( ((menu_hier[menu_selected]) == menu_selhier) /* menu appropriate */ &&
		   (0 != (MenuMode & (menu_mode[menu_selected] & MENU_MODEMASK))) /* mode appropriate */
		   ) {
	  break;
	}
      }
      /* No valid menu items, go back */
      if (MENU_MAX == ui8_1) {
	menu_selected = menu_selhier = 0;
      }
    }
  }

  LCD_refresh();
#ifdef UNIT_TEST
  //  move(0, 0);
  //  printw("menu_selhier:%d  menu_selected:%d", menu_selhier, menu_selected);

  /* Provide means to excape the infinite hold */
  if (ASCII_F2 == KbdData) {
    KBD_RESET_KEY;
    return;
  }
#endif

  /* Forever stuck in this maze.. can't ever get out */
  goto menu_main_start;
}

// Not unit tested
///* Sales are easily accessable from table */
//void
//menuSaleAdd(uint8_t *sale)
//{
//  uint16_t sale_start, sale_end;
//  uint16_t sale_start_sector, sale_end_sector, sale_newend_sector;
//  uint8_t  nbytes, ui2;
//
//  /* init */
//  EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.sale_start), (uint8_t *)&sale_start, sizeof(uint16_t));
//  EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.sale_end), (uint8_t *)&sale_end, sizeof(uint16_t));
//  sale_start_sector = sale_start & ~((uint16_t) (FLASH_SECTOR_SIZE-1));
//  sale_end_sector   = sale_end & ~((uint16_t) (FLASH_SECTOR_SIZE-1));
//  assert(sale_start_sector != sale_end_sector);
//
//  /* compute if there is overflow sector
//     Implemented as a circular buffer. Possible combinations are
//     start ->         | end   ->
//     end   ->         | start ->
//
//     start : pointer to start of valid data
//     end   : pointer to start of free  space
//   */
//  nbytes = SALE_INFO_SIZEOF + (((sale[0] & SALE_INFO_BYTE_NITEM_MASK) >> SALE_INFO_BYTE_NITEM_SHIFT) * (uint16_t)SALE_SIZEOF);
//  sale_newend_sector = (sale_end+nbytes-1) & ~((uint16_t) (FLASH_SECTOR_SIZE-1));
//  if (sale_newend_sector >= FLASH_DATA_END) {
//    sale_newend_sector = FLASH_DATA_START;
//  }
//  assert ( (sale_newend_sector <= sale_start_sector) || (sale_newend_sector >= sale_end_sector) );
//  assert ( (sale_newend_sector >= FLASH_DATA_START)  && (sale_newend_sector < FLASH_DATA_END) );
//
//  /* when new sector is taken, erase it */
//  if (sale_end_sector != sale_newend_sector) {
//    /* If new sector is already occupied */
//    for (; sale_start_sector==sale_newend_sector; ) {
//      /* Need to delete few sectors... on ToT */
//      menu_sale_free_old_sector();
//      /* */
//      EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.sale_start), (uint8_t *)&sale_start, sizeof(uint16_t));
//      sale_start_sector = sale_start & ~((uint16_t) (FLASH_SECTOR_SIZE-1));
//    }
//    FlashEraseSector(sale_newend_sector);
//  }
//
//  /* Bytes are available ..., just store */
//  sale_index_it((sale_info *)sale, sale_end);
//  for (ui2=0; ui2<nbytes; ui2++) {
//    if (0 != sale[0])
//      FlashWriteByte(sale_end, sale[0]);
//    sale_end++, sale++;
//    if (sale_end >= FLASH_DATA_END) {
//      sale_end = FLASH_DATA_START;
//    }
//  }
//  EEPROM_STORE_WRITE((uint16_t)&(EEPROM_DATA.sale_end), (uint8_t *)&sale_end, sizeof(uint16_t));
//}
//
// Not unit tested
//void
//menu_sale_delete_month(uint8_t del_month)
//{
//  uint16_t sale_start, sale_end;
//  uint16_t sale_start_sector, sale_end_sector, sale_newend_sector;
//  uint8_t  nbytes, ui2;
//
//  /* init */
//  EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.sale_start), (uint8_t *)&sale_start, sizeof(uint16_t));
//  EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.sale_end), (uint8_t *)&sale_end, sizeof(uint16_t));
//  sale_start_sector = sale_start & ~((uint16_t) (FLASH_SECTOR_SIZE-1));
//  sale_end_sector   = sale_end & ~((uint16_t) (FLASH_SECTOR_SIZE-1));
//
//  /* Move forward : Not required to reflash the new sector */
//  {
//    uint8_t ui3[3];
//    sale_info *si;
//    do {
//      si = (void *)ui3;
//      ui3[0] = FlashReadByte(sale_start);
//      ui3[1] = FlashReadByte(sale_start+1);
//      ui3[2] = FlashReadByte(sale_start+2);
//      if ((del_month == si->date_mm) && (sale_start!=sale_end)) {
//	sale_start += SALE_INFO_SIZEOF + (((ui3[0] & SALE_INFO_BYTE_NITEM_MASK) >> SALE_INFO_BYTE_NITEM_SHIFT) * SALE_SIZEOF);
//	if (sale_start >= FLASH_DATA_END) {
//	  sale_start = FLASH_DATA_START + (sale_start % FLASH_SECTOR_SIZE);
//	}
//	sale_start_sector = sale_start & ~((uint16_t) (FLASH_SECTOR_SIZE-1));
//      }
//    } while(del_month == si->date_mm);
//    EEPROM_STORE_WRITE((uint16_t)&(EEPROM_DATA.sale_start), (uint8_t *)&sale_start, sizeof(uint16_t));
//  }
//}
//
// Not unit tested
//uint16_t
//menu_sale_find(uint8_t *dmy, uint16_t id)
//{
//  sale_info  *si;
//  uint16_t ui1, vptr;
//  uint8_t idx;
//
//  idx = (dmy[1] - 1) * 4;
//  if (dmy[0] <= 7) {}
//  else if (dmy[0] <= 14) idx ++;
//  else if (dmy[0] <= 21) idx += 2;
//  else idx += 3;
//
//  EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.sale_date_ptr[idx]), (uint8_t *)&vptr, sizeof(uint16_t));
//  si = (sale_info *)vptr;
//  if ((0 == si) || (0 == id)) return (uint16_t)FLASH_ADDR_INVALID;
//
//  for (ui1=0; ui1<id; vptr=(uint16_t)si) {
//    uint8_t ui3[3];
//    sale_info* si_t = (void *)ui3;
//    ui3[0] = FlashReadByte((uint16_t)si);
//    ui3[1] = FlashReadByte((uint16_t)(si+1));
//    ui3[2] = FlashReadByte((uint16_t)(si+2));
//    if (dmy[1] != si_t->date_mm)
//      return (void *) FLASH_ADDR_INVALID;
//    /* move past this record */
//    si = (sale_info *)( ((uint8_t *)si) + SALE_INFO_SIZEOF + (((ui3[0] & SALE_INFO_BYTE_NITEM_MASK) >> SALE_INFO_BYTE_NITEM_SHIFT) * SALE_SIZEOF) );
//    if (dmy[0] < si_t->date_dd) {
//    } else if (dmy[0] == si_t->date_dd) {
//      ui1++;
//    } else { /* less # items for this day */
//      return (void *) FLASH_ADDR_INVALID;
//    }
//  }
//
//  /* */
//  assert(vptr);
//  si = (sale_info *)vptr;
//  if ((si->property) & SALE_INFO_DELETED)
//    return (uint16_t) FLASH_ADDR_INVALID;
//  return (uint16_t)vptr;
//}
//
// Not unit tested
//void
//menu_sale_free_old_sector()
//{
//  uint16_t ui1, ui6;
//  uint8_t ui2, ui4, ui5;
//  uint16_t sale_start, sale_end;
//  uint16_t sale_start_sector, sale_end_sector, sale_newstart_sector;
//
//  /* Find the first reference to sale data */
//  EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.sale_date_old_ptr_month), (uint8_t *)&ui2, sizeof(uint8_t));
//  ui4 = ui2 = (ui2-1)*4;
//  do {
//    ui4 %= 12*4;
//    EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.sale_date_ptr[ui4]), (uint8_t *)&ui1, sizeof(uint16_t));
//    if (0 != ui1) break;
//    ui4++;
//  } while (ui2 != ui4);
//  EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.sale_start), (uint8_t *)&sale_start, sizeof(uint16_t));
//  sale_start_sector = sale_start & ~((uint16_t) (FLASH_SECTOR_SIZE-1));
//  EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.sale_end), (uint8_t *)&sale_end, sizeof(uint16_t));
//  sale_end_sector = sale_end & ~((uint16_t) (FLASH_SECTOR_SIZE-1));
//  assert(ui1 == sale_start);
//
//  /* We just need to delete one sector of data */
//  {
//    uint8_t ui3[3];
//    sale_info *si = (void *)ui3;
//    for (sale_newstart_sector=sale_start_sector; sale_newstart_sector==sale_start_sector;) {
//      ui3[0] = FlashReadByte(sale_start);
//      if (sale_start!=sale_end) {
//	sale_start += SALE_INFO_SIZEOF + (((ui3[0] & SALE_INFO_BYTE_NITEM_MASK) >> SALE_INFO_BYTE_NITEM_SHIFT) * SALE_SIZEOF);
//	if (sale_start >= FLASH_DATA_END) {
//	  sale_start = FLASH_DATA_START + (sale_start % FLASH_SECTOR_SIZE);
//	}
//	sale_newstart_sector = sale_start & ~((uint16_t) (FLASH_SECTOR_SIZE-1));
//      }
//    }
//    EEPROM_STORE_WRITE((uint16_t)&(EEPROM_DATA.sale_start), (uint8_t *)&sale_start, sizeof(uint16_t));
//
//    /* Clear all month references before this month */
//    ui3[0] = FlashReadByte(sale_start);
//    ui3[1] = FlashReadByte(sale_start+1);
//    ui3[2] = FlashReadByte(sale_start+2);
//    EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.sale_date_old_ptr_month), (uint8_t *)&ui2, sizeof(uint8_t));
//    if (ui2 != si->date_mm) { /* we have deleted few months of data */
//      ui1 = 0;
//      for (ui2=INCR_MONTH(ui2); ui2!=si->date_mm; ui2=INCR_MONTH(ui2)) {
//	ui5 = (ui2-1)*4;
//	for (ui4=0; ui4<4; ui4++) {
//	  EEPROM_STORE_WRITE((uint16_t)&(EEPROM_DATA.sale_date_ptr[ui5+ui4]), (uint8_t *)&ui1, sizeof(uint16_t));
//	}
//      }
//      /* move pointers */
//      ui2 = si->date_mm;
//      ui2 = (ui2-1)*4;
//      for (ui1=0, ui6=0, ui4=0, ui5=8; ui4<4; ui4++, ui5+=7) {
//	if (si->date_dd < ui5) {
//	  if (0 == ui1)
//	    ui1 = sale_start;
//	  else
//	    EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.sale_date_ptr[ui2+ui4]), (uint8_t *)&ui1, sizeof(uint16_t));
//	}
//	EEPROM_STORE_WRITE((uint16_t)&(EEPROM_DATA.sale_date_old_ptr[ui4]), (uint8_t *)&ui1, sizeof(uint16_t));
//	EEPROM_STORE_WRITE((uint16_t)&(EEPROM_DATA.sale_date_ptr[ui2+ui4]), (uint8_t *)&ui6, sizeof(uint16_t));
//      }
//      assert(0 != ui1);
//    } else { /* we have deleted < 1 month of data */
//      for (ui1=0, ui6=0, ui4=0, ui5=8; ui4<4; ui4++, ui5+=7) {
//	if (si->date_dd < ui5) {
//	  if (0 == ui1)
//	    ui1 = sale_start;
//	  else
//	    EEPROM_STORE_READ((uint16_t)&(EEPROM_DATA.sale_date_old_ptr[ui4]), (uint8_t *)&ui1, sizeof(uint16_t));
//	}
//	EEPROM_STORE_WRITE((uint16_t)&(EEPROM_DATA.sale_date_old_ptr[ui4]), (uint8_t *)&ui1, sizeof(uint16_t));
//      }
//      assert(0 != ui1);
//    }
//    ui2 = si->date_mm;
//    EEPROM_STORE_WRITE((uint16_t)&(EEPROM_DATA.sale_date_old_ptr_month), (uint8_t *)&ui2, sizeof(uint8_t));
//  }
//}

// Not unit tested
void
menuSDLoadItem(uint8_t mode)
{
  FATFS FS;
  FIL   fp;
  UINT  ret_size, ui1;
  uint16_t ui16_1;
  uint8_t ui8_1;

  /* */
  f_mount(&FS, ".", 1);
  if (FR_OK != f_open(&fp, SD_ITEM_FILE, FA_READ)) {
    LCD_ALERT("File open error");
    goto menuSDLoadItemExit;
  }

  /* Check for crc in file */
  bufSS[BUFSS_SIZE-2] = 0, bufSS[BUFSS_SIZE-1] = 0;
  ui8_1 = 0, ui16_1 = 0;
  while (FR_OK == f_read(&fp, bufSS, BUFSS_SIZE-2, &ret_size)) {
    if (0 == ret_size) break;
    if (0 != ui16_1) { /* skip first time */
      ui16_1 = _crc16_update(ui16_1, bufSS[BUFSS_SIZE-2]);
      ui16_1 = _crc16_update(ui16_1, bufSS[BUFSS_SIZE-1]);
    }
    for (ui1=0; ui1<(ret_size-2); ui1++) {
      ui16_1 = _crc16_update(ui16_1, bufSS[ui1]);
    }
    bufSS[BUFSS_SIZE-1] = bufSS[ret_size-1];
    bufSS[BUFSS_SIZE-2] = bufSS[ret_size-2];
    ui8_1 = (ret_size + ui8_1) % ITEM_SIZEOF;
    //    printf("ret_size:%d ui8_1:%d ITEM_SIZEOF:%d\n", ret_size, ui8_1, ITEM_SIZEOF);
  }
  ret_size = bufSS[BUFSS_SIZE-2];
  ret_size <<= 8;
  ret_size |= bufSS[BUFSS_SIZE-1];
  if ((0 == ui16_1) || (ui16_1 != ret_size) || (4 != ui8_1)) {
    //    printf("ui16_1:%x ret_size:%x ui8_1:%d\n", ui16_1, ret_size, ui8_1);
    LCD_ALERT_16N("File error ", ui16_1);
    goto menuSDLoadItemExit;
  }

  /* */
  assert(FR_OK == f_lseek(&fp, 0));
  assert(FR_OK == f_read(&fp, bufSS, 2, &ret_size));
  assert(2 == ret_size);
  ui1 = bufSS[0];
  ui1<<=8, ui1 |= bufSS[1];
  ui16_1 = 0;
  while (FR_OK == f_read(&fp, bufSS, ITEM_SIZEOF, &ret_size)) {
    if (ITEM_SIZEOF != ret_size) break; /* reached last */
    ee24xx_write_bytes(ui16_1, bufSS, ITEM_SIZEOF);
//    printf("writing to addr : %x : ", ui16_1);
//    for (ui8_1=0; ui8_1<ITEM_SIZEOF; ui8_1++) {
//      printf("%x ", bufSS[ui8_1]);
//    }
//    printf("\n");
    ui16_1 += (ITEM_SIZEOF>>EEPROM_MAX_DEVICES_LOGN2);
  }
  assert(ui16_1 == (ui1*(ITEM_SIZEOF>>EEPROM_MAX_DEVICES_LOGN2)));
  assert(2 == ret_size); /* crc would be pending */
  /* Clear all other memeory */
  for (ui8_1=0; ui8_1<ITEM_SIZEOF; ui8_1++)
    bufSS[ui8_1] = 0;
  while ((EEPROM_ADDR_INVALID - ui16_1) > ITEM_SIZEOF) {
    ee24xx_write_bytes(ui16_1, bufSS, ITEM_SIZEOF);
    ui16_1 += (ITEM_SIZEOF>>EEPROM_MAX_DEVICES_LOGN2);
  }

 menuSDLoadItemExit:
  /* */
  f_mount(NULL, "", 0);
}

// Not unit tested
void
menuSDSaveItem(uint8_t mode)
{
  menu_unimplemented(__LINE__);
}
