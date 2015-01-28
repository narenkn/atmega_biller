#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/crc16.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>
#include <util/twi.h>
#include <avr/sleep.h>

#include "ep_ds.h"
#include "version.h"
#include "assert.h"
#include "lcd.h"
#include "kbd.h"
#include "ep_store.h"
#include "billing.h"
#include "i2c.h"
#include "uart.h"
#include "a1micro2mm.h"
#if FF_ENABLE
#include "ff.h"
#endif
#include "menu.h"
#include "main.h"

#define ROW_JOIN
#define COL_JOIN
#define MENU_HIER(A)
#define MENU_MODE(A)
#define MENU_NAME(A) A
#define MENU_FUNC(A)
#define ARG1(A, B)
#define ARG2(A, B)
const uint8_t menu_names[] PROGMEM = MENU_ITEMS;
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
#define MENU_STR1_IDX_VAT   3
#define MENU_STR1_IDX_CONFI 4
#define MENU_STR1_IDX_ITEM  5
#define MENU_STR1_IDX_SALEQTY  6
#define MENU_STR1_IDX_FINALIZ  7
#define MENU_STR1_IDX_PRINT  8
#define MENU_STR1_IDX_SAVE   9
#define MENU_STR1_IDX_DADAT 10
#define MENU_STR1_IDX_NAME  11
#define MENU_STR1_IDX_REPLA 12
#define MENU_STR1_IDX_DAY   13
#define MENU_STR1_IDX_MONTH 14
#define MENU_STR1_IDX_YEAR  15
#define MENU_STR1_IDX_PRODCODE  16
#define MENU_STR1_IDX_UNICODE   17
#define MENU_STR1_IDX_ENTRYES   18
#define MENU_STR1_IDX_COMNDISC  19
#define MENU_STR1_IDX_NUM_ITEMS 20
const uint8_t menu_str1[] PROGMEM =
  "Price   " /* 0 */
  "Discount" /* 1 */
  "Serv.Tax" /* 2 */
  "Vat     " /* 3 */
  "Confirm?" /* 4 */
  "Item/id " /* 5 */
  "Sale Qty" /* 6 */
  "Finaliz?" /* 7 */
  "Print?  " /* 8 */
  "Save?   " /* 9 */
  "Delete? " /*10 */
  "Name    " /*11 */
  "Replace?" /*12 */
  "Date    " /*13 */
  "Month   " /*14 */
  "Year    " /*15 */
  "ProdCode" /*16 */
  "Unicode " /*17 */
  "Entr:Yes" /*18 */
  "ComnDisc" /*19 */
  ;

/* */
const uint8_t BillFileName[] PROGMEM = SD_BILLING_FILE;

/* */
static uint8_t MenuMode = MENU_MRESET;
static uint8_t LoginUserId = 0; /* 0 is invalid */
uint8_t devStatus = 0;   /* 0 is no err */

/* data struct for FF */
#if FF_ENABLE
FATFS FS;
FIL   Fil;
#endif

/* Diagnosis */
uint16_t diagStatus;

/* Helper routine to obtain input from user */
void
menuGetOpt(const uint8_t *prompt, menu_arg_t *arg, uint8_t opt)
{
  uint8_t item_type = (opt & MENU_ITEM_TYPE_MASK);
  uint32_t val = 0;
  uint8_t ui8_1, ui8_2;
  uint8_t sbuf[LCD_MAX_COL], *buf, buf_idx;

  if (MENU_ITEM_NONE == opt) return;

  /* init */
  if (MENU_ITEM_STR == item_type) {
    assert(0 != arg->value.str.len);
    for (ui8_1=0; ui8_1<arg->value.str.len; ui8_1++)
      arg->value.str.sptr[ui8_1] = ' ';
    buf = arg->value.str.sptr;
  } else {
    for (ui8_1=0; ui8_1<LCD_MAX_COL; ui8_1++)
      sbuf[ui8_1] = ' ';
    buf = sbuf;
  }
  buf_idx = 0;

  /* Get a string */
  do {
    /* Ask a question */
    LCD_CLRLINE(LCD_MAX_ROW-1);
    LCD_WR_NP((const uint8_t *)prompt, MENU_PROMPT_LEN);
    LCD_PUTCH('?');
    for (ui8_1=MENU_PROMPT_LEN+1,
	   ui8_2=(buf_idx<(LCD_MAX_COL-MENU_PROMPT_LEN)) ? 0 : buf_idx-(LCD_MAX_COL-MENU_PROMPT_LEN-1);
	 (ui8_1<LCD_MAX_COL) && (ui8_2<buf_idx); ui8_1++, ui8_2++) {
      if (opt&MENU_ITEM_PASSWD) {
	LCD_PUTCH('*');
      } else {
	LCD_PUTCH(buf[ui8_2]);
      }
    }
    LCD_refresh();

    /* input */
    KBD_RESET_KEY;
    KBD_GETCH;

    switch (keyHitData.KbdData) {
    case ASCII_BACKSPACE:
    case ASCII_LEFT:
    case ASCII_UP:
      if (buf_idx) {
	buf_idx--;
	buf[buf_idx] = ' ';
	LCD_cmd(LCD_CMD_DEC_CUR);
      }
      break;
    case ASCII_LF:
    case ASCII_ENTER:
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
      buf[buf_idx] = keyHitData.KbdData;
      /* Don't overflow buffer */
      if (MENU_ITEM_STR == item_type) {
	if (buf_idx < arg->value.str.len) {
	  buf_idx++;
	}
      } else {
	if (buf_idx < (LCD_MAX_COL-1)) {
	  buf_idx++;
	}
      }
    }
  } while (keyHitData.KbdData != ASCII_ENTER);

  menu_error = 1;
  if ((MENU_ITEM_ID == item_type) || (MENU_ITEM_FLOAT == item_type)) {
    for (ui8_1=0; ui8_1<buf_idx; ui8_1++) {
      if ((buf[ui8_1] >= '0') && (buf[ui8_1] <= '9')) {
	val *= 10;
	val += buf[ui8_1] - '0';
	menu_error = 0;
      } else {
	break;
      }
    }
    arg->value.integer.i8  = val&0xFF;
    arg->value.integer.i16 = val;
    arg->value.integer.i32 = val;
  } else if ((MENU_ITEM_DATE == item_type) || (MENU_ITEM_MONTH == item_type)) {
    /* format DDMMYYYY || format MMYYYY */
    menu_error = 0;
    for (ui8_1=0; ui8_1<(item_type+2); ui8_1++) {
      if ((buf[ui8_1] < '0') || (buf[ui8_1] > '9'))
	menu_error++;
    }
    if (0 == menu_error) {
      /* Date */
      if (MENU_ITEM_DATE == item_type) {
	for (ui8_1=0; ui8_1<2; ui8_1++) {
	  val *= 10;
	  val += buf[0] - '0';
	  buf++;
	}
	if ((0 == val) || (val > 31)) menu_error++;
	arg->value.date.day = val;
      }
      /* Month */
      val = 0;
      for (ui8_1=0; ui8_1<2; ui8_1++) {
	val *= 10;
	val += buf[0] - '0';
	buf++;
      }
      if ((0 == val) || (val > 12)) menu_error++;
      arg->value.date.month = val;
      /* Year */
      val = 0;
      for (ui8_1=0; ui8_1<4; ui8_1++) {
	val *= 10;
	val += buf[0] - '0';
	buf++;
      }
      if (val < 2014) menu_error++;
      arg->value.date.year = val-1980;
    }
  } else if (MENU_ITEM_TIME == item_type) {
    /* format HHMM */
    menu_error = 0;
    for (ui8_1=0; ui8_1<4; ui8_1++) {
      if ((buf[ui8_1] < '0') || (buf[ui8_1] > '9'))
	menu_error++;
    }
    if (0 == menu_error) {
      /* Hour */
      for (ui8_1=0; ui8_1<2; ui8_1++) {
	val *= 10;
	val += buf[0] - '0';
	buf++;
      }
      if (val>23) menu_error++;
      arg->value.time.hour = val;
      /* Mins */
      val = 0;
      for (ui8_1=0; ui8_1<2; ui8_1++) {
	val *= 10;
	val += buf[0] - '0';
	buf++;
      }
      if (val > 59) menu_error++;
      arg->value.time.min = val;
    }
  } else if (MENU_ITEM_STR == item_type) {
    menu_error = 0;
  } else assert(0);

  /* */
  if (0 == menu_error) {
    arg->valid = item_type;
  } else if (opt & MENU_ITEM_OPTIONAL) {
  } else if ((opt & MENU_ITEM_DONTCARE_ON_PREV) && (arg == &arg2)) {
  } else {
    arg->valid = MENU_ITEM_NONE;
  }
}

const uint8_t menu_str2[] PROGMEM = "Yes\0No ";

