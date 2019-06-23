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
#include <avr/wdt.h>

#include "ep_ds.h"
#include "version.h"
#include "assert.h"
#include "lcd.h"
#include "kbd.h"
#include "ep_store.h"
#include "item.h"
#include "billing.h"
#include "i2c.h"
#include "uart.h"
#include "a1micro2mm.h"
#include "integer.h"
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

typedef uint8_t (*menu_func_t)(uint8_t mode);

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
#define MENU_STR1_IDX_DELETE 10
#define MENU_STR1_IDX_NAME  11
#define MENU_STR1_IDX_REPLA 12
#define MENU_STR1_IDX_DAY   13
#define MENU_STR1_IDX_MONTH 14
#define MENU_STR1_IDX_YEAR  15
#define MENU_STR1_IDX_PRODCODE  16
#define MENU_STR1_IDX_UNICODE   17
#define MENU_STR1_IDX_ENTRYES   18
#define MENU_STR1_IDX_COMNDISC  19
#define MENU_STR1_IDX_OLD       20
#define MENU_STR1_IDX_FILEERR   21
#define MENU_STR1_IDX_INVALID   22
#define MENU_STR1_IDX_SUCCESS   23 /* Keep this last, used by LCD_ALERT */
#define MENU_STR1_IDX_NUM_ITEMS 24
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
  "Old:    " /*20 */
  "FileErr " /*21 */
  "Invalid!" /*22 */
  "Success!" /*23 */
  ;

/* */
static uint8_t MenuMode;
uint8_t devStatus;   /* 0 is no err */

/* Diagnosis */
uint16_t diagStatus;

/* indexing */
#define ITEM_SUBIDX_NAME 2
typedef struct  __attribute__((packed)) {
  uint8_t crc_name3;
  uint8_t crc_prod_code;
} itemIdxs_t;
itemIdxs_t      itIdxs[ITEM_MAX];
uint16_t   numValidItems;

/* Found issue with memset */
#define menuMemset(S, c, N) do {			\
    for (uint16_t _ui16_1=0; _ui16_1<N; _ui16_1++) {	\
      ((uint8_t *)S)[_ui16_1] = c;				\
    }							\
  } while (0)

#define MENU_GET_OPT(Q, A, T, H) do {		\
    menuGetOpt(Q, A, T, H);			\
  } while (T != (A)->valid)

/* Helper routine to obtain input from user */
void
menuGetOpt(const uint8_t *prompt, menu_arg_t *arg, uint8_t opt, menuGetOptHelper helper)
{
  uint8_t item_type = (opt & MENU_ITEM_TYPE_MASK);
  uint32_t val, prev_helper = 0;
  uint16_t ui16_1, ui16_2, buf_idx;
  uint8_t sbuf[LCD_MAX_COL], *buf;

  if (MENU_ITEM_NONE == opt) return;
  arg->valid = MENU_ITEM_NONE;

  /* init */
  if (MENU_ITEM_STR == item_type) {
    assert(0 != arg->value.str.len);
    for (ui16_1=0; ui16_1<arg->value.str.len; ui16_1++)
      arg->value.str.sptr[ui16_1] = ' ';
    buf = arg->value.str.sptr;
  } else {
    for (ui16_1=0; ui16_1<LCD_MAX_COL; ui16_1++)
      sbuf[ui16_1] = ' ';
    buf = sbuf;
  }
  buf_idx = 0;

  /* Assume it is a string, then typecast it */
  uint8_t loop_in = 1;
  do {
    prev_helper = helper ? (*helper)(buf, buf_idx, prev_helper) : prev_helper;
    /* Ask a question */
    LCD_CLRLINE(LCD_MAX_ROW-1);
    LCD_WR_NP((const char *)prompt, MENU_PROMPT_LEN);
    LCD_PUTCH('?');
    for (ui16_1=MENU_PROMPT_LEN+1,
	   ui16_2=(buf_idx<(LCD_MAX_COL-MENU_PROMPT_LEN)) ? 0 : buf_idx-(LCD_MAX_COL-MENU_PROMPT_LEN-1);
	 (ui16_1<LCD_MAX_COL) && (ui16_2<buf_idx); ui16_1++, ui16_2++) {
      if (opt&MENU_ITEM_PASSWD) {
	LCD_PUTCH('*');
      } else {
	LCD_PUTCH(buf[ui16_2]);
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
      }
      break;
    case ASCII_LF:
    case ASCII_ENTER:
      break;
    case ASCII_ESCAPE:
    case ASCII_RIGHT:
    case ASCII_DOWN:
    case ASCII_PRNSCRN:
    case ASCII_NUMLK:
    case ASCII_UNDEF:
    case ASCII_DEFINED:
    case ASCII_F2:
      loop_in = 0;
      break;
    default:
      prev_helper = 0; /* start searching from first */
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
  } while (loop_in && (keyHitData.KbdData != ASCII_ENTER));

  menu_error = 1;
  if (ASCII_ESCAPE == keyHitData.KbdData)
    return;

  if (0 != prev_helper) {
    arg->value.integer.i8  = prev_helper&0xFF;
    arg->value.integer.i16 = prev_helper;
    arg->value.integer.i32 = prev_helper;
    item_type = MENU_ITEM_ID;
    menu_error = 0;
  } else if ( (MENU_ITEM_ID == item_type) || (MENU_ITEM_FLOAT == item_type) ) {
    val = 0;
    for (ui16_1=0; ui16_1<buf_idx; ui16_1++) {
      if ((buf[ui16_1] >= '0') && (buf[ui16_1] <= '9')) {
	val *= 10;
	val += buf[ui16_1] - '0';
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
    for (ui16_1=0; ui16_1<(item_type+2); ui16_1++) {
      if ((buf[ui16_1] < '0') || (buf[ui16_1] > '9'))
	menu_error++;
    }
    if (0 == menu_error) {
      /* Date */
      val = 0;
      if (MENU_ITEM_DATE == item_type) {
	val |= (buf[0] - '0') & 0x3; /* max: 3 */
	val <<= 4;
	val |= (buf[1] - '0') % 10; /* max: 9 */
	if ((0 == val) || (val > 0x31)) menu_error++;
	arg->value.date.day = val;
	buf += 2;
      }
      /* Month */
      val = 0;
      val |= (buf[0] - '0') & 0x1; /* max: 1 */
      val <<= 4;
      val |= (buf[1] - '0') % 13; /* max: 2 */
      if ((0 == val) || (val > 0x12)) menu_error++;
      arg->value.date.month = val;
      buf += 2;
      /* Year */
      val = 0;
      buf += 2;
      val |= (buf[0] - '0') & 0xF; /* max: 3 */
      val <<= 4;
      val |= (buf[1] - '0') % 10; /* max: 9 */
      buf += 2;
      if (val < 0x16) menu_error++;
      arg->value.date.year = val;
    }
  } else if (MENU_ITEM_TIME == item_type) {
    /* format HHMM */
    menu_error = 0;
    for (ui16_1=0; ui16_1<4; ui16_1++) {
      if ((buf[ui16_1] < '0') || (buf[ui16_1] > '9'))
	menu_error++;
    }
    if (0 == menu_error) {
      /* Hour */
      val = 0;
      val |= (buf[0] - '0') % 3; /* max: 2 */
      val <<= 4;
      val |= (buf[1] - '0') % 4; /* max: 3 */
      buf += 2;
      if (val>0x23) menu_error++;
      arg->value.time.hour = val;
      /* Mins */
      val = 0;
      val |= (buf[0] - '0') % 7; /* max: 6 */
      val <<= 4;
      val |= (buf[1] - '0') % 10; /* max: 10 */
      buf += 2;
      if (val > 0x59) menu_error++;
      arg->value.time.min = val;
    }
  } else if (MENU_ITEM_STR == item_type) {
    menu_error = (0 == buf_idx) ? 1 : 0;
  } else assert(0);

  /* */
  if (0 == menu_error) {
    arg->valid = item_type;
  } else if (opt & MENU_ITEM_OPTIONAL) {
  } else if ((opt & MENU_ITEM_DONTCARE_ON_PREV) && (arg == &arg2)) {
  } else {
    arg->valid = MENU_ITEM_NONE;
  }
  //move(0, 30); printw("GetOpot:'%s' err:%d", buf, menu_error);
}

#define soft_reset()        \
  do {			    \
    wdt_enable(WDTO_15MS);  \
    for(;;) { }		    \
  } while(0)

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
    LCD_WR_NP((const char *)quest, size);
    LCD_PUTCH(':');
    LCD_WR_P((const char *)menu_str2+((ret)*4));
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
    } else if (ASCII_ESCAPE == keyHitData.KbdData) {
      return 1;
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
    LCD_WR_NP((const char *)quest, ui8_1);
    LCD_WR_P((const char *)PSTR(":"));
    LCD_WR_N((uint8_t *)(opt_arr+(ret*choice_len)), choice_len);
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
eeprom_update_byte_NP(uint16_t addr, const char *pstr, uint8_t size)
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
uint8_t
menuFactorySettings(uint8_t mode)
{
  uint8_t ui8_1;
  uint16_t ui16_1;

  assert(MENU_MSUPER == MenuMode);

  /* confirm before proceeding */
  if (0 == (mode & MENU_NOCONFIRM)) {
    ui8_1 = menuGetYesNo((const uint8_t *)PSTR("Fact Reset"), 11);
    if (0 != ui8_1) return 0;
  }

  /* date, time */
#ifdef UNIT_TEST
  ui8_1 = 1;
#else
  ui8_1 = menuGetYesNo((const uint8_t *)PSTR("Set Date/Time?"), 11);
#endif
  if (0 == ui8_1) {
    MENU_GET_OPT(menu_prompt_str+(MENU_PR_DATE*MENU_PROMPT_LEN), &arg1, MENU_ITEM_DATE, NULL);
    MENU_GET_OPT(menu_prompt_str+(MENU_PR_TIME*MENU_PROMPT_LEN), &arg2, MENU_ITEM_TIME, NULL);
  } else { /* BCD format */
    arg1.value.date.year = 0x15;
    arg1.value.date.month = 1;
    arg1.value.date.day = 1;
    arg1.valid = MENU_ITEM_DATE;
    arg2.value.time.hour = 9;
    arg2.value.time.min = 0;
    arg2.valid = MENU_ITEM_TIME;
  }
  if (MENU_ITEM_DATE == arg1.valid) {
    timerDateSet(arg1.value.date.year, arg1.value.date.month, arg1.value.date.day);
  }
  if (MENU_ITEM_TIME == arg2.valid) {
    timerTimeSet(arg1.value.time.hour, arg1.value.time.min);
  }

  /* Show progress */
  LCD_CLRLINE(0);
  LCD_WR_NP((const char *)PSTR("FactRst Progress"), 16);
  LCD_CLRLINE(LCD_MAX_ROW-1);
  LCD_refresh();

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

  /* index to start searching for empty item */
  eeprom_update_word((uint16_t *)offsetof(struct ep_store_layout, unused_ItemLastUsed), 0);

  /* */
  eeprom_update_byte_NP(offsetof(struct ep_store_layout, shop_name),
			PSTR("Sri Ganapathy Medicals"), SHOP_NAME_SZ_MAX);
  eeprom_update_byte_NP(offsetof(struct ep_store_layout, b_head),
			PSTR("12 Agraharam street, New Tippasandara,\n Bangalore - 52\n TIN:299007249"), HEADER_SZ_MAX);
  eeprom_update_byte_NP(offsetof(struct ep_store_layout, b_foot),
			PSTR("Welcome & come back..."), FOOTER_SZ_MAX);
  eeprom_update_byte_NP(offsetof(struct ep_store_layout, currency),
			PSTR("Rupees"), EPS_WORD_LEN);
  eeprom_update_byte_NP(offsetof(struct ep_store_layout, b_pfix),
			PSTR("A000"), EPS_WORD_LEN);
  eeprom_update_byte_NP(offsetof(struct ep_store_layout, caption),
			PSTR("Invoice"), EPS_CAPTION_SZ_MAX);
  LCD_PUTCH('.'); LCD_refresh();

  /* All numerical data */
  eeprom_update_word((uint16_t *)offsetof(struct ep_store_layout, RndOff), 50);

  /* */
  eeprom_update_byte((uint8_t *)offsetof(struct ep_store_layout, idle_wait), 5);
  eeprom_update_word((uint16_t *)offsetof(struct ep_store_layout, unused_next_billaddr), EEPROM_SALE_START_ADDR);
  LCD_PUTCH('.'); LCD_refresh();

  /* At the end */
  LCD_CLRSCR; LCD_refresh();

  return 0;
}

void
menuInit()
{
  uint16_t ui16_1, ui16_2, ui16_3;
  uint8_t ui8_1, ui8_2;
  MenuMode = MENU_MRESET;

  /* csv2dat depends on this number (ITEM_MAX/ITEM_MAX_ADDR) */
#if UNICODE_ENABLE
  assert((40+16) == ITEM_SIZEOF);
#else
  assert(40 == ITEM_SIZEOF);
#endif
  assert ((ITEM_SIZEOF+LCD_MAX_COL+LCD_MAX_COL+4) < BUFSS_SIZE);
  assert ((SALE_SIZEOF+LCD_MAX_COL+LCD_MAX_COL+4) < BUFSS_SIZE);
  assert(1 == sizeof(uint8_t));
  //  assert(sizeof(void *) == sizeof(uint16_t));
  assert(((offsetof(struct item, name)&(0xFFFF<<EEPROM_MAX_DEVICES_LOGN2))>>EEPROM_MAX_DEVICES_LOGN2) == (offsetof(struct item, name)>>EEPROM_MAX_DEVICES_LOGN2));
  assert(0 == (ITEM_SIZEOF % (1<<EEPROM_MAX_DEVICES_LOGN2)));

  /* init global vars */
  devStatus = 0;
  diagStatus = 0;

  /* Identify capability of device from serial number
     if byte0-4 of internal EEPROM is FAC7, then do factory
     reset and copy serial number
   */
  uint32_t ui32_1;
  for (ui8_1=0, ui32_1=0; ui8_1<4; ui8_1++) {
    ui32_1 <<= 8;
    ui32_1 |= eeprom_read_byte((uint8_t *)(uint16_t)ui8_1);
  }
  for (uint8_t loopCnt=0; loopCnt<2; loopCnt++) {
    for (ui8_1=0, ui16_1=0; ui8_1<(SERIAL_NO_MAX-2); ui8_1++) {
      ui16_1 = _crc16_update(ui16_1, eeprom_read_byte((uint8_t *)offsetof(struct ep_store_layout, unused_serial_no)+ui8_1));
    }
    ui16_2 = eeprom_read_byte((uint8_t *)offsetof(struct ep_store_layout, unused_serial_no)+SERIAL_NO_MAX-2);
    ui16_2 <<= 8;
    ui16_2 |= eeprom_read_byte((uint8_t *)offsetof(struct ep_store_layout, unused_serial_no)+SERIAL_NO_MAX-1);
    /* the CRC needs to be in the printable char set */
    if (ui16_2 == ui16_1) {
      ui8_1 = eeprom_read_byte((uint8_t *)offsetof(struct ep_store_layout, unused_serial_no)+SERIAL_NO_MAX-3);
      devStatus |= ('1' == ui8_1) ? DS_DEV_1K : ('5' == ui8_1) ? DS_DEV_5K :
        ('2' == ui8_1) ? DS_DEV_20K : DS_DEV_INVALID;
    } else {
      devStatus |= DS_DEV_INVALID;
    }
    if ((devStatus & DS_DEV_INVALID) &&
        (0xFAC7051A == ui32_1)) { /* do factory setting */
      /* Serial # doesn't exist, load from addr 0 */
      for (ui8_1=SERIAL_NO_MAX; ui8_1>0;) {
        ui8_1--;
        ui8_2 = eeprom_read_byte((uint8_t *)(uint16_t)ui8_1+4);
        eeprom_update_byte((uint8_t *)offsetof(struct ep_store_layout, unused_serial_no)+ui8_1,
                           ui8_2);
      }
      continue;
    } else if (devStatus & DS_DEV_INVALID) {
#ifndef UNIT_TEST
      return; /* Invalid device */
#endif
    }
    break;
  }

  /* Now apply factory setting */
  if (0xFAC7051A == ui32_1) {
    MenuMode = MENU_MSUPER;
    menuFactorySettings(MenuMode | MENU_NOCONFIRM);
  }

  /* Report # of bills that could be made */
  struct sale *sl = (void *)(bufSS+LCD_MAX_COL+2+LCD_MAX_COL+2);
  menuMemset((void *)sl, 0, SALE_SIZEOF);
  ui16_2 = eeprom_read_word((uint16_t *)(offsetof(struct ep_store_layout, unused_next_billaddr)));
  for (ui16_1=0, ui16_3=0; ui16_1<EEPROM_SALE_MAX_BILLS; ui16_1++) {
    ee24xx_read_bytes(ui16_2, (void *)sl, SALE_DATA_EXP_ITEMS_SIZEOF);
    if (0xFFFF != (sl->crc ^ sl->crc_invert)) { /* not valid bill */
      ui16_3++;
    } else
      break;

    ui16_2 = EEPROM_NEXT_SALE_RECORD(ui16_2);
  }
  LCD_CLRLINE(0);
  lcdD(ui16_3);
  LCD_WR_NP((const char *)PSTR(" Bill Free"), 10);
  LCD_refresh();

  /* Re-scan and index all items */
  struct item *it = (void *)bufSS;
  numValidItems = 0;
  for ( ui16_1=0, ui16_2=0; ui16_1 < ITEM_MAX;
	ui16_1++, ui16_2 += (ITEM_SIZEOF>>2) ) {
    ee24xx_read_bytes(ui16_2, bufSS, ITEM_SIZEOF);
    for (ui8_1=0, ui8_2=0; ui8_1<(ITEM_SIZEOF-2); ui8_1++)
      ui8_2 = _crc_ibutton_update(ui8_2, bufSS[ui8_1]);
    if ((ui8_2 == bufSS[ITEM_SIZEOF-1]) &&
	((~ui8_2) == bufSS[ITEM_SIZEOF-2])) {
      /* valid item, update lookup */
      for (ui8_1=0, ui8_2=0; ui8_1<ITEM_NAME_BYTEL; ui8_1++) {
	ui8_2 = _crc_ibutton_update(ui8_2, it->name[ui8_1]);
	if (2 == ui8_1)
	  itIdxs[ui16_1].crc_name3 = ui8_2;
      }
      eeprom_update_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_itIdxName)), ui8_2);
      for (ui8_1=0, ui8_2=0; ui8_1<ITEM_PROD_CODE_BYTEL; ui8_1++)
	ui8_2 = _crc_ibutton_update(ui8_2, it->prod_code[ui8_1]);
      itIdxs[ui16_1].crc_prod_code = ui8_2;
      numValidItems++;
    } else {
      itIdxs[ui16_1].crc_name3 = 0xFF, itIdxs[ui16_1].crc_prod_code = 0xFF;
      if (0xFAC7051A == ui32_1) {
	eeprom_update_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_itIdxName))+ui16_1, 0xFF);
      }
    }
  }
  LCD_ALERT_N((const char *)PSTR("#Items:"), numValidItems);

  MenuMode = MENU_MSUPER;
}

