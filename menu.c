#include <stdint.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <avr/boot.h>

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

const uint8_t    menu_prompt_str[] PROGMEM = MENU_PROMPTS;
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
#define MENU_STR1_IDX_SALEQTY  8
#define MENU_STR1_IDX_FINALIZ 9
#define MENU_STR1_IDX_PRINT 10
#define MENU_STR1_IDX_SAVE  11
#define MENU_STR1_IDX_DADAT 12
#define MENU_STR1_IDX_NAME  13
#define MENU_STR1_IDX_REPLA 14
#define MENU_STR1_IDX_DAY   15
#define MENU_STR1_IDX_MONTH 16
#define MENU_STR1_IDX_YEAR  17
#define MENU_STR1_IDX_PRODCODE  18
#define MENU_STR1_IDX_UNICODE   19
#define MENU_STR1_IDX_ENTRYES   20
#define MENU_STR1_IDX_COMNDISC  21
#define MENU_STR1_IDX_NUM_ITEMS 22
const uint8_t menu_str1[] PROGMEM =
  "Price   " /* 0 */
  "Discount" /* 1 */
  "Serv.Tax" /* 2 */
  "Yes     " /* 3 */
  "No      " /* 4 */
  "Vat     " /* 5 */
  "Confirm?" /* 6 */
  "Item/id " /* 7 */
  "Sale Qty" /* 8 */
  "Finaliz?" /* 9 */
  "Print?  " /*10 */
  "Save?   " /*11 */
  "Delete? " /*12 */
  "Name    " /*13 */
  "Replace?" /*14 */
  "Date    " /*15 */
  "Month   " /*16 */
  "Year    " /*17 */
  "ProdCode" /*18 */
  "Unicode " /*19 */
  "Entr:Yes" /*20 */
  "ComnDisc" /*21 */
  ;

/* */
#define BILLFILENAME "billing.csv"
const uint8_t BillFileName[] PROGMEM = BILLFILENAME;

/* */
static uint8_t MenuMode = MENU_MRESET;
static uint8_t LoginUserId = 0; /* 0 is invalid */
static uint8_t devStatus = 0;   /* 0 is no err */
#define DS_NO_SD    (1<<0)
#define DS_NO_TFT   (1<<1)
#define DS_DEV_1K   (1<<2)
#define DS_DEV_5K   (1<<3)
#define DS_DEV_20K  (1<<4)
#define DS_DEV_INVALID (0xFF)