uint8_t
menuGetYesNo(const uint8_t *quest, uint8_t size)
{
  uint8_t ret;

  size %= 12; /* 5 bytes for :Yes? */
  assert(size);
  for (ret=0; ;) {
    ret &= 1;
    LCD_CLRLINE(LCD_MAX_ROW-1);
    LCD_WR_NP((const uint8_t *)quest, size);
    LCD_PUTCH(':');
    LCD_WR_P((const uint8_t *)menu_str2+((ret)*3));
    LCD_PUTCH('?');
    LCD_refresh();

    KBD_RESET_KEY;
    KBD_GETCH;

    if ((ASCII_RIGHT == keyHitData.KbdData) || (ASCII_DOWN == keyHitData.KbdData)) {
      ret++;
    } else if ((ASCII_LEFT == keyHitData.KbdData) || (ASCII_UP == keyHitData.KbdData)) {
      ret--;
    } else if (ASCII_ENTER == keyHitData.KbdData) {
      return ret & 1;
    } else {
      ret = 1;
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
    LCD_CLRLINE(LCD_MAX_ROW-1);
    LCD_WR_NP((const uint8_t *)quest, ui8_1);
    LCD_WR_P((const uint8_t *)PSTR(":"));
    LCD_WR_N((char *)(opt_arr+(ret*choice_len)), choice_len);
    LCD_refresh();

    KBD_RESET_KEY;
    KBD_GETCH;

    if ((ASCII_RIGHT == keyHitData.KbdData) || (ASCII_DOWN == keyHitData.KbdData)) {
      ret = ((max_idx-1)==ret) ? 0 : ret+1;
    } else if ((ASCII_LEFT == keyHitData.KbdData) || (ASCII_UP == keyHitData.KbdData)) {
      ret = (0==ret) ? max_idx-1 : ret-1;
    } else if (ASCII_ENTER == keyHitData.KbdData) {
      return ret;
    }
  } while (1);
  assert (0);
}

void
eeprom_update_byte_NP(uint16_t addr, const uint8_t *pstr, uint8_t size)
{
  uint8_t ui8_1;
  for (; size; size--) {
    ui8_1 = (0 == pstr) ? 0 : pgm_read_byte(pstr);
    if (0 == ui8_1) break;
    eeprom_update_byte((uint8_t *)addr, ui8_1);
    addr++;
    pstr++;
  }
  for (; size; size--) {
    eeprom_update_byte((uint8_t *)addr, ' ');
    addr++;
  }
}

// Not unit tested
/* Load in the factory settings */
void
menuFactorySettings(uint8_t mode)
{
  uint8_t ui8_1;
  uint16_t ui16_1;

  assert(MENU_MSUPER == MenuMode);

  /* confirm before proceeding */
  ui8_1 = menuGetYesNo((const uint8_t *)PSTR("Fact Reset"), 11);
  if (0 != ui8_1) return;
  LCD_CLRSCR;
  LCD_WR_NP((const uint8_t *)PSTR("FactRst Progress"), 16);
  LCD_refresh();
  LCD_CLRLINE(LCD_MAX_ROW-1);

  /* store & restore serial #
     Eraze all locations
  */
  for (ui16_1=0; ui16_1<SERIAL_NO_MAX; ui16_1++)
    bufSS[ui16_1] = eeprom_read_byte((uint8_t *)offsetof(struct ep_store_layout, unused_serial_no)+ui16_1);
  LCD_PUTCH('.'); LCD_refresh();
  for (ui16_1=0; ui16_1<EP_STORE_LAYOUT_SIZEOF; ui16_1++)
    eeprom_update_byte((uint8_t *)ui16_1, 0);
  LCD_PUTCH('.'); LCD_refresh();
  for (ui16_1=0; ui16_1<SERIAL_NO_MAX; ui16_1++)
    eeprom_update_byte((uint8_t *)offsetof(struct ep_store_layout, unused_serial_no)+ui16_1,
		     bufSS[ui16_1]);
  LCD_PUTCH('.'); LCD_refresh();

  /* Mark all items as deleted : (0==id) */
  for (ui8_1=0; ui8_1<ITEM_SIZEOF; ui8_1++)
    bufSS[ui8_1] = 0;
  for (ui16_1=0; ui16_1 < (ITEM_MAX_ADDR>>EEPROM_MAX_DEVICES_LOGN2);
       ui16_1 += (ITEM_SIZEOF>>EEPROM_MAX_DEVICES_LOGN2) ) {
    ee24xx_write_bytes(ui16_1+(offsetof(struct item, id)>>EEPROM_MAX_DEVICES_LOGN2),
		       bufSS+offsetof(struct item, id), EEPROM_MAX_DEVICES_LOGN2);
  }
  LCD_PUTCH('.'); LCD_refresh();

  /* */
  eeprom_update_byte_NP(offsetof(struct ep_store_layout, shop_name),
			(const uint8_t *)PSTR("Sri Ganapathy Medicals"), SHOP_NAME_SZ_MAX);
  eeprom_update_byte_NP(offsetof(struct ep_store_layout, b_head),
			(const uint8_t *)PSTR("12 Agraharam street, New Tippasandara,\n Bangalore - 52\n TIN:299007249"), HEADER_SZ_MAX);
  eeprom_update_byte_NP(offsetof(struct ep_store_layout, b_foot),
			(const uint8_t *)PSTR("Welcome & come back..."), FOOTER_SZ_MAX);
  eeprom_update_byte_NP(offsetof(struct ep_store_layout, currency),
			(const uint8_t *)PSTR("Rupees"), EPS_WORD_LEN);
  eeprom_update_byte_NP(offsetof(struct ep_store_layout, b_pfix),
			(const uint8_t *)PSTR("A000"), EPS_WORD_LEN);
  eeprom_update_byte_NP(offsetof(struct ep_store_layout, caption),
			(const uint8_t *)PSTR("Invoice"), EPS_CAPTION_SZ_MAX);
  LCD_PUTCH('.'); LCD_refresh();

  /* user names & passwd needs to be reset */
  eeprom_update_byte_NP(offsetof(struct ep_store_layout, unused_users),
			(const uint8_t *)PSTR("admin"), EPS_MAX_UNAME);
  for (ui8_1=1; ui8_1<=EPS_MAX_USERS; ui8_1++) {
    eeprom_update_byte_NP(offsetof(struct ep_store_layout, unused_users) + (EPS_MAX_UNAME*ui8_1),
			  (const uint8_t *)0, EPS_MAX_UNAME);
  }
  ui16_1 = 0;
  ui16_1 = _crc16_update(ui16_1, 'a');
  ui16_1 = _crc16_update(ui16_1, 'd');
  ui16_1 = _crc16_update(ui16_1, 'm');
  ui16_1 = _crc16_update(ui16_1, 'i');
  ui16_1 = _crc16_update(ui16_1, 'n');
  eeprom_update_word((uint16_t *)offsetof(struct ep_store_layout, unused_passwds), ui16_1);
  for (ui8_1=1; ui8_1<=EPS_MAX_USERS; ui8_1++) {
    eeprom_update_word((uint16_t *)offsetof(struct ep_store_layout, unused_passwds) + (sizeof(uint16_t)*ui8_1), 0);
  }
  LCD_PUTCH('.'); LCD_refresh();

  /* All numerical data */
  eeprom_update_word((uint16_t *)offsetof(struct ep_store_layout, RndOff), 50);

  /* */
  eeprom_update_byte((uint8_t *)offsetof(struct ep_store_layout, idle_wait), 5);
  LCD_PUTCH('.'); LCD_refresh();

  /* At the end, log out the user */
  menuUserLogout(mode|MENU_NOCONFIRM);
  LCD_PUTCH('.'); LCD_refresh();
}

void
menuUnimplemented(uint32_t line)
{
  LCD_CLRLINE(LCD_MAX_ROW-1);
  LCD_WR_NP((const uint8_t *)PSTR("unimplemented "), 14);
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
#if MENU_USER_ENABLE
  uint8_t ui8_1, ui8_2, ui8_3;
  uint16_t ui16_1;
  assert(MENU_MSUPER == MenuMode);

  /* Choose the user to replace it with */
  assert(MENU_ITEM_STR == arg1.valid);
  assert(MENU_ITEM_STR == arg2.valid);
  for (ui8_2=0; ui8_2<EPS_MAX_UNAME; ui8_2++) {
    ui8_3 = arg1.value.str.sptr[ui8_2];
    /* check alnum? */
    if ((!isalnum(ui8_3)) && (ui8_2 > 0))
      break;
    else if (!isalnum(ui8_3)) {
      LCD_ALERT(PSTR("Invalid User"));
      LCD_refresh();
      KBD_RESET_KEY;
      KBD_GETCH;
      return;
    }
  }
  /* where to replace it ? */
  for (
       ui8_2=0, ui16_1=offsetof(struct ep_store_layout, unused_users)+EPS_MAX_UNAME;
       ui8_2<(EPS_MAX_USERS*EPS_MAX_UNAME); ui8_2++, ui16_1++ ) {
    bufSS[(LCD_MAX_COL*3)+ui8_2] = eeprom_read_byte((uint8_t *)ui16_1);
  }
  ui8_2 = menuGetChoice((const uint8_t *)PSTR("Replace at?"), bufSS+(LCD_MAX_COL*3), EPS_MAX_UNAME, EPS_MAX_USERS-1) + 1;
  if (0 != menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_CONFI*MENU_PROMPT_LEN), MENU_PROMPT_LEN)) {
    LCD_ALERT(PSTR("Aborting!"));
    return;
  }
  /* Check user name is unique before accepting */
  for (ui8_3=0; ui8_3<EPS_MAX_USERS; ui8_3++) {
    if (ui8_2 == ui8_3) continue; /* skip choosen name */
    ui16_1 = offsetof(struct ep_store_layout, unused_users) + (((uint16_t)EPS_MAX_USERS) * ui8_3);
    for (ui8_1=0; ui8_1<EPS_MAX_UNAME; ui8_1++) {
      if (arg1.value.str.sptr[ui8_1] != eeprom_read_byte((uint8_t *)ui16_1+ui8_1))
	break;
    }
    if (EPS_MAX_UNAME == ui8_1) {
      LCD_ALERT(PSTR("Duplicate User"));
      return;
    }
  }

  /* modify at will */
  ui8_3 = LoginUserId;
  LoginUserId = ui8_2;
  menuSetPasswd(mode & ~MENU_MVALIDATE);
  LoginUserId = ui8_3;
  for (ui8_3=0; ui8_3<EPS_MAX_UNAME; ui8_3++) {
    eeprom_update_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_users)+((ui8_2-1)*EPS_MAX_UNAME)+ui8_3), arg1.value.str.sptr[ui8_3]);
  }
#endif
}

/* Set my password, arg1 is old passwd, arg2 is new passwd */
void
menuSetPasswd(uint8_t mode)
{
#if MENU_USER_ENABLE
  uint16_t crc_old = 0, crc_new = 0;
  uint8_t ui8_2, ui8_3, ui8_4;

  /* */
  assert(0 != LoginUserId);
  assert(MENU_MRESET != MenuMode);

  /* Compute CRC on old password, check */
  if (0 != (mode & (~MENU_MODEMASK) & MENU_MVALIDATE)) {
    assert(MENU_ITEM_STR == arg1.valid);
    for (ui8_4=0; ui8_4<arg1.value.str.len; ui8_4++) {
      ui8_2 = arg1.value.str.sptr[ui8_4];
      /* check isprintable? */
//      for (ui8_3=0; (isgraph(ui8_2)) && (ui8_3<(KCHAR_COLS*KCHAR_ROWS)); ui8_3++) {
//	if (ui8_2 == keyChars[ui8_3])
//	  break;
//      }
//      if (ui8_3 < (KCHAR_ROWS*KCHAR_COLS))
      //printf("sptr:%s ", arg1.value.str.sptr);
      if (isgraph(ui8_2)) {
	crc_old = _crc16_update(crc_old, ui8_2);
      }
    }
    //printf(" crc_computed:0x%x\n", crc_old);

    if (eeprom_read_word((uint16_t *)offsetof(struct ep_store_layout, unused_passwds[(LoginUserId-1)])) != crc_old) {
      LCD_ALERT(PSTR("Passwd Wrong!!"));
      return;
    }
  }

  /* update mine only */
  assert(MENU_ITEM_STR == arg2.valid);
  for (ui8_4=0; ui8_4<arg2.value.str.len; ui8_4++) {
    ui8_2 = arg2.value.str.sptr[ui8_4];
    /* check isprintable? */
    for (ui8_3=0; (isgraph(ui8_2)) && (ui8_3<(KCHAR_COLS*KCHAR_ROWS)); ui8_3++) {
      if (ui8_2 == keyChars[ui8_3])
	break;
    }
    if (ui8_3 < (KCHAR_ROWS*KCHAR_COLS))
      crc_new = _crc16_update(crc_new, ui8_2);
  }
  //printf("updating crc:0x%x\n", crc_new);

  eeprom_update_word((uint16_t *)(offsetof(struct ep_store_layout, unused_passwds)+((LoginUserId-1)*sizeof(uint16_t))), crc_new);
  LCD_ALERT(PSTR("Passwd Updated"));
#endif
}

/* Logout an user */
void
menuUserLogout(uint8_t mode)
{
#if MENU_USER_ENABLE
  if ( (mode & MENU_NOCONFIRM) ||
       (0 == menuGetYesNo((const uint8_t *)PSTR("Logout?"), 7)) ) {
    LoginUserId = 0;
    MenuMode = MENU_MRESET;
  }
#endif
}

/* Login an user */
void
menuUserLogin(uint8_t mode)
{
#if MENU_USER_ENABLE
  uint16_t crc = 0;
  uint8_t ui2, ui3, ui4, ui5;

  assert(MENU_ITEM_STR == arg1.valid);
  assert(MENU_ITEM_STR == arg2.valid);

  for ( ui2=0; ui2<(EPS_MAX_USERS+1); ui2++ ) {
    for ( ui3=0; ui3<EPS_MAX_UNAME; ui3++ ) {
      ui4 = eeprom_read_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_users)+(ui2*EPS_MAX_UNAME)+ui3));
      if (ui4 != arg1.value.str.sptr[ui3]) {
	break;
      } else if ((0 == ui3) && (0 == ui2) && (!isgraph(ui4)))
	break;
    }
    if (EPS_MAX_UNAME == ui3)
      goto menuUserLogin_found;
  }
  LCD_ALERT(PSTR("No user"));
  LoginUserId = 0;
  return;

 menuUserLogin_found:
  assert(ui2 < (EPS_MAX_USERS+1));
  for (ui4=0; ui4<arg2.value.str.len; ui4++) {
    ui3 = arg2.value.str.sptr[ui4];
    /* check isprintable? */
    for (ui5=0; isgraph(ui3) && (ui5<(KCHAR_COLS*KCHAR_ROWS)); ui5++) {
      if (ui3 == keyChars[ui5])
	break;
    }
    if (ui5 < (KCHAR_ROWS*KCHAR_COLS))
      crc = _crc16_update(crc, ui3);
  }

  if (eeprom_read_word((uint16_t *)(offsetof(struct ep_store_layout, unused_passwds) + (ui2*sizeof(uint16_t)))) != crc) {
    LCD_ALERT(PSTR("Wrong Passwd"));
    LoginUserId = 0;
    return;
  }

  /* */
  MenuMode = (0 == ui2) ? MENU_MSUPER : MENU_MNORMAL;
  LoginUserId = ui2+1;
#endif
}

// Not unit tested
void
menuInit()
{
  uint16_t ui16_1, ui16_2;
  uint8_t ui8_1, ui8_2;
  MenuMode = MENU_MRESET;

  /* csv2dat depends on this number (ITEM_MAX/ITEM_MAX_ADDR) */
  assert(56 == ITEM_SIZEOF);
  assert ((ITEM_SIZEOF+LCD_MAX_COL+LCD_MAX_COL+4) < BUFSS_SIZE);
  assert(1 == sizeof(uint8_t));
  //  assert(sizeof(void *) == sizeof(uint16_t));
  assert(((offsetof(struct item, name)&(0xFFFF<<EEPROM_MAX_DEVICES_LOGN2))>>EEPROM_MAX_DEVICES_LOGN2) == (offsetof(struct item, name)>>EEPROM_MAX_DEVICES_LOGN2));
  assert(0 == (ITEM_SIZEOF % (1<<EEPROM_MAX_DEVICES_LOGN2)));

  /* edit eeprom for users[0] = 'admin' */
  eeprom_update_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_users)+0), 'a');
  eeprom_update_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_users)+1), 'd');
  eeprom_update_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_users)+2), 'm');
  eeprom_update_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_users)+3), 'i');
  eeprom_update_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_users)+4), 'n');
  for (ui8_1=5; ui8_1<EPS_MAX_UNAME; ui8_1++)
    eeprom_update_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_users)+ui8_1), ' ');

  /* init global vars */
  devStatus = 0;
  diagStatus = 0;

  /* Used to find if we are looping multiple times */
  ui8_2 = 0;

  /* Identify capability of device from serial number */
 menuInitIdentifyDevice:
  ui16_1 = 0;
  for (ui8_1=0; ui8_1<(SERIAL_NO_MAX-2); ui8_1++) {
    ui16_1 = _crc16_update(ui16_1, eeprom_read_byte((uint8_t *)offsetof(struct ep_store_layout, unused_serial_no)+ui8_1));
  }
  ui16_2 = eeprom_read_byte((uint8_t *)offsetof(struct ep_store_layout, unused_serial_no)+SERIAL_NO_MAX-2);
  ui16_2 <<= 8;
  ui16_2 |= eeprom_read_byte((uint8_t *)offsetof(struct ep_store_layout, unused_serial_no)+SERIAL_NO_MAX-1);
  /* the CRC needs to be in the printable char set */
  if ((0 == ui8_2) && (ui16_2 != ui16_1)) {
    /* Serial # doesn't exist, load from addr 0 */
    for (ui8_1=0; ui8_1<SERIAL_NO_MAX; ui8_1++) {
      ui8_2 = eeprom_read_byte((uint8_t *)(uint16_t)ui8_1);
      eeprom_update_byte((uint8_t *)offsetof(struct ep_store_layout, unused_serial_no)+ui8_1,
			 ui8_2);
    }

    MenuMode = MENU_MSUPER;
    menuFactorySettings(MenuMode);

    ui8_2 = 1;
    goto menuInitIdentifyDevice;
  } else if (ui16_2 == ui16_1) {
    ui8_1 = eeprom_read_byte((uint8_t *)offsetof(struct ep_store_layout, unused_serial_no)+SERIAL_NO_MAX-4);
    devStatus |= ('1' == ui8_1) ? DS_DEV_1K : ('5' == ui8_1) ? DS_DEV_5K :
      ('2' == ui8_1) ? DS_DEV_20K : DS_DEV_INVALID;
  } else {
    devStatus |= DS_DEV_INVALID;
  }