void
menuScanF(char *str, uint16_t *ui16)
{
  if ((str[0] >= '0') && (str[0] <= '9')) {
    *ui16 *= 10;
    *ui16 += str[0]-'0';
    menuScanF(str+1, ui16);
  }
}

uint16_t
menuItemGetOptHelper(uint8_t *str, uint16_t strlen, uint16_t prev)
{
  struct item it;
  uint16_t ui16_1;

  if (0 == strlen) return 0;

  LCD_CLRLINE(0);

  it.unused_find_best_match = 1;
  ui16_1 = menuItemFind(str, NULL, &it, prev);
  if ((0 == ui16_1) || (ui16_1>ITEM_MAX)) { /* not found */
    ui16_1 = menuItemFind(NULL, str, &it, prev);
    if ((0 == ui16_1) || (ui16_1>ITEM_MAX)) { /* not found */
      SSCANF((char *)str, &ui16_1);
      //printf("id is %d", ui16_1);
      if ( (ui16_1 > 0) && (ui16_1 <= ITEM_MAX) ) {
	ee24xx_read_bytes(itemAddr(ui16_1), (void *)&it, ITEM_SIZEOF);
	if (0xFF == (it.unused_crc ^ it.unused_crc_invert))
	  goto menuItemGetOptHelperFound;
      }
      LCD_WR_P(PSTR("No match"));
      return 0;
    }
  }

 menuItemGetOptHelperFound:
  /* found item */
  LCD_PUT_UINT(it.id);
  LCD_PUTCH(':');
  LCD_WR_N(it.name, 8);
  LCD_PUTCH(',');
  LCD_PUT_UINT(it.cost);

  return ui16_1;
}