/* Helper routine to obtain input from user */
void
menuGetOpt(const uint8_t *prompt, menu_arg_t *arg, uint8_t opt)
{
  uint8_t col_id;
  uint8_t item_type = (opt & MENU_ITEM_TYPE_MASK);
  uint8_t *lbp = (uint8_t *) lcd_buf[LCD_MAX_ROW-1], *lp;
  uint32_t val = 0;
  uint8_t ui1;

  if (MENU_ITEM_NONE == opt) return;

  /* init */
  LCD_CLRSCR;
  lcd_buf_prop = (opt & MENU_ITEM_PASSWD) ? LCD_PROP_NOECHO_L2 : 0;

  /* Ask a question */
  LCD_WR_LINE_NP(0, 0, prompt, MENU_PROMPT_LEN);
  LCD_POS(0, MENU_PROMPT_LEN);
  LCD_WR(" ?");
  LCD_refresh();

  /* Set the prompt */
  col_id = 0;
  lp = &(lcd_buf[LCD_MAX_ROW-1][0]);

  /* Get a string */
  do {
    /* added cursor functionality */
    if (col_id < (LCD_MAX_COL-1))
      lp[col_id] = '_';
    LCD_refresh();
    KBD_RESET_KEY;
    KBD_GETCH;

    switch (KbdData) {
    case ASCII_BACKSPACE:
    case ASCII_LEFT:
    case ASCII_UP:
      col_id--;
      break;
    case ASCII_LF:
    case ASCII_ENTER:
      if (col_id < (LCD_MAX_COL-1)) {
	lp[col_id] = ' ';
      }
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
      lp[col_id] = KbdData;
      /* Don't overflow buffer */
      if (col_id < LCD_MAX_COL)
	col_id++;
    }
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
    arg->value.integer.i8  = val&0xFF;
    arg->value.integer.i16 = val;
    arg->value.integer.i32 = val;
  } else if ((MENU_ITEM_DATE == item_type) || (MENU_ITEM_MONTH == item_type)) {
    /* format DDMMYYYY || format MMYYYY */
    menu_error = 0;
    for (ui1=0; ui1<(item_type+2); ui1++) {
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
      if (val < 2014) menu_error++;
      arg->value.date.year = val-1980;
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
  } else {
    arg->valid = MENU_ITEM_NONE;
  }

  lcd_buf_prop = 0;
}

uint8_t
menuGetYesNo(const uint8_t *quest, uint8_t size)
{
  uint8_t ui8_1, ui8_2, ret;
  
  for (ret=0; ;) {
    ret &= 1;
    LCD_WR_LINE_NP(LCD_MAX_ROW-1, 0, quest, size);
    LCD_POS(LCD_MAX_ROW-1, size);
    LCD_PUTCH(':');
    LCD_PUTCH(' ');
    //printf("lcd_buf[1]:%p lcd_buf_p:%p\n", lcd_buf[1], LCD_CUR_POS_P);
    //printf("before lcd_buf[1]:'%s'\n", lcd_buf[1]);
    PSTR2STRN(menu_str1+((MENU_STR1_IDX_YesNo+ret)*MENU_PROMPT_LEN), LCD_CUR_POS_P, ui8_1, ui8_2, 3);
    //printf("after  lcd_buf[1]:'%s'\n", lcd_buf[1]);
    LCD_refresh();

    KBD_RESET_KEY;
    KBD_GETCH;

    if ((ASCII_RIGHT == KbdData) || (ASCII_DOWN == KbdData)) {
      ret++;
    } else if ((ASCII_LEFT == KbdData) || (ASCII_UP == KbdData)) {
      ret--;
    } else if (ASCII_ENTER == KbdData) {
      return ret & 1;
    }
  }
  assert(0);
}

/* Helper routine to obtain choice from user */
uint8_t
menuGetChoice(const uint8_t *quest, uint8_t *opt_arr, uint8_t choice_len, uint8_t max_idx)
{
  uint8_t ret = 0, ui8_1;

  do {
    assert(ret < max_idx);

    ui8_1 = LCD_MAX_COL-choice_len-1;
    LCD_WR_LINE_NP(LCD_MAX_ROW-1, 0, quest, ui8_1);
    LCD_POS(LCD_MAX_ROW-1, ui8_1);
    LCD_WR(":");
    LCD_WR_N((opt_arr+(ret*choice_len)), choice_len);
    LCD_refresh();

    KBD_RESET_KEY;
    KBD_GETCH;

    if ((ASCII_RIGHT == KbdData) || (ASCII_DOWN == KbdData)) {
      ret = ((max_idx-1)==ret) ? 0 : ret+1;
    } else if ((ASCII_LEFT == KbdData) || (ASCII_UP == KbdData)) {
      ret = (0==ret) ? max_idx-1 : ret-1;
    } else if (ASCII_ENTER == KbdData) {
      return ret;
    }
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
  uint8_t ui8_1;
  uint16_t ui16_1;

  assert(MENU_MSUPER == MenuMode);

  /* Mark all items as deleted : (0==id) */
  for (ui8_1=0; ui8_1<ITEM_SIZEOF; ui8_1++)
    bufSS[ui8_1] = 0;
  for (ui16_1=0;
       (EEPROM_MAX_ADDRESS-ui16_1+1) >= (ITEM_SIZEOF>>EEPROM_MAX_DEVICES_LOGN2);
       ui16_1 += (ITEM_SIZEOF>>EEPROM_MAX_DEVICES_LOGN2) ) {
    ee24xx_write_bytes(ui16_1+(offsetof(struct item, id)>>EEPROM_MAX_DEVICES_LOGN2),
		       bufSS+offsetof(struct item, id), EEPROM_MAX_DEVICES_LOGN2);
  }

  menuInit();
}

void
menuUnimplemented(uint32_t line)
{
  LCD_WR_LINE_N(LCD_MAX_ROW-1, 0, "unimplemented ", 14);
  LCD_PUT_UINT8X(line);
  LCD_refresh();
}

/* Set others passwd : only admin can do this
   arg1 : user name
   arg2 : passwd
 */
void
menuSetUserPasswd(uint8_t mode)
{
  uint8_t ui8_1, ui8_2, ui8_3;
  uint16_t ui16_1;
  assert(MENU_MSUPER == MenuMode);

  /* Choose the user to replace it with */
  assert(MENU_ITEM_STR == arg1.valid);
  assert(MENU_ITEM_STR == arg2.valid);
  for (ui8_2=0; ui8_2<EPS_MAX_UNAME; ui8_2++) {
    ui8_3 = arg1.value.sptr[ui8_2];
    /* check alnum? */
    if ((!isalnum(ui8_3)) && (ui8_2 > 0))
      break;
    else if (!isalnum(ui8_3)) {
      LCD_ALERT("Invalid User");
      LCD_refresh();
      KBD_RESET_KEY;
      KBD_GETCH;
      return;
    }
  }
  /* where to replace it ? */
  for (
       ui8_2=0, ui16_1=offsetof(struct ep_store_layout, users)+EPS_MAX_UNAME;
       ui8_2<(EPS_MAX_USERS*EPS_MAX_UNAME); ui8_2++, ui16_1++ ) {
    bufSS[(LCD_MAX_COL*3)+ui8_2] = eeprom_read_byte((uint8_t *)ui16_1);
  }
  ui8_2 = menuGetChoice(PSTR("Replace at?"), bufSS+(LCD_MAX_COL*3), EPS_MAX_UNAME, EPS_MAX_USERS-1) + 1;
  if (0 != menuGetYesNo(menu_str1+(MENU_STR1_IDX_CONFI*MENU_PROMPT_LEN), MENU_PROMPT_LEN)) {
    LCD_ALERT("Aborting!");
    return;
  }
  /* Check user name is unique before accepting */
  for (ui8_3=0; ui8_3<EPS_MAX_USERS; ui8_3++) {
    if (ui8_2 == ui8_3) continue; /* skip choosen name */
    ui16_1 = offsetof(struct ep_store_layout, users) + (((uint16_t)EPS_MAX_USERS) * ui8_3);
    for (ui8_1=0; ui8_1<EPS_MAX_UNAME; ui8_1++) {
      if (arg1.value.sptr[ui8_1] != eeprom_read_byte(ui16_1+ui8_1))
	break;
    }
    if (EPS_MAX_UNAME == ui8_1) {
      LCD_ALERT("Duplicate User");
      return;
    }
  }

  /* modify at will */
  ui8_3 = LoginUserId;
  LoginUserId = ui8_2;
  menuSetPasswd(mode & ~MENU_MVALIDATE);
  LoginUserId = ui8_3;
  for (ui8_3=0; ui8_3<EPS_MAX_UNAME; ui8_3++) {
    eeprom_update_byte((uint8_t *)(offsetof(struct ep_store_layout, users)+((ui8_2-1)*EPS_MAX_UNAME)+ui8_3), arg1.value.sptr[ui8_3]);
  }
}

/* Set my password, arg1 is old passwd, arg2 is new passwd */
void
menuSetPasswd(uint8_t mode)
{
  uint16_t crc_old = 0, crc_new = 0;
  uint8_t ui8_2, ui8_3, ui8_4;

  /* */
  assert(0 != LoginUserId);
  assert(MENU_MRESET != MenuMode);

  /* Compute CRC on old password, check */
  if (0 != (mode & (~MENU_MODEMASK) & MENU_MVALIDATE)) {
    assert(MENU_ITEM_STR == arg1.valid);
    for (ui8_4=0; ui8_4<LCD_MAX_COL; ui8_4++) {
      ui8_2 = arg1.value.sptr[ui8_4];
      /* check isprintable? */
      for (ui8_3=0; (isgraph(ui8_2)) && (ui8_3<(KCHAR_COLS*KCHAR_ROWS)); ui8_3++) {
	if (ui8_2 == keyChars[ui8_3])
	  break;
      }
      if (ui8_3 < (KCHAR_ROWS*KCHAR_COLS))
	crc_old = _crc16_update(crc_old, ui8_2);
    }

    if (eeprom_read_word((uint16_t *)offsetof(struct ep_store_layout, passwds[(LoginUserId-1)])) != crc_old) {
      LCD_ALERT("Passwd Wrong!!");
      return;
    }
  }

  /* update mine only */
  assert(MENU_ITEM_STR == arg2.valid);
  for (ui8_4=0; ui8_4<LCD_MAX_COL; ui8_4++) {
    ui8_2 = arg2.value.sptr[ui8_4];
    /* check isprintable? */
    for (ui8_3=0; (isgraph(ui8_2)) && (ui8_3<(KCHAR_COLS*KCHAR_ROWS)); ui8_3++) {
      if (ui8_2 == keyChars[ui8_3])
	break;
    }
    if (ui8_3 < (KCHAR_ROWS*KCHAR_COLS))
      crc_new = _crc16_update(crc_new, ui8_2);
  }

  eeprom_update_word((uint16_t *)(offsetof(struct ep_store_layout, passwds)+((LoginUserId-1)*sizeof(uint16_t))), crc_new);
  LCD_ALERT("Passwd Updated");
}

/* Logout an user */
void
menuUserLogout(uint8_t mode)
{
  if (0 == menuGetYesNo(PSTR("Logout?"), 7)) {
    LoginUserId = 0;
    MenuMode = MENU_MRESET;
  }
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

// Not unit tested
void
menuInit(void)
{
  uint16_t ui16_1, ui16_2;
  uint8_t ui8_1, ui8_2;
  MenuMode = MENU_MRESET;

  assert ((ITEM_SIZEOF+LCD_MAX_COL+LCD_MAX_COL+4) < BUFSS_SIZE);
  
  assert(1 == sizeof(uint8_t));
  //  assert(sizeof(void *) == sizeof(uint16_t));
  assert(((offsetof(struct item, name)&(0xFFFF<<EEPROM_MAX_DEVICES_LOGN2))>>EEPROM_MAX_DEVICES_LOGN2) == (offsetof(struct item, name)>>EEPROM_MAX_DEVICES_LOGN2));
  assert(0 == (ITEM_SIZEOF % (1<<EEPROM_MAX_DEVICES_LOGN2)));

  /* edit eeprom for users[0] = 'admin' */
  eeprom_update_byte((uint8_t *)(offsetof(struct ep_store_layout, users)+0), 'a');
  eeprom_update_byte((uint8_t *)(offsetof(struct ep_store_layout, users)+1), 'd');
  eeprom_update_byte((uint8_t *)(offsetof(struct ep_store_layout, users)+2), 'm');
  eeprom_update_byte((uint8_t *)(offsetof(struct ep_store_layout, users)+3), 'i');
  eeprom_update_byte((uint8_t *)(offsetof(struct ep_store_layout, users)+4), 'n');
  for (ui8_1=5; ui8_1<EPS_MAX_UNAME; ui8_1++)
    eeprom_update_byte((uint8_t *)(offsetof(struct ep_store_layout, users)+ui8_1), ' ');

  /* init global vars */
  devStatus = 0;

  /* When start, check billing.csv for proper version and move it to _x.old files if wrong version */
  FATFS FS;
  FIL   Fil;
  UINT ret_val;
  memset(&FS, 0, sizeof(FS));
  memset(&Fil, 0, sizeof(FIL));
  //  change_sd(0); /* FIXME: */
  PSTR2STR(BillFileName, bufSS, ui8_1, ui8_2);
  if ( (FR_OK == f_mount(&FS, ".", 1)) &&
       (f_open(&Fil, bufSS, FA_WRITE|FA_CREATE_ALWAYS) == FR_OK) ) {
    if (f_size(&Fil) > 0) { /* check version */
      f_read(&Fil, bufSS, 2, &ret_val);
      assert(2 == ret_val);
      ui16_1 = bufSS[0]; ui16_1 <<= 8; ui16_1 |= bufSS[1];
      if (GIT_HASH_CRC != ui16_1) {
	LCD_ALERT("Moved old data");
	for (ui8_1=1; ui8_1; ui8_1++) {
	  sprintf(bufSS+LCD_MAX_COL, "%s.%d", bufSS, ui8_1);
	  if (FR_OK != f_stat(bufSS+LCD_MAX_COL, NULL))
	    break;
	}
	f_rename(bufSS, bufSS+LCD_MAX_COL);
      }
    }
  } else { /* SD not found */
    LCD_ALERT("No SD Found");
    devStatus |= DS_NO_SD;
  }
  f_mount(NULL, "", 0);

  /* Identify capability of device from serial number */
  ui16_1 = 0;
  for (ui8_1=0; ui8_1<(SERIAL_NO_MAX-2); ui8_1++) {
    ui16_1 = _crc16_update(ui16_1, eeprom_read_byte(offsetof(struct ep_store_layout, unused_serial_no)+ui8_1));
  }
  ui16_2 = eeprom_read_byte(offsetof(struct ep_store_layout, unused_serial_no)+SERIAL_NO_MAX-2);
  ui16_2 <<= 8;
  ui16_2 |= eeprom_read_byte(offsetof(struct ep_store_layout, unused_serial_no)+SERIAL_NO_MAX-1);
  /* FIXME, the CRC needs to be in the printable char set */
  if (ui16_2 == ui16_1) {
    devStatus |= DS_DEV_1K;
  } else if ( (0 == ((ui16_2 ^ (ui16_1>>8)) & 0xFF)) &&
	      (0 == ((ui16_2 ^ (ui16_1<<8)) & 0xFF00)) ) {
    devStatus |= DS_DEV_5K;
  } else if ( (0 == ((ui16_2 ^ (ui16_1>>8)) & 0xFF)) &&
	      (0 == ((ui16_2 ^ (ui16_1<<8)) & 0xFF00)) ) {
    devStatus |= DS_DEV_20K;
  } else {
    devStatus |= DS_DEV_INVALID;
  }
}

// Not unit tested
/* Indexing at the following levels
   1. Complete product code
   2. Complete name
#ifdef 4 == ITEM_SUBIDX_NAME
   3. First word of name
   4. First 3 letters of name
#endif

  Normally 5K items are needed to be supported :
    So, need 5K*4*2(bytes/sig) = 40K bytes to index it
    In this case indexing could be stored in Flash
  Medical purpose, max of 20K items :
    So, need 20K*2*2 = 80K bytes : (2 == ITEM_SUBIDX_NAME)
 */
const uint16_t itemIdxs[ITEM_MAX * ITEM_SUBIDX_NAME] PROGMEM =
  { [ 0 ... (ITEM_MAX * ITEM_SUBIDX_NAME - 1) ] = 0 };

// Not unit tested
void
menuBilling(uint8_t mode)
{
  uint8_t ui8_1, ui8_2, ui8_3, ui8_4, ui8_5;
  uint16_t ui16_1, ui16_2;
  UINT    brw;

  struct sale *sl = (void *)(bufSS+LCD_MAX_COL+2+LCD_MAX_COL+2);
  for (ui8_2=0; ui8_2<SALE_SIZEOF; ui8_2++) {
    ((uint8_t *)sl)[ui8_2] = 0;
  }

  /* memory requirements */
  //printf("%d %d\n", (SALE_SIZEOF+LCD_MAX_COL+LCD_MAX_COL+4+(EPS_MAX_VAT_CHOICE*4)), BUFSS_SIZE);
  assert((SALE_SIZEOF+LCD_MAX_COL+LCD_MAX_COL+4+(EPS_MAX_VAT_CHOICE*4)) < BUFSS_SIZE);

  /* if modification of bill is requested... */
  if (mode & MENU_MMODBILL) {
    /* FIXME: */
  }

  for (ui8_5=0; ;) {
    /* already added item, just confirm */
    if (0 != sl->items[ui8_5].quantity)
      goto menuBillingConfirm;
    /* can't take more items */
    if (ui8_5 >= MAX_ITEMS_IN_BILL) {
      ui8_5--;
      goto menuBillingConfirm;
    }
    
  menuBillingStartBilling:
    ui8_4 = 0;  /* item not found */
    ui8_2 = 1;  /* is a digit */
    ui8_3 = 0;  /* not empty */
    ui16_2 = 0; /* int value */

    /* Get arg and find if valid, str or num */
    arg1.valid = MENU_ITEM_NONE;
    arg1.value.sptr = bufSS;
    menuGetOpt(menu_str1+(MENU_STR1_IDX_ITEM*MENU_PROMPT_LEN), &arg1, MENU_ITEM_STR);
    for (ui8_1=0; ui8_1<LCD_MAX_COL; ui8_1++) {
      if (!isgraph(arg1.value.sptr[ui8_1]))
	continue;
      ui8_3 = 1;
      ui8_2 &= isdigit(arg1.value.sptr[ui8_1]);
      if (ui8_2) {
	ui16_2 *= 10;
	ui16_2 += arg1.value.sptr[ui8_1] - '0';
      }
    }

    /* Just Enter was hit, may be the user wants to proceed to billing */
    if (0 == ui8_3) {
      LCD_WR_LINE_N(0, 0, menu_str1+(MENU_STR1_IDX_FINALIZ*MENU_PROMPT_LEN), MENU_PROMPT_LEN);
      LCD_WR_LINE_N(LCD_MAX_ROW-1, 0, menu_str1+(MENU_STR1_IDX_ENTRYES*MENU_PROMPT_LEN), MENU_PROMPT_LEN);
      LCD_refresh();
      KBD_RESET_KEY;
      KBD_GETCH;
      if ( (ASCII_ENTER == KbdData) ||
	   (0 == sl->items[0].quantity) /* no items yet */ ) {
	break;
      } else {
	ui8_5--;
	goto menuBillingConfirm;
      }
    }

    for (ui16_1=0; (EEPROM_MAX_ADDRESS-ui16_1+1)>=(ITEM_SIZEOF>>EEPROM_MAX_DEVICES_LOGN2);
	 ui16_1+=(ITEM_SIZEOF>>EEPROM_MAX_DEVICES_LOGN2)) {
      ee24xx_read_bytes(ui16_1, (void *)&(sl->it[0]), ITEM_SIZEOF);
      /* invalid item */
      if ((0 == sl->it[0].id) || (sl->it[0].is_disabled))
	continue;
      //printf("id:%d ui16_2:%d\n", sl->it[0].id, ui16_2);
      if (ui8_2) { /* integer */
	if (sl->it[0].id == ui16_2)
	  break;
      } else { /* string */
	/* FIXME: Speed up search.. */
	ui8_4 = 1 /* full match */;
	for (ui8_3=0; ui8_3<ITEM_NAME_BYTEL; ui8_3++) {
	  if ( (!isgraph(arg1.value.sptr[ui8_3])) &&
	       (!isgraph(sl->it[0].name[ui8_3])) ) {
	    continue;
	  } else { /* match */
	    ui8_4 &= (toupper(arg1.value.sptr[ui8_3]) == toupper(sl->it[0].name[ui8_3])) ? 1 : 0;
	    //printf("ui8_4:%d ui8_3:%d arg1[ui8_3]:'%c' it.name:'%c'\n", ui8_4, ui8_3, arg1.value.sptr[ui8_3], sl->it[0].name[ui8_3]);
	  }
	}
	if (1 == ui8_4)
	  break;
      }
    }
    ui8_4 = (EEPROM_MAX_ADDRESS-ui16_1+1) >= (ITEM_SIZEOF>>EEPROM_MAX_DEVICES_LOGN2);
    //printf("chars:%d match:%d\n", ui8_3, ui8_4);

    if (1 != ui8_4) continue; /* match not found */

    /* common inputs */
    sl->items[ui8_5].ep_item_ptr = ui16_1;
    sl->items[ui8_5].cost = sl->it[0].cost;
    sl->items[ui8_5].discount = sl->it[0].discount;
    sl->items[ui8_5].has_serv_tax = sl->it[0].has_serv_tax;
    sl->items[ui8_5].has_common_discount = sl->it[0].has_common_discount;
    sl->items[ui8_5].vat_sel = sl->it[0].vat_sel;
    do {
      arg2.valid = MENU_ITEM_NONE;
      menuGetOpt(menu_str1+(MENU_STR1_IDX_SALEQTY*MENU_PROMPT_LEN), &arg2, MENU_ITEM_FLOAT);
      sl->items[ui8_5].quantity = arg2.value.integer.i16;
    } while (MENU_ITEM_NONE == arg2.valid);

  menuBillingConfirm:
    /* Enable edit of earlier added item  */
    do {
      /* Display item for confirmation */
      LCD_WR_LINE_N(0, 0, menu_str1+(MENU_STR1_IDX_CONFI*MENU_PROMPT_LEN), MENU_PROMPT_LEN);
      LCD_WR_LINE_N_EE24XX(LCD_MAX_ROW-1, 0, ((sl->items[ui8_5].ep_item_ptr)+(offsetof(struct item, name)>>EEPROM_MAX_DEVICES_LOGN2)), 7);
      LCD_WR_SPRINTF(LCD_MAX_ROW-1, 8, bufSS, "%d", sl->items[ui8_5].quantity);
      LCD_refresh();

      KBD_RESET_KEY;
      KBD_GETCH;
      if (ASCII_ENTER == KbdData) {
	ui8_5++;
      } else if ( (ASCII_LEFT == KbdData) ||
		(ASCII_RIGHT == KbdData) ) {
	/* FIXME: add default values from item data */
	do {
	  arg2.valid = MENU_ITEM_NONE;
	  menuGetOpt(menu_str1+(MENU_STR1_IDX_PRICE*MENU_PROMPT_LEN), &arg2, MENU_ITEM_FLOAT);
	  sl->items[ui8_5].cost = arg2.value.integer.i16;
	} while (MENU_ITEM_NONE == arg2.valid);
	do {
	  arg2.valid = MENU_ITEM_NONE;
	  menuGetOpt(menu_str1+(MENU_STR1_IDX_DISCO*MENU_PROMPT_LEN), &arg2, MENU_ITEM_FLOAT);
	  sl->items[ui8_5].discount = arg2.value.integer.i16;
	} while (MENU_ITEM_NONE == arg2.valid);
	LCD_CLRSCR;
	sl->items[ui8_5].has_serv_tax = menuGetYesNo(menu_str1+(MENU_STR1_IDX_S_TAX*MENU_PROMPT_LEN), MENU_PROMPT_LEN);
	sl->items[ui8_5].has_common_discount = menuGetYesNo(menu_str1+(MENU_STR1_IDX_COMNDISC*MENU_PROMPT_LEN), MENU_PROMPT_LEN);

	/* vat */
	for (ui8_4=0; ui8_4<EPS_MAX_VAT_CHOICE; ui8_4++) {
	  ui16_2 = eeprom_read_word((uint16_t *)(offsetof(struct ep_store_layout, Vat) + (sizeof(uint16_t)*ui8_4)));
	  for (ui8_2=0; ui8_2<4; ui8_2++) {
	    (bufSS+SALE_SIZEOF+(ui8_4*4))[ui8_2] = '0' + ui16_2%10;
	    ui16_2 /= 10;
	  }
	}
	sl->items[ui8_5].vat_sel = menuGetChoice(menu_str1+(MENU_STR1_IDX_VAT*MENU_PROMPT_LEN), (bufSS+SALE_SIZEOF), 4, EPS_MAX_VAT_CHOICE);

	do {
	  arg2.valid = MENU_ITEM_NONE;
	  menuGetOpt(menu_str1+(MENU_STR1_IDX_SALEQTY*MENU_PROMPT_LEN), &arg2, MENU_ITEM_FLOAT);
	  sl->items[ui8_5].quantity = arg2.value.integer.i16;
	} while (MENU_ITEM_NONE == arg2.valid);
      } else if ( (ASCII_UP == KbdData) ||
		  (ASCII_DOWN == KbdData) ) {
	if (ASCII_UP == KbdData) {
	  ui8_5 ? --ui8_5 : 0;
	} else {
	  if ( (ui8_5 < MAX_ITEMS_IN_BILL) &&
	       (sl->items[ui8_5].quantity > 0) )
	    ui8_5++;
	}
	ee24xx_read_bytes(sl->items[ui8_5].ep_item_ptr, (void *)&(sl->it[0]), ITEM_SIZEOF);
      } else if (ASCII_DEL == KbdData) {
	for (ui8_2=ui8_5; sl->items[ui8_2+1].quantity>0; ui8_2++) {
	  memcpy(&(sl->items[ui8_2]), &(sl->items[ui8_2+1]), ITEM_SIZEOF);
	}
	sl->items[ui8_2].quantity = 0;
      } else
	continue;
    } while (0);

  }

  /* Why would somebody make a 0 item bill? */
  for (; sl->items[ui8_5].quantity>0; ui8_5++) ;
  if (0 == ui8_5) {
    LCD_ALERT("No bill items");
    return;
  }

  /* Calculate bill, confirm */
  uint32_t ui32_1, ui32_2;
  sl->t_stax = 0, sl->t_vat = 0, sl->t_discount = 0, sl->total = 0;
  ui32_2 = 0;
  for (ui8_3=0; ui8_3<ui8_5; ui8_3++) {
    ui32_1 = sl->items[ui8_3].cost > sl->items[ui8_3].discount ?
      (sl->items[ui8_3].cost - sl->items[ui8_3].discount) :
      sl->items[ui8_3].cost;
    if (0 != sl->items[ui8_3].discount) {
      sl->t_discount += sl->items[ui8_3].cost - ui32_1;
    } else if (sl->items[ui8_3].has_common_discount) {
      ui32_2 = eeprom_read_byte((void *)offsetof(struct ep_store_layout, ComnDis));
      ui32_2 <<= 8;
      ui32_2 |= eeprom_read_byte((void *)offsetof(struct ep_store_layout, ComnDis) + 1);
      ui32_2 = ((10000 - ui32_2) * ui32_1) / 100;
      /* ui32_1 = FIXME: complete this */
      sl->t_discount += ui32_2;
      ui32_1 -= ui32_2;
    }
    ui32_1 *= sl->items[ui8_3].quantity;

    sl->total += ui32_1;

    if (sl->items[ui8_3].has_serv_tax) {
      ui32_2 = eeprom_read_byte((void *)offsetof(struct ep_store_layout, ServTax));
      ui32_2 <<= 8;
      ui32_2 |= eeprom_read_byte((void *)offsetof(struct ep_store_layout, ServTax) + 1);
      ui32_2 *= ui32_1;
      ui32_2 /= 100;
      sl->t_stax += ui32_2;
      sl->total  += ui32_2;
    } else if (sl->items[ui8_3].vat_sel < EPS_MAX_VAT_CHOICE) {
      ui32_2 = eeprom_read_byte((void *)offsetof(struct ep_store_layout, Vat) + (sizeof(uint16_t)*sl->items[ui8_3].vat_sel));
      ui32_2 <<= 8;
      ui32_2 |= eeprom_read_byte((void *)offsetof(struct ep_store_layout, Vat) + (sizeof(uint16_t)*sl->items[ui8_3].vat_sel) + 1);
      ui32_2 *= ui32_1;
      ui32_2 /= 100;
      sl->t_vat += ui32_2;
      sl->total  += ui32_2;
    }
  }

  /* set sale-info */
  sl->info.n_items = ui8_5;
  ui32_2 = get_fattime();
  sl->info.date_dd = ((ui32_2>>FAT_DATE_OFFSET)&FAT_DATE_MASK)+1;
  sl->info.date_mm = ((ui32_2>>FAT_MONTH_OFFSET)&FAT_MONTH_MASK)+1;
  sl->info.date_yy = ((ui32_2>>FAT_YEAR_OFFSET)&FAT_YEAR_MASK);
  sprintf( bufSS, "sale_%02d_%02d_%04d.dat", ((ui32_2>>FAT_DATE_OFFSET)&FAT_DATE_MASK)+1,
	   ((ui32_2>>FAT_MONTH_OFFSET)&FAT_MONTH_MASK)+1, ((ui32_2>>FAT_YEAR_OFFSET)&FAT_YEAR_MASK)+1980 );
  sl->info.time_hh = ((ui32_2>>FAT_HOUR_OFFSET)&FAT_HOUR_MASK);
  sl->info.time_mm = ((ui32_2>>FAT_MIN_OFFSET)&FAT_MIN_MASK);
  sl->info.time_ss = ((ui32_2>>FAT_SEC_OFFSET)&FAT_SEC_MASK);
  for (ui8_2=0; ui8_2<EPS_MAX_UNAME; ui8_2++)
    sl->info.user[ui8_2] = eeprom_read_byte((void *) (offsetof(struct ep_store_layout, users) + (EPS_MAX_UNAME*(LoginUserId-1)) + ui8_2));

  /* Save the bill to SD */
  FATFS FatFs1;
  FIL Fil;
  UINT ret_val;
  memset(&FatFs1, 0, sizeof(FatFs1));
  memset(&Fil, 0, sizeof(FIL));
  //  change_sd(0); /* FIXME: */
  f_mount(&FatFs1, "", 0);		/* Give a work area to the default drive */
  if (f_open(&Fil, bufSS, FA_WRITE|FA_CREATE_ALWAYS) == FR_OK) {	/* Create a file */
    /* Move to end of the file to append data */
    ui32_2 = f_size(&Fil);
    if (0 != ui32_2) {    /* file is new */
      assert( 2 == (ui32_2 %
		    (SALE_SIZEOF+((MAX_ITEMS_IN_BILL-1)*ITEM_SIZEOF)+2)) );
      /* If version doesn't match, escape... */
      f_read(&Fil, bufSS, 2, &ret_val);
      assert(2 == ret_val);
      ui16_2 = bufSS[0]; ui16_2 <<= 8; ui16_2 |= bufSS[1];
      if (GIT_HASH_CRC != ui16_2) {
	LCD_ALERT("Del old files");
      }
      /* */
      f_lseek(&Fil, ui32_2-2);
      f_read(&Fil, bufSS, 2, &ret_val);
      assert(2 == ret_val);
      ui16_2 = bufSS[0]; ui16_2 <<= 8; ui16_2 |= bufSS[1];
    } else {     /* file already exists */
      /* new file, set it up ... */
      ui16_2 = 0;
      ui16_2 = _crc16_update(ui16_2, (GIT_HASH_CRC>>8)&0xFF);
      ui16_2 = _crc16_update(ui16_2, GIT_HASH_CRC&0xFF);
      bufSS[0] = GIT_HASH_CRC>>8;
      bufSS[1] = GIT_HASH_CRC&0xFF;
      f_write(&Fil, (void *)bufSS, 2, &ret_val);
      assert(2 == ret_val);
    }

    /* now save the data */
    f_write(&Fil, (void *)sl, SALE_SIZEOF-ITEM_SIZEOF, &ret_val);
    assert((SALE_SIZEOF-ITEM_SIZEOF) == ret_val);
    for (ui8_4=0; ui8_4<(SALE_SIZEOF-ITEM_SIZEOF); ui8_4++)
      ui16_2 = _crc16_update(ui16_2, ((uint8_t *)sl)[ui8_4]);
    /* save item */
    for (ui8_4=0; ui8_4<ui8_5; ui8_4++) {
      ee24xx_read_bytes(sl->items[ui8_4].ep_item_ptr, (void *)&(sl->it[0]), ITEM_SIZEOF);
      f_write(&Fil, &(sl->it[0]), ITEM_SIZEOF, &ret_val);
      for (ui8_3=0; ui8_3<ITEM_SIZEOF; ui8_3++)
	ui16_2 = _crc16_update(ui16_2, ((uint8_t *)&(sl->it[0]))[ui8_3]);
      assert(ITEM_SIZEOF == ret_val);
    }
    memset((void *)&(sl->it[0]), 0, ITEM_SIZEOF);
    for (; ui8_4<MAX_ITEMS_IN_BILL; ui8_4++) {
      f_write(&Fil, &(sl->it[0]), ITEM_SIZEOF, &ret_val);
      assert(ITEM_SIZEOF == ret_val);
      for (ui8_3=0; ui8_3<ITEM_SIZEOF; ui8_3++)
	ui16_2 = _crc16_update(ui16_2, 0);
    }
    /* */
    bufSS[0] = ui16_2>>8;
    bufSS[1] = ui16_2;
    f_write(&Fil, bufSS, 2, &ret_val);
    assert(2 == ret_val);
    /* */
    f_close(&Fil);
  } else
    LCD_ALERT("Can't save bill");
  f_mount(NULL, "", 0);

  /* Now print the bill */
  KBD_GETCH;
  menuPrnBill(sl);
}

void
menuAddItem(uint8_t mode)
{
  uint8_t ui8_1, ui8_2, ui8_3;
  uint16_t ui16_1, ui16_2;
  struct item *it = (void *)(bufSS+LCD_MAX_COL+2+LCD_MAX_COL+2);
  uint8_t *bufSS_ptr = (void *) it;
  uint8_t choice[EPS_MAX_VAT_CHOICE*MENU_PROMPT_LEN];

  /* conditions required to modify */
  if (mode & MENU_MODITEM) {
    if ( (MENU_PR_FLOAT != arg1.valid) || (0 == arg1.value.integer.i32) )
      goto menuItemInvalidArg;
    it->id = arg1.value.integer.i32;
  } else {
    it->id = 0;
  }

  /* Find space to place item */
  for (ui16_1=0, ui16_2=0; (EEPROM_MAX_ADDRESS-ui16_1+1)>=(ITEM_SIZEOF>>EEPROM_MAX_DEVICES_LOGN2);
       ui16_1+=(ITEM_SIZEOF>>EEPROM_MAX_DEVICES_LOGN2), ui16_2++) {
    ee24xx_read_bytes(ui16_1, (void *)it, ITEM_SIZEOF);
    if (((mode&MENU_MODITEM) ? arg1.value.integer.i16 : 0 ) == it->id) {
      //      printf("Storing at id :%d\n", ui16_2);
      break;
    }
  }
  if ( (EEPROM_MAX_ADDRESS-ui16_1+1) < (ITEM_SIZEOF>>EEPROM_MAX_DEVICES_LOGN2) ) {  /* check if we have enough space here */
    if (mode & MENU_MODITEM) {
      LCD_ALERT("Invalid ID");
    } else {
      LCD_ALERT("Items full");
    }
    return;
  }
  assert (it->id < ITEM_MAX);
  goto menuItemSaveArg;

 menuItemInvalidArg:
  LCD_ALERT("Invalid Argument");
  return;

 menuItemSaveArg:
  /* init */
  for (ui8_1=0; ui8_1<ITEM_SIZEOF; ui8_1++) {
    bufSS_ptr[ui8_1] = 0;
  }
  it->id = ui16_2+1;

  if (0 == (mode & MENU_MODITEM)) {
    assert (MENU_ITEM_STR == arg1.valid);
    for (ui8_1=0; ui8_1<ITEM_NAME_BYTEL; ui8_1++) {
      it->name[ui8_1] = isgraph(lcd_buf[LCD_MAX_ROW-1][ui8_1]) ? toupper(lcd_buf[LCD_MAX_ROW-1][ui8_1]) : ' ';
    }
  }

  /* Cost, discount */
  arg1.valid = MENU_ITEM_NONE;
  menuGetOpt(menu_str1+(MENU_STR1_IDX_PRICE*MENU_PROMPT_LEN), &arg1, MENU_ITEM_FLOAT);
  it->cost = arg1.value.integer.i32;
  arg2.valid = MENU_ITEM_NONE;
  menuGetOpt(menu_str1+(MENU_STR1_IDX_DISCO*MENU_PROMPT_LEN), &arg2, MENU_ITEM_FLOAT);
  it->discount = arg2.value.integer.i32;
  if ((MENU_ITEM_FLOAT != arg1.valid) || (MENU_ITEM_FLOAT != arg2.valid))
    goto menuItemInvalidArg;
  //printf("id:%d cost:%d discount:%d @0x%x\n", it->id, it->cost, it->discount, ui16_1);

  /* Product code, name unicode */
  arg1.valid = MENU_ITEM_NONE;
  arg1.value.sptr = bufSS;
  menuGetOpt(menu_str1+(MENU_STR1_IDX_PRODCODE*MENU_PROMPT_LEN), &arg1, MENU_ITEM_STR);
  for (ui8_1=0; ui8_1<LCD_MAX_COL; ui8_1++) {
    it->prod_code[ui8_1] = toupper(arg1.value.sptr[ui8_1]);
  }
  arg2.valid = MENU_ITEM_NONE;
  arg2.value.sptr = bufSS+LCD_MAX_COL;
  menuGetOpt(menu_str1+(MENU_STR1_IDX_UNICODE*MENU_PROMPT_LEN), &arg2, MENU_ITEM_STR);
  if ((MENU_ITEM_STR != arg1.valid) || (MENU_ITEM_STR != arg2.valid))
    goto menuItemInvalidArg;

  /* */
  for (ui8_1=0, ui8_2=0, ui8_3=0; ui8_1<LCD_MAX_COL; ui8_1++) {
    if ( (!isalnum(lcd_buf[LCD_MAX_ROW-1][ui8_1]))
	 || (!isgraph(lcd_buf[LCD_MAX_ROW-1][ui8_1])) )
      continue;
    ui8_3 <<= 4;
    ui8_3 |= ('9' <= lcd_buf[LCD_MAX_ROW-1][ui8_1]) ? (lcd_buf[LCD_MAX_ROW-1][ui8_1] - '0') :
      ( ('F' <= lcd_buf[LCD_MAX_ROW-1][ui8_1]) ? (lcd_buf[LCD_MAX_ROW-1][ui8_1]-'A'+10) :
	(lcd_buf[LCD_MAX_ROW-1][ui8_1]-'a'+10) );
    ui8_2 ++;
    if (0 == (ui8_2&1)) {
      it->name_unicode[ui8_2>>1] = ui8_3;
    }
  }

  /* vat */
  for (ui8_1=0; ui8_1<EPS_MAX_VAT_CHOICE; ui8_1++) {
    ui16_2 = eeprom_read_word((uint16_t *)(offsetof(struct ep_store_layout, Vat) + (sizeof(uint16_t)*ui8_1)));
    for (ui8_2=0; ui8_2<4; ui8_2++) {
      (choice+(ui8_1*MENU_PROMPT_LEN)+MENU_PROMPT_LEN-ui8_1)[0] = '0' + ui16_2%10;
      ui16_2 /= (uint16_t)10;
    }
  }
  it->vat_sel = menuGetChoice(menu_str1+(MENU_STR1_IDX_VAT*MENU_PROMPT_LEN), choice, MENU_PROMPT_LEN, EPS_MAX_VAT_CHOICE);

  /* choices */
  it->has_serv_tax = menuGetYesNo(menu_str1+(MENU_STR1_IDX_S_TAX*MENU_PROMPT_LEN), MENU_PROMPT_LEN);

  /* Confirm */
  if (0 != menuGetYesNo(menu_str1+(MENU_STR1_IDX_CONFI*MENU_PROMPT_LEN), MENU_PROMPT_LEN))
    return;

  /* Now save item */
//  for (ui8_3=0; ui8_3<ITEM_SIZEOF;) {
//    printf("%02x ", bufSS_ptr[ui8_3]);
//    ui8_3++;
//    if (0 == (ui8_3%16)) printf("\n");
//  }
//  printf("\n");
  ui8_3 = ee24xx_write_bytes((uint8_t *)ui16_1, bufSS_ptr, ITEM_SIZEOF);
  assert(ITEM_SIZEOF == ui8_3);

  /* other wise indexing would span across pages*/
  assert(0 == (SPM_PAGESIZE % 4));
  assert(BUFSS_SIZE >= SPM_PAGESIZE);
  
  /* indexing for faster search */
  uint8_t sreg = SREG;
  cli(); /* Disable interrupts */

  eeprom_busy_wait ();

  uint32_t addr = itemIdxs + (it->id-1);
  boot_page_erase (addr & ~((uint32_t)(SPM_PAGESIZE-1)));
  boot_spm_busy_wait ();      // Wait until the memory is erased.

  for (ui16_1=0; ui16_1<SPM_PAGESIZE; ui16_1++) {
    boot_page_fill((addr & ~((uint32_t)(SPM_PAGESIZE-1))) + ui16_1, pgm_read_mem((addr & ~((uint32_t)(SPM_PAGESIZE-1))) + ui16_1) );
  }

  if (0 != menuIndexItem(it)) {
    boot_page_write ((addr & ~((uint32_t)(SPM_PAGESIZE-1))));     // Store buffer in flash page.
  }

  boot_spm_busy_wait();       // Wait until the memory is written.

  // Reenable RWW-section again. We need this if we want to jump back
  // to the application after bootloading.
  boot_rww_enable ();

  // Re-enable interrupts (if they were ever enabled).
  SREG = sreg;
}

void
menuDelItem(uint8_t mode)
{
  uint16_t ui16_1, ui16_2;
  uint8_t  ui8_1, ui8_2;
  struct item *it = (void *)(bufSS+LCD_MAX_COL+2+LCD_MAX_COL+2);
  uint8_t *bufSS_ptr = (void *) it;

  /* check validity of argument */
  if ( (MENU_ITEM_ID != arg1.valid) || (0 == arg1.value.integer.i16) ||
       (arg1.value.integer.i16 > ITEM_MAX) ) {
    ERROR("Invalid Option");
    return;
  }

  /* find address for the id */
  ui16_2 = arg1.value.integer.i16-1;
  ui16_1 = menuItemAddr(ui16_2);
  ee24xx_read_bytes(ui16_1, (void *)it, ITEM_SIZEOF);
  if ((0 == it->id) || (it->is_disabled))
    return;

  /* delete */
  it->id = 0;
  ee24xx_write_bytes(ui16_1+(offsetof(struct item, id)>>EEPROM_MAX_DEVICES_LOGN2),
  		     bufSS_ptr+offsetof(struct item, id), 1<<EEPROM_MAX_DEVICES_LOGN2);
}

// Not unit tested
void
menuIndexAllItems()
{
  uint16_t ui16_1, ui16_2, page;
  uint8_t *buf = bufSS;
  uint8_t sreg, mods;

  // Disable interrupts.
  sreg = SREG;
  cli();
  eeprom_busy_wait();

#define MIAI_PAGE_OPENED 0x10
  assert( 0 == (SPM_PAGESIZE % (ITEM_SUBIDX_NAME*sizeof(uint16_t))) );
  mods = 0, page = 0;
  for (ui16_1=0; ui16_1<ITEM_MAX; ui16_1++) {
    /* copy page on start of buffer */
    if (!(mods & MIAI_PAGE_OPENED)) {
      for (ui16_2=0; ui16_2<SPM_PAGESIZE; ui16_2++) {
	buf[ui16_2] = pgm_read_mem(((uint8_t *)itemIdxs) + (ui16_1*ITEM_SUBIDX_NAME*sizeof(uint16_t)) + ui16_2);
      }
      mods = 0;
    }

    mods |= menuIndexItem((struct item *)menuItemAddr(ui16_1)) ? 1 : 0;

    if (0 == ((ui16_1+1)%(SPM_PAGESIZE/(ITEM_SUBIDX_NAME*sizeof(uint16_t))))) {
      if (0 != mods) {
	boot_page_erase(page);
	boot_spm_busy_wait();      // Wait until the memory is erased.
	boot_page_write(page);     // Store buffer in flash page.
	boot_spm_busy_wait();      // Wait until the memory is written.
      }
      mods &= ~MIAI_PAGE_OPENED;
      page += SPM_PAGESIZE/(ITEM_SUBIDX_NAME*sizeof(uint16_t));
    }
  }
  if (0 != mods) {
    boot_page_erase(page);
    boot_spm_busy_wait();
    boot_page_write(page);
    boot_spm_busy_wait();
  }
#undef MIAI_PAGE_OPENED

  // Reenable RWW-section again. We need this if we want to jump back
  // to the application after bootloading.
  boot_rww_enable ();

  // Re-enable interrupts (if they were ever enabled).
  SREG = sreg;
}

// Not unit tested
/* Index this one item */
uint8_t
menuIndexItem(struct item *it)
{
  uint16_t ui16_1;
  uint32_t ui32_1;
  uint8_t ui8_1, ui8_2, ret;

  /* init */
  ret = 0;
  ui32_1 = itemIdxs + (it->id-1);

  /* clear index of invalid items */
  if ((it->is_disabled) || (0 == it->id)) {
    if ( (0 != pgm_read_mem((void *)ui32_1)) ||
	 (0 != pgm_read_mem((void *)(ui32_1+1))) ) {
      boot_page_fill((void *)(ui32_1), 0);
      boot_page_fill((void *)(ui32_1+1), 0);
      ret+=2;
    }
    if ( (0 != pgm_read_mem((void *)(ui32_1+2))) ||
	 (0 != pgm_read_mem((void *)(ui32_1+3))) ) {
      boot_page_fill((void *)(ui32_1+2), 0);
      boot_page_fill((void *)(ui32_1+3), 0);
      ret+=2;
    }
    if ( (0 != pgm_read_mem((void *)(ui32_1+4))) ||
	 (0 != pgm_read_mem((void *)(ui32_1+5))) ) {
      boot_page_fill((void *)(ui32_1+4), 0);
      boot_page_fill((void *)(ui32_1+5), 0);
      ret+=2;
    }
    if ( (0 != pgm_read_mem((void *)(ui32_1+6))) ||
	 (0 != pgm_read_mem((void *)(ui32_1+7))) ) {
      boot_page_fill((void *)(ui32_1+6), 0);
      boot_page_fill((void *)(ui32_1+7), 0);
      ret+=2;
    }

    return ret;
  }

  /* */
  ui16_1 = 0;
  for (ui8_1=0; ui8_1<ITEM_PROD_CODE_BYTEL; ui8_1++)
    ui16_1 = _crc16_update(ui16_1, it->prod_code[ui8_1]);
  if ( ((ui16_1&0xFF) != pgm_read_mem((void *)(ui32_1+1))) ||
       (((ui16_1>>8)&0xFF) != pgm_read_mem((void *)(ui32_1+0))) ) {
    boot_page_fill((void *)(ui32_1), ui32_1>>8);
    boot_page_fill((void *)(ui32_1+1), ui32_1);
    ret++;
  }

  /* */
  ui8_2 = -1;
  ui16_1 = 0;
  for (ui8_1=0; ui8_1<ITEM_NAME_BYTEL; ui8_1++) {
    ui16_1 = _crc16_update(ui16_1, it->name[ui8_1]);
    if ( (-1 == ui8_2) && (ui8_1 > 0) && (' ' == it->name[ui8_1]) &&
	 (' ' != it->name[ui8_1-1]) )
      ui8_2 = ui8_1;
  }
  if ( (((ui16_1>>8)&0xFF) != pgm_read_mem((void *)(ui32_1+2))) ||
       (((ui16_1)&0xFF) != pgm_read_mem((void *)(ui32_1+3))) ) {
    boot_page_fill((void *)(ui32_1+2), ui16_1>>8);
    boot_page_fill((void *)(ui32_1+3), ui16_1);
    ret+=2;
  }
#if 4 == ITEM_SUBIDX_NAME
  /* first word of name */
  ui16_1 = 0;
  if (-1 == ui8_2) {
    /* empty or very big name */
  } else {
    /* */
    for (ui8_1=0; ui8_1<ui8_2; ui8_1++)
      ui16_1 = _crc16_update(ui16_1, it->name[ui8_1]);
  }
  if ( (((ui16_1>>8)&0xFF) != pgm_read_mem((void *)(ui32_1+4))) ||
       (((ui16_1)&0xFF) != pgm_read_mem((void *)(ui32_1+5))) ) {
    boot_page_fill((void *)(ui32_1+4), ui16_1>>8);
    boot_page_fill((void *)(ui32_1+5), ui16_1);
    ret++;
  }
  /* first 3 letters of name */
  assert(3 <=ITEM_NAME_BYTEL);
  ui16_1 = 0;
  for (ui8_1=0; ui8_1<3; ui8_1++)
    ui16_1 = _crc16_update(ui16_1, it->name[ui8_1]);
  if ( (((ui16_1>>8)&0xFF) != pgm_read_mem((void *)(ui32_1+6))) ||
       (((ui16_1)&0xFF) != pgm_read_mem((void *)(ui32_1+7))) ) {
    boot_page_fill((void *)(ui32_1+6), ui16_1>>8);
    boot_page_fill((void *)(ui32_1+7), ui16_1);
    ret++;
  }
#endif

  return ret;
}

// Not unit tested
uint16_t
menuItemFind(uint8_t *name, uint8_t *prod_code)
{
  /* FIXME: can either name or prod_code be NULL? */
  uint16_t crc_n, crc_pc, idx, ui16_1;
  uint8_t  ui8_1, ui8_2;

  /* upcase inputs */
  for (ui8_1=0; ui8_1<ITEM_NAME_BYTEL; ui8_1++) {
    name[ui8_1] = toupper(name[ui8_1]);
  }
  for (ui8_1=0; ui8_1<ITEM_PROD_CODE_BYTEL; ui8_1++) {
    prod_code[ui8_1] = toupper(prod_code[ui8_1]);
  }

  /* calculate signature indices */
  crc_n = 0; ui8_2 = -1;
  for (ui8_1=0; ui8_1<ITEM_NAME_BYTEL; ui8_1++) {
    crc_n = _crc16_update(crc_n, name[ui8_1]);
    if ((ui8_1 > 0) && (' ' == name[ui8_1]) && (' ' != name[ui8_1-1]))
      ui8_2 = ui8_1;
  }
  crc_pc = 0;
  for (ui8_1=0; ui8_1<ITEM_PROD_CODE_BYTEL; ui8_1++)
    crc_pc = _crc16_update(crc_pc, prod_code[ui8_1]);
#if 4 == ITEM_SUBIDX_NAME
  uint16_t crc_nw, crc_n3;
  crc_nw = 0;
  if (-1 != ui8_2) {
    for (ui8_1=0; ui8_1<ui8_2; ui8_1++)
      crc_nw = _crc16_update(crc_nw, name[ui8_1]);
  }
  crc_n3 = 0;
  for (ui8_1=0; ui8_1<3; ui8_1++)
    crc_n3 = _crc16_update(crc_n3, name[ui8_1]);
#endif

  for (idx=0, ui16_1=0; idx<ITEM_MAX; idx++, ui16_1+=ITEM_SIZEOF) {
    if ( (crc_n == pgm_read_mem(((uint8_t *)&(itemIdxs[menuItemIdxOff(idx)]))+0))
	 || (crc_pc == pgm_read_mem(((uint8_t *)&(itemIdxs[menuItemIdxOff(idx)]))+1))
#if 4 == ITEM_SUBIDX_NAME
	 || (crc_nw != pgm_read_mem(((uint8_t *)&(itemIdxs[menuItemIdxOff(idx)]))+2))
	 || (crc_n3 != pgm_read_mem(((uint8_t *)&(itemIdxs[menuItemIdxOff(idx)]))+3))
#endif
	 ) {
      if (NULL != name) {
	for (ui8_1=0; ui8_1<ITEM_NAME_BYTEL; ui8_1++) {
	  if (name[ui8_1] != eeprom_read_byte(ui16_1+offsetof(struct item, name)+ui8_1))
	    break;
	  if (ui8_1 == ITEM_NAME_BYTEL)
	    return idx;
	}
      } else {
	assert(NULL != prod_code);
	for (ui8_1=0; ui8_1<ITEM_PROD_CODE_BYTEL; ui8_1++) {
	  if (name[ui8_1] != eeprom_read_byte(ui16_1+offsetof(struct item, prod_code)+ui8_1))
	    break;
	  if (ui8_1 == ITEM_PROD_CODE_BYTEL)
	    return idx;
	}
      }
    }
  }

  assert(ITEM_MAX < ((uint16_t)-1));
  return -1;
}

// Not tested
void
menuPrintTestPage(uint8_t mode)
{
  PRINTER_PRINT_TEST_PAGE;
}

// Not tested
void
menuDiagPrinter(uint8_t mode)
{
  uint8_t ui8_1 = printerStatus();
  if (0 == ui8_1) {
    LCD_ALERT("Printer:No Paper");
  } else if (ui8_1 >= 60) {
    LCD_ALERT("Printer Too Hot");
  } else {
    LCD_ALERT("Printer OK");
  }
}

// Not unit tested
void
menuPrnBill(struct sale *sl)
{
  uint8_t ui8_1, ui8_2, ui8_3;

  /* Header */
  ui8_2 = 0;
  for (ui8_1=0; ui8_1<HEADER_SZ_MAX; ui8_1++) {
    ui8_3 = eeprom_read_byte((void *)(offsetof(struct ep_store_layout, b_head)+ui8_1));
    ui8_2 = ('\n' == ui8_3) ? 0 :
      ( (PRINTER_MAX_CHARS_ON_LINE == ui8_2) ? 0 : ui8_2+1 );
    if (0 == ui8_2) {
      PRINTER_PRINT('\n');
    }
    if ('\n' != ui8_3)
      PRINTER_PRINT(ui8_3);
  }

  /* Caption, user, Date */
  for (ui8_1=0; ui8_1<EPS_CAPTION_SZ_MAX; ui8_1++) {
    ui8_3 = eeprom_read_byte((void *)(offsetof(struct ep_store_layout, caption)+ui8_1));
    if ('\n' != ui8_3)
      PRINTER_PRINT(ui8_3);
  }
  PRINTER_PRINT(' '); PRINTER_PRINT(' ');
  PRINTER_PRINT('u'); PRINTER_PRINT('s'); PRINTER_PRINT('e');
  PRINTER_PRINT('r'); PRINTER_PRINT(':');
  for (ui8_1=0; ui8_1<EPS_MAX_UNAME; ui8_1++) {
    ui8_3 = eeprom_read_byte((void *)(offsetof(struct ep_store_layout, users)+ui8_1));
    assert ('\n' != ui8_3);
    PRINTER_PRINT(ui8_3);
  }
  PRINTER_PRINT(' '); PRINTER_PRINT(' ');
  PRINTER_SPRINTF(bufSS, "%2d", 1+sl->info.date_dd); PRINTER_PRINT('/');
  PRINTER_SPRINTF(bufSS, "%2d", 1+sl->info.date_mm); PRINTER_PRINT('/');
  PRINTER_SPRINTF(bufSS, "%4d", 1980+sl->info.date_yy); PRINTER_PRINT(' ');
  PRINTER_SPRINTF(bufSS, "%2d", sl->info.time_hh); PRINTER_PRINT(':');
  PRINTER_SPRINTF(bufSS, "%02d", sl->info.time_mm); PRINTER_PRINT(':');
  PRINTER_SPRINTF(bufSS, "%02d", sl->info.time_ss); PRINTER_PRINT('\n');

  /* Items */
  for (ui8_1=0; ui8_1<sl->info.n_items; ui8_1++) {
    if (EEPROM_MAX_ADDRESS != sl->items[ui8_1].ep_item_ptr) {
      ee24xx_write_bytes(sl->items[ui8_1].ep_item_ptr,
			 (void *)&(sl->it[0]), ITEM_SIZEOF);
      ui8_2 = 0;
    } else {
      ui8_2 = ui8_1;
    }
    PRINTER_SPRINTF(bufSS, "%2d. ", ui8_1);
    for (ui8_3=0; ui8_3<ITEM_NAME_BYTEL; ui8_3++)
      PRINTER_PRINT(sl->it[0].name[ui8_3]);
    PRINTER_SPRINTF(bufSS, " %4d", sl->items[ui8_3].cost);
    PRINTER_SPRINTF(bufSS, "(-%4d)", sl->items[ui8_3].discount);
    PRINTER_SPRINTF(bufSS, " %4d", sl->items[ui8_3].quantity);
    PRINTER_SPRINTF(bufSS, " %6d\n", sl->items[ui8_3].cost * sl->items[ui8_3].quantity);
  }

  /* Total */
  PRINTER_SPRINTF(bufSS, "Total Discount : %.2f\n", sl->t_discount);
  PRINTER_SPRINTF(bufSS, "Total VAT      : %.2f\n", sl->t_vat);
  PRINTER_SPRINTF(bufSS, "Total Serv Tax : %.2f\n", sl->t_stax);
  PRINTER_SPRINTF(bufSS, "Bill Total (Rs): %.2f\n", sl->total);

  /* Footer */
  ui8_2 = 0;
  for (ui8_1=0; ui8_1<FOOTER_SZ_MAX; ui8_1++) {
    ui8_3 = eeprom_read_byte((void *)(offsetof(struct ep_store_layout, b_foot)+ui8_1));
    ui8_2 = ('\n' == ui8_3) ? 0 :
      ( (PRINTER_MAX_CHARS_ON_LINE == ui8_2) ? 0 : ui8_2+1 );
    if (0 == ui8_2) {
      PRINTER_PRINT('\n');
    }
    if ('\n' != ui8_3)
      PRINTER_PRINT(ui8_3);
  }
}

// Not unit tested
void
menuBillReports(uint8_t mode)
{
  FATFS FS;
  FIL   Fil;
  UINT  ret_val;
  struct sale *sl = (void *)(bufSS+LCD_MAX_COL+2+LCD_MAX_COL+2);
  uint16_t ui16_1, ui16_2;
  uint32_t ui32_1, ui32_2;
  uint8_t ui8_1;

  /* */
  memset(&FS, 0, sizeof(FS));
  memset(&Fil, 0, sizeof(Fil));
  f_mount(&FS, ".", 1);
  if (FR_OK != f_open(&Fil, SD_ITEM_FILE, FA_READ)) {
    LCD_ALERT("File open error");
    return;
  }

  /* check that dates are in order */
  if ((MENU_ITEM_DATE != arg1.valid) || (MENU_ITEM_DATE != arg2.valid))
    return;
  if ( ! ( (arg1.value.date.year < arg2.value.date.year) ||
	   ( (arg1.value.date.year == arg2.value.date.year) &&
	     (arg1.value.date.month < arg2.value.date.month) ) ||
	   ( (arg1.value.date.year == arg2.value.date.year) &&
	     (arg1.value.date.month == arg2.value.date.month) &&
	     (arg1.value.date.day < arg2.value.date.day) ) ) ) {
    LCD_ALERT("Date Wrong Order");
    return;
  }
  if ( !validDate(arg1.value.date.day, arg1.value.date.month, arg1.value.date.year) ||
       !validDate(arg2.value.date.day, arg2.value.date.month, arg2.value.date.year) ) {
    LCD_ALERT("Invalid Date");
    return;
  }

  /* If version doesn't match, escape... */
  f_read(&Fil, (bufSS+LCD_MAX_COL+2+LCD_MAX_COL+2), 2, &ret_val);
  assert(2 == ret_val);
  ui16_1 = bufSS[LCD_MAX_COL+2+LCD_MAX_COL+2];
  ui16_1 <<= 8; ui16_1 |= bufSS[LCD_MAX_COL+2+LCD_MAX_COL+2+1];
  if (GIT_HASH_CRC != ui16_2) {
    LCD_ALERT("Old Format   ");
    return;
  }

  /* Find # records */
  ui32_1 = (f_size(&Fil)-2) / (sizeof(struct sale) + ((MAX_ITEMS_IN_BILL-1)*sizeof(struct item)));
  assert (0 == ((f_size(&Fil)-2) % (sizeof(struct sale) + ((MAX_ITEMS_IN_BILL-1)*sizeof(struct item)))));

  /* iterate records */
  for (ui32_2=0, ui8_1=1; ;) {
    /* Display this item */
    f_lseek( &Fil, 2+(ui32_2*(sizeof(struct sale) + ((MAX_ITEMS_IN_BILL-1)*sizeof(struct item)))) );
    f_read(&Fil, (void *)sl, sizeof(struct sale), &ret_val);
    assert(sizeof(struct sale) == ret_val);

    /* FIXME: complete this.. */
  }

  /* */
  f_mount(NULL, "", 0);
}

// Not unit tested
void
menuShowBill(uint8_t mode)
{
  FATFS FS;
  FIL   Fil;
  UINT  ret_val;
  uint8_t  ui8_1;
  uint16_t ui16_1, ui16_2;
  uint32_t ui32_1, ui32_2;
  struct sale *sl = (void *)(bufSS+LCD_MAX_COL+2+LCD_MAX_COL+2);

  if (MENU_ITEM_STR != arg2.valid) {
    assert(MENU_ITEM_NONE == arg2.valid);
  }

  /* */
  memset(&FS, 0, sizeof(FS));
  memset(&Fil, 0, sizeof(Fil));
  f_mount(&FS, ".", 1);
  if (FR_OK != f_open(&Fil, SD_ITEM_FILE, FA_READ)) {
    LCD_ALERT("File open error");
    return;
  }

  /* If version doesn't match, escape... */
  f_read(&Fil, (bufSS+LCD_MAX_COL+2+LCD_MAX_COL+2), 2, &ret_val);
  assert(2 == ret_val);
  ui16_1 = bufSS[LCD_MAX_COL+2+LCD_MAX_COL+2];
  ui16_1 <<= 8; ui16_1 |= bufSS[LCD_MAX_COL+2+LCD_MAX_COL+2+1];
  if (GIT_HASH_CRC != ui16_2) {
    LCD_ALERT("Old Format   ");
    return;
  }

  /* Find # records */
  ui32_1 = (f_size(&Fil)-2) / (sizeof(struct sale) + ((MAX_ITEMS_IN_BILL-1)*sizeof(struct item)));
  assert (0 == ((f_size(&Fil)-2) % (sizeof(struct sale) + ((MAX_ITEMS_IN_BILL-1)*sizeof(struct item)))));

  /* iterate records */
  for (ui32_2=0, ui8_1=1; ;) {
    /* Display this item */
    f_lseek( &Fil, 2+(ui32_2*(sizeof(struct sale) + ((MAX_ITEMS_IN_BILL-1)*sizeof(struct item)))) );
    f_read(&Fil, (void *)sl, sizeof(struct sale), &ret_val);
    if (MENU_ITEM_DATE == arg1.valid) {
      if ( (sl->info.date_yy != arg1.value.date.year) ||
	   (sl->info.date_mm != arg1.value.date.month) ||
	   (sl->info.date_dd != arg1.value.date.day) ) {
	if ((0 == (ui8_1&1)) && (ui32_2>1)) { /* prev */
	  ui32_2--;
	  continue;
	} else if ((1 == (ui8_1&1)) && (ui32_2<(ui32_1-1))) { /* next */
	  ui32_2++;
	  continue;
	} else if ((ui8_1 & 0xF0) > 0x10) {
	  break;
	} else { /* not found, change dir */
	  ui8_1 += 0x10;
	  ui8_1 ^= 1;
	  continue;
	}
      }
    }
    assert(sizeof(struct sale) == ret_val);

    /* FIXME: Display bill */

    /* according to user's wish */
    KBD_RESET_KEY;
    KBD_GETCH;
    if ((ASCII_ENTER == KbdData) || (ASCII_PRNSCRN == KbdData)) {
      menuPrnBill(sl);
    } else if ( ASCII_UP == KbdData ) {
      ui8_1 = 0;
    } else if ( ASCII_DOWN == KbdData ) {
      ui8_1 = 1;
    } else if ( (ASCII_LEFT == KbdData) || (ASCII_RIGHT == KbdData) ) {
      break;
    }
  }

  /* */
  f_mount(NULL, "", 0);
}

const struct menu_vars MenuVars[] PROGMEM = { MENU_VAR_TABLE };

// Not unit tested
void
menuSettingString(uint16_t addr, const uint8_t *quest, uint8_t max_chars)
{
  uint8_t ui8_1, ui8_2, ui8_3;
  for (ui8_1 = 0; ui8_1 < max_chars;) {
    arg1.valid = MENU_ITEM_NONE;
    menuGetOpt(quest, &arg1, MENU_ITEM_STR);
    if (MENU_ITEM_STR != arg1.valid)
      continue;
    for (ui8_2=0, ui8_3=0; ui8_2<LCD_MAX_COL; ui8_2++) {
      if (!isgraph(arg1.value.sptr[ui8_3]) && !isblank(arg1.value.sptr[ui8_3]))
	break;
      if (ui8_1 >= max_chars) break;
      eeprom_update_byte((uint8_t *)(addr+ui8_1), arg1.value.sptr[ui8_3]);
      ui8_1++;
      ui8_3++;
    }
    for (;ui8_2<LCD_MAX_COL;ui8_2++) {
      if (ui8_1 >= max_chars) break;
      eeprom_update_byte((uint8_t *)(addr+ui8_1), ' ');
      ui8_1++;
    }
  }
}

// Not unit tested
void
menuSettingUint32(uint16_t addr, const uint8_t *quest)
{
  arg1.valid = MENU_ITEM_NONE;
  menuGetOpt(quest, &arg1, MENU_ITEM_ID);
  if (MENU_ITEM_ID != arg1.valid)
    assert(0);

  uint32_t val = arg1.value.integer.i32;
  eeprom_update_byte((void *)(addr+3), val>>24);
  eeprom_update_byte((void *)(addr+2), val>>16);
  eeprom_update_byte((void *)(addr+1), val>>8);
  eeprom_update_byte((void *)(addr), val);
}

// Not unit tested
void
menuSettingUint16(uint16_t addr, const uint8_t *quest)
{
  arg1.valid = MENU_ITEM_NONE;
  menuGetOpt(quest, &arg1, MENU_ITEM_ID);
  if (MENU_ITEM_ID != arg1.valid)
    assert(0);

  uint16_t val = arg1.value.integer.i16;
  eeprom_update_byte((void *)(addr+1), val>>8);
  eeprom_update_byte((void *)(addr), val);
}

// Not unit tested
void
menuSettingUint8(uint16_t addr, const uint8_t *quest)
{
  arg1.valid = MENU_ITEM_NONE;
  menuGetOpt(quest, &arg1, MENU_ITEM_ID);
  if (MENU_ITEM_ID != arg1.valid)
    assert(0);

  eeprom_update_byte((void *)(addr), arg1.value.integer.i16);
}

// Not unit tested
void
menuSettingBit(uint16_t addr, const uint8_t *quest, uint8_t size, uint8_t offset)
{
  uint8_t ui8_1, ui8_2;
  
  arg1.valid = MENU_ITEM_NONE;
  menuGetOpt(quest, &arg1, MENU_ITEM_ID);
  if (MENU_ITEM_ID != arg1.valid)
    assert(0);

  ui8_1 = eeprom_read_byte((void *)(addr));
  ui8_2 = (1<<size)-1;
  ui8_1 &= ~(ui8_2<<offset);
  ui8_1 |= (arg1.value.integer.i16 & ui8_2) << offset;
  eeprom_update_byte((void *)(addr), ui8_1);
}

// Not unit tested
void
menuSettingSet(uint8_t mode)
{
  uint8_t ui8_1, ui8_2;
  uint16_t ui16_1;

 menuSettingSetStart:
  for (ui8_1=1; ;) {
    ui8_1 %= (MENU_VARS_SIZE+1);
    if (0 == ui8_1) {
      LCD_WR_LINE_NP(LCD_MAX_COL-1, 0, PSTR("Quit ?"), sizeof("Quit ?"));
    } else {
      LCD_WR_LINE_NP(LCD_MAX_COL-1, 0, PSTR("Change "), sizeof("Change "));
      LCD_WR_LINE_NP(LCD_MAX_COL-1, sizeof("Change "), &(MenuVars[ui8_1-1].name[0]), MENU_VAR_NAME_LEN);
    }
    KBD_RESET_KEY;
    KBD_GETCH;
    if (ASCII_ENTER == KbdData) {
      break;
    } else if ((ASCII_LEFT == KbdData) || (ASCII_UP == KbdData)) {
      ui8_1 = (ui8_1 > 0) ? ui8_1-1 : MENU_VARS_SIZE;
    } else if ((ASCII_RIGHT == KbdData) || (ASCII_DOWN == KbdData)) {
      ui8_1 = (ui8_1 > MENU_VARS_SIZE) ? 0 : ui8_1+1;
    }
  }
  if (0 == ui8_1) return;

  ui8_2 = pgm_read_mem(&(MenuVars[ui8_1-1].type));
  ui16_1 = pgm_read_mem(((uint8_t *)&(MenuVars[ui8_1-1].ep_ptr))+1);
  ui8_1 <= 8;
  ui16_1 |= pgm_read_mem((uint8_t *)&(MenuVars[ui8_1-1].ep_ptr));
  switch (ui8_2) {
  case TYPE_UINT8:
    menuSettingUint8(ui16_1, &(MenuVars[ui8_1-1].name[0]));
  case TYPE_UINT16:
    menuSettingUint16(ui16_1, &(MenuVars[ui8_1-1].name[0]));
    break;
  case TYPE_UINT32:
    menuSettingUint32(ui16_1, &(MenuVars[ui8_1-1].name[0]));
    break;
  case TYPE_STRING:
    menuSettingString(ui16_1, &(MenuVars[ui8_1-1].name[0]), pgm_read_mem(&(MenuVars[ui8_1-1].size)));
    break;
  case TYPE_BIT:
    menuSettingBit(ui16_1, &(MenuVars[ui8_1-1].name[0]), pgm_read_mem(&(MenuVars[ui8_1-1].size)), pgm_read_mem(&(MenuVars[ui8_1-1].size2)));
    break;
  default:
    assert(0);
    return;
  }

  goto menuSettingSetStart;
}

// Not unit tested
// FIXME: not complete
void
menuSettingPrint(uint8_t mode)
{
  uint8_t ui8_1, ui8_2, ui8_3;
  uint16_t ui16_1;
  uint32_t ui32_1;

  for (ui8_1=0; ui8_1<MENU_VARS_SIZE; ui8_1++) {
    ui8_2 = pgm_read_mem(&(MenuVars[ui8_1].type));
    ui16_1 = pgm_read_mem(((uint8_t *)&(MenuVars[ui8_1].ep_ptr))+1);
    ui8_1 <= 8;
    ui16_1 |= pgm_read_mem((uint8_t *)&(MenuVars[ui8_1].ep_ptr));

    for (ui8_2=0; ui8_2<ITEM_NAME_BYTEL; ui8_2++)
      PRINTER_PRINT(pgm_read_mem(&(MenuVars[ui8_1].name[ui8_2])));
    PRINTER_PRINT('\t');
    if ( (TYPE_UINT8 == ui8_2) || (TYPE_UINT16 == ui8_2) ||
	 (TYPE_UINT32 == ui8_2) || (TYPE_BIT == ui8_2) ) {
      for (ui8_3=0, ui32_1=0; ui8_3<(ui8_2&0x7); ui8_3++) {
	ui32_1 <<= 8;
	//ui32_1 |= pgm_read_mem(0); /* FIXME */
      }
    } else if (TYPE_STRING == ui8_2) {
      //ui32_1 = pgm_read_mem(0); /* FIXME */
    } else {
      assert(0);
    }
  }

}

// Not unit tested
void
menuSetDateTime(uint8_t mode)
{
  if (MENU_PR_FROM_DATE == arg1.valid) {
    timerDateSet(arg1.value.date.year, arg1.value.date.month, arg1.value.date.day);
  }
  if (MENU_PR_TIME == arg2.valid) {
    timerTimeSet(arg2.value.time.hour, arg2.value.time.min);
  }
}

// Not unit tested
void
menuDelAllBill(uint8_t mode)
{
  FATFS FS;
  uint16_t ui16_1;
  uint8_t ui8_1, ui8_2;
  uint8_t *ui8_1p = (bufSS+LCD_MAX_COL+2+LCD_MAX_COL+2);

  /* init */
  PSTR2STR(BillFileName, ui8_1p, ui8_1, ui8_2);

  /* */
  memset(&FS, 0, sizeof(FS));
  f_mount(&FS, ".", 1);
  if (FR_OK == f_unlink(ui8_1p)) {
    LCD_WR_LINE_NP(LCD_MAX_COL-1, 0, PSTR(BILLFILENAME " Del'd"), sizeof(BILLFILENAME)+sizeof(" Del'd"));
    _delay_ms(1000);
  }

  /* */
  f_mount(NULL, "", 0);
}

// Not unit tested
void
menuRunDiag(uint8_t mode)
{
  uint8_t  ui2, ui3, ui4;

  /* FIXME: Verify LCD */

  /* FIXME: Verify TFT */

  /* FIXME: Adjust LCD/TFT brightness */

  /* FIXME: Run Printer : Print test page */

  /* FIXME: Verify unused EEPROM spaces : Write/readback */

  /* FIXME: Verify 24c512 */

  /* FIXME: Test timer */

  /* FIXME: Verify Flash */

  /* FIXME: Verify Keypad : Ask user to press a key and display it */

  /* FIXME: Verify Keyboard */

  /* FIXME: Verify Printer */

  /* FIXME: Diagonise Battery charging */

  /* FIXME: Check weighing machine connectivity */

  /* FIXME: Verify SD card */

  /* FIXME: Verify Clock speed */

  /* FIXME: Verify Buzzer */

  /* FIXME: Remove unimplemented line */
  menuUnimplemented(__LINE__);
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

menuMainStart:
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
    if ( (arg1.valid == menu_args[(menu_selected<<1)]) &&
	 (arg2.valid == menu_args[(menu_selected<<1)+1]) ) {
#ifdef UNIT_TEST_MENU_1
      UNIT_TEST_MENU_1(menu_selected);
#else
      (menu_handlers[menu_selected])(menu_mode[menu_selected]);
#endif
    }
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
      if (MENU_MAX <= ui8_1) {
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
      if (MENU_MAX <= ui8_1) {
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
  goto menuMainStart;
}

void
menuSDLoadItem(uint8_t mode)
{
  FATFS FS;
  FIL   fp;
  UINT  ret_size, ui1;
  uint16_t ui16_1;
  uint8_t ui8_1;

  /* */
  memset(&FS, 0, sizeof(FS));
  memset(&fp, 0, sizeof(fp));
  f_mount(&FS, ".", 1);
  if (FR_OK != f_open(&fp, SD_ITEM_FILE, FA_READ)) {
    LCD_ALERT("File open error");
    goto menuSDLoadItemExit;
  }

  /* check version string */
  if (0 == f_size(&fp)) {
    LCD_ALERT("File size error");
    goto menuSDLoadItemExit;
  } else if (FR_OK != f_read(&fp, bufSS, GIT_HASH_SMALL_LEN, &ret_size)) {
    LCD_ALERT("File error");
    goto menuSDLoadItemExit;
  } else if (GIT_HASH_SMALL_LEN != ret_size) {
    LCD_ALERT("File error ");
    goto menuSDLoadItemExit;
  }
  for (ui8_1=0; ui8_1<GIT_HASH_SMALL_LEN; ui8_1++) {
    if (GIT_HASH_SMALL[ui8_1] != bufSS[ui8_1]) {
      LCD_ALERT("Incompatible file");
      goto menuSDLoadItemExit;
    }
  }

  /* Mark all other items as deleted : (0==id) */
  for (ui16_1=0; (EEPROM_MAX_ADDRESS-ui16_1+1)>=(ITEM_SIZEOF>>EEPROM_MAX_DEVICES_LOGN2);
       ui16_1+=(ITEM_SIZEOF>>EEPROM_MAX_DEVICES_LOGN2)) {
    /* id 0 is invalid */
    ee24xx_write_bytes(ui16_1+(offsetof(struct item, id)>>EEPROM_MAX_DEVICES_LOGN2), bufSS+offsetof(struct item, id), 1<<EEPROM_MAX_DEVICES_LOGN2);
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
  }
  ret_size = bufSS[BUFSS_SIZE-2];
  ret_size <<= 8;
  ret_size |= bufSS[BUFSS_SIZE-1];
  if ((0 == ui16_1) || (ui16_1 != ret_size) || (2 != ui8_1)) {
    //    printf("ui16_1:%x ret_size:%x, ui8_1:%d\n", ui16_1, ret_size, ui8_1);
    LCD_ALERT_16N("File error ", ui16_1);
    goto menuSDLoadItemExit;
  }

  /* */
  assert(FR_OK == f_lseek(&fp, 0));
  struct item *it = (void *)bufSS;
  while (FR_OK == f_read(&fp, bufSS, ITEM_SIZEOF, &ret_size)) {
    if (ITEM_SIZEOF != ret_size) break; /* reached last */
    if (0 == it->id) continue;
    //printf("loading id:%d\n", it->id);
    ui16_1 = menuItemAddr((it->id-1));
    ee24xx_write_bytes(ui16_1, bufSS, ITEM_SIZEOF);
  }
  assert(2 == ret_size); /* crc would be pending */

 menuSDLoadItemExit:
  /* */
  f_mount(NULL, "", 0);
}

// Not unit tested
void
menuSDSaveItem(uint8_t mode)
{
  FATFS FS;
  FIL   fp;
  UINT  ret_size, ui1;
  uint16_t ui16_1, ui16_2, signature;
  struct item *it = (void *)bufSS;
  uint8_t ui8_1;

  /* init */
  signature = 0;
  memset(&FS, 0, sizeof(FS));
  memset(&fp, 0, sizeof(fp));

  /* */
  f_mount(&FS, ".", 1);
  if (FR_OK != f_open(&fp, SD_ITEM_FILE, FA_WRITE)) {
    LCD_ALERT("File open error");
    goto menuSDSaveItemExit;
  }

  /* Add version string */
  sprintf(bufSS, "%" GIT_HASH_SMALL_LEN_STR "s", GIT_HASH_SMALL);
  f_write(&fp, bufSS, GIT_HASH_SMALL_LEN, &ret_size);
  for (ui8_1=0; ui8_1<GIT_HASH_SMALL_LEN; ui8_1++)
    signature = _crc16_update(signature, bufSS[ui8_1]);
  assert(GIT_HASH_SMALL_LEN == ret_size);

  /* */
  for (ui16_1=0; (EEPROM_MAX_ADDRESS-ui16_1+1)>=(ITEM_SIZEOF>>EEPROM_MAX_DEVICES_LOGN2);
       ui16_1+=(ITEM_SIZEOF>>EEPROM_MAX_DEVICES_LOGN2)) {
    ee24xx_read_bytes(ui16_1, bufSS, ITEM_SIZEOF);
    /* fix name, prod_code to uc */
    for (ui8_1=0; ui8_1<ITEM_NAME_BYTEL; ui8_1++) {
      it->name[ui8_1] = toupper(it->name[ui8_1]);
    }
    for (ui8_1=0; ui8_1<ITEM_PROD_CODE_BYTEL; ui8_1++) {
      it->prod_code[ui8_1] = toupper(it->prod_code[ui8_1]);
    }

    /* */
    if ((0 != it->id) && (! it->is_disabled)) {
      for (ui16_2=0; ui16_2<ITEM_SIZEOF; ui16_2+=ret_size) {
	f_write(&fp, bufSS+ui16_2, ITEM_SIZEOF-ui16_2, &ret_size);
      }
      for (ui8_1=0; ui8_1<ITEM_SIZEOF; ui8_1++) {
	signature = _crc16_update(signature, bufSS[ui8_1]);
      }
    }
  }
  bufSS[1] = signature; signature>>=8; bufSS[0] = signature;
  f_write(&fp, bufSS, 2, &ret_size);
  assert(2 == ret_size);

 menuSDSaveItemExit:
  /* */
  f_mount(NULL, "", 0);
}

// Not unit tested
void
menuSDLoadSettings(uint8_t mode)
{
  FATFS FS;
  FIL   fp;
  UINT  ret_size, ui1;
  uint16_t ui16_1, ui16_2;
  uint8_t ui8_1;
  uint8_t serial_no[SERIAL_NO_MAX];

  /* init */
  memset(&FS, 0, sizeof(FS));
  memset(&fp, 0, sizeof(fp));

  /* */
  f_mount(&FS, ".", 1);
  if (FR_OK != f_open(&fp, SD_SETTINGS_FILE, FA_READ)) {
    LCD_ALERT("File open error");
    goto menuSDLoadSettingsExit;
  }

  /* check version string */
  if (0 == f_size(&fp)) {
    LCD_ALERT("File size error");
    goto menuSDLoadSettingsExit;
  } else if (FR_OK != f_read(&fp, bufSS, GIT_HASH_SMALL_LEN, &ret_size)) {
    LCD_ALERT("File error");
    goto menuSDLoadSettingsExit;
  } else if (GIT_HASH_SMALL_LEN != ret_size) {
    LCD_ALERT("File error ");
    goto menuSDLoadSettingsExit;
  }
  for (ui8_1=0; ui8_1<GIT_HASH_SMALL_LEN; ui8_1++) {
    if (GIT_HASH_SMALL[ui8_1] != bufSS[ui8_1]) {
      LCD_ALERT("Incompatible file");
      goto menuSDLoadSettingsExit;
    }
  }

  /* save serial number for updating later */
  for (ui8_1=0; ui8_1<SERIAL_NO_MAX; ui8_1++)
    serial_no[ui8_1] = eeprom_read_byte(offsetof(struct ep_store_layout, unused_serial_no)+ui8_1);

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
    goto menuSDLoadSettingsExit;
  }

  /* */
  assert(FR_OK == f_lseek(&fp, 0));
  ui16_1 = 0;
  while (FR_OK == f_read(&fp, bufSS, BUFSS_SIZE, &ret_size)) {
    if (BUFSS_SIZE != ret_size) break; /* reached last */
    ui16_2 = ((EP_STORE_LAYOUT_SIZEOF-ui16_1)<ret_size) ? (EP_STORE_LAYOUT_SIZEOF-ui16_1): ret_size;
    eeprom_update_block((const void *)bufSS, (void *)ui16_1, ui16_2);
    ui16_1 += ui16_2;
  }
  assert(EP_STORE_LAYOUT_SIZEOF == ui16_1); /* crc would be pending */

  /* restore serial number */
  for (ui8_1=0; ui8_1<SERIAL_NO_MAX; ui8_1++)
    eeprom_update_byte(offsetof(struct ep_store_layout, unused_serial_no)+ui8_1, serial_no[ui8_1]);
  
 menuSDLoadSettingsExit:
  /* */
  f_close(&fp);
  f_mount(NULL, "", 0);
}

// Not unit tested
void
menuSDSaveSettings(uint8_t mode)
{
  FATFS FS;
  FIL   fp;
  UINT  ret_size, ui1;
  uint16_t ui16_1, ui16_2, ui16_3, signature;
  uint8_t ui8_1;

  /* init */
  signature = 0;
  memset(&FS, 0, sizeof(FS));
  memset(&fp, 0, sizeof(fp));

  /* */
  f_mount(&FS, ".", 1);
  if (FR_OK != f_open(&fp, SD_SETTINGS_FILE, FA_WRITE)) {
    LCD_ALERT("File open error");
    goto menuSDSaveSettingsExit;
  }

  /* Add version string */
  sprintf(bufSS, "%" GIT_HASH_SMALL_LEN_STR "s", GIT_HASH_SMALL);
  f_write(&fp, bufSS, GIT_HASH_SMALL_LEN, &ret_size);
  for (ui8_1=0; ui8_1<GIT_HASH_SMALL_LEN; ui8_1++)
    signature = _crc16_update(signature, bufSS[ui8_1]);
  assert(GIT_HASH_SMALL_LEN == ret_size);

  /* */
  for (ui16_1=0; ui16_1<EP_STORE_LAYOUT_SIZEOF; ui16_1+=ui16_3) {
    ui16_3 = (EP_STORE_LAYOUT_SIZEOF-ui16_1) > BUFSS_SIZE ? BUFSS_SIZE :
      (EP_STORE_LAYOUT_SIZEOF-ui16_1);
    eeprom_read_block((void *)bufSS, (const void *)ui16_1, ui16_3);
    for (ui16_2=0; ui16_2<ui16_3; ui16_2+=ret_size) {
      f_write(&fp, bufSS+ui16_2, ITEM_SIZEOF-ui16_2, &ret_size);
    }
    for (ui16_2=0; ui16_2<ui16_3; ui16_2++) {
      signature = _crc16_update(signature, bufSS[ui16_2]);
    }
  }
  assert(ui16_1 == EP_STORE_LAYOUT_SIZEOF);
  bufSS[1] = signature; signature>>=8; bufSS[0] = signature;
  f_write(&fp, bufSS, 2, &ret_size);
  assert(2 == ret_size);

 menuSDSaveSettingsExit:
  /* */
  f_mount(NULL, "", 0);
}