#if FF_ENABLE
  if (0 == (devStatus & DS_DEV_INVALID)) {
    /* When start, check billing.csv for proper version and move it to _x.old files if wrong version */
    UINT ret_val;
    memset(&FS, 0, sizeof(FS));
    memset(&Fil, 0, sizeof(Fil));
    //  change_sd(0); /* FIXME: */
    PSTR2STR(BillFileName, bufSS, ui8_1, ui8_2);
    if ( (FR_OK == f_mount(&FS, ".", 1)) &&
	 (f_open(&Fil, (char *)bufSS, FA_WRITE|FA_CREATE_ALWAYS) == FR_OK) ) {
      if (f_size(&Fil) > 0) { /* check version */
	f_read(&Fil, bufSS, 2, &ret_val);
	assert(2 == ret_val);
	ui16_1 = bufSS[0]; ui16_1 <<= 8; ui16_1 |= bufSS[1];
	if (GIT_HASH_CRC != ui16_1) {
	  LCD_ALERT(PSTR("Moved old data"));
	  for (ui8_1=1; ui8_1; ui8_1++) {
	    sprintf((char *)bufSS+LCD_MAX_COL, "%s.%d", bufSS, ui8_1);
	    if (FR_OK != FR_OK/*FIXME: f_stat(bufSS+LCD_MAX_COL, NULL)*/)
	      break;
	  }
	  /*FIXME: f_rename(bufSS, bufSS+LCD_MAX_COL);*/
	}
      }
    } else { /* SD not found */
      LCD_ALERT(PSTR("No SD Found"));
      devStatus |= DS_NO_SD;
    }
    f_mount(NULL, "", 0);
  }
#endif

#if !MENU_USER_ENABLE
  LoginUserId = 1;
  MenuMode = MENU_MSUPER;
#endif
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
typedef uint16_t itemIdxs_t[ITEM_SUBIDX_NAME];

// Not unit tested
void
menuBilling(uint8_t mode)
{
  uint8_t ui8_1, ui8_2, ui8_3, ui8_4, ui8_5;
  uint16_t ui16_1, ui16_2;

  struct sale *sl = (void *)(bufSS+LCD_MAX_COL+2+LCD_MAX_COL+2);
  for (ui8_2=0; ui8_2<SALE_SIZEOF; ui8_2++) {
    ((uint8_t *)sl)[ui8_2] = 0;
  }

  /* memory requirements */
  //printf("%d %d\n", (SALE_SIZEOF+LCD_MAX_COL+LCD_MAX_COL+4+(EPS_MAX_VAT_CHOICE*4)), BUFSS_SIZE);
  assert((SALE_SIZEOF+LCD_MAX_COL+LCD_MAX_COL+4+(EPS_MAX_VAT_CHOICE*4)) <= BUFSS_SIZE);

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
    arg1.value.str.sptr = bufSS;
    arg1.value.str.len = LCD_MAX_COL;
    menuGetOpt(menu_str1+(MENU_STR1_IDX_ITEM*MENU_PROMPT_LEN), &arg1, MENU_ITEM_STR);
    for (ui8_1=0; ui8_1<LCD_MAX_COL; ui8_1++) {
      if (!isgraph(arg1.value.str.sptr[ui8_1]))
	continue;
      ui8_3 = 1;
      ui8_2 &= isdigit(arg1.value.str.sptr[ui8_1]);
      if (ui8_2) {
	ui16_2 *= 10;
	ui16_2 += arg1.value.str.sptr[ui8_1] - '0';
      }
    }

    /* Just Enter was hit, may be the user wants to proceed to billing */
    if (0 == ui8_3) {
      LCD_CLRLINE(0);
      LCD_WR_NP((const uint8_t *)menu_str1+(MENU_STR1_IDX_FINALIZ*MENU_PROMPT_LEN), MENU_PROMPT_LEN);
      LCD_CLRLINE(LCD_MAX_ROW-1);
      LCD_WR_NP((const uint8_t *)menu_str1+(MENU_STR1_IDX_ENTRYES*MENU_PROMPT_LEN), MENU_PROMPT_LEN);
      LCD_refresh();
      KBD_RESET_KEY;
      KBD_GETCH;
      if ( (ASCII_ENTER == keyHitData.KbdData) ||
	   (0 == sl->items[0].quantity) /* no items yet */ ) {
	break;
      } else {
	ui8_5--;
	goto menuBillingConfirm;
      }
    }

    /* Find item */
    ui16_1 = menuItemFind(arg1.value.str.sptr, arg1.value.str.sptr, sl->it, 0);
    if (0 == ui16_1) {
      LCD_CLRLINE(LCD_MAX_ROW-1);
      LCD_WR_NP((const uint8_t *)PSTR("Invalid Item"), 12);
      _delay_ms(1000);
      continue; /* match not found */
    }

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
      LCD_CLRLINE(0);
      LCD_WR_NP((const uint8_t *)menu_str1+(MENU_STR1_IDX_CONFI*MENU_PROMPT_LEN), MENU_PROMPT_LEN);
      ee24xx_read_bytes((uint16_t)(((sl->items[ui8_5].ep_item_ptr)+(offsetof(struct item, name)>>EEPROM_MAX_DEVICES_LOGN2))), bufSS, 7);
      bufSS[8] = 0;
      LCD_WR((char *)bufSS);
      sprintf((char *)bufSS, "%d", sl->items[ui8_5].quantity);
      LCD_WR((char *)bufSS);
      LCD_refresh();

      KBD_RESET_KEY;
      KBD_GETCH;
      if (ASCII_ENTER == keyHitData.KbdData) {
	ui8_5++;
      } else if ( (ASCII_LEFT == keyHitData.KbdData) ||
		(ASCII_RIGHT == keyHitData.KbdData) ) {
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
	sl->items[ui8_5].has_serv_tax = menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_S_TAX*MENU_PROMPT_LEN), MENU_PROMPT_LEN);
	sl->items[ui8_5].has_common_discount = menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_COMNDISC*MENU_PROMPT_LEN), MENU_PROMPT_LEN);

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
      } else if ( (ASCII_UP == keyHitData.KbdData) ||
		  (ASCII_DOWN == keyHitData.KbdData) ) {
	if (ASCII_UP == keyHitData.KbdData) {
	  ui8_5 ? --ui8_5 : 0;
	} else {
	  if ( (ui8_5 < MAX_ITEMS_IN_BILL) &&
	       (sl->items[ui8_5].quantity > 0) )
	    ui8_5++;
	}
	ee24xx_read_bytes(sl->items[ui8_5].ep_item_ptr, (void *)&(sl->it[0]), ITEM_SIZEOF);
      } else if (ASCII_DEL == keyHitData.KbdData) {
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
    LCD_ALERT(PSTR("No bill items"));
    return;
  }

  /* Calculate bill, confirm */
  uint32_t ui32_1=0, ui32_2;
  sl->t_stax = 0, sl->t_vat = 0, sl->t_discount = 0, sl->total = 0;
  ui32_2 = 0;
  for (ui8_3=0; ui8_3<ui8_5; ui8_3++) {
    if (sl->items[ui8_3].cost > sl->items[ui8_3].discount) {
      ui32_1 = (sl->items[ui8_3].cost - sl->items[ui8_3].discount);
      sl->t_discount += sl->items[ui8_3].discount;
    } else if (sl->items[ui8_3].has_common_discount) {
      ui32_1 = eeprom_read_byte((uint8_t *)offsetof(struct ep_store_layout, ComnDis));
      ui32_1 <<= 8;
      ui32_1 |= eeprom_read_byte((uint8_t *)(offsetof(struct ep_store_layout, ComnDis) + 1));
      ui32_1 = ((10000 - ui32_1) * sl->items[ui8_3].cost);
      sl->t_discount += (sl->items[ui8_3].cost - ui32_1);
    }
    ui32_1 *= sl->items[ui8_3].quantity;

    sl->total += ui32_1;

    if (sl->items[ui8_3].has_serv_tax) {
      ui32_2 = eeprom_read_byte((uint8_t *)offsetof(struct ep_store_layout, ServTax));
      ui32_2 <<= 8;
      ui32_2 |= eeprom_read_byte((uint8_t *)offsetof(struct ep_store_layout, ServTax) + 1);
      ui32_2 *= ui32_1;
      ui32_2 /= 100;
      sl->t_stax += ui32_2;
      sl->total  += ui32_2;
    } else if (sl->items[ui8_3].vat_sel < EPS_MAX_VAT_CHOICE) {
      ui32_2 = eeprom_read_byte((uint8_t *)offsetof(struct ep_store_layout, Vat) + (sizeof(uint16_t)*sl->items[ui8_3].vat_sel));
      ui32_2 <<= 8;
      ui32_2 |= eeprom_read_byte((uint8_t *)offsetof(struct ep_store_layout, Vat) + (sizeof(uint16_t)*sl->items[ui8_3].vat_sel) + 1);
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
  sl->info.time_hh = ((ui32_2>>FAT_HOUR_OFFSET)&FAT_HOUR_MASK);
  sl->info.time_mm = ((ui32_2>>FAT_MIN_OFFSET)&FAT_MIN_MASK);
  sl->info.time_ss = ((ui32_2>>FAT_SEC_OFFSET)&FAT_SEC_MASK);
  for (ui8_2=0; ui8_2<EPS_MAX_UNAME; ui8_2++)
    sl->info.user[ui8_2] = eeprom_read_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_users) + (EPS_MAX_UNAME*(LoginUserId-1)) + ui8_2));

#if FF_ENABLE
  if (DS_DEV_INVALID != (devStatus & DS_DEV_INVALID)) {
  /* Save the bill to SD */
  UINT ret_val;
  memset(&FS, 0, sizeof(FS));
  memset(&Fil, 0, sizeof(Fil));
  //  change_sd(0); /* FIXME: */
  f_mount(&FS, "", 0);		/* Give a work area to the default drive */
  strncpy_P((char *)bufSS, SD_BILLING_FILE, sizeof(SD_BILLING_FILE));
  bufSS[sizeof(SD_BILLING_FILE)] = 0;
  if (f_open(&Fil, (char *)bufSS, FA_READ|FA_WRITE|FA_CREATE_ALWAYS) == FR_OK) {	/* Create a file */
    do {
      /* Move to end of the file to append data */
      ui32_2 = f_size(&Fil);
      if (0 != ui32_2) {    /* file is old */
	assert( 2 == (ui32_2 %
		      (SALE_SIZEOF+((MAX_ITEMS_IN_BILL-1)*ITEM_SIZEOF)+2)) );
	/* If version doesn't match, escape... */
	f_lseek(&Fil, 0);
	f_read(&Fil, bufSS, 2, &ret_val);
	assert(2 == ret_val);
	ui16_2 = bufSS[0]; ui16_2 <<= 8; ui16_2 |= bufSS[1];
	if (GIT_HASH_CRC != ui16_2) {
	  LCD_ALERT(PSTR("Del old files"));
	  break;
	}
	/* */
	f_lseek(&Fil, ui32_2-2);
	f_read(&Fil, bufSS, 2, &ret_val);
	assert(2 == ret_val);
	ui16_2 = bufSS[0]; ui16_2 <<= 8; ui16_2 |= bufSS[1];
	f_lseek(&Fil, ui32_2-2);
      } else {     /* file already exists */
	/* new file, set it up ... */
	f_lseek(&Fil, 0);
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
    } while (0);
  } else
    LCD_ALERT(PSTR("Can't save bill"));
  f_mount(NULL, "", 0);
  }
#endif

  /* Now print the bill */
  menuPrnBill(sl);
}

void
menuAddItem(uint8_t mode)
{
#if MENU_ITEM_FUNC
  uint8_t ui8_1, ui8_2, ui8_3;
  uint16_t ui16_1, ui16_2, ui16_3, ui16_4;
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
  for (ui16_1=0, ui16_2=0; ui16_1 < (ITEM_MAX_ADDR>>EEPROM_MAX_DEVICES_LOGN2);
       ui16_1+=(ITEM_SIZEOF>>EEPROM_MAX_DEVICES_LOGN2), ui16_2++) {
    ee24xx_read_bytes(ui16_1, (void *)it, ITEM_SIZEOF);
    if ( (((mode&MENU_MODITEM) ? arg1.value.integer.i16 : 0 ) == it->id)  &&
	 (0 == it->is_disabled) ) {
      //printf("Storing at id :%d @0x%x\n", ui16_2, ui16_1);
      break;
    }
  }
  if ( ui16_1 >= (ITEM_MAX_ADDR>>EEPROM_MAX_DEVICES_LOGN2) ) {  /* check if we have enough space here */
    if (mode & MENU_MODITEM) {
      LCD_ALERT(PSTR("Invalid ID"));
    } else {
      LCD_ALERT(PSTR("Items full"));
    }
    return;
  }
  assert (it->id < ITEM_MAX);
  goto menuItemSaveArg;

 menuItemInvalidArg:
  LCD_ALERT(PSTR("Invalid Argument"));
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
      it->name[ui8_1] = isgraph(arg1.value.str.sptr[ui8_1]) ? toupper(arg1.value.str.sptr[ui8_1]) : ' ';
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
  arg1.value.str.sptr = bufSS;
  arg1.value.str.len = ITEM_PROD_CODE_BYTEL;
  menuGetOpt(menu_str1+(MENU_STR1_IDX_PRODCODE*MENU_PROMPT_LEN), &arg1, MENU_ITEM_STR);
  for (ui8_1=0; ui8_1<LCD_MAX_COL; ui8_1++) {
    it->prod_code[ui8_1] = isgraph(arg1.value.str.sptr[ui8_1]) ? 
      toupper(arg1.value.str.sptr[ui8_1]) : ' ';
  }
  arg2.valid = MENU_ITEM_NONE;
  arg2.value.str.sptr = bufSS+ITEM_PROD_CODE_BYTEL;
  arg2.value.str.len = ITEM_NAME_UNI_BYTEL;
  menuGetOpt(menu_str1+(MENU_STR1_IDX_UNICODE*MENU_PROMPT_LEN), &arg2, MENU_ITEM_STR);
  if ((MENU_ITEM_STR != arg1.valid) || (MENU_ITEM_STR != arg2.valid))
    goto menuItemInvalidArg;

  /* both shouldn't be empty */
  if ((0 == isgraph(it->name[0])) && (0 == isgraph(it->prod_code[0]))) {
    LCD_ALERT(PSTR("Invalid item"));
    return;
  }

  /* */
  for (ui8_1=0, ui8_2=0, ui8_3=0; ui8_1<ITEM_NAME_UNI_BYTEL; ui8_1++) {
    if ( (!isalnum(arg2.value.str.sptr[ui8_1]))
	 || (!isgraph(arg2.value.str.sptr[ui8_1])) )
      continue;
    ui8_3 <<= 4;
    ui8_3 |= ('9' <= arg2.value.str.sptr[ui8_1]) ? (arg2.value.str.sptr[ui8_1] - '0') :
      ( ('F' <= arg2.value.str.sptr[ui8_1]) ? (arg2.value.str.sptr[ui8_1]-'A'+10) :
	(arg2.value.str.sptr[ui8_1]-'a'+10) );
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
  it->has_serv_tax = menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_S_TAX*MENU_PROMPT_LEN), MENU_PROMPT_LEN);

  /* Confirm */
  if (0 != menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_CONFI*MENU_PROMPT_LEN), MENU_PROMPT_LEN))
    return;

  /* Now save item */