uint8_t
menuBilling(uint8_t mode)
{
  uint8_t ui8_1, ui8_2, ui8_3, ui8_4, ui8_5;
  uint16_t ui16_1, ui16_2, ui16_3;
  uint32_t ui32_1, ui32_2;
  uint8_t choice[EPS_MAX_VAT_CHOICE*MENU_PROMPT_LEN];

  struct sale *sl = (void *)(bufSS+LCD_MAX_COL+2+LCD_MAX_COL+2);
  ui16_3 = eeprom_read_word((uint16_t *)(offsetof(struct ep_store_layout, unused_next_billaddr)));
  if ((ui16_3 < EEPROM_SALE_START_ADDR) || (ui16_3 > EEPROM_SALE_END_ADDR)) {
    ui16_3 = EEPROM_SALE_START_ADDR;
  }

  /* memory requirements */
  //  printf("%d %d\n", (SALE_SIZEOF+LCD_MAX_COL+LCD_MAX_COL+4), BUFSS_SIZE);
  assert((SALE_SIZEOF+LCD_MAX_COL+LCD_MAX_COL+4) <= BUFSS_SIZE);

  /* load old bill */
  if (mode & MENU_MODITEM) {
    /* assume we have error */
    ui8_4 = 0;

    /* input validity */
    if (MENU_ITEM_DATE != arg1.valid) {
      timerDateGet(choice);
      arg1.value.date.year = choice[2];
      arg1.value.date.month = choice[1];
      arg1.value.date.day = choice[0];
      arg1.valid = MENU_ITEM_DATE;
    }

    /* iterate through all records */
    ui16_2 = ui16_3;
    for (ui8_1=0, ui16_1=0; ui16_1<EEPROM_SALE_MAX_BILLS; ui16_1++) {
      ui16_2 = EEPROM_PREV_SALE_RECORD(ui16_2);
      ee24xx_read_bytes(ui16_2, (void *)sl, SALE_DATA_EXP_ITEMS_SIZEOF);
      if (0xFFFF != (sl->crc ^ sl->crc_invert)) /* not valid bill */
	continue;
      if ( ((MENU_ITEM_DATE == arg1.valid) &&
	   (sl->info.date_yy == arg1.value.date.year) &&
	   (sl->info.date_mm == arg1.value.date.month) &&
	   (sl->info.date_dd == arg1.value.date.day)) ||
	   ((MENU_ITEM_ID == arg2.valid) &&
	    (sl->info.bill_id == arg2.value.integer.i16)) ) {
	/* User consent */
	LCD_ALERT(PSTR("Found Bill.."));
	LCD_CLRLINE(0);
	LCD_PUT_UINT(sl->info.bill_id);
	LCD_WR_NP(PSTR(": Amt:"), 6);
	LCD_PUT_UINT(sl->total);
	if (0 != menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_CONFI*MENU_PROMPT_LEN), MENU_PROMPT_LEN))
	  goto menuModBillReturn;
	else {
	  ui8_1 = 1;
	  break;
	}
      }
    }
    if (! ui8_1) {
      LCD_ALERT(PSTR("Bill Not Found"));
      goto menuModBillReturn;
    } else {
      //move(0, 0);
      //printw("info date:%d/%d mm:%d/%d day:%d/%d\n", sl->info.date_yy, arg1.value.date.year, sl->info.date_mm, arg1.value.date.month, sl->info.date_dd, arg1.value.date.day);
    }

    /* Now check all bill items, delete any non-existing item */
    for (ui8_5=MAX_ITEMS_IN_BILL, ui8_4=0, ui8_3=0; ui8_5; ) {
      ui8_5--;

      /* valid product sold? */
      if (0 == sl->items[ui8_5].quantity)
	continue;
      ui8_4++;

      /* */
      ee24xx_read_bytes(sl->items[ui8_5].ep_item_ptr, (uint8_t *)sl->it, ITEM_SIZEOF);
      if ((0 == sl->it[0].id) || (sl->it[0].is_disabled)) {
	LCD_ALERT_N(PSTR("No Item: "), itemId(sl->items[ui8_5].ep_item_ptr));
	ui8_3++;
	ui8_4--;
	/* Delete item */
	assert(sl->info.n_items);
	sl->info.n_items--;
	for (ui8_2=ui8_5; ui8_2<(MAX_ITEMS_IN_BILL-1); ui8_2++) {
	  memmove( ((char *)sl) + offsetof(struct sale, items) + (ui8_2*sizeof(struct sale_item)),
		   ((char *)sl) + offsetof(struct sale, items) + ((ui8_2+1)*sizeof(struct sale_item)),
		   sizeof(struct sale_item) );
	  sl->items[ui8_2+1].quantity = 0;
	}
      } else /* update item details */ {
	sl->items[ui8_5].cost = sl->it[0].cost;
	sl->items[ui8_5].discount = sl->it[0].discount;
	sl->items[ui8_5].has_serv_tax = sl->it[0].has_serv_tax;
	sl->items[ui8_5].has_common_discount = sl->it[0].has_common_discount;
	sl->items[ui8_5].vat_sel = sl->it[0].vat_sel;
	sl->items[ui8_5].has_vat = sl->it[0].has_vat;
      }
    }
    if (0 == ui8_4) {
      LCD_ALERT(PSTR("Empty bill"));
      if (0 != menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_DELETE*MENU_PROMPT_LEN), MENU_PROMPT_LEN)) {
	sl->crc_invert = (0 != sl->crc) ? 0xFFFF : 0;
	if (ui8_1) { /* delete bill */
	  ee24xx_write_bytes(ui16_2, (void *)sl, 2);
	}
      }
      goto menuModBillReturn;
    }
    if (0 != ui8_3) {
      LCD_ALERT_N(PSTR("# removed:"), ui8_3);
      if (0 != menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_CONFI*MENU_PROMPT_LEN), MENU_PROMPT_LEN))
	goto menuModBillReturn;
    }
    ui8_4 = 0xFF;

  menuModBillReturn:
    if (0xFF != ui8_4)
      return MENU_RET_NOTAGAIN;
  } else {
    /* Prepare location to store.... */
    ee24xx_read_bytes(ui16_3, (void *)sl, SALE_DATA_EXP_ITEMS_SIZEOF);
    if (0xFFFF == (sl->crc ^ sl->crc_invert)) { /* valid bill */
      LCD_CLRLINE(0);
      LCD_PUT_UINT(sl->info.bill_id);
      LCD_WR_NP(PSTR(": Amt:"), 6);
      LCD_PUT_UINT(sl->total);
      if (0 != menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_DELETE*MENU_PROMPT_LEN), MENU_PROMPT_LEN))
	return MENU_RET_NOTAGAIN;
    }
    menuMemset((void *)sl, 0, SALE_SIZEOF);
  }

  /* Wait for weighing machine */
  while (UART0_NONE != uart0_func) {
    /* put the device to sleep */
    sleep_enable();
    sleep_cpu();
    /* some event has to occur to come here */
    sleep_disable();
  }
  uart0_func = UART0_WEIGHMC;

  for (ui8_5=0; ;) {
    /* already added item, just confirm */
    if (0 != sl->items[ui8_5].quantity)
      goto menuBillingConfirm;
    /* can't take more items */
    if (ui8_5 >= MAX_ITEMS_IN_BILL) {
      goto menuBillingBill;
    }
    
    ui8_4 = 0;  /* item not found */
    ui16_2 = 0; /* int value */

    /* Get arg and find if valid, str or num */
    arg1.valid = MENU_ITEM_NONE;
    arg1.value.str.sptr = bufSS;
    arg1.value.str.len = LCD_MAX_COL;
    menuGetOpt(menu_str1+(MENU_STR1_IDX_ITEM*MENU_PROMPT_LEN), &arg1, MENU_ITEM_STR, menuItemGetOptHelper);
    if (MENU_ITEM_STR == arg1.valid) {
      ui8_2 = 1;  /* is a digit */
      for (ui8_1=0; ui8_1<LCD_MAX_COL; ui8_1++) {
	if (!isgraph(arg1.value.str.sptr[ui8_1]))
	  continue;
	ui8_2 &= isdigit(arg1.value.str.sptr[ui8_1]) ? 1 : 0;
	if (ui8_2) {
	  ui16_2 *= 10;
	  ui16_2 += arg1.value.str.sptr[ui8_1] - '0';
	}
      }
      if (ui8_2) { /* integer input */
	if ((ui16_2==0) || (ui16_2 > ITEM_MAX))
	  continue; /* get input again */
      } else { /* string input */
	sl->it[0].unused_find_best_match = 1;
	ui16_2 = menuItemFind(arg1.value.str.sptr, arg1.value.str.sptr, sl->it, 0);
	if ((ui16_2==0) || (ui16_2 > ITEM_MAX))
	  continue; /* get input again */
      }
    } else if (MENU_ITEM_ID == arg1.valid) {
      ui16_2 = arg1.value.integer.i16;
      if ((ui16_2==0) || (ui16_2 > ITEM_MAX))
	continue; /* get input again */
    }

    /* Just Enter was hit, may be the user wants to proceed to billing */
    if (0 == ui16_2) {
      goto menuBillingBill;
    }

    /* Find item */
    assert( (ui16_2 > 0) && (ui16_2 <= ITEM_MAX) );
    ee24xx_read_bytes(itemAddr(ui16_2), (uint8_t *)sl->it, ITEM_SIZEOF);
    if ( (0 == sl->it[0].id) || (sl->it[0].is_disabled) ) {
      LCD_CLRLINE(0);
      LCD_WR_NP((const char *)PSTR("Invalid Item"), 12);
      _delay_ms(1000);
      continue; /* match not found */
    }

    /* Display item to be billed */
    LCD_CLRLINE(0);
    lcdD(ui8_5+1);
    LCD_PUTCH(' ');
    LCD_PUT_UINT(sl->it[0].id);
    LCD_PUTCH(':');
    LCD_WR_N(sl->it[0].name, ITEM_NAME_BYTEL);
    //LCD_PUTCH(',');
    //LCD_PUT_UINT(sl->it[0].cost);

    /* common inputs */
    sl->items[ui8_5].ep_item_ptr = itemAddr(ui16_2);
    sl->items[ui8_5].cost = sl->it[0].cost;
    sl->items[ui8_5].discount = sl->it[0].discount;
    sl->items[ui8_5].has_serv_tax = sl->it[0].has_serv_tax;
    sl->items[ui8_5].has_common_discount = sl->it[0].has_common_discount;
    sl->items[ui8_5].vat_sel = sl->it[0].vat_sel;
    sl->items[ui8_5].has_vat = sl->it[0].has_vat;
    arg2.valid = MENU_ITEM_NONE;
    menuGetOpt(menu_str1+(MENU_STR1_IDX_SALEQTY*MENU_PROMPT_LEN), &arg2, MENU_ITEM_FLOAT, NULL);
    if (MENU_ITEM_NONE == arg2.valid) /* start afresh */
      continue;
    sl->items[ui8_5].quantity = arg2.value.integer.i16;

  menuBillingConfirm:
    /* Enable edit of earlier added item  */
    do {
      /* Display item to be billed */
      LCD_CLRLINE(0);
      LCD_PUT_UINT(sl->it[0].id);
      LCD_PUTCH(':');
      LCD_WR_N(sl->it[0].name, ITEM_NAME_BYTEL);
      //LCD_PUTCH(',');
      //LCD_PUT_UINT(sl->it[0].cost);

      /* Display item for confirmation */
      LCD_CLRLINE(LCD_MAX_ROW-1);
      LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_CONFI*MENU_PROMPT_LEN), MENU_PROMPT_LEN);
      //ee24xx_read_bytes((uint16_t)(((sl->items[ui8_5].ep_item_ptr)+(offsetof(struct item, name)>>EEPROM_MAX_DEVICES_LOGN2))), bufSS, 7);
      LCD_PUT_UINT(sl->items[ui8_5].quantity);
      LCD_PUTCH('=');
      ui16_1 = sl->it[0].cost;
      ui16_1 *= sl->items[ui8_5].quantity;
      LCD_PUT_UINT(ui16_1);

      KBD_RESET_KEY;
      KBD_GETCH;
      if (ASCII_ENTER == keyHitData.KbdData) { /* accept */
	ui8_5++;
	LCD_CLRLINE(0);
	LCD_WR_NP(PSTR("Added to bill!"), 14);
      } else if ( (ASCII_UP == keyHitData.KbdData) ||
		  (ASCII_RIGHT == keyHitData.KbdData) ) { /* override */
	/* add default values from item data */
	/* override cost */
	LCD_CLRLINE(0);
	LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 3);
	LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_PRICE*MENU_PROMPT_LEN), MENU_PROMPT_LEN);
	LCD_PUTCH(':');
	LCD_PUT_UINT(sl->items[ui8_5].cost);
	arg2.valid = MENU_ITEM_NONE;
	menuGetOpt(menu_str1+(MENU_STR1_IDX_PRICE*MENU_PROMPT_LEN), &arg2, MENU_ITEM_FLOAT, NULL);
	if (MENU_ITEM_FLOAT == arg2.valid)
	  sl->items[ui8_5].cost = arg2.value.integer.i16;

	/* override discount */
	LCD_CLRLINE(0);
	LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 3);
	LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_DISCO*MENU_PROMPT_LEN), MENU_PROMPT_LEN);
	LCD_PUTCH(':');
	LCD_PUT_UINT(sl->items[ui8_5].discount);
	arg2.valid = MENU_ITEM_NONE;
	menuGetOpt(menu_str1+(MENU_STR1_IDX_DISCO*MENU_PROMPT_LEN), &arg2, MENU_ITEM_FLOAT, NULL);
	if (MENU_ITEM_FLOAT == arg2.valid)
	  sl->items[ui8_5].discount = arg2.value.integer.i16;

	/* override has_serv_tax */
	LCD_CLRLINE(0);
	LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 3);
	LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_S_TAX*MENU_PROMPT_LEN), MENU_PROMPT_LEN);
	LCD_PUTCH(':');
	LCD_PUT_UINT(sl->items[ui8_5].has_serv_tax);
	sl->items[ui8_5].has_serv_tax = menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_S_TAX*MENU_PROMPT_LEN), MENU_PROMPT_LEN);

	/* override has_common_discount */
	LCD_CLRLINE(0);
	LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 3);
	LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_COMNDISC*MENU_PROMPT_LEN), MENU_PROMPT_LEN);
	LCD_PUTCH(':');
	LCD_PUT_UINT(sl->items[ui8_5].has_common_discount);
	sl->items[ui8_5].has_common_discount = menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_COMNDISC*MENU_PROMPT_LEN), MENU_PROMPT_LEN);

	/* override vat */
	LCD_CLRLINE(0);
	LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 3);
	LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_VAT*MENU_PROMPT_LEN), 4);
	LCD_PUTCH(':');
	for (ui8_4=0; ui8_4<EPS_MAX_VAT_CHOICE; ui8_4++) {
	  ui16_2 = eeprom_read_word((uint16_t *)(offsetof(struct ep_store_layout, Vat) + (sizeof(uint16_t)*ui8_4)));
	  for (ui8_2=1; ui8_2<=4; ui8_2++) {
	    (choice+(ui8_4*MENU_PROMPT_LEN)+MENU_PROMPT_LEN-ui8_2)[0] = '0' + ui16_2%10;
	    ui16_2 /= (uint16_t)10;
	  }
	  for (; ui8_2<=MENU_PROMPT_LEN; ui8_2++) {
	    (choice+(ui8_4*MENU_PROMPT_LEN)+MENU_PROMPT_LEN-ui8_2)[0] = ' ';
	  }
	  if (ui8_4 == sl->it[0].vat_sel) {
	    LCD_WR_N(choice+(ui8_4*MENU_PROMPT_LEN), MENU_PROMPT_LEN);
	  }
	}
	sl->items[ui8_5].vat_sel = menuGetChoice(menu_str1+(MENU_STR1_IDX_VAT*MENU_PROMPT_LEN), choice, MENU_PROMPT_LEN, EPS_MAX_VAT_CHOICE);

	/* override quantity */
	LCD_CLRLINE(0);
	LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 3);
	LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_SALEQTY*MENU_PROMPT_LEN), MENU_PROMPT_LEN);
	LCD_PUTCH(':');
	LCD_PUT_UINT(sl->items[ui8_5].quantity);
	arg2.valid = MENU_ITEM_NONE;
	menuGetOpt(menu_str1+(MENU_STR1_IDX_SALEQTY*MENU_PROMPT_LEN), &arg2, MENU_ITEM_FLOAT, NULL);
	if (MENU_ITEM_FLOAT == arg2.valid)
	  sl->items[ui8_5].quantity = arg2.value.integer.i16;

	LCD_CLRLINE(0);
	LCD_WR_P(PSTR("Override Done!"));
      } else if ( (ASCII_LEFT == keyHitData.KbdData) ||
		  (ASCII_DOWN == keyHitData.KbdData) ) {
	/* move to prev item to edit it */
	ui8_5 ? --ui8_5 : sl->info.n_items-1;
	ee24xx_read_bytes(sl->items[ui8_5].ep_item_ptr, (void *)sl->it, ITEM_SIZEOF);
      } else if (ASCII_DEL == keyHitData.KbdData) {
	/* delete the item */
	for (ui8_2=ui8_5; sl->items[ui8_2+1].quantity>0; ui8_2++) {
	  memcpy(&(sl->items[ui8_2]), &(sl->items[ui8_2+1]), sizeof(struct sale_item));
	}
	sl->items[ui8_2].quantity = 0;
	LCD_CLRLINE(0);
	LCD_WR_P(PSTR("Deleted!"));
      } else {
	LCD_CLRLINE(0);
	LCD_WR_P(PSTR("Skipped, Retry!"));
	continue;
      }
    } while (0);
  }

  /* Give away weighing machine */
  uart0_func = UART0_NONE;

 menuBillingBill:
  /* Why would somebody make a 0 item bill? */
  for (ui8_3=0, ui8_5=0; ui8_3<MAX_ITEMS_IN_BILL; ui8_3++) {
    if (sl->items[ui8_5].quantity > 0)
      ui8_5++;
  }
  if (0 == ui8_5) {
    LCD_CLRLINE(LCD_MAX_ROW-1);
    LCD_ALERT(PSTR("No bill items"));
    return MENU_RET_NOTAGAIN;
  }

  /* Final confirmation before billing */
  LCD_CLRLINE(0);
  LCD_WR_P(PSTR("Total Items: "));
  LCD_PUT_UINT(ui8_5);
  LCD_CLRLINE(1);
  if (0 != menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_FINALIZ*MENU_PROMPT_LEN), MENU_PROMPT_LEN))
    return 0;

  /* Calculate bill, confirm */
  ui32_1=0;
  sl->t_stax = 0, sl->t_vat = 0, sl->t_discount = 0, sl->total = 0;
  ui32_2 = 0;
  for (ui8_3=0; ui8_3<ui8_5; ui8_3++) {
    if ( (0 != sl->items[ui8_3].discount) && (sl->items[ui8_3].cost > sl->items[ui8_3].discount) ) {
      ui32_1 = sl->items[ui8_3].cost - sl->items[ui8_3].discount;
      sl->t_discount += sl->items[ui8_3].discount * sl->items[ui8_3].quantity;
    } else if (sl->items[ui8_3].has_common_discount) {
      ui32_1 = eeprom_read_byte((uint8_t *)offsetof(struct ep_store_layout, ComnDis));
      ui32_1 <<= 8;
      ui32_1 |= eeprom_read_byte((uint8_t *)(offsetof(struct ep_store_layout, ComnDis) + 1));
      if (ui32_1 <= 10000) {
	ui32_1 = (10000 - ui32_1) * sl->items[ui8_3].cost;
	ui32_1 /= 100;
	sl->t_discount += (sl->items[ui8_3].cost - ui32_1) * sl->items[ui8_3].quantity;
      } else {
	ui32_1 = sl->items[ui8_3].cost;
	LCD_ALERT(PSTR("Err: ComnDis > 100%"));
      }
    } else
      ui32_1 = sl->items[ui8_3].cost;

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
    }
    if (sl->items[ui8_3].vat_sel < EPS_MAX_VAT_CHOICE) {
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
  sl->info.date_dd = ((ui32_2>>FAT_DATE_OFFSET)&FAT_DATE_MASK);
  sl->info.date_mm = ((ui32_2>>FAT_MONTH_OFFSET)&FAT_MONTH_MASK);
  sl->info.date_yy = ((ui32_2>>FAT_YEAR_OFFSET)&FAT_YEAR_MASK);
  sl->info.time_hh = ((ui32_2>>FAT_HOUR_OFFSET)&FAT_HOUR_MASK);
  sl->info.time_mm = ((ui32_2>>FAT_MIN_OFFSET)&FAT_MIN_MASK);
  sl->info.time_ss = ((ui32_2>>FAT_SEC_OFFSET)&FAT_SEC_MASK);

  /* Restart numbering if another day!! */
  if (0 == (mode & MENU_MODITEM)) {
    sl->info.bill_id = eeprom_read_word((uint16_t *)(offsetof(struct ep_store_layout, unused_LastBillId))) + 1;
    if ( (sl->info.date_yy != eeprom_read_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_lastBillYear)))) ||
	 (sl->info.date_mm != eeprom_read_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_lastBillMonth)))) ||
	 (sl->info.date_dd != eeprom_read_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_lastBillDate)))) ) {
      sl->info.bill_id = 1;
      eeprom_update_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_lastBillYear)), sl->info.date_yy);
      eeprom_update_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_lastBillMonth)), sl->info.date_mm);
      eeprom_update_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_lastBillDate)), sl->info.date_dd);
    }
    eeprom_update_word((uint16_t *)(offsetof(struct ep_store_layout, unused_LastBillId)), sl->info.bill_id);
  }

  /* now save the data */
  for (ui16_1=0, ui16_2=0; ui16_1<SALE_DATA_EXP_ITEMS_SIZEOF; ui16_1++)
    ui16_2 = _crc16_update(ui16_2, ((uint8_t *)sl)[ui16_1]);
  sl->crc = ui16_2;
  sl->crc_invert = ~ui16_2;
  ee24xx_write_bytes(ui16_3, (uint8_t *)sl, SALE_DATA_EXP_ITEMS_SIZEOF);

  /* update next bill addr */
  ui16_3 = EEPROM_NEXT_SALE_RECORD(ui16_3);
  eeprom_update_word((uint16_t *)(offsetof(struct ep_store_layout, unused_next_billaddr)), ui16_3);

  /* */
  LCD_ALERT(PSTR("Bill Saved"));

  /* Now print the bill */
  menuPrnBill(sl, menuPrnBillEE24xxHelper);

  return 0;
}