//  for (ui8_3=0; ui8_3<ITEM_SIZEOF;) {
//    printf("%02x ", bufSS_ptr[ui8_3]);
//    ui8_3++;
//    if (0 == (ui8_3%16)) printf("\n");
//  }
//  printf("\n");
  ui8_3 = ee24xx_write_bytes(ui16_1, bufSS_ptr, ITEM_SIZEOF);
  assert(ITEM_SIZEOF == ui8_3);

  /* other wise indexing would span across pages*/
  assert(0 == (SPM_PAGESIZE % 4));
  assert(BUFSS_SIZE >= SPM_PAGESIZE);
  
  /* indexing for faster search...
     make sure the data always falls within one SPM PAGE
   */
  for (ui8_1=0, ui16_2=0; ui8_1<ITEM_PROD_CODE_BYTEL; ui8_1++)
    ui16_2 = _crc16_update(ui16_2, it->name[ui8_1]);
  ui8_2 = -1;
  for (ui8_1=0, ui16_1=0; ui8_1<ITEM_NAME_BYTEL; ui8_1++) {
    ui16_1 = _crc16_update(ui16_1, it->name[ui8_1]);
    if ((ui8_1 > 0) && isgraph(it->name[ui8_1]) &&
	(' ' == it->name[ui8_1-1])) {
      if (-1 == ui8_2) ui8_2 = ui8_1-1;
    }
  }
  for (ui8_1=0, ui16_3=0; (-1 != ui8_2) && (ui8_1<ui8_2); ui8_1++)
    ui16_3 = _crc16_update(ui16_3, it->name[ui8_1]);
  for (ui8_1=0, ui16_4=0; ui8_1<3; ui8_1++)
    ui16_4 = _crc16_update(ui16_4, it->name[ui8_1]);
  assert(0 == (SPM_PAGESIZE % (ITEM_SUBIDX_NAME*sizeof(uint16_t))));

  /* only valid items needs to be buffered */
  menuIndexItem(it);
#endif
}

void
menuDelItem(uint8_t mode)
{
#if MENU_ITEM_FUNC
  uint16_t ui16_1, ui16_2;
  struct item *it = (void *)(bufSS+LCD_MAX_COL+2+LCD_MAX_COL+2);

  /* check validity of argument */
  if ( (MENU_ITEM_ID != arg1.valid) || (0 == arg1.value.integer.i16) ||
       (arg1.value.integer.i16 > ITEM_MAX) ) {
    ERROR(PSTR("Invalid Option"));
    return;
  }

  /* find address for the id */
  ui16_2 = arg1.value.integer.i16-1;
  ui16_1 = menuItemAddr(ui16_2);
  ee24xx_read_bytes(ui16_1, (void *)it, ITEM_SIZEOF);
  if ((0 == it->id) || (it->is_disabled))
    return;
  assert(ui16_2 == (it->id-1));

  /* delete flash indexing as well */
  ui16_2 = ((uint32_t)((uint32_t)ITEM_MAX_ADDR + (sizeof(itemIdxs_t)*(it->id-1)))) >> EEPROM_MAX_DEVICES_LOGN2;
  ee24xx_write_bytes(ui16_2, NULL, sizeof(itemIdxs_t));

  /* delete */
  it->id = 0;
  //  ee24xx_write_bytes(ui16_1, NULL, ITEM_SIZEOF);
  ee24xx_write_bytes(ui16_1+(offsetof(struct item, id)>>EEPROM_MAX_DEVICES_LOGN2),
  		     NULL, 1<<EEPROM_MAX_DEVICES_LOGN2);
#endif
}

/* Index this one item */
void
menuIndexItem(struct item *it)
{
  uint16_t ui16_1;
  uint32_t ui32_1;
  uint8_t ui8_1, ui8_2;
  itemIdxs_t itIdx;

  /* init */
  ui32_1 = (uint32_t)((uint32_t)((uint32_t)ITEM_MAX_ADDR + (sizeof(itemIdxs_t) * (it->id-1))) >> EEPROM_MAX_DEVICES_LOGN2);
  assert(ui32_1 < EEPROM_MAX_ADDRESS);
  ee24xx_read_bytes(ui32_1, (uint8_t *)&itIdx, sizeof(itemIdxs_t));

  /* if not valid or if disabled */
  if ( (0 == it->id) || (it->is_disabled) ) {
    for (ui8_1=0; ui8_1<ITEM_SUBIDX_NAME; ui8_1++)
      if (0 != itIdx[ui8_1]) break;
    if (ui8_1 != ITEM_SUBIDX_NAME) {
      for (ui8_1=0; ui8_1<ITEM_SUBIDX_NAME; ui8_1++)
	itIdx[ui8_1] = 0;
      ee24xx_write_bytes(ui32_1, (uint8_t *)&itIdx, sizeof(itemIdxs_t));
    }
  }

  /* */
  ui8_2 = 0;
  ui16_1 = 0;
  for (ui8_1=0; ui8_1<ITEM_PROD_CODE_BYTEL; ui8_1++)
    ui16_1 = _crc16_update(ui16_1, it->prod_code[ui8_1]);
  if (ui16_1 != itIdx[0]) {
    itIdx[0] = ui16_1;
    ui8_2++;
  }
  //  printf("sig1:0x%0x\n", ui16_1);

  /* */
  ui8_2 = -1;
  ui16_1 = 0;
  for (ui8_1=0; ui8_1<ITEM_NAME_BYTEL; ui8_1++) {
    ui16_1 = _crc16_update(ui16_1, it->name[ui8_1]);
    if ( (-1 == ui8_2) && (ui8_1 > 0) && (' ' == it->name[ui8_1]) &&
	 (' ' != it->name[ui8_1-1]) )
      ui8_2 = ui8_1;
  }
  if (ui16_1 != itIdx[1]) {
    itIdx[1] = ui16_1;
    ui8_2++;
  }
  //  printf("sig2:0x%0x\n", ui16_1);
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
  if (ui16_1 != itIdx[2]) {
    itIdx[2] = ui16_1;
    ui8_2++;
  }
  //  printf("sig3:0x%0x\n", ui16_1);
  /* first 3 letters of name */
  assert(3 <=ITEM_NAME_BYTEL);
  ui16_1 = 0;
  for (ui8_1=0; ui8_1<3; ui8_1++)
    ui16_1 = _crc16_update(ui16_1, it->name[ui8_1]);
  if (ui16_1 != itIdx[3]) {
    itIdx[3] = ui16_1;
    ui8_2++;
  }
  //  printf("sig4:0x%0x\n", ui16_1);
#endif

  if (0 != ui8_2) {
    ee24xx_write_bytes(ui32_1, (uint8_t *)&itIdx, sizeof(itemIdxs_t));
  }
}

/* either name or prod_code be NULL */
uint16_t
menuItemFind(uint8_t *name, uint8_t *prod_code, struct item *it, uint16_t idx)
{
  itemIdxs_t itIdx, itIdx1;
  uint16_t ui16_1;
  uint8_t  ui8_1, ui8_2;

  /* upcase inputs */
  assert( (NULL != name) || (NULL != prod_code) );
  if (NULL != name) {
    for (ui8_1=0; ui8_1<ITEM_NAME_BYTEL; ui8_1++) {
      name[ui8_1] = toupper(name[ui8_1]);
    }
  }
  if (NULL != prod_code) {
    for (ui8_1=0; ui8_1<ITEM_PROD_CODE_BYTEL; ui8_1++) {
      prod_code[ui8_1] = toupper(prod_code[ui8_1]);
    }
  }

  /* calculate signature indices */
  itIdx[1] = 0; ui8_2 = -1;
  if (NULL != name) {
    for (ui8_1=0; ui8_1<ITEM_NAME_BYTEL; ui8_1++) {
      itIdx[1] = _crc16_update(itIdx[1], name[ui8_1]);
      if ((ui8_1 > 0) && (' ' == name[ui8_1]) && (' ' != name[ui8_1-1]))
	ui8_2 = ui8_1;
    }
  }
  itIdx[0] = 0;
  if (NULL != prod_code) {
    for (ui8_1=0; ui8_1<ITEM_PROD_CODE_BYTEL; ui8_1++)
      itIdx[0] = _crc16_update(itIdx[0], prod_code[ui8_1]);
  } 
  itIdx[2] = 0;
  itIdx[3] = 0;
#if 4 == ITEM_SUBIDX_NAME
  if (NULL != name) {
    if (-1 != ui8_2) {
      for (ui8_1=0; ui8_1<ui8_2; ui8_1++)
	itIdx[2] = _crc16_update(itIdx[2], name[ui8_1]);
    }
    for (ui8_1=0; ui8_1<3; ui8_1++)
      itIdx[3] = _crc16_update(itIdx[3], name[ui8_1]);
  }
#endif

  for (ui16_1=((ITEM_MAX_ADDR+(idx*sizeof(itemIdxs_t)))>>EEPROM_MAX_DEVICES_LOGN2); idx<ITEM_MAX;
       idx++, ui16_1+=(sizeof(itemIdxs_t)>>EEPROM_MAX_DEVICES_LOGN2)) {
    ee24xx_read_bytes(ui16_1, (void *)itIdx1, sizeof(itemIdxs_t));
    //    printf("idx:%d itIdx[0]:%x itIdx[1]:%x itIdx1[0]:%x itIdx1[1]:%x\n",
    //	   idx, itIdx[0], itIdx[1], itIdx1[0], itIdx1[1]);
    ee24xx_read_bytes(menuItemAddr(idx), (uint8_t *)it, ITEM_SIZEOF);
    if ((NULL != name) && (NULL != prod_code)) {
      if ((itIdx[0] == itIdx1[0]) && (itIdx[1] == itIdx1[1])) {
	if ( (0 == strncmp((char *)name, (const char *)it->name, ITEM_NAME_BYTEL)) &&
	     (0 == strncmp((char *)prod_code, (const char *)(it->prod_code), ITEM_PROD_CODE_BYTEL)) )
	  return idx+1;
      }
    } else if ((NULL != name) && (itIdx[1] == itIdx1[1])) {
      if (0 == strncmp((char *)name, (const char *)(it->name), ITEM_NAME_BYTEL))
	return idx+1;
    } else if ((NULL != prod_code) && (itIdx[0] == itIdx1[0])) {
      if (0 == strncmp((char *)prod_code, (const char *)(it->prod_code), ITEM_PROD_CODE_BYTEL))
	return idx+1;
    }
  }

  assert(ITEM_MAX < ((uint16_t)-1));
  return -1;
}

// Not tested
void
menuPrintTestPage(uint8_t mode)
{
#if MENU_DIAG_FUNC
  PRINTER_PRINT_TEST_PAGE;
#endif
}