uint8_t
menuAddItem(uint8_t mode)
{
#if MENU_ITEM_FUNC
  uint8_t ui8_1, ui8_2, ui8_3;
  uint16_t ui16_1, ui16_2, ui16_3;
  struct item *it = (void *)(bufSS+LCD_MAX_COL+2+LCD_MAX_COL+2);
  uint8_t *bufSS_ptr = (void *) it;
  uint8_t choice[EPS_MAX_VAT_CHOICE*MENU_PROMPT_LEN];

  /* init */
  for (ui8_1=0; ui8_1<ITEM_SIZEOF; ui8_1++) {
    bufSS_ptr[ui8_1] = 0;
  }
  assert(numValidItems <= ITEM_MAX);

  /* conditions required to modify */
  if (mode & MENU_MODITEM) {
    if ( (MENU_ITEM_ID != arg1.valid) ||
	 (0 == arg1.value.integer.i16) ||
	 (arg1.value.integer.i16 > ITEM_MAX) )
      goto menuItemInvalidArg;
    it->id = arg1.value.integer.i16;
  } else if (numValidItems >= ITEM_MAX) {
    LCD_ALERT(PSTR("Items Mem Full.."));
    return MENU_RET_NOTAGAIN;
  } else {
    it->id = (MENU_ITEM_ID == arg2.valid) ? arg2.value.integer.i16 : 0;
  }

  /* Find space to place item */
  if ((0 == it->id) || (it->id > ITEM_MAX)) {
    ui16_2 = eeprom_read_word((uint16_t *)offsetof(struct ep_store_layout, unused_ItemLastUsed));
    for (ui16_3=0; ui16_3<ITEM_MAX; ui16_3++) {
      ui16_2++; ui16_2 = (ui16_2<=ITEM_MAX) ? ui16_2 : 1; /* next id */
      if ( (0xFF == itIdxs[ui16_2-1].crc_name3) &&
	   (0xFF == itIdxs[ui16_2-1].crc_prod_code) ) {
	ui16_1 = itemAddr(ui16_2);
	ee24xx_read_bytes(ui16_1+(ITEM_SIZEOF>>2)-1, ((uint8_t *)it)+(ITEM_SIZEOF>>2)-1, 4);
	if (0xFF != (it->unused_crc ^ it->unused_crc_invert)) {
	  /* found space */
	  it->id = ui16_2;
	  break;
	}
      }
    }
    if ( ui16_3 >= ITEM_MAX ) {
      LCD_ALERT(PSTR("Items full"));
      return MENU_RET_NOTAGAIN;
    }
  }
  ui16_2 = it->id;
  ui16_1 = itemAddr(it->id);
  ee24xx_read_bytes(ui16_1, (void *)it, ITEM_SIZEOF);
  it->id = ui16_2;
  it->is_disabled = 1;
  /* Store last used id in eeprom & seed it next time */
  eeprom_update_word((uint16_t *)offsetof(struct ep_store_layout, unused_ItemLastUsed), ui16_2);
  goto menuItemSaveArg;

 menuItemInvalidArg:
  LCD_ALERT(PSTR("Invalid Argument"));
 menuItemReturn:
  return 0;

 menuItemSaveArg:
  assert (it->id <= ITEM_MAX);
  assert (0 != it->id);
  LCD_CLRLINE(0);
  LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 4);
  if (isgraph(it->name[0])) {
    LCD_WR_N(it->name, (ITEM_NAME_BYTEL+4)>LCD_MAX_COL ? (LCD_MAX_COL-4) : ITEM_NAME_BYTEL);
  } else if (isgraph(it->prod_code[0])) {
    LCD_WR_N(it->prod_code, (ITEM_PROD_CODE_BYTEL+4)>LCD_MAX_COL ? (LCD_MAX_COL-4) : ITEM_PROD_CODE_BYTEL);
  } else {
    LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_INVALID*MENU_PROMPT_LEN), MENU_PROMPT_LEN);
  }
  if (0 != menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_REPLA*MENU_PROMPT_LEN), MENU_PROMPT_LEN))
    goto menuItemReturn;
  for (ui8_1=0; ui8_1<ITEM_NAME_BYTEL; ui8_1++) {
    it->name[ui8_1] = (MENU_ITEM_STR == arg1.valid) ?
      ( isgraph(arg1.value.str.sptr[ui8_1]) ? toupper(arg1.value.str.sptr[ui8_1]) : ' ' ) :
      ( isgraph(it->name[ui8_1]) ? toupper(it->name[ui8_1]) : ' ' );
  }

  /* Product code */
  LCD_CLRLINE(0);
  LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 4);
  LCD_WR_N(it->prod_code, (ITEM_PROD_CODE_BYTEL+4)>LCD_MAX_COL ? (LCD_MAX_COL-4) : ITEM_PROD_CODE_BYTEL);
  assert(ITEM_PROD_CODE_BYTEL <= (LCD_MAX_COL*3));
  arg1.valid = MENU_ITEM_NONE;
  arg1.value.str.sptr = bufSS;
  arg1.value.str.len = ITEM_PROD_CODE_BYTEL;
  menuGetOpt(menu_str1+(MENU_STR1_IDX_PRODCODE*MENU_PROMPT_LEN), &arg1, MENU_ITEM_STR, NULL);
  if (MENU_ITEM_STR == arg1.valid) {
    for (ui8_1=0; ui8_1<ITEM_PROD_CODE_BYTEL; ui8_1++) {
      it->prod_code[ui8_1] = isgraph(arg1.value.str.sptr[ui8_1]) ?
	toupper(arg1.value.str.sptr[ui8_1]) : ' ';
    }
  }

  /* both shouldn't be empty */
  if ((0 == isgraph(it->name[0])) && (0 == isgraph(it->prod_code[0]))) {
    goto menuItemInvalidArg;
  }

  /* Cost */
  LCD_CLRLINE(0);
  LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 4);
  LCD_PUT_UINT(it->cost);
  arg1.valid = MENU_ITEM_NONE;
  menuGetOpt(menu_str1+(MENU_STR1_IDX_PRICE*MENU_PROMPT_LEN), &arg1, MENU_ITEM_FLOAT, NULL);
  if (MENU_ITEM_FLOAT == arg1.valid) {
    it->cost = arg1.value.integer.i32;
  } else if (0 == it->is_disabled) {
    goto menuItemInvalidArg;
  }

  /* Discount */
  LCD_CLRLINE(0);
  LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 4);
  LCD_PUT_UINT(it->discount);
  arg2.valid = MENU_ITEM_NONE;
  menuGetOpt(menu_str1+(MENU_STR1_IDX_DISCO*MENU_PROMPT_LEN), &arg2, MENU_ITEM_FLOAT, NULL);
  if (MENU_ITEM_FLOAT == arg2.valid) {
    it->discount = arg2.value.integer.i32;
  } else if (0 == it->is_disabled) {
    goto menuItemInvalidArg;
  }

  /* vat */
  LCD_CLRLINE(0);
  LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 4);
  for (ui8_1=0; ui8_1<EPS_MAX_VAT_CHOICE; ui8_1++) {
    ui16_2 = eeprom_read_word((uint16_t *)(offsetof(struct ep_store_layout, Vat) + (sizeof(uint16_t)*ui8_1)));
    for (ui8_2=1; ui8_2<=4; ui8_2++) {
      (choice+(ui8_1*MENU_PROMPT_LEN)+MENU_PROMPT_LEN-ui8_2)[0] = '0' + ui16_2%10;
      ui16_2 /= (uint16_t)10;
    }
    for (; ui8_2<=MENU_PROMPT_LEN; ui8_2++) {
      (choice+(ui8_1*MENU_PROMPT_LEN)+MENU_PROMPT_LEN-ui8_2)[0] = ' ';
    }
    if (ui8_1 == it->vat_sel) {
      LCD_WR_N(choice+(ui8_1*MENU_PROMPT_LEN), MENU_PROMPT_LEN);
    }
  }
  it->vat_sel = menuGetChoice(menu_str1+(MENU_STR1_IDX_VAT*MENU_PROMPT_LEN), choice, MENU_PROMPT_LEN, EPS_MAX_VAT_CHOICE);

  /* choices */
  LCD_CLRLINE(0);
  LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 4);
  LCD_WR_NP((const char *)menu_str2+((it->has_serv_tax)*4), 4);
  it->has_serv_tax = menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_S_TAX*MENU_PROMPT_LEN), MENU_PROMPT_LEN);

  /* Confirm */
  LCD_CLRLINE(0);
  LCD_WR_P(PSTR("Item: "));
  LCD_WR_N(it->name, (ITEM_NAME_BYTEL+4)>LCD_MAX_COL ? (LCD_MAX_COL-4) : ITEM_NAME_BYTEL);
  if (0 != menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_CONFI*MENU_PROMPT_LEN), MENU_PROMPT_LEN))
    return 0;

  /* From here on ... the item is complete */
  it->is_disabled = 0;

  /* Calculate CRC */
  for (ui8_1=0, ui8_2=0; ui8_1<(ITEM_SIZEOF-2); ui8_1++) {
    ui8_2 = _crc_ibutton_update(ui8_2, bufSS_ptr[ui8_1]);
  }
  it->unused_crc = ui8_2;
  it->unused_crc_invert = ~ui8_2;

  /* only valid items needs to be buffered */
  menuIndexItem(it);
  numValidItems = (0 == (mode & MENU_MODITEM)) ? numValidItems+1 : numValidItems;
  assert(numValidItems <= ITEM_MAX);

  /* Now save item */
  ui8_3 = ee24xx_write_bytes(ui16_1, bufSS_ptr, ITEM_SIZEOF);
  assert(ITEM_SIZEOF == ui8_3);

  /* index item */
  uint8_t cn, cpc, cn3;
  for (ui8_1=0, cn=cn3=0; ui8_1<ITEM_NAME_BYTEL; ui8_1++) {
    cn = _crc_ibutton_update(cn, it->name[ui8_1]);
    if (2 == ui8_1)
      cn3 = cn;
  }
  for (ui8_1=0, cpc=0; ui8_1<ITEM_PROD_CODE_BYTEL; ui8_1++) {
    cpc = _crc_ibutton_update(cpc, it->prod_code[ui8_1]);
  }
  itIdxs[it->id-1].crc_prod_code = cpc;
  itIdxs[it->id-1].crc_name3 = cn3;
  eeprom_update_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_itIdxName))+it->id-1, cn);

  /* Give +ve report */
  LCD_ALERT((const char *)menu_str1+(MENU_STR1_IDX_SUCCESS*MENU_PROMPT_LEN));