// Not unit tested
void
menuPrnBill(struct sale *sl)
{
  uint8_t ui8_1, ui8_2, ui8_3;
  uint8_t prnBuf[256];

  /* Shop name */
  PRINTER_FONT_ENLARGE(2);
  PRINTER_JUSTIFY(PRINTER_JCENTER);
  for (ui8_2=ui8_1=0; ui8_1<SHOP_NAME_SZ_MAX; ) {
    if (ui8_2 && (0 == (ui8_2%(PRINTER_MAX_CHARS_ON_LINE>>1)))) {
      PRINTER_PRINT('\n');
      ui8_2 = 0;
    } else {
      ui8_3 = eeprom_read_byte((uint8_t *)(offsetof(struct ep_store_layout, shop_name)+ui8_1));
      PRINTER_PRINT(ui8_3);
      ui8_1++;
    }
    ui8_2++;
  }
  PRINTER_FONT_ENLARGE(1);

  /* Header */
  for (ui8_1=0; ui8_1<HEADER_SZ_MAX; ui8_1++) {
    ui8_3 = eeprom_read_byte((uint8_t *)(offsetof(struct ep_store_layout, b_head)+ui8_1));
    PRINTER_PRINT(ui8_3);
  }

  PRINTER_JUSTIFY(PRINTER_JLEFT);

  /* Caption, user, Date */
  for (ui8_1=0; ui8_1<EPS_CAPTION_SZ_MAX; ui8_1++) {
    ui8_3 = eeprom_read_byte((uint8_t *)(offsetof(struct ep_store_layout, caption)+ui8_1));
    if ('\n' != ui8_3)
      PRINTER_PRINT(ui8_3);
  }
  PRINTER_PRINT(' '); PRINTER_PRINT(' ');
  PRINTER_PRINT('u'); PRINTER_PRINT('s'); PRINTER_PRINT('e');
  PRINTER_PRINT('r'); PRINTER_PRINT(':');
  for (ui8_1=0; ui8_1<EPS_MAX_UNAME; ui8_1++) {
    ui8_3 = sl->info.user[ui8_1];
    assert ('\n' != ui8_3);
    PRINTER_PRINT(ui8_3);
  }
  PRINTER_PRINT(' '); PRINTER_PRINT(' ');
  PRINTER_SPRINTF(prnBuf, "%2d", 1+sl->info.date_dd); PRINTER_PRINT('/');
  PRINTER_SPRINTF(prnBuf, "%2d", 1+sl->info.date_mm); PRINTER_PRINT('/');
  PRINTER_SPRINTF(prnBuf, "%4d", 1980+sl->info.date_yy); PRINTER_PRINT(' ');
  PRINTER_SPRINTF(prnBuf, "%2d", sl->info.time_hh); PRINTER_PRINT(':');
  PRINTER_SPRINTF(prnBuf, "%02d", sl->info.time_mm); PRINTER_PRINT(':');
  PRINTER_SPRINTF(prnBuf, "%02d", sl->info.time_ss); PRINTER_PRINT('\n');

  /* Items */
  for (ui8_1=0; ui8_1<sl->info.n_items; ui8_1++) {
    if (EEPROM_MAX_ADDRESS != sl->items[ui8_1].ep_item_ptr) {
      ee24xx_write_bytes(sl->items[ui8_1].ep_item_ptr,
			 (void *)&(sl->it[0]), ITEM_SIZEOF);
      ui8_2 = 0;
    } else {
      ui8_2 = ui8_1;
    }
    PRINTER_SPRINTF(prnBuf, "%2u. ", (unsigned int)ui8_1);
    for (ui8_3=0; ui8_3<ITEM_NAME_BYTEL; ui8_3++)
      PRINTER_PRINT(sl->it[0].name[ui8_3]);
    PRINTER_SPRINTF(prnBuf, " %4u", (unsigned int)sl->items[ui8_3].cost);
    PRINTER_SPRINTF(prnBuf, "(-%4u)", (unsigned int)sl->items[ui8_3].discount);
    PRINTER_SPRINTF(prnBuf, " %4u", (unsigned int)sl->items[ui8_3].quantity);
    PRINTER_SPRINTF(prnBuf, " %6u\n", (unsigned int)sl->items[ui8_3].cost * sl->items[ui8_3].quantity);
  }

  /* Total */
  PRINTER_SPRINTF(prnBuf, "Total Discount : %.2f\n", (double)sl->t_discount);
  PRINTER_SPRINTF(prnBuf, "Total VAT      : %.2f\n", (double)sl->t_vat);
  PRINTER_SPRINTF(prnBuf, "Total Serv Tax : %.2f\n", (double)sl->t_stax);
  PRINTER_SPRINTF(prnBuf, "Bill Total (Rs): %.2f\n", (double)sl->total);

  /* Footer */
  ui8_2 = 0;
  for (ui8_1=0; ui8_1<FOOTER_SZ_MAX; ui8_1++) {
    ui8_3 = eeprom_read_byte((uint8_t *)(offsetof(struct ep_store_layout, b_foot)+ui8_1));
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
  struct sale *sl = (void *)(bufSS+LCD_MAX_COL+2+LCD_MAX_COL+2);
  uint16_t ui16_1, ui16_2;
  uint32_t ui32_1, ui32_2;

  /* if dates not provided assume today */
  {
    uint8_t ymd[3];
    timerDateGet(ymd);
    if (MENU_ITEM_DATE != arg1.valid) {
      arg1.valid = MENU_ITEM_DATE;
      arg1.value.date.day = ymd[0];
      arg1.value.date.month = ymd[1];
      arg1.value.date.year = ymd[2];
    }
    if (MENU_ITEM_DATE != arg2.valid) {
      arg2.valid = MENU_ITEM_DATE;
      arg2.value.date.day = ymd[0];
      arg2.value.date.month = ymd[1];
      arg2.value.date.year = ymd[2];
    }
  }

  /* check that dates are in order */
  if ( ! ( (arg1.value.date.year < arg2.value.date.year) ||
	   ( (arg1.value.date.year == arg2.value.date.year) &&
	     (arg1.value.date.month < arg2.value.date.month) ) ||
	   ( (arg1.value.date.year == arg2.value.date.year) &&
	     (arg1.value.date.month == arg2.value.date.month) &&
	     (arg1.value.date.day < arg2.value.date.day) ) ) ) {
    LCD_ALERT(PSTR("Date Wrong Order"));
    return;
  }
  if ( !validDate(arg1.value.date.day, arg1.value.date.month, arg1.value.date.year) ||
       !validDate(arg2.value.date.day, arg2.value.date.month, arg2.value.date.year) ) {
    LCD_ALERT(PSTR("Invalid Date"));
    return;
  }

  /* */
#if FF_ENABLE
  UINT  ret_val;
  if (DS_DEV_INVALID != (devStatus & DS_DEV_INVALID)) {
  memset(&FS, 0, sizeof(FS));
  memset(&Fil, 0, sizeof(Fil));
  f_mount(&FS, ".", 1);
  if (FR_OK != f_open(&Fil, SD_ITEM_FILE, FA_READ)) {
    LCD_ALERT(PSTR("File open error"));
    f_mount(NULL, "", 0);
    return;
  }

  /* If version doesn't match, escape... */
  f_read(&Fil, (bufSS+LCD_MAX_COL+2+LCD_MAX_COL+2), 2, &ret_val);
  assert(2 == ret_val);
  ui16_1 = bufSS[LCD_MAX_COL+2+LCD_MAX_COL+2];
  ui16_1 <<= 8; ui16_1 |= bufSS[LCD_MAX_COL+2+LCD_MAX_COL+2+1];
  if (GIT_HASH_CRC != ui16_2) {
    LCD_ALERT(PSTR("Incompatible File"));
    f_mount(NULL, "", 0);
    return;
  }

  /* Find # records */
  ui32_1 = f_size(&Fil);
  if (0 == ui32_1) return; /* don't expect to find anything */
  assert (0 == ((ui32_1-4) % (sizeof(struct sale) + ((MAX_ITEMS_IN_BILL-1)*sizeof(struct item)))));
  ui32_1 = (ui32_1-4) / (sizeof(struct sale) + ((MAX_ITEMS_IN_BILL-1)*sizeof(struct item)));

  /* iterate records */
  for (ui32_2=0; ui32_2<ui32_1; ui32_2++) {
    /* Display this item */
    f_lseek( &Fil, 2+(ui32_2*(sizeof(struct sale) + ((MAX_ITEMS_IN_BILL-1)*sizeof(struct item)))) );
    f_read(&Fil, (void *)sl, sizeof(struct sale), &ret_val);
    assert(sizeof(struct sale) == ret_val);

    /* complete this.. */
    if ( (arg1.value.date.year < sl->info.date_yy) ||
	 ((arg1.value.date.year == sl->info.date_yy) && (arg1.value.date.month < sl->info.date_mm)) ||
	 ((arg1.value.date.year == sl->info.date_yy) && (arg1.value.date.month == sl->info.date_mm) && (arg1.value.date.day < sl->info.date_dd)) )
      continue;
    else if ( (sl->info.date_yy < arg2.value.date.year) ||
	 ((sl->info.date_yy == arg2.value.date.year) && (sl->info.date_mm < arg2.value.date.month)) ||
	 ((sl->info.date_yy == arg2.value.date.year) && (sl->info.date_mm == arg2.value.date.month) && (sl->info.date_dd < arg2.value.date.day)) )
      continue;

    /* FIXME: */
  }

  /* */
  f_mount(NULL, "", 0);
  }
#endif
}

// Not unit tested
void
menuShowBill(uint8_t mode)
{
#if FF_ENABLE
  if (DS_DEV_INVALID != (devStatus & DS_DEV_INVALID)) {
  UINT  ret_val;
  uint8_t  ui8_1;
  uint16_t ui16_1, ui16_2;
  uint32_t ui32_1, ui32_2;
  struct sale *sl = (void *)(bufSS+LCD_MAX_COL+2+LCD_MAX_COL+2);

  /* */
  memset(&FS, 0, sizeof(FS));
  memset(&Fil, 0, sizeof(Fil));
  f_mount(&FS, ".", 1);
  if (FR_OK != f_open(&Fil, SD_ITEM_FILE, FA_READ)) {
    LCD_ALERT(PSTR("File open error"));
    f_mount(NULL, "", 0);
    return;
  }

  /* If version doesn't match, escape... */
  f_read(&Fil, (bufSS+LCD_MAX_COL+2+LCD_MAX_COL+2), 2, &ret_val);
  assert(2 == ret_val);
  ui16_1 = bufSS[LCD_MAX_COL+2+LCD_MAX_COL+2];
  ui16_1 <<= 8; ui16_1 |= bufSS[LCD_MAX_COL+2+LCD_MAX_COL+2+1];
  if (GIT_HASH_CRC != ui16_2) {
    LCD_ALERT(PSTR("Old Format   "));
    f_mount(NULL, "", 0);
    return;
  }
  ui32_2 = f_size(&Fil);
  if ( (0 == ui32_2) ||
       (0 != ((ui32_2-4) % (sizeof(struct sale) + ((MAX_ITEMS_IN_BILL-1)*sizeof(struct item))))) ) {
    LCD_ALERT(PSTR("Corrupted File"));
    f_mount(NULL, "", 0);
    return;
  }

  /* Find # records */
  ui32_1 = (f_size(&Fil)-4) / (sizeof(struct sale) + ((MAX_ITEMS_IN_BILL-1)*sizeof(struct item)));

  /* iterate records */
  for (ui32_2=0, ui8_1=1; ;) {
    /* Display this item */
    f_lseek( &Fil, 2+(ui32_2*(sizeof(struct sale) + ((MAX_ITEMS_IN_BILL-1)*sizeof(struct item)))) );
    f_read(&Fil, (void *)sl, sizeof(struct sale), &ret_val);
    assert(sizeof(struct sale) == ret_val);
    if (MENU_ITEM_DATE == arg1.valid) {
      if ( (sl->info.date_yy != arg1.value.date.year) ||
	   (sl->info.date_mm != arg1.value.date.month) ||
	   (sl->info.date_dd != arg1.value.date.day) ) {
	if ((0 == (ui8_1&1)) && (ui32_2>0)) { /* prev */
	  ui32_2--;
	  continue;
	} else if ((1 == (ui8_1&1)) && (ui32_2<(ui32_1-1))) { /* next */
	  ui32_2++;
	  continue;
	} else if ((ui8_1 & 0xF0) > 0x10) {
	  break;
	} else { /* not found, change direction */
	  ui8_1 += 0x10;
	  ui8_1 ^= 1;
	  continue;
	}
      }
    }
    ui8_1 &= ~0xF0;

    /* FIXME: Display bill */

    /* according to user's wish */
    KBD_RESET_KEY;
    KBD_GETCH;
    if ((ASCII_ENTER == keyHitData.KbdData) || (ASCII_PRNSCRN == keyHitData.KbdData)) {
      menuPrnBill(sl);
    } else if ( ASCII_UP == keyHitData.KbdData ) {
      ui8_1 = 0;
    } else if ( ASCII_DOWN == keyHitData.KbdData ) {
      ui8_1 = 1;
    } else if ( (ASCII_LEFT == keyHitData.KbdData) || (ASCII_RIGHT == keyHitData.KbdData) ) {
      break;
    }
  }

  /* */
  f_mount(NULL, "", 0);
  }
#endif
}

#if MENU_SETTING_ENABLE
const struct menu_vars MenuVars[] PROGMEM = { MENU_VAR_TABLE };

void
menuSettingString(uint16_t addr, const uint8_t *quest, uint16_t max_chars)
{
  uint16_t ui16_1;

  do {
    arg1.valid = MENU_ITEM_NONE;
    arg1.value.str.sptr = bufSS;
    arg1.value.str.len = max_chars;
    menuGetOpt(quest, &arg1, MENU_ITEM_STR);
  } while (MENU_ITEM_STR != arg1.valid);
#ifndef UNIT_TEST
  if (menuGetYesNo((const uint8_t *)PSTR("Confirm?"), 8))
    return;
#endif

  for (ui16_1=0; ui16_1<max_chars; ui16_1++) {
    //    if (!isgraph(arg1.value.str.sptr[ui16_1]))
    //      arg1.value.str.sptr[ui16_1] = ' ';
    eeprom_update_byte((uint8_t *)addr, arg1.value.str.sptr[ui16_1]);
    addr++;
  }
}

void
menuSettingUint32(uint16_t addr, const uint8_t *quest)
{
  do {
    arg1.valid = MENU_ITEM_NONE;
    menuGetOpt(quest, &arg1, MENU_ITEM_ID);
  } while (MENU_ITEM_ID != arg1.valid);
#ifndef UNIT_TEST
  if (menuGetYesNo((const uint8_t *)PSTR("Confirm?"), 8))
    return;
#endif

  uint32_t val = arg1.value.integer.i32;
  eeprom_update_dword((void *)addr, val);
}

void
menuSettingUint16(uint16_t addr, const uint8_t *quest)
{
  arg1.valid = MENU_ITEM_NONE;
  do {
    menuGetOpt(quest, &arg1, MENU_ITEM_ID);
  } while  (MENU_ITEM_ID != arg1.valid);
#ifndef UNIT_TEST
  if (menuGetYesNo((const uint8_t *)PSTR("Confirm?"), 8))
    return;
#endif

  uint16_t val = arg1.value.integer.i16;
  eeprom_update_word((uint16_t *)addr, val);
}

void
menuSettingUint8(uint16_t addr, const uint8_t *quest)
{
  do {
    arg1.valid = MENU_ITEM_NONE;
    menuGetOpt(quest, &arg1, MENU_ITEM_ID);
  } while (MENU_ITEM_ID != arg1.valid);
#ifndef UNIT_TEST
  if (menuGetYesNo((const uint8_t *)PSTR("Confirm?"), 8))
    return;
#endif

  uint8_t val = arg1.value.integer.i16;
  eeprom_update_byte((void *)addr, val);
}

void
menuSettingBit(uint16_t addr, const uint8_t *quest, uint8_t size, uint8_t offset)
{
  uint8_t ui8_1, ui8_2;
  
  do {
    arg1.valid = MENU_ITEM_NONE;
    menuGetOpt(quest, &arg1, MENU_ITEM_ID);
  } while (MENU_ITEM_ID != arg1.valid);
#ifndef UNIT_TEST
  if (menuGetYesNo((const uint8_t *)PSTR("Confirm?"), 8))
    return;
#endif

  ui8_1 = eeprom_read_byte((uint8_t *)(addr));
  ui8_2 = (1<<size)-1;
  ui8_1 &= ~(ui8_2<<offset);
  ui8_1 |= (arg1.value.integer.i16 & ui8_2) << offset;
  eeprom_update_byte((void *)(addr), ui8_1);
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
    ui8_2 = pgm_read_byte(MenuVars+ui8_1+offsetof(struct menu_vars, type));
    ui16_1 = pgm_read_word(MenuVars+ui8_1+offsetof(struct menu_vars, ep_ptr));
    for (ui8_2=0; ui8_2<ITEM_NAME_BYTEL; ui8_2++)
      PRINTER_PRINT(pgm_read_byte(MenuVars+ui8_1+offsetof(struct menu_vars, name)+ui8_2));
    PRINTER_PRINT('\t');
    if ( (TYPE_UINT8 == ui8_2) || (TYPE_UINT16 == ui8_2) ||
	 (TYPE_UINT32 == ui8_2) || (TYPE_BIT == ui8_2) ) {
      for (ui8_3=0, ui32_1=0; ui8_3<(ui8_2&0x7); ui8_3++) {
	ui32_1 <<= 8;
	//ui32_1 |= pgm_read_byte(0); /* FIXME */
      }
    } else if (TYPE_STRING == ui8_2) {
      //ui32_1 = pgm_read_byte(0); /* FIXME */
    } else {
      assert(0);
    }
  }

}
#endif

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

void
menuSettingSet(uint8_t mode)
{
#if MENU_SETTING_ENABLE
  uint8_t ui8_1, ui8_2;
  uint16_t ui16_1;

 menuSettingSetStart:
  for (ui8_1=1; ;) {
    ui8_1 %= (MENU_VARS_SIZE+1);
    LCD_CLRLINE(LCD_MAX_ROW-1);
    LCD_WR_NP((const uint8_t *)PSTR("Settings"), sizeof("Settings"));
    if (0 == ui8_1) {
      LCD_CLRLINE(LCD_MAX_ROW-1);
      LCD_WR_NP((const uint8_t *)PSTR("Quit ?"), sizeof("Quit ?"));
    } else {
      LCD_CLRLINE(LCD_MAX_ROW-1);
      LCD_WR_NP((const uint8_t *)PSTR("Change "), sizeof("Change "));
      LCD_WR_NP((const uint8_t *)((MenuVars+(ui8_1-1))+offsetof(struct menu_vars, name)), MENU_VAR_NAME_LEN);
    }
    LCD_refresh();
    KBD_RESET_KEY;
    KBD_GETCH;
    if (ASCII_ENTER == keyHitData.KbdData) {
      break;
    } else if ((ASCII_LEFT == keyHitData.KbdData) || (ASCII_UP == keyHitData.KbdData)) {
      ui8_1 = (ui8_1 > 0) ? ui8_1-1 : MENU_VARS_SIZE;
    } else if ((ASCII_RIGHT == keyHitData.KbdData) || (ASCII_DOWN == keyHitData.KbdData)) {
      ui8_1 = (ui8_1 > MENU_VARS_SIZE) ? 0 : ui8_1+1;
    }
  }
  if (0 == ui8_1) return;

  ui8_2 = pgm_read_byte(((uint8_t *)(MenuVars+(ui8_1-1)))+offsetof(struct menu_vars, type));
  ui16_1 = pgm_read_byte(((uint8_t *)(MenuVars+(ui8_1-1)))+offsetof(struct menu_vars, ep_ptr)+1);
  ui16_1 <<= 8;
  ui16_1 |= pgm_read_byte(((uint8_t *)(MenuVars+(ui8_1-1)))+offsetof(struct menu_vars, ep_ptr));
  switch (ui8_2) {
  case TYPE_UINT8:
    menuSettingUint8(ui16_1, ((uint8_t *)(MenuVars+(ui8_1-1)))+offsetof(struct menu_vars, name));
  case TYPE_UINT16:
    menuSettingUint16(ui16_1, ((uint8_t *)(MenuVars+(ui8_1-1)))+offsetof(struct menu_vars, name));
    break;
  case TYPE_UINT32:
    menuSettingUint32(ui16_1, ((uint8_t *)(MenuVars+(ui8_1-1)))+offsetof(struct menu_vars, name));
    break;
  case TYPE_STRING:
    menuSettingString(ui16_1, ((uint8_t *)(MenuVars+(ui8_1-1)))+offsetof(struct menu_vars, name),
		      pgm_read_byte(((uint8_t *)(MenuVars+(ui8_1-1)))+offsetof(struct menu_vars, name)+offsetof(struct menu_vars, size)));
    break;
  case TYPE_BIT:
    menuSettingBit( ui16_1, ((uint8_t *)(MenuVars+(ui8_1-1)))+offsetof(struct menu_vars, name),
		    pgm_read_byte(((uint8_t *)(MenuVars+(ui8_1-1)))+offsetof(struct menu_vars, name)+offsetof(struct menu_vars, size)),
		    pgm_read_byte(((uint8_t *)(MenuVars+(ui8_1-1)))+offsetof(struct menu_vars, name)+offsetof(struct menu_vars, size2)) );
    break;
  default:
    assert(0);
    return;
  }

  goto menuSettingSetStart;
#endif
}

// Not unit tested
void
menuDelAllBill(uint8_t mode)
{
#if MENU_DELBILL && FF_ENABLE
  if (DS_DEV_INVALID != (devStatus & DS_DEV_INVALID)) {
  uint8_t ui8_1, ui8_2;
  uint8_t *ui8_1p = (bufSS+LCD_MAX_COL+2+LCD_MAX_COL+2);

  /* init */
  PSTR2STR(BillFileName, ui8_1p, ui8_1, ui8_2);

  /* */
  memset(&FS, 0, sizeof(FS));
  f_mount(&FS, ".", 1);
  if (FR_OK == f_unlink((char *)ui8_1p)) {
    LCD_CLRLINE(LCD_MAX_ROW-1);
    LCD_WR_NP((const uint8_t *)PSTR(SD_BILLING_FILE " Del'd"), sizeof(SD_BILLING_FILE)+sizeof(" Del'd"));
    _delay_ms(1000);
  }

  /* */
  f_mount(NULL, "", 0);
  }
#endif
}

#if MENU_DIAG_FUNC
#define DIAG_FLASHMEM_SIZE   16
const uint8_t diagFlashMem[DIAG_FLASHMEM_SIZE] PROGMEM =
  { [ 0 ... (DIAG_FLASHMEM_SIZE - 1) ] = 0 };
#endif