#endif

  return MENU_RET_NOTAGAIN;
}

/* Indexing at the following levels
RAM:
   1. Complete product code
   2. Complete name
Internal EEPROM:
   3. First 3 letters of name
 */
void
menuIndexItem(struct item *it)
{
  uint16_t ui16_1;
  uint8_t ui8_1, ui8_2, ui8_3;

  /* init */
  ui16_1 = itemAddr(it->id);

  /* Delete indexing */
  if ( (0 == it->id) || (it->id > ITEM_MAX) || (it->is_disabled) ) {
    itIdxs[it->id-1].crc_name3 = 0xFF,
      itIdxs[it->id-1].crc_prod_code = 0xFF;
    eeprom_update_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_itIdxName))+it->id-1, 0xFF);
    assert(0); /* I suppose we never reach here */
    return;
  }

  /* */
  for (ui8_1=0, ui8_2=0; ui8_1<ITEM_PROD_CODE_BYTEL; ui8_1++) {
    it->prod_code[ui8_1] = toupper(it->prod_code[ui8_1]);
    ui8_2 = _crc_ibutton_update(ui8_2, it->prod_code[ui8_1]);
  }
  itIdxs[it->id-1].crc_prod_code = ui8_2;
  //  printf("prod_code:0x%0x\n", ui8_2);

  /* */
  for (ui8_1=0, ui8_2; ui8_1<ITEM_NAME_BYTEL; ui8_1++) {
    it->name[ui8_1] = toupper(it->name[ui8_1]);
    ui8_2 = _crc_ibutton_update(ui8_2, it->name[ui8_1]);
    if (2 == ui8_1)
      ui8_3 = ui8_2;
  }
  //  printf("name:0x%0x name[3]:0x%x\n", ui8_2, ui8_3);

  eeprom_update_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_itIdxName))+it->id-1, ui8_2);
}

/* either name or prod_code be NULL
   idx: starting idx for recursive find's
 */
uint16_t
menuItemFind(uint8_t *name, uint8_t *prod_code, struct item *it, uint16_t idx)
{
  uint16_t ui16_1, ui16_2;
  uint8_t  ui8_1, cn=0, cpc=0, cn3=0;
  uint8_t  ufbm = it->unused_find_best_match;

  /* upcase inputs & calculate signature */
  if (NULL != name) {
    for (ui8_1=0, cn=0; ui8_1<ITEM_NAME_BYTEL; ui8_1++) {
      name[ui8_1] = toupper(name[ui8_1]);
      cn = _crc_ibutton_update(cn, name[ui8_1]);
      if (2 == ui8_1) cn3 = cn;
    }
  }
  if (NULL != prod_code) {
    for (ui8_1=0, cpc=0; ui8_1<ITEM_PROD_CODE_BYTEL; ui8_1++) {
      prod_code[ui8_1] = toupper(prod_code[ui8_1]);
      cpc = _crc_ibutton_update(cpc, prod_code[ui8_1]);
    }
  }

  /* indices */
  idx = (idx>0) && (idx<=ITEM_MAX) ? idx : 1;
  for (ui16_1=0; ui16_1<ITEM_MAX; ui16_1++,
	 idx = ((idx+1)>ITEM_MAX) ? 1 : idx+1) {
    if ( (0xFF == itIdxs[idx-1].crc_prod_code) &&
	 (0xFF == itIdxs[idx-1].crc_name3) )
      continue;
    if (NULL == name) {
      if (NULL == prod_code)
	return idx;
      else if (itIdxs[idx-1].crc_prod_code != cpc)
	continue;
    } else { /* NULL != name */
      if ( (itIdxs[idx-1].crc_name3 == cn3) && (1 == ufbm) )
	;
      else if ( (NULL != prod_code) &&
		(itIdxs[idx-1].crc_prod_code != cpc) &&
		(eeprom_read_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_itIdxName))+idx-1) != cn) )
	continue;
      else if ( (NULL == prod_code) &&
		(eeprom_read_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_itIdxName))+idx-1) != cn) )
	continue;
    }
    ui16_2 = itemAddr(idx);
    ee24xx_read_bytes(ui16_2, (void *)it, ITEM_SIZEOF);
    if (0xFF != (it->unused_crc ^ it->unused_crc_invert))
      continue;
    if ((NULL != name) && (NULL != prod_code)) {
      if ( (0 == strncmp((char *)name, (const char *)it->name, ITEM_NAME_BYTEL)) &&
	   (0 == strncmp((char *)prod_code, (const char *)(it->prod_code), ITEM_PROD_CODE_BYTEL)) )
	return idx;
    } else if (NULL != name) {
      if ( (0 == strncmp((char *)name, (const char *)it->name, 3)) )
	return idx;
    } else if (NULL != prod_code) {
      if (0 == strncmp((char *)prod_code, (const char *)(it->prod_code), ITEM_PROD_CODE_BYTEL))
	return idx;
    }
  }

  return -1;
}

uint8_t
menuDelItem(uint8_t mode)
{
#if MENU_ITEM_FUNC
  uint16_t ui16_1, ui16_2;
  struct item *it = (void *)(bufSS+LCD_MAX_COL+2+LCD_MAX_COL+2);

  /* check validity of argument */
  if ( (MENU_ITEM_ID != arg1.valid) || (0 == arg1.value.integer.i16) ||
       (arg1.value.integer.i16 > ITEM_MAX) ) {
    LCD_ALERT(PSTR("Invalid Option"));
    return 0;
  }

  /* Give +ve report */
  LCD_ALERT((const char *)menu_str1+(MENU_STR1_IDX_SUCCESS*MENU_PROMPT_LEN));
  _delay_ms(500);

  /* find address for the id */
  ui16_1 = arg1.value.integer.i16;
  ui16_2 = itemAddr(ui16_1);
  ee24xx_read_bytes(ui16_2+(ITEM_SIZEOF>>2)-1, ((uint8_t *)it)+ITEM_SIZEOF-4, 4);
  //  ee24xx_read_bytes(ui16_2, (void *)it, ITEM_SIZEOF);
  if (0xFF != (it->unused_crc ^ it->unused_crc_invert))
    return MENU_RET_NOTAGAIN;

  /* delete */
  assert(numValidItems > 0); 
  itIdxs[ui16_1-1].crc_name3 = 0xFF,
    itIdxs[ui16_1-1].crc_prod_code = 0xFF;
  eeprom_update_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_itIdxName))+ui16_1-1, 0xFF);
  ee24xx_write_bytes(ui16_2+(ITEM_SIZEOF>>2)-1, NULL, 4);
  numValidItems--;
#endif

  return MENU_RET_NOTAGAIN;
}

void
menuPrintTestPage(uint8_t mode)
{
#if MENU_DIAG_FUNC && MENU_PRINTER_ENABLE
  PRINTER_PRINT_TEST_PAGE;
#endif
}

void
menuPrnD(uint32_t var)
{
  if (var>9)
    menuPrnD(var/10);
  PRINTER_PRINT('0'+(var%10));
}

void
menuPrnF(uint32_t var)
{
  menuPrnD(var/100);
  PRINTER_PRINT('.');
  menuPrnD(var%100);
}

// Not unit tested
void
menuPrnBill(struct sale *sl, menuPrnBillItemHelper nitem)
{
#if MENU_PRINTER_ENABLE
  uint8_t ui8_1, ui8_2, ui8_3;

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

  /* Caption, Date */
  for (ui8_1=0; ui8_1<EPS_CAPTION_SZ_MAX; ui8_1++) {
    ui8_3 = eeprom_read_byte((uint8_t *)(offsetof(struct ep_store_layout, caption)+ui8_1));
    if ('\n' != ui8_3)
      PRINTER_PRINT(ui8_3);
  }
  PRINTER_PRINT(' '); PRINTER_PRINT(' ');
  PRINTER_PRINT_D(1+sl->info.date_dd); PRINTER_PRINT('/');
  PRINTER_PRINT_D(1+sl->info.date_mm); PRINTER_PRINT('/');
  PRINTER_PRINT_D(1980+sl->info.date_yy); PRINTER_PRINT(' ');
  PRINTER_PRINT_D(sl->info.time_hh); PRINTER_PRINT(':');
  PRINTER_PRINT_D(sl->info.time_mm); PRINTER_PRINT(':');
  PRINTER_PRINT_D(sl->info.time_ss); PRINTER_PRINT('\n');

  /* Items */
  for (ui8_1=0; ui8_1<sl->info.n_items; ui8_1++) {
    if (EEPROM_MAX_ADDRESS != sl->items[ui8_1].ep_item_ptr) {
      nitem(sl->items[ui8_1].ep_item_ptr, &(sl->it[0]), ui8_1);
      ui8_2 = 0;
    } else {
      ui8_2 = ui8_1;
    }
    PRINTER_PSTR(PSTR("Sl. Item  Nos  Price Disc Tax \n"));
    PRINTER_PSTR(PSTR("------------------------------\n"));
    PRINTER_PRINT_D((unsigned int)(ui8_1+1));
    PRINTER_PRINT('.'); PRINTER_PRINT(' ');
    for (ui8_3=0; ui8_3<ITEM_NAME_BYTEL; ui8_3++)
      PRINTER_PRINT(sl->it[0].name[ui8_3]);
    PRINTER_PRINT(' ');
    PRINTER_PRINT_D((unsigned int)sl->items[ui8_1].cost);
    PRINTER_PRINT('('); PRINTER_PRINT('-');
    PRINTER_PRINT_D((unsigned int)sl->items[ui8_1].discount);
    PRINTER_PRINT(')'); PRINTER_PRINT(' ');
    PRINTER_PRINT_D((unsigned int)sl->items[ui8_1].quantity);
    PRINTER_PRINT(' ');
    PRINTER_PRINT_D((unsigned int)sl->items[ui8_1].cost * sl->items[ui8_1].quantity);
    PRINTER_PRINT(' ');
  }

  /* Total */
  PRINTER_FONT_ENLARGE(2);
  PRINTER_PSTR(PSTR("Total Discount : "));
  PRINTER_PRINT_F(sl->t_discount);
  PRINTER_PRINT('\n');
  PRINTER_PSTR(PSTR("Total VAT      : "));
  PRINTER_PRINT_F(sl->t_vat);
  PRINTER_PRINT('\n');
  PRINTER_PSTR(PSTR("Total Serv Tax : "));
  PRINTER_PRINT_F(sl->t_stax);
  PRINTER_PRINT('\n');
  PRINTER_PSTR(PSTR("Bill Total (Rs): "));
  PRINTER_PRINT_F(sl->total);
  PRINTER_PRINT('\n');

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
#endif
}

// Not unit tested
uint8_t
menuBillReports(uint8_t mode)
{
#if FF_ENABLE
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
    return 0;
  }
  if ( !validDate(arg1.value.date.day, arg1.value.date.month, arg1.value.date.year) ||
       !validDate(arg2.value.date.day, arg2.value.date.month, arg2.value.date.year) ) {
    LCD_ALERT(PSTR("Invalid Date"));
    return 0;
  }

  /* */
  if (0 == ((devStatus & DS_DEV_INVALID) | (DS_NO_SD&devStatus))) {
    UINT  ret_val;
    menuMemset(&FS, 0, sizeof(FS));
    menuMemset(&Fil, 0, sizeof(Fil));
    f_mount(&FS, ".", 1);
    if (FR_OK != f_open(&Fil, SD_ITEM_FILE, FA_READ)) {
      LCD_ALERT(PSTR("File open error"));
      f_mount(NULL, "", 0);
      return 0;
    }

    /* If version doesn't match, escape... */
    f_read(&Fil, (bufSS+LCD_MAX_COL+2+LCD_MAX_COL+2), 2, &ret_val);
    assert(2 == ret_val);
    ui16_1 = bufSS[LCD_MAX_COL+2+LCD_MAX_COL+2];
    ui16_1 <<= 8; ui16_1 |= bufSS[LCD_MAX_COL+2+LCD_MAX_COL+2+1];
    if (GIT_HASH_CRC != ui16_1) {
      LCD_ALERT((const char *)menu_str1+(MENU_STR1_IDX_FILEERR*MENU_PROMPT_LEN));
      f_mount(NULL, "", 0);
      return 0;
    }

    /* Find # records */
    ui32_1 = f_size(&Fil);
    if (0 == ui32_1) return 0; /* don't expect to find anything */
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

  return MENU_RET_NOTAGAIN;
}

uint8_t
menuShowBill(uint8_t mode)
{
  uint8_t  ui8_1;
  uint16_t ui16_2, ui16_3;
  struct sale *sl = (void *)(bufSS+LCD_MAX_COL+2+LCD_MAX_COL+2);

  /* */
  ui16_2 = eeprom_read_word((uint16_t *)(offsetof(struct ep_store_layout, unused_next_billaddr)));
  ui8_1 = 0; /* go prev */
  while (ui16_2) {

    for (ui16_3=ui16_2; ui16_3; ) {
      /* take valid record */
      ui16_3 = (0 == ui8_1) ? EEPROM_PREV_SALE_RECORD(ui16_3) :
	EEPROM_NEXT_SALE_RECORD(ui16_3);
      if (ui16_3 == ui16_2) { /* came one big round */
	ui16_3 = 0;
	break;
      }
      ee24xx_read_bytes(ui16_3, (void *)sl, SALE_DATA_EXP_ITEMS_SIZEOF);
      if (0xFFFF != (sl->crc ^ sl->crc_invert)) { /* not valid bill */
	continue;
      }

      if ((MENU_ITEM_DATE != arg1.valid) && (MENU_ITEM_ID != arg2.valid)) {
	/* user wants any bill */
	break;
      } else if ((MENU_ITEM_DATE == arg1.valid) && (MENU_ITEM_ID == arg2.valid)) {
	if ( (sl->info.date_yy == arg1.value.date.year) &&
	     (sl->info.date_mm == arg1.value.date.month) &&
	     (sl->info.date_dd == arg1.value.date.day) &&
	     (sl->info.bill_id == arg2.value.integer.i32) ) {
	  /* user wants this date's/id bill */
	  break;
	}
      } else if (MENU_ITEM_ID == arg2.valid) {
	if (sl->info.bill_id == arg2.value.integer.i32) {
	  /* user provided bill id */
	  break;
	}
      } else { /* should be MENU_ITEM_DATE == arg1.valid */
	if ( (sl->info.date_yy == arg1.value.date.year) &&
	     (sl->info.date_mm == arg1.value.date.month) &&
	     (sl->info.date_dd == arg1.value.date.day) ) {
	  /* user provided date */
	  break;
	}
      }
    }

    /* */
    if (0 == ui16_3) {
      LCD_ALERT(PSTR("Bill Not Found"));
      return MENU_RET_NOTAGAIN;
    } else {
      assert (0xFFFF == (sl->crc ^ sl->crc_invert)); /* valid bill */
      ui16_2 = ui16_3; /* next base */
    }

    /* Display bill */
    LCD_CLRLINE(0);
    LCD_PUTCH('#');
    lcdD(sl->info.bill_id);
    LCD_PUTCH(' ');
    LCD_PUTCH('R'); LCD_PUTCH('s');
    lcdFd(sl->total);
    LCD_CLRLINE(LCD_MAX_ROW-1);
    lcdD(sl->info.date_dd);
    LCD_PUTCH('/');
    lcdD(sl->info.date_mm);
    LCD_WR_P(PSTR(" EscPrnDel"));

    /* according to user's wish */
    KBD_RESET_KEY;
    KBD_GETCH;
    if (ASCII_ENTER == keyHitData.KbdData) {
      /* keep direction */
    } else if (ASCII_PRNSCRN == keyHitData.KbdData) {
      menuPrnBill(sl, menuPrnBillEE24xxHelper);
    } else if ((ASCII_LEFT == keyHitData.KbdData) || (ASCII_UP == keyHitData.KbdData)) {
      ui8_1 = 0;
    } else if ((ASCII_RIGHT == keyHitData.KbdData) || (ASCII_DOWN == keyHitData.KbdData)) {
      ui8_1 = 1;
    } else if (ASCII_ESCAPE == keyHitData.KbdData) {
      goto menuShowBillReturn;
    } else if (ASCII_DEL == keyHitData.KbdData) {
      /* Delete bill here */
      if (0 != menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_DELETE*MENU_PROMPT_LEN), MENU_PROMPT_LEN))
	return MENU_RET_NOTAGAIN;
      sl->crc_invert = (0 != sl->crc) ? 0xFFFF : 0;
      if (ui8_1) { /* delete bill */
	ee24xx_write_bytes(ui16_2, (void *)sl, 2);
      }
      break;
    }
  }

  /* */
  menuShowBillReturn:
    return MENU_RET_NOTAGAIN;
}

void
menuPrnBillEE24xxHelper(uint16_t item_id, struct item *it, uint16_t it_index)
{
#if MENU_PRINTER_ENABLE
  ee24xx_read_bytes(item_id, (void *)it, SALE_DATA_EXP_ITEMS_SIZEOF);
#endif
}

#if MENU_SETTING_ENABLE
const struct setting_vars SettingVars[] PROGMEM = { SETTING_VAR_TABLE };

void
menuSettingString(uint16_t addr, const uint8_t *quest, uint16_t max_chars)
{
  uint16_t ui16_1;
  uint8_t  ui8_1;

  arg1.valid = MENU_ITEM_NONE;
  arg1.value.str.sptr = bufSS;
  arg1.value.str.len = max_chars;

  LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 3);
  LCD_PUTCH(':');
  for (ui16_1=0; ui16_1<max_chars; ui16_1++) {
    ui8_1 = eeprom_read_byte((uint8_t *)(addr+ui16_1));
    arg1.value.str.sptr[ui16_1] = isgraph(ui8_1) ? ui8_1 : ' ';
  }
  LCD_WR_N(arg1.value.str.sptr, LCD_MAX_COL-4);

  MENU_GET_OPT(quest, &arg1, MENU_ITEM_STR, NULL);
#ifndef UNIT_TEST
  if (0 != menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_CONFI*MENU_PROMPT_LEN), MENU_PROMPT_LEN))
    return;
#endif

  if (MENU_ITEM_STR == arg1.valid) {
    for (ui16_1=0; ui16_1<max_chars; ui16_1++) {
      if (!isgraph(arg1.value.str.sptr[ui16_1]))
	arg1.value.str.sptr[ui16_1] = ' ';
      eeprom_update_byte((uint8_t *)addr, arg1.value.str.sptr[ui16_1]);
      addr++;
    }
  }
}

void
menuSettingUint32(uint16_t addr, const uint8_t *quest)
{
  uint32_t val;

  LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 3);
  LCD_PUTCH(':');
  val = eeprom_read_dword((uint32_t *)addr);
  LCD_PUT_UINT(val);

  arg1.valid = MENU_ITEM_NONE;
  MENU_GET_OPT(quest, &arg1, MENU_ITEM_ID, NULL);
#ifndef UNIT_TEST
  if (0 != menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_CONFI*MENU_PROMPT_LEN), MENU_PROMPT_LEN))
    return;
#endif

  if (MENU_ITEM_ID == arg1.valid) {
    val = arg1.value.integer.i32;
    eeprom_update_dword((void *)addr, val);
  }
}

void
menuSettingUint16(uint16_t addr, const uint8_t *quest)
{
  uint16_t val;

  LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 3);
  LCD_PUTCH(':');
  val = eeprom_read_word((uint16_t *)addr);
  LCD_PUT_UINT(val);

  arg1.valid = MENU_ITEM_NONE;
  MENU_GET_OPT(quest, &arg1, MENU_ITEM_ID, NULL);
#ifndef UNIT_TEST
  if (0 != menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_CONFI*MENU_PROMPT_LEN), MENU_PROMPT_LEN))
    return;
#endif

  if (MENU_ITEM_ID == arg1.valid) {
    val = arg1.value.integer.i16;
    eeprom_update_word((uint16_t *)addr, val);
  }
}

void
menuSettingUint8(uint16_t addr, const uint8_t *quest)
{
  uint8_t val;

  LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 3);
  LCD_PUTCH(':');
  val = eeprom_read_byte((uint8_t *)addr);
  LCD_PUT_UINT(val);

  arg1.valid = MENU_ITEM_NONE;
  MENU_GET_OPT(quest, &arg1, MENU_ITEM_ID, NULL);
#ifndef UNIT_TEST
  if (0 != menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_CONFI*MENU_PROMPT_LEN), MENU_PROMPT_LEN))
    return;
#endif

  if (MENU_ITEM_ID == arg1.valid) {
    val = arg1.value.integer.i16;
    eeprom_update_byte((void *)addr, val);
  }
}

void
menuSettingBit(uint16_t addr, const uint8_t *quest, uint8_t size, uint8_t offset)
{
  uint8_t ui8_1, ui8_2;

  LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 3);
  LCD_PUTCH(':');
  ui8_1 = (eeprom_read_byte((uint8_t *)addr) >> offset);
  ui8_1 &= (1<<size)-1;
  LCD_PUT_UINT(ui8_1);

  arg1.valid = MENU_ITEM_NONE;
  MENU_GET_OPT(quest, &arg1, MENU_ITEM_ID, NULL);
#ifndef UNIT_TEST
  if (0 != menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_CONFI*MENU_PROMPT_LEN), MENU_PROMPT_LEN))
    return;
#endif

  if (MENU_ITEM_ID == arg1.valid) {
    ui8_1 = eeprom_read_byte((uint8_t *)(addr));
    ui8_2 = (1<<size)-1;
    ui8_1 &= ~(ui8_2<<offset);
    ui8_1 |= (arg1.value.integer.i16 & ui8_2) << offset;
    eeprom_update_byte((void *)(addr), ui8_1);
  }
}
#endif