// Not unit tested
void
menuRunDiag(uint8_t mode)
{
#if MENU_DIAG_FUNC
  uint16_t ui16_1, rand_seed;
  uint8_t  ui8_1, ui8_2;

  /* init */
  rand_seed = get_fattime();

  /* Verify LCD */
  LCD_CLRSCR;
  LCD_CLRLINE(0);
  LCD_WR_NP((const uint8_t *)PSTR("Diagnosis LCD"), 16);
  _delay_ms(1000);
  for (ui8_1=0; ui8_1<LCD_MAX_ROW; ui8_1++) {
    LCD_WR_NP((const uint8_t *)PSTR("\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"), LCD_MAX_COL);
  }
  LCD_refresh();
  _delay_ms(2000);
  LCD_CLRLINE(0);
  for (ui8_1=0; ui8_1<(LCD_MAX_ROW*LCD_MAX_COL); ui8_1++) {
    LCD_PUTCH(('a' + ui8_1));
  }
  LCD_refresh();
  _delay_ms(2000);
  diagStatus |= (0 == menuGetYesNo((const uint8_t *)PSTR("Can see abcd?"), 13)) ? DIAG_LCD : 0;
  if (0 == (diagStatus&DIAG_LCD)) {
    for (ui8_1=0; ui8_1<3; ui8_1++) {
      BUZZER_ON;
      _delay_ms(1000);
      BUZZER_OFF;
      _delay_ms(1000);
    }
    eeprom_update_word((uint16_t *)(offsetof(struct ep_store_layout, unused_DiagStat)), diagStatus);
    return;
  }

  /* FIXME: Verify TFT */
  LCD_CLRSCR;
  LCD_WR_NP((const uint8_t *)PSTR("Diagnosis TFT"), 13);
  _delay_ms(1000);

  /* FIXME: Adjust LCD/TFT brightness */
  LCD_CLRSCR;
  LCD_WR_NP((const uint8_t *)PSTR("Diag Display Bri"), 16);
  _delay_ms(1000);

  /* Run Printer : Print test page */
  LCD_CLRSCR;
  LCD_WR_NP((const uint8_t *)PSTR("Diagnosis Prntr"), 15);
  _delay_ms(1000);
  ui8_1 = printerStatus();
  if (0 == ui8_1) {
    LCD_ALERT(PSTR("Printer:No Paper"));
    diagStatus |= DIAG_PRINTER;
  } else if (ui8_1 >= 60) {
    LCD_ALERT(PSTR("Printer Too Hot"));
  } else {
    LCD_ALERT(PSTR("Printer OK"));
    diagStatus |= DIAG_PRINTER;
  }
  /* print default bill */
  {
    struct sale sl;
    /* FIXME: */
    menuPrnBill(&sl);
  }

  /* Verify unused EEPROM spaces : Write/readback */
  LCD_CLRSCR;
  LCD_WR_NP((const uint8_t *)PSTR("Diagnosis Mem1"), 14);
  _delay_ms(1000);
  ui16_1 = offsetof(struct ep_store_layout, unused_scratch);
  srand(rand_seed);
  for (ui8_1=0; ui8_1<SCRATCH_MAX; ui8_1++, ui16_1++) {
    ui8_2 = rand();
    eeprom_update_byte((uint8_t *)ui16_1, ui8_2);
  }
  ui16_1 = offsetof(struct ep_store_layout, unused_scratch);
  srand(rand_seed);
  for (ui8_1=0; ui8_1<SCRATCH_MAX; ui8_1++, ui16_1++) {
    ui8_2 = rand();
    if (ui8_2 != eeprom_read_byte((uint8_t *)ui16_1))
      break;
  }
  diagStatus |= (ui8_1==SCRATCH_MAX)? DIAG_MEM1 : 0;

  /* Verify 24c512 */
  LCD_CLRSCR;
  LCD_WR_NP((const uint8_t *)PSTR("Diagnosis Mem3"), 14);
  _delay_ms(1000);
  struct item *it = (void *)bufSS;
  for (ui16_1=0; ui16_1 < (ITEM_MAX_ADDR>>EEPROM_MAX_DEVICES_LOGN2);
       ui16_1+=(ITEM_SIZEOF>>EEPROM_MAX_DEVICES_LOGN2)) {
    ee24xx_read_bytes(ui16_1, bufSS, ITEM_SIZEOF);
    if (0 == it->id)
      break;
  }
  if (0 == it->id) { /* found empty space */
    srand(rand_seed);
    for (ui8_1=0; ui8_1<ITEM_SIZEOF; ui8_1++)
      bufSS[ui8_1] = rand();
    it->id = 0; /* only that field determines */
    ee24xx_write_bytes(ui16_1, bufSS, ITEM_SIZEOF);
    for (ui8_1=0; ui8_1<ITEM_SIZEOF; ui8_1++)
      if (rand() != bufSS[ui8_1])
	break;
    diagStatus |= (ITEM_SIZEOF == ui8_1) ? DIAG_MEM3 : 0;
  } else {
    LCD_ALERT(PSTR("Item Full"));
  }

  /* Test timer */
  LCD_CLRSCR;
  LCD_WR_NP((const uint8_t *)PSTR("Diagnosis Timer"), 15);
  _delay_ms(1000);
  {
    uint8_t buf3[3];
    for (ui8_1=0; ui8_1<10; ui8_1++) {
      timerDateGet(buf3);
      LCD_CLRLINE(LCD_MAX_ROW-1);
      LCD_PUTCH(('0'+((buf3[0]>>4)&0xF)));
      LCD_PUTCH(('0'+(buf3[0]&0xF)));
      LCD_PUTCH(('/'));
      LCD_PUTCH(('0'+((buf3[1]>>4)&0xF)));
      LCD_PUTCH(('0'+(buf3[1]&0xF)));
      LCD_PUTCH(('/'));
      LCD_PUTCH(('0'+((buf3[2]>>4)&0xF)));
      LCD_PUTCH(('0'+(buf3[2]&0xF)));
      timerTimeGet(buf3);
      LCD_PUTCH(('0'+((buf3[0]>>4)&0xF)));
      LCD_PUTCH(('0'+(buf3[0]&0xF)));
      LCD_PUTCH(':');
      LCD_PUTCH(('0'+((buf3[1]>>4)&0xF)));
      LCD_PUTCH(('0'+(buf3[1]&0xF)));
      LCD_PUTCH(':');
      LCD_PUTCH(('0'+((buf3[2]>>4)&0xF)));
      LCD_PUTCH(('0'+(buf3[2]&0xF)));
      LCD_refresh();
      _delay_ms(500);
    }
    diagStatus |= (0 == menuGetYesNo((const uint8_t *)PSTR("Date/Time Corrt?"), 16)) ? DIAG_TIMER : 0;
  }

  /* Verify Flash */
  LCD_CLRSCR;
  LCD_WR_NP((const uint8_t *)PSTR("Diagnosis Mem2"), 14);
  _delay_ms(1000);
  srand(rand_seed);
  for (ui8_1=0; ui8_1<DIAG_FLASHMEM_SIZE; ui8_1++) {
    ui8_2 = rand();
    /* FIXME: write flash */
  }
  srand(rand_seed);
  ui16_1 = (uint16_t)&diagFlashMem;
  for (ui8_1=0; ui8_1<DIAG_FLASHMEM_SIZE; ui8_1++, ui16_1++) {
    ui8_2 = rand();
    if (ui8_2 != pgm_read_byte(ui16_1))
      break;
  }
  diagStatus |= (ui8_1 == DIAG_FLASHMEM_SIZE) ? DIAG_MEM2 : 0;

  /* Verify Keypad : Ask user to press a key and display it */
  LCD_CLRSCR;
  LCD_WR_NP((const uint8_t *)PSTR("Diagnosis Keypad"), 16);
  _delay_ms(1000);
  LCD_CLRLINE(LCD_MAX_ROW-1);
  LCD_WR_NP((const uint8_t *)PSTR("Hit \xDB \xDB to exit"), 16);
  LCD_refresh();
  _delay_ms(2000);
  LCD_CLRSCR;
  LCD_WR_NP((const uint8_t *)PSTR("Key Entered..."), 14);
  for (ui8_2=0; ui8_2<LCD_MAX_COL; ui8_2++)
    bufSS[ui8_2] = ' ';
  for (ui8_1=0; ;) {
    LCD_refresh();
    KBD_RESET_KEY;
    KBD_GETCH;
    if (ASCII_ENTER == keyHitData.KbdData) {
      ui8_1++;
      if (ui8_1 >= 2)
	break;
    } else {
      ui8_1 = 0;
      for (ui8_2=0; ui8_2<(LCD_MAX_COL-1); ui8_2++)
	bufSS[ui8_2] = bufSS[ui8_2+1];
      if (isgraph(keyHitData.KbdData)) {
	bufSS[LCD_MAX_COL-1] = keyHitData.KbdData;
      } else {
	bufSS[LCD_MAX_COL-1] = ' ';
      }
      LCD_CLRLINE(LCD_MAX_ROW-1);
      LCD_WR_N((char *)bufSS, LCD_MAX_COL);
    }
  }
  diagStatus |= (0 == menuGetYesNo((const uint8_t *)PSTR("Did Keypad work?"), 16)) ? DIAG_KEYPAD : 0;

  /* Verify Keyboard */
  LCD_CLRSCR;
  LCD_WR_NP((const uint8_t *)PSTR("Diagnosis PS2"), 13);
  _delay_ms(1000);
  LCD_CLRLINE(LCD_MAX_ROW-1);
  LCD_WR_NP((const uint8_t *)PSTR("Hit \xDB \xDB to exit"), 16);
  LCD_refresh();
  _delay_ms(2000);
  LCD_CLRSCR;
  LCD_WR_NP((const uint8_t *)PSTR("Key Entered..."), 14);
  for (ui8_2=0; ui8_2<LCD_MAX_COL; ui8_2++)
    bufSS[ui8_2] = ' ';
  for (ui8_1=0; ;) {
    LCD_refresh();
    KBD_RESET_KEY;
    KBD_GETCH;
    if (!isgraph(keyHitData.KbdData)) {
      ui8_1++;
      if (ui8_1 >= 2)
	break;
    } else {
      ui8_1 = 0;
      for (ui8_2=0; ui8_2<(LCD_MAX_COL-1); ui8_2++)
	bufSS[ui8_2] = bufSS[ui8_2+1];
      if (isgraph(keyHitData.KbdData)) {
	bufSS[LCD_MAX_COL-1] = keyHitData.KbdData;
      } else {
	bufSS[LCD_MAX_COL-1] = ' ';
      }
      LCD_CLRLINE(LCD_MAX_ROW-1);
      LCD_WR_N((char *)bufSS, LCD_MAX_COL);
    }
  }
  diagStatus |= (0 == menuGetYesNo((const uint8_t *)PSTR("Did PS2 worked?"), 15)) ? DIAG_PS2 : 0;

  /* FIXME: Check weighing machine connectivity */
  LCD_CLRSCR;
  LCD_WR_NP((const uint8_t *)PSTR("Diag Weighing Mc"), 16);
  _delay_ms(1000);
  LCD_CLRLINE(LCD_MAX_ROW-1);
  LCD_WR_NP((const uint8_t *)PSTR("Hit \xDB \xDB to exit"), 16);
  LCD_refresh();
  _delay_ms(2000);
  LCD_CLRSCR;
  LCD_WR_NP((const uint8_t *)PSTR("Weight in KGs..."), 16);
  LCD_refresh();
  KBD_RESET_KEY;
  for (ui8_1=0; ;) {
    /* FIXME: display the weight */
    LCD_refresh();
    _delay_ms(500);
    if (KBD_HIT) {
      if (ASCII_ENTER == keyHitData.KbdData) {
	ui8_1++;
	if (ui8_1 >= 2)
	  break;
      }
      KBD_RESET_KEY;
    }
  }
  diagStatus |= (0 == menuGetYesNo((const uint8_t *)PSTR("Did Weigh m/c?"), 14)) ? DIAG_WEIGHING_MC : 0;

#if FF_ENABLE
  if (DS_DEV_INVALID != (devStatus & DS_DEV_INVALID)) {
  /* FIXME: Verify SD card */
  LCD_CLRSCR;
  LCD_WR_NP((const uint8_t *)PSTR("Diagnosis SD"), 12);
  _delay_ms(1000);
  {
    UINT bw;
    if (FR_OK == f_mount(&FS, "", 0)) {
      if (f_open(&Fil, "diag.txt", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {
	if (FR_OK == f_write(&Fil, "Diag test file creation\r\n", 25, &bw)) {
	  diagStatus |= (25 == bw) ? DIAG_SD : 0;
	  f_close(&Fil);
	} else {
	}
      } else {
      }
      f_mount(NULL, "", 0);
    } else {
    }
  }
  }
#endif

  /* Verify Buzzer */
  LCD_CLRSCR;
  LCD_WR_NP((const uint8_t *)PSTR("Diagnosis Buzzer"), 16);
  _delay_ms(1000);
  for (ui8_1=0; ui8_1<5; ui8_1++) {
    BUZZER_ON;
    _delay_ms(1000);
    BUZZER_OFF;
  }
  diagStatus |= (0 == menuGetYesNo((const uint8_t *)PSTR("Did Buzzer Buzz?"), 16)) ? DIAG_BUZZER : 0;

  /* save status*/
  eeprom_update_word((uint16_t *)(offsetof(struct ep_store_layout, unused_DiagStat)), diagStatus);

  /* FIXME : Print the status */
  LCD_CLRSCR;
  if (0 != menuGetYesNo((const uint8_t *)PSTR("Print Status?"), 13))
    return;
  /* We can't Diagonise Battery charging, so print sentence */
#endif
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
  uint8_t menu_selected, menu_selhier, ui8_1, ui8_2;

  /* initialize
     0==menu_selhier : hier not selected
   */
  menu_selected = 0;
  menu_selhier = 0;

menuMainStart:
  /* Clear known key press */
  KBD_RESET_KEY;

  /* First select a Menu */
  LCD_CLRSCR;
  eeprom_read_block(bufSS, (uint8_t *)offsetof(struct ep_store_layout, shop_name), SHOP_NAME_SZ_MAX);
  bufSS[SHOP_NAME_SZ_MAX] = 0;
  if (0 == menu_selhier) {
    /* Display shop name */
    LCD_WR_N((char *)bufSS, LCD_MAX_COL);
    LCD_CLRLINE(LCD_MAX_ROW-1);
    LCD_WR_NP((const uint8_t *)(menu_hier_names+(menu_selected*MENU_HIER_NAME_SIZE)), MENU_HIER_NAME_SIZE);
  } else {
    /* Shop name (8 chars) */
    LCD_WR_N((char *)bufSS, ((7<SHOP_NAME_SZ_MAX)?7:SHOP_NAME_SZ_MAX));
    LCD_PUTCH('>');
    LCD_WR_NP((const uint8_t *)(menu_hier_names+((menu_selhier-1)*MENU_HIER_NAME_SIZE)), ((LCD_MAX_COL-8)<MENU_HIER_NAME_SIZE)?(LCD_MAX_COL-8):MENU_HIER_NAME_SIZE);
    LCD_CLRLINE(LCD_MAX_ROW-1);
    LCD_WR_NP((const uint8_t *)(menu_names+(menu_selected*MENU_NAMES_LEN)), MENU_NAMES_LEN);
  }
  LCD_refresh();

  /* Wait until get command from user */
  KBD_RESET_KEY;
  KBD_GETCH;

  if ((ASCII_ENTER == keyHitData.KbdData) && (0 == menu_selhier)) {
    menu_selhier = menu_selected + 1;
    for (ui8_1=0; ui8_1<pgm_read_byte(&MENU_MAX); ui8_1++) {
      if ( ((pgm_read_byte(menu_hier+ui8_1)) == menu_selhier) /* menu appropriate */ &&
	   (0 != (MenuMode & (pgm_read_byte(menu_mode+ui8_1) & MENU_MODEMASK))) /* mode appropriate */
	   ) {
	menu_selected = ui8_1;
	break;
      }
    }
    /* No valid menu items, go back */
    if (pgm_read_byte(&MENU_MAX) == ui8_1) {
      menu_selhier = 0; /* menu_selected remains the same */
    }
  } else if ((ASCII_ENTER == keyHitData.KbdData) && (0 != menu_selhier)) {
    /* */
    LCD_CLRSCR;
    LCD_WR_NP((const uint8_t *)(menu_hier_names+((menu_selhier-1)*MENU_HIER_NAME_SIZE)), MENU_HIER_NAME_SIZE);
    LCD_PUTCH('>');
    LCD_WR_NP((const uint8_t *)(menu_names+(menu_selected*MENU_NAMES_LEN)), ((MENU_NAMES_LEN>6)?6:MENU_NAMES_LEN));

    /* Get choices before menu function is called */
    KBD_RESET_KEY;
    arg1.valid = MENU_ITEM_NONE;
    arg1.value.str.sptr = bufSS;
    arg1.value.str.len = LCD_MAX_COL;
    LCD_CLRSCR;
    menuGetOpt(menu_prompt_str+(pgm_read_byte(menu_prompts+(menu_selected<<1))*MENU_PROMPT_LEN), &arg1, pgm_read_byte(menu_args+(menu_selected<<1)));
    KBD_RESET_KEY;
    arg2.valid = MENU_ITEM_NONE;
    arg2.value.str.sptr = bufSS+LCD_MAX_COL+2;
    arg2.value.str.len = LCD_MAX_COL;
    LCD_CLRSCR;
    menuGetOpt(menu_prompt_str+(pgm_read_byte(menu_prompts+(menu_selected<<1)+1)*MENU_PROMPT_LEN), &arg2, pgm_read_byte(menu_args+(menu_selected<<1)+1));
    if ( (arg1.valid == (MENU_ITEM_TYPE_MASK&pgm_read_byte(menu_args+(menu_selected<<1)))) &&
	 (arg2.valid == (MENU_ITEM_TYPE_MASK&pgm_read_byte(menu_args+(menu_selected<<1)+1))) ) {
#ifdef UNIT_TEST_MENU_1
      UNIT_TEST_MENU_1(menu_selected);
#else
      if (0 == (devStatus & DS_DEV_INVALID)) {
	//printf("call 0x%x\n", pgm_read_dword(menu_handlers+menu_selected));
	((menu_func_t)(uint16_t)pgm_read_dword((void *)(menu_handlers+menu_selected)))(menu_mode[menu_selected]);
      }
#endif
    }
  } else if ((ASCII_LEFT == keyHitData.KbdData) || (ASCII_UP == keyHitData.KbdData)) {
    if (0 == menu_selhier) {
      /* selection of menu */
      menu_selected = (0 == menu_selected) ? MENU_HIER_MAX-1 : menu_selected-1;
    } else { /* (0 != menu_selhier) && (0 != menu_selected) */
      ui8_2 = menu_selected;
      for (ui8_1=0; ui8_1<pgm_read_byte(&MENU_MAX); ui8_1++) {
	menu_selected--;
	if ( ((pgm_read_byte(menu_hier+menu_selected)) == menu_selhier) /* menu appropriate */ &&
	     (0 != (MenuMode & (pgm_read_byte(menu_mode+menu_selected) & MENU_MODEMASK))) /* mode appropriate */
	     ) {
	  break;
	}
      }
      /* No valid menu items, go back */
      if (pgm_read_byte(&MENU_MAX) <= ui8_1) {
	if ((ASCII_LEFT == keyHitData.KbdData) ||
	  (ASCII_UP == keyHitData.KbdData)) {
	  menu_selhier = menu_selected = 0;
	} else {
	  menu_selected = ui8_2;
	}
      }
    }
  } else if ((ASCII_RIGHT == keyHitData.KbdData) || (ASCII_DOWN == keyHitData.KbdData)) {
    if (0 == menu_selhier) {
      menu_selected = (menu_selected >= (MENU_HIER_MAX-1)) ? 0 : menu_selected+1;
    } else {
      ui8_2 = menu_selected;
      for (ui8_1=0; ui8_1<pgm_read_byte(&MENU_MAX); ui8_1++) {
	menu_selected = (pgm_read_byte(&MENU_MAX) < (1+menu_selected)) ? menu_selected+1 : 0;
	if ( ((pgm_read_byte(menu_hier+menu_selected)) == menu_selhier) /* menu appropriate */ &&
	     (0 != (MenuMode & (pgm_read_byte(menu_mode+menu_selected) & MENU_MODEMASK))) /* mode appropriate */
	     ) {
	  break;
	}
      }
      /* No valid menu items, go back */
      if (pgm_read_byte(&MENU_MAX) <= ui8_1) {
	if (ASCII_DOWN == keyHitData.KbdData) {
	  menu_selhier = menu_selected = 0;
	} else {
	  menu_selected = ui8_2;
	}
      }
    }
  }

  LCD_refresh();
#ifdef UNIT_TEST
  //  move(0, 0);
  //  printw("menu_selhier:%d  menu_selected:%d", menu_selhier, menu_selected);

  /* Provide means to excape the infinite hold */
  if (ASCII_F2 == keyHitData.KbdData) {
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
#if FF_ENABLE
  if (DS_DEV_INVALID != (devStatus & DS_DEV_INVALID)) {
  UINT  ret_size, ui1;
  uint16_t ui16_1;
  uint8_t ui8_1;

  /* */
  memset(&FS, 0, sizeof(FS));
  memset(&Fil, 0, sizeof(Fil));
  f_mount(&FS, ".", 1);
  if (FR_OK != f_open(&Fil, SD_ITEM_FILE, FA_READ)) {
    LCD_ALERT(PSTR("File open error"));
    goto menuSDLoadItemExit;
  }

  /* check version string */
  if (0 == f_size(&Fil)) {
    LCD_ALERT(PSTR("File size error"));
    goto menuSDLoadItemExit;
  } else if (FR_OK != f_read(&Fil, bufSS, GIT_HASH_SMALL_LEN, &ret_size)) {
    LCD_ALERT(PSTR("File1 error"));
    goto menuSDLoadItemExit;
  } else if (GIT_HASH_SMALL_LEN != ret_size) {
    LCD_ALERT(PSTR("File2 error "));
    goto menuSDLoadItemExit;
  }
  for (ui8_1=0; ui8_1<GIT_HASH_SMALL_LEN; ui8_1++) {
    if (GIT_HASH_SMALL[ui8_1] != bufSS[ui8_1]) {
      LCD_ALERT(PSTR("Incompatible file"));
      goto menuSDLoadItemExit;
    }
  }

  /* Mark all other items as deleted : (0==id) */
  for (ui16_1=0; ui16_1 < (ITEM_MAX_ADDR>>EEPROM_MAX_DEVICES_LOGN2);
       ui16_1+=(ITEM_SIZEOF>>EEPROM_MAX_DEVICES_LOGN2)) {
    /* id 0 is invalid */
    ee24xx_write_bytes(ui16_1+(offsetof(struct item, id)>>EEPROM_MAX_DEVICES_LOGN2), NULL, BUFSS_SIZE);
  }

  /* Check for crc in file */
  assert(FR_OK == f_lseek(&Fil, 0));
  bufSS[BUFSS_SIZE-2] = 0, bufSS[BUFSS_SIZE-1] = 0;
  ui8_1 = 0, ui16_1 = 0;
  while (FR_OK == f_read(&Fil, bufSS, BUFSS_SIZE-2, &ret_size)) {
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
  if ((0 == ui16_1) || (ui16_1 != ret_size) || (9 != ui8_1)) {
    //printf("ui16_1:%x ret_size:%x, ui8_1:%d\n", ui16_1, ret_size, ui8_1);
    LCD_ALERT_16N(PSTR("File3 error "), ui16_1);
    goto menuSDLoadItemExit;
  }

  /* init indexes with 0s */
  ee24xx_write_bytes((ITEM_MAX_ADDR>>EEPROM_MAX_DEVICES_LOGN2), NULL, EEPROM_MAX_ADDRESS-ITEM_MAX_ADDR+1);

  /* */
  assert(FR_OK == f_lseek(&Fil, GIT_HASH_SMALL_LEN));
  struct item *it = (void *)bufSS;
  while (FR_OK == f_read(&Fil, bufSS, ITEM_SIZEOF, &ret_size)) {
    if (ITEM_SIZEOF != ret_size) break; /* reached last */
    if (0 == it->id) continue;
    //printf("loading id:%d\n", it->id);
    ui16_1 = menuItemAddr((it->id-1));
    menuIndexItem(it);
    ee24xx_write_bytes(ui16_1, bufSS, ITEM_SIZEOF);
  }
  assert(2 == ret_size); /* crc would be pending */

  /* when all goes well */
  LCD_ALERT(PSTR("Item Loaded.."));

 menuSDLoadItemExit:
  /* */
  f_mount(NULL, "", 0);
  }
#endif
}

void
menuSDSaveItem(uint8_t mode)
{
#if MENU_SDSAVE_EN && FF_ENABLE
  if (DS_DEV_INVALID != (devStatus & DS_DEV_INVALID)) {
  UINT  ret_size;
  uint16_t ui16_1, ui16_2, signature;
  struct item *it = (void *)bufSS;
  uint8_t ui8_1;

  /* init */
  memset(&FS, 0, sizeof(FS));
  memset(&Fil, 0, sizeof(Fil));

  /* */
  f_mount(&FS, ".", 1);
  if (FR_OK != f_open(&Fil, SD_ITEM_FILE ".out", FA_WRITE)) {
    LCD_ALERT(PSTR("File open error"));
    goto menuSDSaveItemExit;
  }

  /* Add version string */
  strncpy_P((char *)bufSS, PSTR(GIT_HASH_SMALL), GIT_HASH_SMALL_LEN);
  signature = 0;
  f_write(&Fil, bufSS, GIT_HASH_SMALL_LEN, &ret_size);
  for (ui8_1=0; ui8_1<GIT_HASH_SMALL_LEN; ui8_1++)
    signature = _crc16_update(signature, bufSS[ui8_1]);
  assert(GIT_HASH_SMALL_LEN == ret_size);

  /* */
  for (ui16_1=0; ui16_1 < (ITEM_MAX_ADDR>>EEPROM_MAX_DEVICES_LOGN2);
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
	f_write(&Fil, bufSS+ui16_2, ITEM_SIZEOF-ui16_2, &ret_size);
      }
      for (ui8_1=0; ui8_1<ITEM_SIZEOF; ui8_1++) {
	signature = _crc16_update(signature, bufSS[ui8_1]);
      }
    }
  }
  bufSS[1] = signature; signature>>=8; bufSS[0] = signature;
  f_write(&Fil, bufSS, 2, &ret_size);
  assert(2 == ret_size);

  /* when all goes well */
  LCD_ALERT(PSTR("Item Saved.."));

 menuSDSaveItemExit:
  /* */
  f_mount(NULL, "", 0);
  }
#endif
}

void
menuSDLoadSettings(uint8_t mode)
{
#if FF_ENABLE
  if (DS_DEV_INVALID != (devStatus & DS_DEV_INVALID)) {
  UINT  ret_size;
  uint16_t ui16_1;
  uint8_t ui8_1;
  uint8_t serial_no[SERIAL_NO_MAX];

  /* init */
  memset(&FS, 0, sizeof(FS));
  memset(&Fil, 0, sizeof(Fil));

  /* */
  f_mount(&FS, ".", 1);
  if (FR_OK != f_open(&Fil, SD_SETTINGS_FILE, FA_READ)) {
    LCD_ALERT(PSTR("File open error"));
    goto menuSDLoadSettingsExit;
  }

  /* check version string */
  if (0 == f_size(&Fil)) {
    LCD_ALERT(PSTR("File size error"));
    goto menuSDLoadSettingsExit;
  } else if (FR_OK != f_read(&Fil, bufSS, GIT_HASH_SMALL_LEN, &ret_size)) {
    LCD_ALERT(PSTR("File error"));
    goto menuSDLoadSettingsExit;
  } else if (GIT_HASH_SMALL_LEN != ret_size) {
    LCD_ALERT(PSTR("File error "));
    goto menuSDLoadSettingsExit;
  }
  for (ui8_1=0; ui8_1<GIT_HASH_SMALL_LEN; ui8_1++) {
    if (GIT_HASH_SMALL[ui8_1] != bufSS[ui8_1]) {
      LCD_ALERT(PSTR("Incompatible file"));
      goto menuSDLoadSettingsExit;
    }
  }

  /* save serial number for updating later */
  for (ui8_1=0; ui8_1<SERIAL_NO_MAX; ui8_1++)
    serial_no[ui8_1] = eeprom_read_byte((uint8_t *)offsetof(struct ep_store_layout, unused_serial_no)+ui8_1);

  /* Check for crc in file */
  bufSS[BUFSS_SIZE-2] = 0, bufSS[BUFSS_SIZE-1] = 0;
  ui16_1 = 0;
  assert(FR_OK == f_lseek(&Fil, 0));
  while (FR_OK == f_read(&Fil, bufSS, BUFSS_SIZE-2, &ret_size)) {
    if (0 == ret_size) break;
    if (0 != ui16_1) { /* skip first time */
      ui16_1 = _crc16_update(ui16_1, bufSS[BUFSS_SIZE-2]);
      ui16_1 = _crc16_update(ui16_1, bufSS[BUFSS_SIZE-1]);
    }
    for (ui8_1=0; ui8_1<(ret_size-2); ui8_1++) {
      ui16_1 = _crc16_update(ui16_1, bufSS[ui8_1]);
    }
    bufSS[BUFSS_SIZE-1] = bufSS[ret_size-1];
    bufSS[BUFSS_SIZE-2] = bufSS[ret_size-2];
  }
  ret_size = bufSS[BUFSS_SIZE-2];
  ret_size <<= 8;
  ret_size |= bufSS[BUFSS_SIZE-1];
  if ((0 == ui16_1) || (ui16_1 != ret_size)) {
    //printf("ui16_1:%x ret_size:%x\n", ui16_1, ret_size);
    LCD_ALERT_16N(PSTR("File error "), ui16_1);
    goto menuSDLoadSettingsExit;
  }

  /* */
  assert(FR_OK == f_lseek(&Fil, 0));
  ui16_1 = 0;
  assert(FR_OK == f_lseek(&Fil, GIT_HASH_SMALL_LEN));
  while (FR_OK == f_read(&Fil, bufSS, BUFSS_SIZE, &ret_size)) {
    if (0 == ret_size) break; /* reached last */
    eeprom_update_block((const void *)bufSS, (void *)ui16_1, ret_size);
    ui16_1 += ret_size;
  }
  assert((EP_STORE_LAYOUT_SIZEOF+2) == ui16_1); /* crc would be pending */

  /* restore serial number */
  for (ui8_1=0; ui8_1<SERIAL_NO_MAX; ui8_1++)
    eeprom_update_byte((uint8_t *)offsetof(struct ep_store_layout, unused_serial_no)+ui8_1, serial_no[ui8_1]);
  
  /* when all goes well */
  LCD_ALERT(PSTR("Settings Loaded.."));

 menuSDLoadSettingsExit:
  /* */
  f_close(&Fil);
  f_mount(NULL, "", 0);
  }
#endif
}

void
menuSDSaveSettings(uint8_t mode)
{
#if MENU_SDSAVE_EN && FF_ENABLE
  if (DS_DEV_INVALID != (devStatus & DS_DEV_INVALID)) {
  UINT  ret_size;
  uint16_t ui16_1, ui16_2, ui16_3, signature;
  uint8_t ui8_1;

  /* init */
  signature = 0;
  memset(&FS, 0, sizeof(FS));
  memset(&Fil, 0, sizeof(Fil));

  /* */
  f_mount(&FS, ".", 1);
  if (FR_OK != f_open(&Fil, SD_SETTINGS_FILE, FA_WRITE)) {
    LCD_ALERT(PSTR("File open error"));
    goto menuSDSaveSettingsExit;
  }

  /* Add version string */
  strncpy_P((char *)bufSS, PSTR(GIT_HASH_SMALL), GIT_HASH_SMALL_LEN);
  f_write(&Fil, bufSS, GIT_HASH_SMALL_LEN, &ret_size);
  for (ui8_1=0; ui8_1<GIT_HASH_SMALL_LEN; ui8_1++)
    signature = _crc16_update(signature, bufSS[ui8_1]);
  assert(GIT_HASH_SMALL_LEN == ret_size);

  /* */
  for (ui16_1=0; ui16_1<EP_STORE_LAYOUT_SIZEOF; ui16_1+=ui16_3) {
    ui16_3 = ((EP_STORE_LAYOUT_SIZEOF-ui16_1) >= BUFSS_SIZE) ?
      BUFSS_SIZE : (EP_STORE_LAYOUT_SIZEOF-ui16_1);
    eeprom_read_block((void *)bufSS, (const void *)ui16_1, ui16_3);
    f_write(&Fil, bufSS, ui16_3, &ret_size);
    assert(ret_size == ui16_3);
    for (ui16_2=0; ui16_2<ui16_3; ui16_2++) {
      signature = _crc16_update(signature, bufSS[ui16_2]);
    }
  }
  assert(ui16_1 == EP_STORE_LAYOUT_SIZEOF);
  bufSS[1] = signature; signature>>=8; bufSS[0] = signature;
  f_write(&Fil, bufSS, 2, &ret_size);
  assert(2 == ret_size);

  /* when all goes well */
  LCD_ALERT(PSTR("Settings Saved.."));

 menuSDSaveSettingsExit:
  /* */
  f_mount(NULL, "", 0);
  }
#endif
}