uint8_t
menuSetDateTime(uint8_t mode)
{
  uint8_t ui8_a[3];

  timerDateGet(ui8_a);
  LCD_CLRLINE(0);
  LCD_WR_P((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN));
  LCD_PUT_UINT(ui8_a[0]);
  LCD_PUT_UINT(ui8_a[1]);
  LCD_PUT_UINT(ui8_a[2]+1980);
  MENU_GET_OPT(menu_prompt_str+(MENU_PR_DATE*MENU_PROMPT_LEN), &arg1, MENU_ITEM_DATE, NULL);
  if (MENU_ITEM_DATE == arg1.valid) {
    timerDateSet(arg1.value.date.year, arg1.value.date.month, arg1.value.date.day);
  }

  timerTimeGet(ui8_a);
  LCD_CLRLINE(0);
  LCD_WR_P(PSTR("Old:"));
  LCD_PUT_UINT(ui8_a[0]);
  LCD_PUT_UINT(ui8_a[1]);
  MENU_GET_OPT(menu_prompt_str+(MENU_PR_TIME*MENU_PROMPT_LEN), &arg2, MENU_PR_TIME, NULL);
  if (MENU_ITEM_TIME == arg2.valid) {
    timerTimeSet(arg2.value.time.hour, arg2.value.time.min);
  }

  return MENU_RET_NOTAGAIN;
}

uint8_t
menuSettingSet(uint8_t mode)
{
#if MENU_SETTING_ENABLE
  uint8_t ui8_1, ui8_2;
  uint16_t ui16_1;

  /* */
  ui8_1 = 1;

 menuSettingSetStart:
  for (; ;) {
    ui8_1 %= (SETTING_VARS_SIZE+1);
    LCD_CLRLINE(0);
    LCD_WR_NP((const char *)PSTR("Settings"), sizeof("Settings"));
    if (0 == ui8_1) {
      LCD_CLRLINE(LCD_MAX_ROW-1);
      LCD_WR_NP((const char *)PSTR("Quit ?"), sizeof("Quit ?"));
    } else {
      LCD_CLRLINE(LCD_MAX_ROW-1);
      LCD_WR_NP((const char *)PSTR("Change:"), sizeof("Change:"));
      LCD_WR_NP(((const char *)(SettingVars+(ui8_1-1)))+offsetof(struct setting_vars, name), SETTING_VAR_NAME_LEN);
    }
    LCD_refresh();
    KBD_RESET_KEY;
    KBD_GETCH;
    if (ASCII_ESCAPE == keyHitData.KbdData) {
      return MENU_RET_NOERROR;
    } else if (ASCII_ENTER == keyHitData.KbdData) {
      break;
    } else if ((ASCII_LEFT == keyHitData.KbdData) || (ASCII_UP == keyHitData.KbdData)) {
      ui8_1 = (ui8_1 > 0) ? ui8_1-1 : SETTING_VARS_SIZE;
    } else if ((ASCII_RIGHT == keyHitData.KbdData) || (ASCII_DOWN == keyHitData.KbdData)) {
      ui8_1 = (ui8_1 > SETTING_VARS_SIZE) ? 0 : ui8_1+1;
    }
  }
  if (0 == ui8_1) return 0; /* user choose to quit */

  LCD_CLRLINE(0);
  ui8_2 = pgm_read_byte(((const uint8_t *)(SettingVars+(ui8_1-1)))+offsetof(struct setting_vars, type));
  ui16_1 = pgm_read_byte(((const uint8_t *)(SettingVars+(ui8_1-1)))+offsetof(struct setting_vars, ep_ptr)+1);
  ui16_1 <<= 8;
  ui16_1 |= pgm_read_byte(((const uint8_t *)(SettingVars+(ui8_1-1)))+offsetof(struct setting_vars, ep_ptr));
  switch (ui8_2) {
  case TYPE_UINT8:
    menuSettingUint8(ui16_1, ((uint8_t *)(SettingVars+(ui8_1-1)))+offsetof(struct setting_vars, name));
    break;
  case TYPE_UINT16:
    menuSettingUint16(ui16_1, ((uint8_t *)(SettingVars+(ui8_1-1)))+offsetof(struct setting_vars, name));
    break;
  case TYPE_UINT32:
    menuSettingUint32(ui16_1, ((uint8_t *)(SettingVars+(ui8_1-1)))+offsetof(struct setting_vars, name));
    break;
  case TYPE_STRING:
    menuSettingString(ui16_1, ((uint8_t *)(SettingVars+(ui8_1-1)))+offsetof(struct setting_vars, name),
		      pgm_read_byte(((uint8_t *)(SettingVars+(ui8_1-1)))+offsetof(struct setting_vars, size)));
    break;
  case TYPE_BIT:
    menuSettingBit( ui16_1, ((uint8_t *)(SettingVars+(ui8_1-1)))+offsetof(struct setting_vars, name),
		    pgm_read_byte(((uint8_t *)(SettingVars+(ui8_1-1)))+offsetof(struct setting_vars, size)),
		    pgm_read_byte(((uint8_t *)(SettingVars+(ui8_1-1)))+offsetof(struct setting_vars, size2)) );
    break;
  default:
    assert(0);
    return 0;
  }

  goto menuSettingSetStart;
#endif
}

uint8_t
menuDelAllBill(uint8_t mode)
{
  uint8_t ui8_1;
  uint16_t ui16_1, ui16_2, ui16_3, ui16_4;
  struct sale *sl = (void *)(bufSS+LCD_MAX_COL+2+LCD_MAX_COL+2);

#if MENU_DELBILL
  ui16_1 = eeprom_read_word((uint16_t *)(offsetof(struct ep_store_layout, unused_next_billaddr)));

  /* memory requirements */
  //printf("%d %d\n", (SALE_SIZEOF+LCD_MAX_COL+LCD_MAX_COL+4), BUFSS_SIZE);
  assert((SALE_SIZEOF+LCD_MAX_COL+LCD_MAX_COL+4) <= BUFSS_SIZE);

  /* input validity */
  if ((MENU_ITEM_DATE != arg1.valid) && (MENU_ITEM_ID != arg2.valid)) {
    /* delete all bills */
    ui8_1 = 0x80;
  }

  /* iterate through all records */
  ui16_2 = ui16_1;
  for (ui16_4=0, ui16_3=0; ui16_3<EEPROM_SALE_MAX_BILLS; ui16_3++, ui8_1 &= ~0x3) {
    ui16_2 = EEPROM_PREV_SALE_RECORD(ui16_2);

    ee24xx_read_bytes(ui16_2, (void *)sl, SALE_DATA_EXP_ITEMS_SIZEOF);
    if (0xFFFF != (sl->crc ^ sl->crc_invert)) /* not valid bill */
      continue;
    if ( (MENU_ITEM_DATE == arg1.valid) &&
	 (sl->info.date_yy == arg1.value.date.year) &&
	 (sl->info.date_mm == arg1.value.date.month) &&
	 (sl->info.date_dd == arg1.value.date.day) ) {
      ui8_1 |= 1;
    } else if ( (MENU_ITEM_ID == arg2.valid) &&
		(sl->info.bill_id == arg2.value.integer.i16) ) {
      ui8_1 |= 2;
    }
    if (MENU_NOCONFIRM == (mode & ~MENU_MODEMASK)) {
      sl->crc_invert = (0 != sl->crc) ? 0xFFFF : 0;
      if (ui8_1) { /* delete bill */
	ee24xx_write_bytes(ui16_2, (void *)sl, 2);
      }
    } else if (ui8_1) { /* first pass */
      ui16_4++;
    }
  }

  if (MENU_NOCONFIRM != (mode & ~MENU_MODEMASK)) {
    LCD_CLRLINE(0);
    LCD_WR_NP((const char *)PSTR("# Bills : "), 10);
    LCD_PUT_UINT(ui16_4);
    if (0 == menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_DELETE*MENU_PROMPT_LEN), MENU_PROMPT_LEN))
      menuDelAllBill(mode|MENU_NOCONFIRM);
  }
#endif

  return MENU_RET_NOTAGAIN;
}

#if MENU_DIAG_FUNC
#define DIAG_FLASHMEM_SIZE   16
const uint8_t diagFlashMem[DIAG_FLASHMEM_SIZE] PROGMEM =
  { [ 0 ... (DIAG_FLASHMEM_SIZE - 1) ] = 0 };
#endif

// Not unit tested
uint8_t
menuRunDiag(uint8_t mode)
{
#if MENU_DIAG_FUNC
  uint16_t ui16_1, ui16_2, rand_seed;
  uint8_t  ui8_1, ui8_2;

  /* init */
  rand_seed = get_fattime();

  /* Verify LCD */
  LCD_CLRSCR;
  LCD_CLRLINE(0);
  LCD_WR_NP((const char *)PSTR("Diagnosis LCD"), 16);
  _delay_ms(1000);
  for (ui8_1=0; ui8_1<LCD_MAX_ROW; ui8_1++) {
    LCD_WR_NP((const char *)PSTR("\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF"), LCD_MAX_COL);
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

#if 0
  /* FIXME: Adjust LCD/TFT brightness */
  LCD_CLRSCR;
  LCD_WR_NP((const char *)PSTR("Diag Display Bri"), 16);
  _delay_ms(1000);
#endif

  /* Run Printer : Print test page */
  LCD_CLRSCR;
  LCD_WR_NP((const char *)PSTR("Diagnosis Prntr"), 15);
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
  PRINTER_PRINT_TEST_PAGE;

  /* Verify unused EEPROM spaces : Write/readback */
  LCD_CLRSCR;
  LCD_WR_NP((const char *)PSTR("Diagnosis Mem1"), 14);
  _delay_ms(1000);
  ui16_1 = offsetof(struct ep_store_layout, unused_scratch);
  srand(rand_seed);
  for (ui8_1=0; ui8_1<SCRATCH_MAX; ui8_1++, ui16_1++) {
    ui8_2 = rand();
    eeprom_update_byte((uint8_t *)ui16_1, ui8_2);
  }
  ui16_1 = offsetof(struct ep_store_layout, unused_scratch);
  srand(rand_seed); rand_seed++;
  for (ui8_1=0; ui8_1<SCRATCH_MAX; ui8_1++, ui16_1++) {
    ui8_2 = rand();
    if (ui8_2 != eeprom_read_byte((uint8_t *)ui16_1))
      break;
  }
  diagStatus |= (ui8_1==SCRATCH_MAX)? DIAG_MEM1 : 0;

  /* Verify 24c512 */
#define EEPROM_CHECK_MAX 10
  LCD_CLRSCR;
  LCD_WR_NP((const char *)PSTR("Diagnosis Mem3"), 14);
  _delay_ms(1000);
  struct sale *sl= (void *)bufSS;
  for (ui16_1=0, ui16_2=EEPROM_SALE_END_ADDR, ui8_1=0;
       (ui16_1<ITEM_MAX) && (ui8_1<EEPROM_CHECK_MAX); ui16_1++) {
    ui16_2 = EEPROM_PREV_SALE_RECORD(ui16_2);
    ee24xx_read_bytes(ui16_2, bufSS, 4);
    if ((rand() & 0x8) &&
	(0xFF != (sl->crc_invert ^ sl->crc))) {
      ui8_1++;
      /* Write read check */
      srand(rand_seed);
      for (ui8_2=0; ui8_2<SALE_DATA_EXP_ITEMS_SIZEOF; ui8_2++)
	bufSS[ui8_2] = rand();
      ee24xx_write_bytes(ui16_2, bufSS, SALE_DATA_EXP_ITEMS_SIZEOF);
      ee24xx_read_bytes(ui16_2, bufSS+SALE_DATA_EXP_ITEMS_SIZEOF, SALE_DATA_EXP_ITEMS_SIZEOF);
      for (ui8_2=0; ui8_2<SALE_DATA_EXP_ITEMS_SIZEOF; ui8_2++)
	if (bufSS[ui8_2] != bufSS[SALE_DATA_EXP_ITEMS_SIZEOF+ui8_2])
	  break;
      diagStatus |= (ui8_2 >= SALE_DATA_EXP_ITEMS_SIZEOF) ? DIAG_MEM3 : 0;
    }
  }
#undef EEPROM_CHECK_MAX

  /* Test timer */
  LCD_CLRSCR;
  LCD_WR_NP((const char *)PSTR("Diagnosis Timer"), 15);
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

  /* Verify Keypad : Ask user to press a key and display it */
  LCD_CLRSCR;
  LCD_WR_NP((const char *)PSTR("Diag Keypad/PS2 "), 16);
  _delay_ms(1000);
  LCD_CLRLINE(LCD_MAX_ROW-1);
  LCD_WR_NP((const char *)PSTR("Hit \xDB \xDB to exit"), 16);
  LCD_refresh();
  _delay_ms(2000);
  LCD_CLRSCR;
  LCD_WR_NP((const char *)PSTR("Key Entered..."), 14);
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
      LCD_WR_N(bufSS, LCD_MAX_COL);
    }
  }
  diagStatus |= (0 == menuGetYesNo((const uint8_t *)PSTR("Did Keypad work?"), 16)) ? DIAG_KEYPAD : 0;

#if 0
  /* FIXME: Check weighing machine connectivity */
  LCD_CLRSCR;
  LCD_WR_NP((const char *)PSTR("Diag Weighing Mc"), 16);
  _delay_ms(1000);
  LCD_CLRLINE(LCD_MAX_ROW-1);
  LCD_WR_NP((const char *)PSTR("Hit \xDB \xDB to exit"), 16);
  LCD_refresh();
  _delay_ms(2000);
  LCD_CLRSCR;
  LCD_WR_NP((const char *)PSTR("Weight in KGs..."), 16);
  LCD_refresh();
  while (UART0_NONE != uart0_func) {
    /* put the device to sleep */
    sleep_enable();
    sleep_cpu();
    /* some event has to occur to come here */
    sleep_disable();
  }
  uart0_func = UART0_WEIGHMC;
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
  uart0_func = UART0_NONE;
#endif

  /* Verify Buzzer */
  LCD_CLRSCR;
  LCD_WR_NP((const char *)PSTR("Diagnosis Buzzer"), 16);
  _delay_ms(1000);
  for (ui8_1=0; ui8_1<5; ui8_1++) {
    BUZZER_ON;
    _delay_ms(1000);
    BUZZER_OFF;
  }
  diagStatus |= (0 == menuGetYesNo((const uint8_t *)PSTR("Did Buzzer Buzz?"), 16)) ? DIAG_BUZZER : 0;

  /* save status*/
  eeprom_update_word((uint16_t *)(offsetof(struct ep_store_layout, unused_DiagStat)), diagStatus);

  /* We can't Diagonise Battery charging, so print sentence */
#endif

  return MENU_RET_NOERROR;
}

#define ROW_JOIN ,
#define COL_JOIN
#define MENU_HIER(A)
#define MENU_MODE(A)
#define MENU_NAME(A)
#define MENU_FUNC(A) A
#define ARG1(A, B)
#define ARG2(A, B)
const menu_func_t menu_handlers[] PROGMEM = {
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
  uint8_t menu_selected, menu_selhier, ui8_1, menuRet;

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
    LCD_WR_N(bufSS, LCD_MAX_COL);
    LCD_CLRLINE(LCD_MAX_ROW-1);
    LCD_WR_NP((const char *)(menu_hier_names+(menu_selected*MENU_HIER_NAME_SIZE)), MENU_HIER_NAME_SIZE);
  } else {
    /* Shop name (8 chars) */
    LCD_WR_N(bufSS, ((7<SHOP_NAME_SZ_MAX)?7:SHOP_NAME_SZ_MAX));
    LCD_PUTCH('>');
    LCD_WR_NP((const char *)(menu_hier_names+((menu_selhier-1)*MENU_HIER_NAME_SIZE)), ((LCD_MAX_COL-8)<MENU_HIER_NAME_SIZE)?(LCD_MAX_COL-8):MENU_HIER_NAME_SIZE);
    LCD_CLRLINE(LCD_MAX_ROW-1);
    LCD_WR_NP((const char *)(menu_names+(menu_selected*MENU_NAMES_LEN)), MENU_NAMES_LEN);
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
  menuMainRedoMenu:
    menuRet = 0;
    /* */
    LCD_CLRSCR;
    LCD_WR_NP((const char *)(menu_hier_names+((menu_selhier-1)*MENU_HIER_NAME_SIZE)), MENU_HIER_NAME_SIZE);
    LCD_PUTCH('>');
    LCD_WR_NP((const char *)(menu_names+(menu_selected*MENU_NAMES_LEN)), ((MENU_NAMES_LEN>(LCD_MAX_COL-MENU_HIER_NAME_SIZE-1))?(LCD_MAX_COL-MENU_HIER_NAME_SIZE-1):MENU_NAMES_LEN));

    /* Get choices before menu function is called */
    KBD_RESET_KEY;
    arg1.valid = MENU_ITEM_NONE;
    arg1.value.str.sptr = bufSS;
    arg1.value.str.len = LCD_MAX_COL;
    menuGetOpt(menu_prompt_str+(pgm_read_byte(menu_prompts+(menu_selected<<1))*MENU_PROMPT_LEN), &arg1, pgm_read_byte(menu_args+(menu_selected<<1)), NULL);
    KBD_RESET_KEY;
    arg2.valid = MENU_ITEM_NONE;
    arg2.value.str.sptr = bufSS+LCD_MAX_COL+2;
    arg2.value.str.len = LCD_MAX_COL;
    menuGetOpt(menu_prompt_str+(pgm_read_byte(menu_prompts+(menu_selected<<1)+1)*MENU_PROMPT_LEN), &arg2, pgm_read_byte(menu_args+(menu_selected<<1)+1), NULL);
    //move(0, 0); printw("arg1.valid:%x arg2.valid:%x", arg1.valid, arg2.valid);
    if ( (arg1.valid == (MENU_ITEM_TYPE_MASK&pgm_read_byte(menu_args+(menu_selected<<1)))) || (MENU_ITEM_OPTIONAL&pgm_read_byte(menu_args+(menu_selected<<1))) ) {
      if ( (arg2.valid == (MENU_ITEM_TYPE_MASK&pgm_read_byte(menu_args+(menu_selected<<1)+1))) || (MENU_ITEM_OPTIONAL&pgm_read_byte(menu_args+(menu_selected<<1)+1)) ) {
	menuRet |= MENU_RET_CALLED;
#ifdef UNIT_TEST_MENU_1
	UNIT_TEST_MENU_1(menu_selected);
#else
	if (0 == (devStatus & DS_DEV_INVALID)) {
	  //printf("call 0x%x\n", pgm_read_dword(menu_handlers+menu_selected));
#ifdef UNIT_TEST
	  menuRet |= (menu_handlers[menu_selected])(pgm_read_byte(menu_mode+menu_selected));
#else
	  menuRet |= ((menu_func_t)(uint16_t)pgm_read_dword((void *)(menu_handlers+menu_selected)))(pgm_read_byte(menu_mode+menu_selected));
#endif
	} else {
	  LCD_ALERT(PSTR("Invalid Device"));
	  return;
	}
#endif
      }
    }
    /* Choose the next best if mode had changed */
    if (0 == (MenuMode & (pgm_read_byte(menu_mode+menu_selected) & MENU_MODEMASK))) {
      for (ui8_1=0; (ui8_1<pgm_read_byte(&MENU_MAX)) && menu_selected; ui8_1++, menu_selected--) {
	if ( ((pgm_read_byte(menu_hier+menu_selected)) == menu_selhier) /* menu appropriate */ &&
	     (0 != (MenuMode & (pgm_read_byte(menu_mode+menu_selected) & MENU_MODEMASK))) /* mode appropriate */
	     ) {
	  break;
	}
      }
      /* No valid menu items, go back */
      if (pgm_read_byte(&MENU_MAX) <= ui8_1) {
	  menu_selhier = menu_selected = 0;
      }
    } else if ( (pgm_read_byte(menu_mode+menu_selected) & MENU_MREDOCALL) &&
		(menuRet & MENU_RET_CALLED) && (0 == (menuRet & MENU_RET_NOTAGAIN)) ) {
      goto menuMainRedoMenu; /* redo, as mode remained constant */
    }
  } else if ((ASCII_LEFT == keyHitData.KbdData) || (ASCII_UP == keyHitData.KbdData)) {
    if (0 == menu_selhier) {
      /* selection of menu */
      menu_selected = (0 == menu_selected) ? MENU_HIER_MAX-1 : menu_selected-1;
    } else { /* (0 != menu_selhier) && (0 != menu_selected) */
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
	menu_selhier = menu_selected = 0;
      }
    }
  } else if ((ASCII_RIGHT == keyHitData.KbdData) || (ASCII_DOWN == keyHitData.KbdData)) {
    if (0 == menu_selhier) {
      menu_selected = ((menu_selected+1) >= MENU_HIER_MAX) ? 0 : menu_selected+1;
    } else {
      for (ui8_1=0; ui8_1<pgm_read_byte(&MENU_MAX); ui8_1++) {
	menu_selected = ((1+menu_selected) < pgm_read_byte(&MENU_MAX)) ? menu_selected+1 : 0;
	if ( ((pgm_read_byte(menu_hier+menu_selected)) == menu_selhier) /* menu appropriate */ &&
	     (0 != (MenuMode & (pgm_read_byte(menu_mode+menu_selected) & MENU_MODEMASK))) /* mode appropriate */
	     ) {
	  break;
	}
      }
      /* No valid menu items, go back */
      if (ui8_1 >= pgm_read_byte(&MENU_MAX)) {
	menu_selhier = menu_selected = 0;
      }
    }
  }

  LCD_refresh();
#ifdef UNIT_TEST
  //move(0, 0);
  //printw("menu_selhier:%d  menu_selected:%d LoginUserId:%d MenuMode:0x%x", menu_selhier, menu_selected, LoginUserId, MenuMode);

  /* Provide means to excape the infinite hold */
  if (ASCII_F2 == keyHitData.KbdData) {
    KBD_RESET_KEY;
    return;
  }
#endif

  /* Forever stuck in this maze.. can't ever get out */
  goto menuMainStart;
}

/* PC Utility guide
  All commands Prefixed with: 0xA5.

  Function           CMD   Args
  read-all-item       B1   
  write-item          B2
  read-all-bill       B3
  reset-device        B4
  exit-utility        B5

  Success             AC
  Fail                BD
 */
void
menuPcUtil()
{
  /* when you return from this function, you either get a WDT reset
     or be in PC connect mode */
  uart0_func = UART0_PC;
  if ( (pcPassIdx < 2) || (0xA5 != pcPassword[0]) ||
       (0xB5 == pcPassword[1]) ) {
    return;
  }

  /* interrupts are cut-off */
  cli();

  /* */
  if (0xB1 == pcPassword[1]) {
    /* ack */
    uartTransmitByte(0xAC);
    uartTransmitByte((uint8_t)(ITEM_MAX>>8));
    uartTransmitByte((uint8_t)ITEM_MAX);
    uartTransmitByte('\r');
    /* send data */
    for (uint16_t ui16_1=0; ui16_1<ITEM_MAX_ADDR;) {
      ee24xx_read_bytes(ui16_1, pcPassword, PCPASS_SIZE);
      for (uint8_t ui8_1=0; (ui8_1<PCPASS_SIZE) &&
	     (ui16_1<ITEM_MAX_ADDR); ui8_1++, ui16_1++) {
	uartTransmitByte(pcPassword[ui8_1]);
      }
    }
    uartTransmitByte('\r');
    /* */
    sei(); return;
  } else if (0xB2 == pcPassword[1]) {
    uint16_t ui16_1;
    uint8_t ui8_1, ui8_2, ui8_3;
    while (1) {
      ui8_3 = uartReceiveByte();
      if ('\r' == ui8_3)
	break;
      if (0xB2 != ui8_3)
	continue;
      /* index to update, ack */
      ui16_1 = uartReceiveByte(); ui16_1 <<= 8;
      ui16_1 |= uartReceiveByte();
      ui8_1 = uartReceiveByte();
      if ((ui16_1 == 0) || (ui16_1 > ITEM_MAX) || ('\r' != ui8_1))
	continue;
      uartTransmitByte(0xAC);
      uartTransmitByte(ui16_1>>8);
      uartTransmitByte(ui16_1);
      uartTransmitByte('\r');
      if (0xB2 != uartReceiveByte())
	continue;
      /* store item */
      for (ui16_1=itemAddr(ui16_1), ui8_1=0; ui8_1<ITEM_SIZEOF; ui8_1 += ui8_2, ui16_1+=ui8_2) {
	ui8_2 = ((ui8_1+PCPASS_SIZE)<=ITEM_SIZEOF) ? PCPASS_SIZE :
	  ITEM_SIZEOF-ui8_1;
	for (ui8_3=0; ui8_3<ui8_2; ui8_3++)
	  pcPassword[ui8_3] = uartReceiveByte();
	ee24xx_write_bytes(ui16_1, pcPassword, ui8_2);
      }
    }
    /* */
    sei(); return;
  } else if (0xB3 == pcPassword[1]) {
    uartTransmitByte(0xAC);
    uartTransmitByte((uint8_t)(ITEM_MAX>>8));
    uartTransmitByte((uint8_t)ITEM_MAX);
    uartTransmitByte('\r');
    for (uint16_t ui16_1=EEPROM_SALE_START_ADDR; ui16_1<EEPROM_SALE_END_ADDR;) {
      ee24xx_read_bytes(ui16_1, pcPassword, PCPASS_SIZE);
      for (uint8_t ui8_1=0; (ui8_1<PCPASS_SIZE) &&
	     (ui16_1<EEPROM_SALE_END_ADDR); ui8_1++, ui16_1++) {
	uartTransmitByte(pcPassword[ui8_1]);
      }
    }
    uartTransmitByte('\r');
    /* */
    sei(); return;
  }

  /* if it reaches here, do WDT reset */
  wdt_enable(WDTO_15MS);
  for (;;) {}
}
