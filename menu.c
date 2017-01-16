#include <stdint.h>
#include <stdbool.h>
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
#include <avr/wdt.h>

#include "ep_ds.h"
#include "version.h"
#include "assert.h"
#include "lcd.h"
#include "kbd.h"
#include "ep_store.h"
#include "billing.h"
#if NVFLASH_EN
#include "flash.h"
#endif
#include "i2c.h"
#include "spi.h"
#include "uart.h"
#include "a1micro2mm.h"
#include "integer.h"
#if FF_ENABLE
#include "diskio.h"
#include "ff.h"
#endif
#include "main.h"
#include "menu.h"

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
#define MENU_STR1_IDX_TAX1  2
#define MENU_STR1_IDX_TAX2  3
#define MENU_STR1_IDX_TAX3  4
#define MENU_STR1_IDX_VAT   5
#define MENU_STR1_IDX_CONFI 6
#define MENU_STR1_IDX_ITEM  7
#define MENU_STR1_IDX_SALEQTY  8
#define MENU_STR1_IDX_FINALIZ  9
#define MENU_STR1_IDX_PRINT  10
#define MENU_STR1_IDX_SAVE   11
#define MENU_STR1_IDX_DELETE 12
#define MENU_STR1_IDX_NAME  13
#define MENU_STR1_IDX_REPLA 14
#define MENU_STR1_IDX_DAY   15
#define MENU_STR1_IDX_MONTH 16
#define MENU_STR1_IDX_YEAR  17
#define MENU_STR1_IDX_PRODCODE  18
#define MENU_STR1_IDX_UNICODE   19
#define MENU_STR1_IDX_ENTRYES   20
#define MENU_STR1_IDX_COMNDISC  21
#define MENU_STR1_IDX_OLD       23
#define MENU_STR1_IDX_FILEERR   24
#define MENU_STR1_IDX_CASHPAY   25
#define MENU_STR1_IDX_INVALID   26
#define MENU_STR1_IDX_HAS_WMC   27
#define MENU_STR1_IDX_IS_TAX_INCL 28
#define MENU_STR1_IDX_HAS_VAT    29
#define MENU_STR1_IDX_HAS_TAX1   30
#define MENU_STR1_IDX_HAS_TAX2   31
#define MENU_STR1_IDX_HAS_TAX3   32
#define MENU_STR1_IDX_HAS_COMDIS 33
#define MENU_STR1_IDX_SUCCESS   35 /* Keep this last, used by LCD_ALERT */
#define MENU_STR1_IDX_NUM_ITEMS 36
const uint8_t menu_str1[] PROGMEM =
  "Price   " /* 0 */
  "Discount" /* 1 */
  "Tax 1   " /* 2 */
  "Tax 2   " /* 3 */
  "Tax 3   " /* 4 */
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
  "Common D" /*21 */
  "iscount?" /*22 */
  "Old:    " /*23 */
  "FileErr " /*24 */
  "CashAmt?" /*25 */
  "Invalid!" /*26 */
  "HasWtMc?" /*27 */
  "TaxIncl?" /*28 */
  "Has Vat?" /*29 */
  "HasTax1?" /*30 */
  "HasTax2?" /*31 */
  "HasTax3?" /*32 */
  "CommonDi" /*33 */
  "scount? " /*34 */
  "Success!" /*35 */
  ;

/* All PSTR strings */
const int8_t PSTR_NO_SD[] PROGMEM = "No SD";
const int8_t PSTR_SKIPOP[] PROGMEM = "Skip Operation";

/* */
static uint8_t MenuMode = MENU_MRESET;
static uint8_t LoginUserId = 0; /* 0 is invalid */
uint8_t devStatus;   /* 0 is no err */

/* data struct for FF */
#if FF_ENABLE
FATFS FS;
FIL   Fil;
#endif

/* Pending actions */
volatile uint8_t menuPendActs;

/* Diagnosis */
uint16_t diagStatus;

/* indexing */
#define ITEM_SUBIDX_NAME 2
typedef struct  __attribute__((packed)) {
  uint8_t crc_name3;
} itemIdxs_t;
itemIdxs_t      itIdxs[ITEM_MAX];
uint16_t   numValidItems;

/* Found issue with memset */
#define menuMemset(S, c, N) do {			\
    for (uint16_t _ui16_1=0; _ui16_1<N; _ui16_1++) {	\
      ((uint8_t *)S)[_ui16_1] = c;			\
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
    prev_helper = helper ? (*helper)(buf, &buf_idx, prev_helper) : prev_helper;
    /* Ask a question */
    LCD_CLRLINE(LCD_MAX_ROW-1);
    LCD_WR_NP((const char *)prompt, MENU_PROMPT_LEN);
    LCD_PUTCH('?');
    if (prev_helper && (item_type == MENU_ITEM_ID)) {
	LCD_PUT_UINT(prev_helper);
    } else {
      for (ui16_1=MENU_PROMPT_LEN+1,
	     ui16_2=(buf_idx<(LCD_MAX_COL-MENU_PROMPT_LEN)) ? 0 : buf_idx-(LCD_MAX_COL-MENU_PROMPT_LEN-1);
	   (ui16_1<LCD_MAX_COL) && (ui16_2<buf_idx); ui16_1++, ui16_2++) {
	if (opt&MENU_ITEM_PASSWD) {
	  LCD_PUTCH('*');
	} else {
	  LCD_PUTCH(buf[ui16_2]);
	}
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
    menu_error = 0;
    item_type = MENU_ITEM_ID;
  } else if ( (MENU_ITEM_ID == item_type) || (MENU_ITEM_FLOAT == item_type) ) {
    uint8_t seenDecimal = 0;
    val = 0;
    for (ui16_1=0; ui16_1<buf_idx; ui16_1++) {
      if ((buf[ui16_1] >= '0') && (buf[ui16_1] <= '9') && (seenDecimal < 3)) {
	val *= 10;
	val += buf[ui16_1] - '0';
	menu_error = 0;
	seenDecimal += seenDecimal;
      } else if (('.' == buf[ui16_1]) && (MENU_ITEM_FLOAT == item_type)) {
	seenDecimal = 1;
	/* skip */
      } else break;
    }
    /* Float is always returned with two decimal digits */
    if (MENU_ITEM_FLOAT == item_type)
      for (seenDecimal=(0==seenDecimal)?1:seenDecimal; seenDecimal<3; seenDecimal+=seenDecimal)
	val *= 10;
    arg->value.integer.i8  = val&0xFF;
    arg->value.integer.i16 = val;
    arg->value.integer.i32 = val;
  } else if ((MENU_ITEM_DATE == item_type) || (MENU_ITEM_MONTH == item_type)) {
    /* format DDMMYYYY || format MMYYYY */
    menu_error = 0;
    for (uint8_t ui8_1=0; ui8_1<(item_type+2); ui8_1++) {
      if ((buf[ui8_1] < '0') || (buf[ui8_1] > '9'))
	menu_error++;
    }
    if (0 == menu_error) {
      /* Date */
      if (MENU_ITEM_DATE == item_type) {
	val = (buf[0] - '0'); /* max: 3 */
	val *= 10;
	val += (buf[1] - '0'); /* max: 9 */
	if ((0 == val) || (val > 31)) menu_error++;
	arg->value.date.day = val;
	buf += 2;
      }
      /* Month */
      val = (buf[0] - '0'); /* max: 1 */
      val *= 10;
      val += (buf[1] - '0'); /* max: 2 */
      if ((0 == val) || (val > 12)) menu_error++;
      arg->value.date.month = val;
      buf += 2;
      /* Year */
      val = (buf[0] - '0');
      val *= 10;
      val += (buf[1] - '0');
      val *= 10;
      val += (buf[2] - '0');
      val *= 10;
      val += (buf[3] - '0');
      buf += 2;
      if ((val < 2000) || (val > 2099)) menu_error++;
      arg->value.date.year = val;
    }
  } else if (MENU_ITEM_TIME == item_type) {
    /* format HHMMSS */
    menu_error = 0;
    for (ui16_1=0; ui16_1<6; ui16_1++) {
      if ((buf[ui16_1] < '0') || (buf[ui16_1] > '9'))
	menu_error++;
    }
    if (0 == menu_error) {
      /* Hour */
      val = (buf[0] - '0');
      val *= 10;
      val += (buf[1] - '0');
      buf += 2;
      if (val>23) menu_error++;
      arg->value.time.hour = val;
      /* Mins */
      val = (buf[0] - '0');
      val *= 10;
      val += (buf[1] - '0');
      buf += 2;
      if (val > 59) menu_error++;
      arg->value.time.min = val;
      /* Sec */
      val = (buf[0] - '0');
      val *= 10;
      val += (buf[1] - '0');
      buf += 2;
      if (val > 59) menu_error++;
      arg->value.time.sec = val;
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

const uint8_t menu_str2[] PROGMEM = "Yes\0No ";

uint8_t
menuGetYesNo(const uint8_t *quest, uint8_t size, uint8_t ret)
{
  assert(size < 12); /* 5 bytes for :Yes? */
  for (; ;) {
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


static uint32_t
menuGetOptFloatInitHelper(uint8_t *str, uint16_t *strlen, uint32_t prev)
{
  static uint32_t _prev;

  if ((NULL == str) || (NULL == strlen)) {
    _prev = prev;
  } else {
    *strlen = sprintf_P((char *)str, PSTR("%d"), prev);
  }

  return prev;
}

static uint32_t floatPercentHelperValue;
static uint32_t
menuFloatPercentHelper(uint8_t *str, uint16_t *strlen, uint32_t prev)
{
  /* Quick check to proceed further */
  if (((*strlen) < 2) || (str[(*strlen)-1] != '%')) return 0;

  /* */
  uint32_t ui32_1;
  SSCANF(str, ui32_1);
  ui32_1 *= floatPercentHelperValue; ui32_1 /= 100;

  return ui32_1;
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

static void
menuSdSaveBillDat(uint16_t ui16_2)
{
  uint16_t ui16_1, ui16_3, ui16_4;
  struct sale *sl = (void *)(bufSS+LCD_MAX_COL+LCD_MAX_COL);

  /* Check location, return if location is free */
  bill_read_bytes(ui16_2, (void *)sl, offsetof(struct sale, info));
  if (0xFFFF != (sl->crc ^ sl->crc_invert))
    return;

  ui16_2 = eeprom_read_word((uint16_t *)(offsetof(struct ep_store_layout, unused_todayStartAddr)));
  ui16_3 = eeprom_read_word((uint16_t *)(offsetof(struct ep_store_layout, unused_nextBillAddr)));
  assert ((ui16_2 >= NVF_SALE_START_ADDR) && (ui16_2 <= NVF_SALE_END_ADDR));

  /* check that first bill is valid anyway */
  bill_read_bytes(ui16_2, (void *)sl, offsetof(struct sale, items));
  assert (0xFFFF == (sl->crc ^ sl->crc_invert));

  /* create the file */
  menuMemset(&FS, 0, sizeof(FS));
  menuMemset(&Fil, 0, sizeof(Fil));
  f_mount(&FS, ".", 1);
  for (uint8_t ui8_2=2; ui8_2; ui8_2--) {
    if (FR_OK == f_chdir("billdat"))
      break;
    assert(ui8_2 >= 2);
    FRESULT fres = f_mkdir("billdat");
    assert(FR_OK == fres);
  }
  sprintf_P((char *)bufSS, PSTR("%02d-%02d-%04d.dat"), sl->info.date_dd, sl->info.date_mm, sl->info.date_yy);
  if (FR_OK != f_open(&Fil, (char *)bufSS, FA_WRITE|FA_OPEN_APPEND)) {
    LCD_ALERT(PSTR("File open error"));
    goto menuSkipFileSave;
  }

  /* write to file */
  UINT ret_size;
  for (ui16_1=0; ui16_1<NVF_SALE_MAX_BILLS;
       ui16_1++, ui16_2 = NVF_NEXT_SALE_RECORD(ui16_2)) {
    bill_read_bytes(ui16_2, (void *)sl, MAX_SIZEOF_1BILL);
    assert (0xFFFF == (sl->crc ^ sl->crc_invert)); /* valid bill */
    f_write(&Fil, (void *)sl, MAX_SIZEOF_1BILL, &ret_size);
    assert (MAX_SIZEOF_1BILL == ret_size);
    /* */
    if (NVF_NEXT_SALE_RECORD(ui16_2) == ui16_3) break;
  }

  /* close file */
  f_close(&Fil);
  if (FR_OK != f_chdir("..")) {
    LCD_ALERT(PSTR("UpDir Error"));
  }
  f_mount(NULL, "", 0);

 menuSkipFileSave:
  /* delete the sectors */
  ui16_2 = eeprom_read_word((uint16_t *)(offsetof(struct ep_store_layout, unused_todayStartAddr)));
  ui16_4 = NVF_4KBLOCK_ADDR(ui16_2) + 1; /* force delete block */
  for (ui16_1=0; ui16_1<NVF_SALE_MAX_BILLS;
       ui16_1++, ui16_2 = NVF_NEXT_SALE_RECORD(ui16_2)) {
    if (NVF_NEXT_SALE_RECORD(ui16_2) == ui16_3) break;
    if (ui16_4 == NVF_4KBLOCK_ADDR(ui16_2))
      continue;
    //printf("Deleting 4K : %x\n", (ui16_2>>3));
    ui16_4 = NVF_4KBLOCK_ADDR(ui16_2);
    nvfBlockErase4K(ui16_4);
  }

  /* now update the next addr to store */
  ui16_2 = eeprom_read_word((uint16_t *)(offsetof(struct ep_store_layout, unused_nextBillAddr)));
  eeprom_update_word((uint16_t *)(offsetof(struct ep_store_layout, unused_todayStartAddr)), ui16_2);
  eeprom_update_word((uint16_t *)(offsetof(struct ep_store_layout, unused_LastBillId)), 0);
}

/* Load in the factory settings */
uint8_t
menuFactorySettings(uint8_t mode)
{
  uint8_t ui8_1;
  uint16_t ui16_1;

  assert(MENU_MSUPER == MenuMode);

  /* confirm before proceeding */
  if (0 == (mode & MENU_NOCONFIRM)) {
    ui8_1 = menuGetYesNo((const uint8_t *)PSTR("Fact Reset"), 11, 0);
    if (0 != ui8_1) return 0;
  }

  /* date, time */
#if UNIT_TEST
  arg1.value.date.year = 2017;
  arg1.value.date.month = 1;
  arg1.value.date.day = 1;
  arg1.valid = MENU_ITEM_DATE;
  arg2.value.time.hour = 9;
  arg2.value.time.min = 0;
  arg2.value.time.sec = 0;
  arg2.valid = MENU_ITEM_TIME;
#else
  MENU_GET_OPT(menu_prompt_str+(MENU_PR_DATE*MENU_PROMPT_LEN), &arg1, MENU_ITEM_DATE, NULL);
  MENU_GET_OPT(menu_prompt_str+(MENU_PR_TIME*MENU_PROMPT_LEN), &arg2, MENU_ITEM_TIME, NULL);
  timerDateSet(arg1.value.date);
  timerTimeSet(arg1.value.time);
#endif

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
    eeprom_update_byte((uint8_t *)offsetof(struct ep_store_layout, unused_serial_no)+ui16_1, bufSS[ui16_1]);
  LCD_PUTCH('.'); LCD_refresh();

  /* index to start searching for empty item */
  eeprom_update_word((uint16_t *)offsetof(struct ep_store_layout, unused_ItemLastUsed), 0);

  /* */
  eeprom_update_byte_NP(offsetof(struct ep_store_layout, ShopName),
			PSTR("Sri Ganapathy Medicals"), SHOP_NAME_SZ_MAX);
  eeprom_update_byte_NP(offsetof(struct ep_store_layout, BillHeader),
			PSTR("12 Agraharam street, New Tippasandara,\n Bangalore - 52\n TIN:299007249"), HEADER_SZ_MAX);
  eeprom_update_byte_NP(offsetof(struct ep_store_layout, BillFooter),
			PSTR("Welcome & come back..."), FOOTER_SZ_MAX);
  eeprom_update_byte_NP(offsetof(struct ep_store_layout, Currency),
			PSTR("Rupees"), EPS_WORD_LEN);
  eeprom_update_byte_NP(offsetof(struct ep_store_layout, BillPrefix),
			PSTR("A000"), EPS_WORD_LEN);
  eeprom_update_byte_NP(offsetof(struct ep_store_layout, Caption),
			PSTR("Invoice"), EPS_CAPTION_SZ_MAX);
  LCD_PUTCH('.'); LCD_refresh();

  /* user names & passwd needs to be reset */
  eeprom_update_byte_NP(offsetof(struct ep_store_layout, unused_users),
			PSTR("admin   "), EPS_MAX_UNAME);
  for (ui8_1=1; ui8_1<=EPS_MAX_USERS; ui8_1++) {
    eeprom_update_byte_NP(offsetof(struct ep_store_layout, unused_users) + (EPS_MAX_UNAME*ui8_1),
			  PSTR("user    "), EPS_MAX_UNAME);
    eeprom_update_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_users) + (EPS_MAX_UNAME*ui8_1)+4), ((ui8_1>9)?'A'-10:'0')+ui8_1);
  }
  ui16_1 = 0;
  ui16_1 = _crc16_update(ui16_1, 'a');
  ui16_1 = _crc16_update(ui16_1, 'd');
  ui16_1 = _crc16_update(ui16_1, 'm');
  ui16_1 = _crc16_update(ui16_1, 'i');
  ui16_1 = _crc16_update(ui16_1, 'n');
  for (ui8_1=0; ui8_1<(EPS_MAX_UNAME-5); ui8_1++)
    ui16_1 = _crc16_update(ui16_1, ' ');
  eeprom_update_word((uint16_t *)offsetof(struct ep_store_layout, unused_passwds), ui16_1);
  LCD_PUTCH('.'); LCD_refresh();
  ui16_1 = 0;
  ui16_1 = _crc16_update(ui16_1, '1');
  ui16_1 = _crc16_update(ui16_1, '2');
  ui16_1 = _crc16_update(ui16_1, '3');
  for (ui8_1=0; ui8_1<(EPS_MAX_UNAME-3); ui8_1++)
    ui16_1 = _crc16_update(ui16_1, ' ');
  for (ui8_1=1; ui8_1<=EPS_MAX_USERS; ui8_1++) {
    eeprom_update_word((uint16_t *)(offsetof(struct ep_store_layout, unused_passwds) + (sizeof(uint16_t)*ui8_1)), ui16_1);
  }
  LCD_PUTCH('.'); LCD_refresh();

  /* All numerical data */
  eeprom_update_word((uint16_t *)offsetof(struct ep_store_layout, RoundOff), 50);

  /* */
  eeprom_update_word((uint16_t *)offsetof(struct ep_store_layout, unused_nextBillAddr), NVF_SALE_START_ADDR);
  LCD_PUTCH('.'); LCD_refresh();

#if NVFLASH_EN
  /* */
  for (ui8_1=0; ui8_1<NVF_NUM_DEVICES; ui8_1++) {
    nvfChipErase(ui8_1);
  }
  LCD_PUTCH('.'); LCD_refresh();
  eeprom_update_word((uint16_t *)(offsetof(struct ep_store_layout, unused_nextBillAddr)), NVF_SALE_START_ADDR);
  eeprom_update_word((uint16_t *)(offsetof(struct ep_store_layout, unused_todayStartAddr)), NVF_SALE_START_ADDR);
  eeprom_update_word((uint16_t *)(offsetof(struct ep_store_layout, unused_LastBillId)), 0);
#endif

  /* At the end, log out the user */
  menuUserLogout(mode|MENU_NOCONFIRM);
  LCD_CLRSCR; LCD_refresh();

  return 0;
}

void
menuInit()
{
  uint16_t ui16_1, ui16_2;
  uint8_t ui8_1, ui8_2;

  /* csv2dat depends on this number (ITEM_MAX/ITEM_MAX_ADDR) */
  assert ((ITEM_SIZEOF+LCD_MAX_COL+LCD_MAX_COL+4) < BUFSS_SIZE);
  assert ((SIZEOF_SALE_EXCEP_ITEMS+LCD_MAX_COL+LCD_MAX_COL) <= BUFSS_SIZE);
  assert(1 == sizeof(uint8_t));
  //  assert(sizeof(void *) == sizeof(uint16_t));
  assert(((offsetof(struct item, name)&(0xFFFF<<EEPROM_ADDR_SHIFT))>>EEPROM_ADDR_SHIFT) == (offsetof(struct item, name)>>EEPROM_ADDR_SHIFT));
  assert(0 == (ITEM_SIZEOF % (1<<EEPROM_ADDR_SHIFT)));

  /* init global vars */
  devStatus = 0;
  diagStatus = 0;
  menuPendActs = 0;

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
      devStatus = ('1' == ui8_1) ? DS_DEV_1K : ('5' == ui8_1) ? DS_DEV_5K :
        ('2' == ui8_1) ? DS_DEV_20K : DS_DEV_INVALID;
    } else {
      devStatus = DS_DEV_INVALID;
    }
    if ((0 == loopCnt) && (devStatus & DS_DEV_INVALID) &&
        (0xFAC7051A == ui32_1)) { /* do factory setting */
      /* Serial # doesn't exist, load from addr 0 */
      for (ui8_1=SERIAL_NO_MAX; ui8_1;) {
	ui8_1--;
        ui8_2 = eeprom_read_byte((uint8_t *)(uint16_t)ui8_1+4);
        eeprom_update_byte((uint8_t *)offsetof(struct ep_store_layout, unused_serial_no)+ui8_1, ui8_2);
      }
      continue;
    } else if (devStatus & DS_DEV_INVALID) {
      return; /* Invalid device */
    }
    break;
  }

  /* Now apply factory setting */
  if (0xFAC7051A == ui32_1) {
    assert(0 == WDT_RESET_WAKEUP);
    MenuMode = MENU_MSUPER;
    menuFactorySettings(MenuMode | MENU_NOCONFIRM);
  }

  /* Store away bills */
#if NVFLASH_EN
  if (WDT_RESET_WAKEUP)
    menuSdSaveBillDat(NVF_SALE_START_ADDR);
#endif

  /* Re-scan and index all items */
  if (WDT_RESET_WAKEUP) goto menuInitSkipItemIdxing;
  struct item *it = (void *)bufSS;
  numValidItems = 0;
  for ( ui16_1=0, ui16_2=0; ui16_1 < ITEM_MAX;
	ui16_1++, ui16_2 += (ITEM_SIZEOF>>2) ) {
    item_read_bytes(ui16_2, bufSS, ITEM_SIZEOF);
    for (ui8_1=0, ui8_2=0; ui8_1<(ITEM_SIZEOF-2); ui8_1++)
      ui8_2 = _crc_ibutton_update(ui8_2, bufSS[ui8_1]);
    if ((ui8_2 == bufSS[ITEM_SIZEOF-1]) &&
	(((uint8_t)(~ui8_2)) == bufSS[ITEM_SIZEOF-2])) {
      /* valid item, update lookup */
      for (ui8_1=0, ui8_2=0; ui8_1<ITEM_NAME_BYTEL; ui8_1++) {
	ui8_2 = _crc_ibutton_update(ui8_2, it->name[ui8_1]);
	if (2 == ui8_1)
	  itIdxs[ui16_1].crc_name3 = ui8_2;
      }
      eeprom_update_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_itIdxName)), ui8_2);
      for (ui8_1=0, ui8_2=0; ui8_1<ITEM_PROD_CODE_BYTEL; ui8_1++)
	ui8_2 = _crc_ibutton_update(ui8_2, it->prod_code[ui8_1]);
      eeprom_update_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_crc_prod_code)), ui8_2);
      numValidItems++;
    } else {
	itIdxs[ui16_1].crc_name3 = 0xFF;
	eeprom_update_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_itIdxName))+ui16_1, 0xFF);
	eeprom_update_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_crc_prod_code))+ui16_1, 0xFF);
    }
  }
  LCD_ALERT_N((const char *)PSTR("#Items:"), numValidItems);
 menuInitSkipItemIdxing:

  MenuMode = MENU_MRESET;
}

uint8_t menuScanFDotSeen;
void
menuScanF(uint8_t *str, uint32_t *ui32)
{
  if (3 == menuScanFDotSeen) return;

  if ((str[0] >= '0') && (str[0] <= '9')) {
    *ui32 *= 10;
    *ui32 += str[0]-'0';
    if (menuScanFDotSeen) menuScanFDotSeen++;
    menuScanF(str+1, ui32);
  } else if (str[0] == '.') {
    menuScanFDotSeen = 1;
    menuScanF(str+1, ui32);
  }
}

static uint32_t
menuItemGetOptHelper(uint8_t *str, uint16_t *strlen, uint32_t prev)
{
  struct item it;
  uint16_t ui16_1;
  uint32_t ui32_1;

  if (0 == (*strlen)) return 0;

  LCD_CLRLINE(0);

  ui16_1 = menuItemFind(str, NULL, &it, prev);
  if ((0 == ui16_1) || (ui16_1>ITEM_MAX)) { /* not found */
    ui16_1 = menuItemFind(NULL, str, &it, prev);
    if ((0 == ui16_1) || (ui16_1>ITEM_MAX)) { /* not found */
      SSCAND(str, ui32_1);
      ui16_1 = ui32_1;
      if ( (ui16_1 > 0) && (ui16_1 <= ITEM_MAX) ) {
	item_read_bytes(itemAddr(ui16_1), (void *)&it, ITEM_SIZEOF);
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
  LCD_WR_N(it.name, 6);
  LCD_PUTCH(',');
  LCD_PUT_FLOAT(it.cost);

  return ui16_1;
}

/* Does 3 menu work
   1. New KOT Bill
     opt1 : Table
     Create new KOT Bill, add to existing KOT Bill for same table.
   2. New Bill (including old KOT)
     opt1 : Table/Optional
     Start new bill from scratch.
   3. Modify Bill (only current date)
     opt1 : Id/Optional
     Re-edit a earlier bill and change its contents.
   4. Void Bill
     opt1 : Id/Optional
     Re-open a void bill.
 */
uint8_t
menuBilling(uint8_t mode)
{
  uint8_t ui8_1, ui8_2, ui8_3, ui8_4=0, ui8_5;
  uint16_t ui16_1, ui16_2, ui16_3, ui16_4;
  uint32_t ui32_1, ui32_2;

  /* */
  struct sale *sl = (void *)(bufSS+LCD_MAX_COL+LCD_MAX_COL);
  ui16_2 = eeprom_read_word((uint16_t *)(offsetof(struct ep_store_layout, unused_nextBillAddr)));
  assert ((ui16_2 >= NVF_SALE_START_ADDR) && (ui16_2 <= NVF_SALE_END_ADDR));

  /* check sufficient args */
  ui8_1 = ui16_4 = 0;
  if (MENU_ITEM_ID == arg1.valid) {
    ui8_1 = arg1.value.integer.i16;
  } else if (MENU_ITEM_ID == arg2.valid) {
    ui16_4 = arg2.value.integer.i16;
  }
  if (MENU_KOTBILL == (mode & ~(MENU_MODEMASK|MENU_MREDOCALL))) {
    assert(ui8_1);
  }

  /* Either of KOT, Modify or Void Bill */
  if (0 != (mode & ~(MENU_MODEMASK|MENU_MREDOCALL))) {
    ui16_2 = eeprom_read_word((uint16_t *)(offsetof(struct ep_store_layout, unused_todayStartAddr)));
   assert ((ui16_2 >= NVF_SALE_START_ADDR) && (ui16_2 <= NVF_SALE_END_ADDR));

    /* iterate through all records */
    for (ui16_1=0; ui16_1<NVF_SALE_MAX_BILLS; ui16_1++) {
      ui16_2 = NVF_NEXT_SALE_RECORD(ui16_2);
      bill_read_bytes(ui16_2, (void *)sl, offsetof(struct sale, items));
      if (0xFFFF != (sl->crc ^ sl->crc_invert)) /* not valid bill */
	break;

      /* User consent */
      LCD_CLRLINE(0);
      LCD_PUT_UINT(sl->info.id);
      LCD_PUTCH(' '); LCD_PUTCH('#');
      LCD_PUT_UINT(sl->info.n_items);
      LCD_PUTCH(' ');
      LCD_PUT_UINT(sl->info.time_hh);
      LCD_PUTCH(':');
      LCD_PUT_UINT(sl->info.time_mm);
      LCD_PUTCH(':');
      LCD_PUT_UINT(sl->info.time_ss);
      LCD_CLRLINE(LCD_MAX_ROW-1);
      if (MENU_KOTBILL == (mode & ~(MENU_MODEMASK|MENU_MREDOCALL))) {
	if (ui8_1 == sl->tableNo)
	  break;
      } else if ( ui16_4 ) {
	if (ui16_4 == sl->info.id)
	  break;
      } else if (0 == menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_CONFI*MENU_PROMPT_LEN), MENU_PROMPT_LEN, 0))
	break;
    }
    if (ui16_1 >= NVF_SALE_MAX_BILLS) {
      LCD_ALERT(PSTR("Bill Memory Full"));
      goto menuModBillReturn;
    } else if (0xFFFF == (sl->crc ^ sl->crc_invert)) { /* got valid bill */
      //move(0, 0);
      //printw("info date:%d/%d mm:%d/%d day:%d/%d\n", sl->info.date_yy, arg1.value.date.year, sl->info.date_mm, arg1.value.date.month, sl->info.date_dd, arg1.value.date.day);
    } else {
      LCD_ALERT(PSTR("Bill Not Found"));
      goto menuModBillReturn;
    }

    /* Now check all bill items, delete any non-existing item */
    for (ui8_5=MAX_ITEMS_IN_BILL, ui8_4=0, ui8_3=0; ui8_5; ) {
      ui8_5--;

      /* valid product sold? */
      if (0 == sl->items[ui8_5].quantity)
	continue;
      ui8_4++;

      /* */
      item_read_bytes(sl->items[ui8_5].ep_item_ptr, (uint8_t *)sl->it, ITEM_SIZEOF);
      if ((0 == sl->it[0].id) || (sl->it[0].is_disabled)) {
	LCD_ALERT_N(PSTR("Item Deleted: "), itemId(sl->items[ui8_5].ep_item_ptr));
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
	sl->items[ui8_5].has_vat = sl->it[0].has_vat;
	sl->items[ui8_5].has_tax1 = sl->it[0].has_tax1;
	sl->items[ui8_5].has_tax2 = sl->it[0].has_tax2;
	sl->items[ui8_5].has_tax3 = sl->it[0].has_tax3;
	sl->items[ui8_5].has_common_discount = sl->it[0].has_common_discount;
	sl->items[ui8_5].is_reverse_tax = sl->it[0].is_reverse_tax;
	sl->items[ui8_5].has_weighing_mc = sl->it[0].has_weighing_mc;
      }
    }
    if (0 == ui8_4) {
      LCD_ALERT(PSTR("Empty bill"));
      if (0 != menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_DELETE*MENU_PROMPT_LEN), MENU_PROMPT_LEN, 0)) {
	sl->crc_invert = (0 != sl->crc) ? 0xFFFF : 0;
	if (ui8_1) { /* delete bill */
	  bill_write_bytes(ui16_2, (void *)sl, 2);
	}
      }
      goto menuModBillReturn;
    }
    if (0 != ui8_3) {
      LCD_ALERT_N(PSTR("# removed:"), ui8_3);
      if (0 != menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_CONFI*MENU_PROMPT_LEN), MENU_PROMPT_LEN, 0))
	goto menuModBillReturn;
    }
    ui8_4 = 0xFF;

  menuModBillReturn:
    if ((0xFF != ui8_4) || (MENU_SHOWBILL == (mode & ~(MENU_MODEMASK|MENU_MREDOCALL))))
      return MENU_RET_NOTAGAIN;
  }
  menuSdSaveBillDat(ui16_2);
  menuMemset((void *)sl, 0, SIZEOF_SALE);

  /* Billing loop */
  for (ui8_5=0; ;) {
    /* already added item, just confirm */
    if (0 != sl->items[ui8_5].quantity)
      goto menuBillingConfirm;
    /* can't take more items */
    if (ui8_5 >= MAX_ITEMS_IN_BILL) {
      goto menuBillingBill;
    }
    
    ui8_4 = 0;  /* item not found */
    ui16_3 = 0; /* int value */

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
	  ui16_3 *= 10;
	  ui16_3 += arg1.value.str.sptr[ui8_1] - '0';
	}
      }
      if (ui8_2) { /* integer input */
	if ((ui16_3==0) || (ui16_3 > ITEM_MAX))
	  continue; /* get input again */
      } else { /* string input */
	ui16_3 = menuItemFind(arg1.value.str.sptr, arg1.value.str.sptr, sl->it, 0);
	if ((ui16_3==0) || (ui16_3 > ITEM_MAX))
	  continue; /* get input again */
      }
    } else if (MENU_ITEM_ID == arg1.valid) {
      ui16_3 = arg1.value.integer.i16;
      if ((ui16_3==0) || (ui16_3 > ITEM_MAX))
	continue; /* get input again */
    }

    /* Just Enter was hit, may be the user wants to proceed to billing */
    if (0 == ui16_3) {
      goto menuBillingBill;
    }

    /* Find item */
    assert( (ui16_3 > 0) && (ui16_3 <= ITEM_MAX) );
    item_read_bytes(itemAddr(ui16_3), (uint8_t *)sl->it, ITEM_SIZEOF);
    if ( (0 == sl->it[0].id) || (sl->it[0].is_disabled) ) {
      LCD_CLRLINE(0);
      LCD_WR_NP((const char *)PSTR("Invalid Item"), 12);
      _delay_ms(1000);
      continue; /* match not found */
    }

    /* Display item to be billed */
    LCD_CLRLINE(0);
    LCD_PUT_UINT(ui8_5+1);
    LCD_PUTCH('.');
    LCD_WR_N(sl->it[0].name, (ITEM_NAME_BYTEL>12)?12:ITEM_NAME_BYTEL);

    /* common inputs */
    sl->items[ui8_5].ep_item_ptr = itemAddr(ui16_3);
    sl->items[ui8_5].cost = sl->it[0].cost;
    sl->items[ui8_5].discount = sl->it[0].discount;
    sl->items[ui8_5].has_vat = sl->it[0].has_vat;
    sl->items[ui8_5].has_tax1 = sl->it[0].has_tax1;
    sl->items[ui8_5].has_tax2 = sl->it[0].has_tax2;
    sl->items[ui8_5].has_tax3 = sl->it[0].has_tax3;
    sl->items[ui8_5].has_common_discount = sl->it[0].has_common_discount;
    sl->items[ui8_5].is_reverse_tax = sl->it[0].is_reverse_tax;
    sl->items[ui8_5].has_weighing_mc = sl->it[0].has_weighing_mc;
    if (! (sl->items[ui8_5].has_weighing_mc) ) {
      arg2.valid = MENU_ITEM_NONE;
      menuGetOpt(menu_str1+(MENU_STR1_IDX_SALEQTY*MENU_PROMPT_LEN), &arg2, MENU_ITEM_FLOAT, NULL);
      if (MENU_ITEM_FLOAT != arg2.valid) /* start afresh */
	continue;
      sl->items[ui8_5].quantity = arg2.value.integer.i32;
      sl->items[ui8_5].quantity *= 10;
    } else {
      assert(0); /* Unimplemented */
    }

  menuBillingConfirm:
    /* Enable edit of earlier added item  */
    do {
      /* Display item to be billed */
      LCD_CLRLINE(0);
      LCD_PUT_UINT(sl->it[0].id);
      LCD_PUTCH('.');
      LCD_WR_N(sl->it[0].name, (ITEM_NAME_BYTEL>6)?6:ITEM_NAME_BYTEL);
      LCD_PUTCH('*');
      LCD_PUT_FLOAT(sl->items[ui8_5].quantity/10);

      /* Display item for confirmation */
      LCD_CLRLINE(LCD_MAX_ROW-1);
      LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_CONFI*MENU_PROMPT_LEN), MENU_PROMPT_LEN);
      ui32_1 = sl->items[ui8_5].cost;
      ui32_1 *= sl->items[ui8_5].quantity;
      ui32_1 /= 1000;
      LCD_PUT_FLOAT(ui32_1);

      KBD_RESET_KEY;
      KBD_GETCH;
      if (ASCII_ENTER == keyHitData.KbdData) { /* accept */
	ui8_5++;
	LCD_CLRLINE(0);
	LCD_WR_NP(PSTR("Added to bill!"), 14);
      } else if ( (ASCII_DOWN == keyHitData.KbdData) ||
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
	  sl->items[ui8_5].cost = arg2.value.integer.i32;

	/* override discount */
	LCD_CLRLINE(0);
	LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 3);
	LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_DISCO*MENU_PROMPT_LEN), MENU_PROMPT_LEN);
	LCD_PUTCH(':');
	LCD_PUT_UINT(sl->items[ui8_5].discount);
	arg2.valid = MENU_ITEM_NONE;
	floatPercentHelperValue = sl->items[ui8_5].cost;
	menuGetOpt(menu_str1+(MENU_STR1_IDX_DISCO*MENU_PROMPT_LEN), &arg2, MENU_ITEM_FLOAT, menuFloatPercentHelper);
	if (MENU_ITEM_FLOAT == arg2.valid)
	  sl->items[ui8_5].discount = arg2.value.integer.i32;

	/* override has_vat */
	LCD_CLRLINE(0);
	LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 3);
	LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_VAT*MENU_PROMPT_LEN), MENU_PROMPT_LEN);
	LCD_PUTCH(':');
	LCD_PUT_UINT(sl->items[ui8_5].has_vat);
	sl->items[ui8_5].has_vat = menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_VAT*MENU_PROMPT_LEN), MENU_PROMPT_LEN, 0);
	/* override tax1 */
	LCD_CLRLINE(0);
	LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 3);
	LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_TAX1*MENU_PROMPT_LEN), MENU_PROMPT_LEN);
	LCD_PUTCH(':');
	LCD_PUT_UINT(sl->items[ui8_5].has_tax1);
	sl->items[ui8_5].has_tax1 = menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_TAX1*MENU_PROMPT_LEN), MENU_PROMPT_LEN, 0);
	/* override tax2 */
	LCD_CLRLINE(0);
	LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 3);
	LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_TAX2*MENU_PROMPT_LEN), MENU_PROMPT_LEN);
	LCD_PUTCH(':');
	LCD_PUT_UINT(sl->items[ui8_5].has_tax2);
	sl->items[ui8_5].has_tax2 = menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_TAX2*MENU_PROMPT_LEN), MENU_PROMPT_LEN, 0);
	/* override tax3 */
	LCD_CLRLINE(0);
	LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 3);
	LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_TAX3*MENU_PROMPT_LEN), MENU_PROMPT_LEN);
	LCD_PUTCH(':');
	LCD_PUT_UINT(sl->items[ui8_5].has_tax3);
	sl->items[ui8_5].has_tax3 = menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_TAX3*MENU_PROMPT_LEN), MENU_PROMPT_LEN, 0);

	/* override has_common_discount */
	LCD_CLRLINE(0);
	LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 3);
	LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_COMNDISC*MENU_PROMPT_LEN), MENU_PROMPT_LEN);
	LCD_PUTCH(':');
	LCD_PUT_UINT(sl->items[ui8_5].has_common_discount);
	sl->items[ui8_5].has_common_discount = menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_COMNDISC*MENU_PROMPT_LEN), MENU_PROMPT_LEN, 0);

	/* override quantity */
	LCD_CLRLINE(0);
	LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 3);
	LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_SALEQTY*MENU_PROMPT_LEN), MENU_PROMPT_LEN);
	LCD_PUTCH(':');
	LCD_PUT_UINT(sl->items[ui8_5].quantity/10);
	arg2.valid = MENU_ITEM_NONE;
	menuGetOpt(menu_str1+(MENU_STR1_IDX_SALEQTY*MENU_PROMPT_LEN), &arg2, MENU_ITEM_FLOAT, NULL);
	if (MENU_ITEM_FLOAT == arg2.valid) {
	  sl->items[ui8_5].quantity = arg2.value.integer.i32;
	  sl->items[ui8_5].quantity *= 10;
	}

	LCD_CLRLINE(0);
	LCD_WR_P(PSTR("Override Done!"));
      } else if ( (ASCII_LEFT == keyHitData.KbdData) ||
		  (ASCII_UP == keyHitData.KbdData) ) {
	/* move to prev item to edit it */
	ui8_5 ? --ui8_5 : sl->info.n_items-1;
	item_read_bytes(sl->items[ui8_5].ep_item_ptr, (void *)sl->it, ITEM_SIZEOF);
      } else if (ASCII_DEL == keyHitData.KbdData) {
	/* delete the item */
	for (ui8_2=ui8_5; sl->items[ui8_2+1].quantity>0; ui8_2++) {
	  memcpy(&(sl->items[ui8_2]), &(sl->items[ui8_2+1]), sizeof(struct sale_item));
	}
	sl->items[ui8_2].quantity = 0;
	LCD_CLRLINE(LCD_MAX_ROW-1);
	LCD_WR_P(PSTR("Deleted!"));
      } else {
	LCD_CLRLINE(LCD_MAX_ROW-1);
	LCD_WR_P(PSTR("Skipped, Retry!"));
	continue;
      }
      break;
    } while (1);
  }

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

  /* Calculate bill, confirm */
  ui32_1=0;
  sl->t_tax1 = sl->t_tax2 = sl->t_tax3 = sl->t_vat = 0;
  sl->t_discount = 0, sl->total = 0;
  ui32_2 = 0;
  for (ui8_3=0; ui8_3<ui8_5; ui8_3++) {
    item_read_bytes(sl->items[ui8_3].ep_item_ptr, (void *)sl->it, ITEM_SIZEOF);

    if ( (0 != sl->items[ui8_3].discount) && (sl->items[ui8_3].cost >= sl->items[ui8_3].discount) ) {
      ui32_1 = (sl->items[ui8_3].cost - sl->items[ui8_3].discount);
      sl->t_discount += sl->items[ui8_3].discount * sl->items[ui8_3].quantity / 10;
    } else if (sl->items[ui8_3].has_common_discount) {
      ui32_1 = eeprom_read_word((uint16_t *)offsetof(struct ep_store_layout, CommonDisc));
      if (ui32_1 <= 10000) {
	ui32_1 = (10000 - ui32_1) * sl->items[ui8_3].cost;
	ui32_1 /= 100;
	sl->t_discount += (sl->items[ui8_3].cost - ui32_1) * sl->items[ui8_3].quantity / 10;
      } else {
	ui32_1 = sl->items[ui8_3].cost;
	LCD_ALERT(PSTR("Err: ComnDis > 100%"));
      }
    } else
      ui32_1 = sl->items[ui8_3].cost;

    if (sl->items[ui8_3].has_vat) {
      ui32_2 = sl->it[0].Vat;
      ui32_2 *= ui32_1;
      ui32_2 /= 100;
      sl->t_vat += ui32_2;
      if (!(sl->items[ui8_3].is_reverse_tax))
	sl->total += ui32_2;
    }
    if (sl->items[ui8_3].has_tax1) {
      ui32_2 = sl->it[0].Tax1;
      ui32_2 *= ui32_1;
      ui32_2 /= 100;
      sl->t_tax1 += ui32_2;
      if (!(sl->items[ui8_3].is_reverse_tax))
	sl->total += ui32_2;
    }
    if (sl->items[ui8_3].has_tax2) {
      ui32_2 = sl->it[0].Tax2;
      ui32_2 *= ui32_1;
      ui32_2 /= 100;
      sl->t_tax2 += ui32_2;
      if (!(sl->items[ui8_3].is_reverse_tax))
	sl->total += ui32_2;
    }
    if (sl->items[ui8_3].has_tax3) {
      ui32_2 = sl->it[0].Tax3;
      ui32_2 *= ui32_1;
      ui32_2 /= 100;
      sl->t_tax3 += ui32_2;
      if (!(sl->items[ui8_3].is_reverse_tax))
	sl->total += ui32_2;
    }

    ui32_1 *= sl->items[ui8_3].quantity / 10;
    sl->total += ui32_1;
  }

  /* Final confirmation before billing */
  LCD_CLRLINE(0);
  LCD_PUTCH("#");
  LCD_PUT_UINT(ui8_5);
  LCD_PUTCH(":");
  LCD_PUT_FLOAT(sl->total);
  LCD_CLRLINE(1);
  if (0 != menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_FINALIZ*MENU_PROMPT_LEN), MENU_PROMPT_LEN, 0))
    return 0;

  /* How much cash paid */
  LCD_CLRLINE(1);
  arg1.valid = MENU_ITEM_NONE;
  menuGetOptFloatInitHelper(NULL, NULL, sl->total);
  do {
    menuGetOpt(menu_str1+(MENU_STR1_IDX_CASHPAY*MENU_PROMPT_LEN), &arg1, MENU_ITEM_FLOAT, menuGetOptFloatInitHelper);
  } while ((MENU_ITEM_FLOAT != arg1.valid) || (arg1.value.integer.i32 > sl->total));
  sl->t_cash_pay = arg1.value.integer.i32;
  sl->t_other_pay = sl->total - arg1.value.integer.i32;

  /* set sale-info */
  sl->info.n_items = ui8_5;
  ui32_2 = get_fattime();
  sl->info.date_dd = ((ui32_2>>FAT_DATE_OFFSET)&FAT_DATE_MASK);
  sl->info.date_mm = ((ui32_2>>FAT_MONTH_OFFSET)&FAT_MONTH_MASK);
  sl->info.date_yy = ((ui32_2>>FAT_YEAR_OFFSET)&FAT_YEAR_MASK);
  sl->info.time_hh = ((ui32_2>>FAT_HOUR_OFFSET)&FAT_HOUR_MASK);
  sl->info.time_mm = ((ui32_2>>FAT_MIN_OFFSET)&FAT_MIN_MASK);
  sl->info.time_ss = ((ui32_2>>FAT_SEC_OFFSET)&FAT_SEC_MASK);
  for (ui8_2=0; ui8_2<EPS_MAX_UNAME; ui8_2++)
    sl->info.user[ui8_2] = eeprom_read_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_users) + (EPS_MAX_UNAME*(LoginUserId-1)) + ui8_2));

  /* id */
  if (MENU_MODITEM == (mode & ~(MENU_MODEMASK|MENU_MREDOCALL))) {
    sl->info.id = eeprom_read_word((uint16_t *)(offsetof(struct ep_store_layout, unused_LastBillId))) + 1;
    eeprom_update_word((uint16_t *)(offsetof(struct ep_store_layout, unused_LastBillId)), sl->info.id);
  }

  /* now save the data */
  for (ui16_1=0, ui16_3=0; ui16_1<SIZEOF_SALE_EXCEP_ITEMS; ui16_1++)
    ui16_3 = _crc16_update(ui16_3, ((uint8_t *)sl)[ui16_1]);
  sl->crc = ui16_3;
  sl->crc_invert = ~ui16_3;
  bill_write_bytes(ui16_2, (uint8_t *)sl, SIZEOF_SALE_EXCEP_ITEMS);

  /* update next bill addr */
  ui16_2 = NVF_NEXT_SALE_RECORD(ui16_2);
  eeprom_update_word((uint16_t *)(offsetof(struct ep_store_layout, unused_nextBillAddr)), ui16_2);

  /* */
  LCD_ALERT(PSTR("Bill Saved!"));

  /* Now print the bill */
  menuPrnBill(sl, menuPrnBillNvfHelper);

  return 0;
}

uint8_t
menuAddItem(uint8_t mode)
{
  uint8_t ui8_1, ui8_2, ui8_3;
  uint16_t ui16_1, ui16_2, ui16_3;
  struct item *it = (void *)(bufSS+LCD_MAX_COL+LCD_MAX_COL);
  uint8_t *bufSS_ptr = (void *) it;

  /* init */
  for (ui8_1=0; ui8_1<ITEM_SIZEOF; ui8_1++) {
    bufSS_ptr[ui8_1] = 0;
  }
  assert(numValidItems <= ITEM_MAX);

  /* conditions required to modify */
  if (MENU_ITEM_ID == arg2.valid) {
    if ( (0 == arg2.value.integer.i16) ||
	 (arg2.value.integer.i16 > ITEM_MAX) )
      goto menuItemInvalidArg;
    it->id = arg2.value.integer.i16;
  }

  /* Find space to place item */
  if (numValidItems >= ITEM_MAX) {
    LCD_ALERT(PSTR("Items Mem Full.."));
    return MENU_RET_NOTAGAIN;
  }
  if ((0 == it->id) || (it->id > ITEM_MAX)) {
    ui16_2 = eeprom_read_word((uint16_t *)offsetof(struct ep_store_layout, unused_ItemLastUsed));
    for (ui16_3=0; ui16_3<ITEM_MAX; ui16_3++) {
      ui16_2++; ui16_2 = (ui16_2<=ITEM_MAX) ? ui16_2 : 1; /* next id */
      if (0xFF == itIdxs[ui16_2-1].crc_name3) {
	ui16_1 = itemAddr(ui16_2);
	item_read_bytes(ui16_1+(ITEM_SIZEOF>>2)-1, ((uint8_t *)it)+ITEM_SIZEOF-4, 4);
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
  item_read_bytes(ui16_1, (void *)it, ITEM_SIZEOF);
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
  if (0 != menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_REPLA*MENU_PROMPT_LEN), MENU_PROMPT_LEN, 0))
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
  LCD_PUT_FLOAT(it->cost);
  arg1.valid = MENU_ITEM_NONE;
  menuGetOpt(menu_str1+(MENU_STR1_IDX_PRICE*MENU_PROMPT_LEN), &arg1, MENU_ITEM_FLOAT, NULL);
  if (MENU_ITEM_FLOAT != arg1.valid) {
    goto menuItemInvalidArg;
  }
  it->cost = arg1.value.integer.i32;

  /* Discount */
  floatPercentHelperValue = it->cost;
  LCD_CLRLINE(0);
  LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 4);
  LCD_PUT_FLOAT(it->discount);
  arg2.valid = MENU_ITEM_NONE;
  menuGetOpt(menu_str1+(MENU_STR1_IDX_DISCO*MENU_PROMPT_LEN), &arg2, MENU_ITEM_FLOAT|MENU_ITEM_OPTIONAL, menuFloatPercentHelper);
  it->discount = (MENU_ITEM_FLOAT == arg2.valid) ? arg2.value.integer.i32 : 0;

  /* Taxes */
  LCD_CLRLINE(0);
  LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 4);
  LCD_PUT_FLOAT(it->Vat);
  arg2.valid = MENU_ITEM_NONE;
  menuGetOpt(menu_str1+(MENU_STR1_IDX_VAT*MENU_PROMPT_LEN), &arg2, MENU_ITEM_FLOAT|MENU_ITEM_OPTIONAL, menuFloatPercentHelper);
  it->Vat = (MENU_ITEM_FLOAT == arg2.valid) ? arg2.value.integer.i32 : 0;
  //
  LCD_CLRLINE(0);
  LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 4);
  LCD_PUT_FLOAT(it->Tax1);
  arg2.valid = MENU_ITEM_NONE;
  menuGetOpt(menu_str1+(MENU_STR1_IDX_TAX1*MENU_PROMPT_LEN), &arg2, MENU_ITEM_FLOAT|MENU_ITEM_OPTIONAL, menuFloatPercentHelper);
  it->Tax1 = (MENU_ITEM_FLOAT == arg2.valid) ? arg2.value.integer.i32 : 0;
  //
  LCD_CLRLINE(0);
  LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 4);
  LCD_PUT_FLOAT(it->Tax2);
  arg2.valid = MENU_ITEM_NONE;
  menuGetOpt(menu_str1+(MENU_STR1_IDX_TAX2*MENU_PROMPT_LEN), &arg2, MENU_ITEM_FLOAT|MENU_ITEM_OPTIONAL, menuFloatPercentHelper);
  it->Tax2 = (MENU_ITEM_FLOAT == arg2.valid) ? arg2.value.integer.i32 : 0;
  //
  LCD_CLRLINE(0);
  LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 4);
  LCD_PUT_FLOAT(it->Tax3);
  arg2.valid = MENU_ITEM_NONE;
  menuGetOpt(menu_str1+(MENU_STR1_IDX_TAX3*MENU_PROMPT_LEN), &arg2, MENU_ITEM_FLOAT|MENU_ITEM_OPTIONAL, menuFloatPercentHelper);
  it->Tax3 = (MENU_ITEM_FLOAT == arg2.valid) ? arg2.value.integer.i32 : 0;

  /* choices */
  LCD_CLRLINE(0);
  LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 4);
  LCD_PUT_UINT(it->has_vat);
  it->has_vat = ~menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_HAS_VAT*MENU_PROMPT_LEN), MENU_PROMPT_LEN, 1);
  LCD_CLRLINE(0);
  LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 4);
  LCD_PUT_UINT(it->has_tax1);
  it->has_tax1 = ~menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_HAS_TAX1*MENU_PROMPT_LEN), MENU_PROMPT_LEN, 1);
  LCD_CLRLINE(0);
  LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 4);
  LCD_PUT_UINT(it->has_tax2);
  it->has_tax2 = ~menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_HAS_TAX2*MENU_PROMPT_LEN), MENU_PROMPT_LEN, 1);
  LCD_CLRLINE(0);
  LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 4);
  LCD_PUT_UINT(it->has_tax3);
  it->has_tax3 = ~menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_HAS_TAX3*MENU_PROMPT_LEN), MENU_PROMPT_LEN, 1);
  LCD_CLRLINE(0);
  LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 4);
  LCD_PUT_UINT(it->has_weighing_mc);
  it->has_weighing_mc = ~menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_HAS_WMC*MENU_PROMPT_LEN), MENU_PROMPT_LEN, 1);
  LCD_CLRLINE(0);
  LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 4);
  LCD_PUT_UINT(it->is_reverse_tax);
  it->is_reverse_tax = ~menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_IS_TAX_INCL*MENU_PROMPT_LEN), MENU_PROMPT_LEN, 0);
  LCD_CLRLINE(0);
  LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 4);
  LCD_PUT_UINT(it->has_common_discount);
  it->has_common_discount = ~menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_HAS_COMDIS*MENU_PROMPT_LEN), MENU_PROMPT_LEN+(11-MENU_PROMPT_LEN), 1);

  /* Confirm */
  LCD_CLRLINE(0);
  LCD_WR_P(PSTR("Item:"));
  LCD_WR_N(it->name, ((ITEM_NAME_BYTEL+5)>LCD_MAX_COL) ? (LCD_MAX_COL-5) : ITEM_NAME_BYTEL);
  if (0 != menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_CONFI*MENU_PROMPT_LEN), MENU_PROMPT_LEN, 0))
    return 0;

  /* From here on ... the item is complete */
  it->is_disabled = 0;

  /* Calculate CRC */
  for (ui8_1=0, ui8_2=0; ui8_1<(ITEM_SIZEOF-2); ui8_1++) {
    ui8_2 = _crc_ibutton_update(ui8_2, bufSS_ptr[ui8_1]);
  }
  /* identify if we are modifying an old item or not */
  if (0xFF != (it->unused_crc ^ it->unused_crc_invert))
    numValidItems++;
  it->unused_crc = ui8_2;
  it->unused_crc_invert = ~ui8_2;

  /* Now save item */
  ui8_3 = item_write_bytes(ui16_1, bufSS_ptr, ITEM_SIZEOF);
  assert(ITEM_SIZEOF == ui8_3);

  /* only valid items needs to be buffered */
  menuIndexItem(it);
  assert(numValidItems <= ITEM_MAX);

  /* Give +ve report */
  LCD_ALERT((const char *)menu_str1+(MENU_STR1_IDX_SUCCESS*MENU_PROMPT_LEN));

  return MENU_RET_NOTAGAIN;
}

/* Indexing at the following levels
RAM:
   1. Complete product code
   2. First 3 letters of name
Internal EEPROM:
   3. Complete name
 */
void
menuIndexItem(struct item *it)
{
  uint8_t ui8_1;

  /* Delete indexing */
  if ( (0 == it->id) || (it->id > ITEM_MAX) || (it->is_disabled) ) {
    itIdxs[it->id-1].crc_name3 = 0xFF;
    eeprom_update_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_itIdxName))+it->id-1, 0xFF);
    eeprom_update_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_crc_prod_code))+it->id-1, 0xFF);
    assert(0); /* I suppose we never reach here */
    return;
  }

  /* index item */
  uint8_t cn, cpc, cn3, na;
  for (ui8_1=0, cn=cn3=0; ui8_1<ITEM_NAME_BYTEL; ui8_1++) {
    na = toupper(it->name[ui8_1]);
    cn = _crc_ibutton_update(cn, na);
    if (2 == ui8_1)
      cn3 = cn;
  }
  for (ui8_1=0, cpc=0; ui8_1<ITEM_PROD_CODE_BYTEL; ui8_1++) {
    cpc = _crc_ibutton_update(cpc, it->prod_code[ui8_1]);
  }
  itIdxs[it->id-1].crc_name3 = cn3;
  eeprom_update_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_itIdxName))+it->id-1, cn);
  eeprom_update_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_crc_prod_code))+it->id-1, cpc);
}

/* either name or prod_code be NULL
   idx: starting idx for recursive find's
 */
uint16_t
menuItemFind(uint8_t *name, uint8_t *prod_code, struct item *it, uint16_t idx)
{
  uint16_t ui16_1, ui16_2;
  uint8_t  ui8_1, cn=0, cpc=0, cn3=0;
  uint8_t  ufbm = 1;

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
    uint8_t o_cpc = eeprom_read_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_crc_prod_code))+idx-1);
    uint8_t o_idxn = eeprom_read_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_itIdxName))+idx-1); 
    if (0xFF == itIdxs[idx-1].crc_name3) {
      if ( (0xFF == o_cpc) && (0xFF == o_idxn) )
	continue;
    }
    if (NULL == name) {
      if (NULL == prod_code)
	return idx;
      else if (o_cpc != cpc)
	continue;
    } else { /* NULL != name */
      if ( (itIdxs[idx-1].crc_name3 == cn3) && (1 == ufbm) )
	;
      else if ( (NULL != prod_code) &&
		(o_cpc != cpc) &&
		(o_idxn != cn) )
	continue;
      else if ( (NULL == prod_code) &&
		(o_idxn != cn) )
	continue;
    }
    ui16_2 = itemAddr(idx);
    item_read_bytes(ui16_2, (void *)it, ITEM_SIZEOF);
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
  uint16_t ui16_1, ui16_2;
  struct item *it = (void *)(bufSS+LCD_MAX_COL+LCD_MAX_COL);

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
  item_read_bytes(ui16_2+(ITEM_SIZEOF>>2)-1, ((uint8_t *)it)+ITEM_SIZEOF-4, 4);
  if (0xFF != (it->unused_crc ^ it->unused_crc_invert))
    return MENU_RET_NOTAGAIN;

  /* delete */
  assert(numValidItems > 0); 
  itIdxs[ui16_1-1].crc_name3 = 0xFF;
  eeprom_update_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_itIdxName))+ui16_1-1, 0xFF);
  eeprom_update_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_crc_prod_code))+ui16_1-1, 0xFF);
  item_write_bytes(ui16_2+(ITEM_SIZEOF>>2)-1, NULL, 4);
  numValidItems--;

  return MENU_RET_NOTAGAIN;
}

void
menuPrintTestPage(uint8_t mode)
{
#if MENU_DIAG_FUNC && MENU_PRINTER_ENABLE
  PRINTER_PRINT_TEST_PAGE;
#endif
}

static void
menuPrnD(uint32_t var)
{
  if (var>9)
    menuPrnD(var/10);
  PRINTER_PRINT('0'+(var%10));
}

static void
menuPrnF(uint32_t var)
{
  menuPrnD(var/100);
  PRINTER_PRINT('.');
  var %= 100;
  if (var < 10) PRINTER_PRINT('0');
  menuPrnD(var%100);
}

static void
menuSdD(uint32_t var)
{
  if (var>9)
    menuSdD(var/10);
  f_putc('0'+(var%10), &Fil);
}

static void
menuSdF(uint32_t var)
{
  menuSdD(var/100);
  f_putc('.', &Fil);
  var %= 100;
  if (var < 10) f_putc('0', &Fil);
  menuSdD(var%100);
}

static uint8_t
menuSdSCheckFix(uint8_t *s, uint8_t size)
{
  uint8_t graphSeen = 0;

  for (;size;++s, size--)
    if (',' == s[0]) /* csv format: not allowed */
      return 2;
    else if (isgraph(s[0])) {
      s[0] = toupper(s[0]);
      graphSeen = 1;
    } else if (' ' == s[0])
      ;
    else if (0 == graphSeen)
      return 1;
    else
      s[0] = ' ';

  return 0;
}

static uint8_t
menuSdSwallowComma()
{
  FSIZE_t loc;
  UINT ret_size;
  uint8_t ui8_a[8];
  uint8_t commaSeen = 0;

  while (1) {
    loc = f_tell(&Fil);
    f_read(&Fil, ui8_a, 1, &ret_size);
    if (1 != ret_size) return 1;
    if (' ' == ui8_a[0]) continue;
    if ((0 == commaSeen) && (',' == ui8_a[0])) {
      commaSeen = 1;
    } else {
      f_lseek(&Fil, loc);
      break;
    }
  }

  return commaSeen ? 0 : 1;
}

static uint8_t
menuSdSwallowUntilNL()
{
  FSIZE_t loc;
  UINT ret_size;
  uint8_t ui8_a[8];
  uint8_t nlSeen = 0;

  while (1) {
    loc = f_tell(&Fil);
    f_read(&Fil, ui8_a, 1, &ret_size);
    if (1 != ret_size) return 1;
    if (('\n' == ui8_a[0]) || ('\r' == ui8_a[0])) {
      nlSeen = 1;
    } else if ((1 == nlSeen) && isgraph(ui8_a[0])) {
      f_lseek(&Fil, loc);
      break;
    }
  }

  return 0;
}

static uint8_t
menuSdScanD(uint32_t *d, uint8_t *s)
{
  FSIZE_t loc;
  UINT  ret_size;

  *d = 0;
  for (uint8_t ui8_1=0; ;ui8_1++) {
    loc = f_tell(&Fil);
    f_read(&Fil, s, 1, &ret_size);
    if (1 != ret_size) return 1;
    if (! isdigit(s[0])) {
      f_lseek(&Fil, loc);
      return ui8_1 ? 0 : 3;
    }
    *d *= 10;
    *d += s[0] - '0';
  } while (1);

  return 2;
}

static uint8_t
menuSdScanF(uint32_t *f, uint8_t *s)
{
  FSIZE_t loc;
  UINT ret_size;

  if (0 != menuSdScanD(f, s))
    return 1;

  loc = f_tell(&Fil);
  f_read(&Fil, s, 1, &ret_size);
  if (1 != ret_size) return 1;
  if ('.' != s[0]) {
    f_lseek(&Fil, loc);
    *f *= 100;
    return 0;
  }

  for (uint8_t ui8_1=2; ui8_1; ui8_1--) {
    *f *= 10;
    loc = f_tell(&Fil);
    f_read(&Fil, s, 1, &ret_size);
    if ((1 != ret_size) || !isdigit(s[0])) {
      f_lseek(&Fil, loc);
    }
    *f += s[0] - '0';
  }
  return 0;
}

static void
menuPrnHeader()
{
  uint8_t ui8_1, ui8_2, ui8_3;

  /* Shop name */
  PRINTER_FONT_ENLARGE(2);
  PRINTER_JUSTIFY(PRINTER_JCENTER);
  for (ui8_2=ui8_1=0; ui8_1<SHOP_NAME_SZ_MAX; ui8_2++, ui8_1++) {
    if (ui8_2 > (PRINTER_MAX_CHARS_ON_LINE>>1)) {
      PRINTER_PRINT('\r'); PRINTER_PRINT('\n');
      ui8_2 = 0;
    }
    ui8_3 = eeprom_read_byte((uint8_t *)(offsetof(struct ep_store_layout, ShopName)+ui8_1));
    if (0 == ui8_3) break;
    PRINTER_PRINT(ui8_3);
  }
  PRINTER_FONT_ENLARGE(1);
  PRINTER_PRINT('\r'); PRINTER_PRINT('\n');
  PRINTER_PRINT('\r'); PRINTER_PRINT('\n');

  /* Header */
  for (ui8_1=0; ui8_1<HEADER_SZ_MAX; ui8_1++) {
    ui8_3 = eeprom_read_byte((uint8_t *)(offsetof(struct ep_store_layout, BillHeader)+ui8_1));
    PRINTER_PRINT(ui8_3);
  }
  PRINTER_PRINT('\r'); PRINTER_PRINT('\n');
  PRINTER_PRINT('\r'); PRINTER_PRINT('\n');

  /* date printed */
  date_t date;
  timerDateGet(date);
  PRINTER_PSTR(PSTR("Print Date:"));
  PRINTER_PRINT_D(date.day); PRINTER_PRINT('/');
  PRINTER_PRINT_D(date.month); PRINTER_PRINT('/');
  PRINTER_PRINT_D(date.year); PRINTER_PRINT(' ');
}

static void
menuPrnFooter()
{
  uint8_t ui8_1, ui8_2, ui8_3;

  /* Footer */
  PRINTER_PRINT('\r'); PRINTER_PRINT('\n');
  PRINTER_PRINT('\r'); PRINTER_PRINT('\n');
  ui8_2 = 0;
  for (ui8_1=0; ui8_1<FOOTER_SZ_MAX; ui8_1++) {
    ui8_3 = eeprom_read_byte((uint8_t *)(offsetof(struct ep_store_layout, BillFooter)+ui8_1));
    ui8_2 = ('\n' == ui8_3) ? 0 :
      ( (PRINTER_MAX_CHARS_ON_LINE == ui8_2) ? 0 : ui8_2+1 );
    if (0 == ui8_2) {
      PRINTER_PRINT('\r'); PRINTER_PRINT('\n');
    }
    if ('\n' != ui8_3)
      PRINTER_PRINT(ui8_3);
  }
  PRINTER_PRINT('\r'); PRINTER_PRINT('\n');
  PRINTER_PRINT('\r'); PRINTER_PRINT('\n');
}

void
menuPrnBill(struct sale *sl, menuPrnBillItemHelper nitem)
{
#if MENU_PRINTER_ENABLE
  uint8_t ui8_1, ui8_3;

  menuPrnHeader();

  /* Caption, Date */
  PRINTER_FONT_ENLARGE(2);
  PRINTER_JUSTIFY(PRINTER_JCENTER);
  for (ui8_1=0; ui8_1<EPS_CAPTION_SZ_MAX; ui8_1++) {
    ui8_3 = eeprom_read_byte((uint8_t *)(offsetof(struct ep_store_layout, Caption)+ui8_1));
    if ('\n' != ui8_3)
      PRINTER_PRINT(ui8_3);
  }
  PRINTER_PRINT('\r'); PRINTER_PRINT('\n');
  PRINTER_FONT_ENLARGE(1);
  PRINTER_JUSTIFY(PRINTER_JLEFT);
  PRINTER_PSTR(PSTR("Bill #"));
  PRINTER_PRINT_D(sl->info.id);
  PRINTER_PRINT(':');
  PRINTER_PRINT_D(sl->info.date_dd); PRINTER_PRINT('-');
  PRINTER_PRINT_D(sl->info.date_mm); PRINTER_PRINT('-');
  PRINTER_PRINT_D(sl->info.date_yy); PRINTER_PRINT(' ');
  PRINTER_PRINT_D(sl->info.time_hh); PRINTER_PRINT(':');
  PRINTER_PRINT_D(sl->info.time_mm); PRINTER_PRINT(':');
  PRINTER_PRINT_D(sl->info.time_ss); PRINTER_PRINT('\r'); PRINTER_PRINT('\n');
  PRINTER_PRINT('\r'); PRINTER_PRINT('\n');
  PRINTER_PSTR(PSTR("Sl. Item             Nos  Price   Disc   Tax \r\n"));
  PRINTER_PSTR(PSTR("---------------------------------------------\r\n"));

  /* Items */
  for (ui8_1=0; ui8_1<sl->info.n_items; ui8_1++) {
    if (EEPROM_MAX_ADDRESS != sl->items[ui8_1].ep_item_ptr) {
      nitem(sl->items[ui8_1].ep_item_ptr, &(sl->it[0]), ui8_1);
    }
    PRINTER_PRINT_D((ui8_1+1));
    PRINTER_PRINT('.'); PRINTER_PRINT(' ');
    for (ui8_3=0; ui8_3<ITEM_NAME_BYTEL; ui8_3++)
      PRINTER_PRINT(sl->it[0].name[ui8_3]);
    PRINTER_PSTR(PSTR("  "));
    PRINTER_PRINT_F(sl->items[ui8_1].cost);
    PRINTER_PSTR(PSTR("  "));
    PRINTER_PRINT_F(sl->items[ui8_1].discount);
    PRINTER_PSTR(PSTR("  "));
    PRINTER_PRINT_F(sl->items[ui8_1].quantity/10);
    PRINTER_PSTR(PSTR("  "));
    uint32_t total = (sl->items[ui8_1].cost-sl->items[ui8_1].discount)/100*sl->items[ui8_1].quantity/1000;
#if 0
    uint32_t t_tax;
    t_tax = total * 
    if (! sl->items[ui8_1].is_reverse_tax) {
      total 
    }
#endif
    PRINTER_PRINT_F(total);
    PRINTER_PRINT('\r'); PRINTER_PRINT('\n');
  }

  /* Total */
  PRINTER_PSTR(PSTR("--------------------------------\r\n"));
  PRINTER_PSTR(PSTR("           "));
  PRINTER_PRINT_F(sl->t_discount);
  PRINTER_PSTR(PSTR(" "));
  PRINTER_PRINT_F(sl->t_vat);
  PRINTER_PSTR(PSTR(" "));
  PRINTER_PRINT_F(sl->t_tax1+sl->t_tax2+sl->t_tax3+sl->t_vat);
  PRINTER_PSTR(PSTR(" "));
  PRINTER_PRINT_F(sl->total);
  PRINTER_PRINT('\r'); PRINTER_PRINT('\n');

  menuPrnFooter();
#endif
}

typedef struct {
  uint32_t oper;
  uint8_t  op;
  uint8_t  valid;
} rhs_t;
typedef struct {
  uint16_t numRhs;
  rhs_t    rhs[1];
} calc_t;

static void
reduceCalc(calc_t *calc)
{
  uint16_t ui16_1;

  /* no more operands */
  if (calc->numRhs < 2) return;

  /* first do all multiplications */
  while (calc->numRhs > 1) {
    for (ui16_1=0; ui16_1<(calc->numRhs-1); ui16_1++) {
      if ('n' == calc->rhs[ui16_1].op) continue; /* invalid op */
      if (('*' == calc->rhs[ui16_1].op) && calc->rhs[ui16_1].valid &&
	  calc->rhs[ui16_1+1].valid) {
	calc->rhs[ui16_1].oper *= calc->rhs[ui16_1+1].oper;
	calc->rhs[ui16_1].op = calc->rhs[ui16_1+1].op;
	calc->rhs[ui16_1+1].valid = 0;
	calc->numRhs--;
	break;
      }
    }
    if (ui16_1 >= (calc->numRhs-1)) /* no '*' in operation */
      break;
    for (ui16_1++; ui16_1<(calc->numRhs-1); ui16_1++) {
      memcpy(((uint8_t *)&(calc->rhs[0]))+(sizeof(rhs_t)*ui16_1),
	     ((uint8_t *)&(calc->rhs[0]))+(sizeof(rhs_t)*(ui16_1+1)),
	     sizeof(rhs_t));
    }
  }

  /* Compute all other ops */
  while (calc->numRhs > 1) {
    for (ui16_1=0; ui16_1<(calc->numRhs-1); ui16_1++) {
      if ('n' == calc->rhs[ui16_1].op) continue; /* invalid op */
      if (calc->rhs[ui16_1].valid && calc->rhs[ui16_1+1].valid) {
	calc->rhs[ui16_1].oper += ('+' == calc->rhs[ui16_1].op) ?
	  calc->rhs[ui16_1+1].oper : -(calc->rhs[ui16_1+1].oper);
	calc->rhs[ui16_1].op = calc->rhs[ui16_1+1].op;
	calc->rhs[ui16_1+1].valid = 0;
 	calc->numRhs--;
	break;
      }
    }
    for (ui16_1++; ui16_1<(calc->numRhs-1); ui16_1++) {
      memcpy(((uint8_t *)&(calc->rhs[0]))+(sizeof(rhs_t)*ui16_1),
	     ((uint8_t *)&(calc->rhs[0]))+(sizeof(rhs_t)*(ui16_1+1)),
	     sizeof(rhs_t));
    }
  }
}

uint8_t
menuCalculator(uint8_t mode)
{
  calc_t *calc = (void *) (bufSS+LCD_MAX_COL+LCD_MAX_COL+8);
  uint8_t *buf = (void *) bufSS;
  uint8_t bufLen;

  for (uint8_t numEscapeRcvd=0; numEscapeRcvd<2; ) {
    /* */
    calc->numRhs = 0;
    calc->rhs[0].oper = 0;
    bufLen = 0;

    while (1) {
      /* Display */
      LCD_CLRLINE(0);
      LCD_WR_N(bufSS, LCD_MAX_COL);
      LCD_CLRLINE(1);
      LCD_PUT_UINT(calc->rhs[0].oper);
      uint8_t ui8_1 = sprintf_P((char *)(bufSS+LCD_MAX_COL), PSTR("%d"), calc->rhs[0].oper);
      LCD_cmd((LCD_CMD_CUR_20|(0x10-ui8_1)));
      LCD_WR((char *)(bufSS+LCD_MAX_COL));

      /* wait for user */
      KBD_RESET_KEY;
      KBD_GETCH;
      if ((keyHitData.KbdData >= '0') &&
	  (keyHitData.KbdData <= '9')) {
	if (bufLen < (LCD_MAX_COL>>1))
	  buf[bufLen++] = keyHitData.KbdData;
	numEscapeRcvd = 0;
      } else if ((ASCII_LEFT == keyHitData.KbdData) && (bufLen>0)) {
	/* one char back */
	--bufLen;
	buf[bufLen] = 0;
	numEscapeRcvd = 0;
      } else if ((ASCII_RIGHT == keyHitData.KbdData) || /* + */
		 (ASCII_LGUI == keyHitData.KbdData) || /* - */
		 (ASCII_ALT == keyHitData.KbdData) /* * */) {
	uint32_t ui32_1;
	numEscapeRcvd = 0;
	/* stray op */
	if (0 == bufLen) continue;
	/* get the operand */
	buf[bufLen] = 0;
	SSCAND(buf, ui32_1);
	calc->rhs[calc->numRhs].oper = ui32_1;
	calc->rhs[calc->numRhs].valid = 1;
	calc->rhs[calc->numRhs+1].valid = 0;
	/* now mark operator */
	calc->rhs[calc->numRhs].op = (ASCII_RIGHT == keyHitData.KbdData) ? '+':
	  (ASCII_LGUI == keyHitData.KbdData) ? '-' : '*';
	calc->numRhs++;
	bufLen = 0;
	if (ASCII_ALT != keyHitData.KbdData)
	  reduceCalc(calc);
      } else if (ASCII_ENTER == keyHitData.KbdData) {
	uint32_t ui32_1;
	/* Compute result */
	if (0 != bufLen) {
	  /* get the operand */
	  buf[bufLen] = 0;
	  SSCAND(buf, ui32_1);
	  calc->rhs[calc->numRhs].oper = ui32_1;
	  calc->rhs[calc->numRhs].valid = 1;
	  calc->rhs[calc->numRhs+1].valid = 0;
	  /* now mark operator */
	  calc->rhs[calc->numRhs].op = 'n';
	  calc->numRhs++;
	  bufLen = 0;
	}
	reduceCalc(calc);
	numEscapeRcvd = 0;
      } else if (ASCII_LEFT == keyHitData.KbdData) {
	numEscapeRcvd++;
	break;
      }
    }
  }

  return MENU_RET_NOTAGAIN;
}

uint8_t
menuViewOldBill(uint8_t mode)
{
#if FF_ENABLE
  uint8_t  ui8_1;
  struct sale *sl = (void *)(bufSS+LCD_MAX_COL+LCD_MAX_COL);
  UINT  ret_val;

  if (RES_OK != disk_inserted()) {
    LCD_ALERT(PSTR_NO_SD);
    return MENU_RET_NOTAGAIN;
  }

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

  /* iterate records */
  FSIZE_t loc = 0;
  ui8_1 = 0;
  while (1) {
    /* this bill */
    f_read(&Fil, (void *)sl, SIZEOF_SALE_EXCEP_ITEMS, &ret_val);
    if (SIZEOF_SALE_EXCEP_ITEMS != ret_val) break;
    assert (0xFFFF == (sl->crc_invert ^ sl->crc));

    /* check for id */
    if ((MENU_ITEM_ID == arg2.valid) && (sl->info.id != arg2.value.integer.i16)) {
      if ((0 == ui8_1) && (0 != loc)) { /* go prev */
	assert(loc >= MAX_SIZEOF_1BILL);
	loc -= MAX_SIZEOF_1BILL;
	f_lseek(&Fil, loc);
	continue;
      } else { /* go next */
	loc += MAX_SIZEOF_1BILL;
	if ((FR_OK != f_lseek(&Fil, loc)) || f_eof(&Fil)) {
	  loc -= MAX_SIZEOF_1BILL;
	  f_lseek(&Fil, loc);
	  continue;
	}
      }
    }

    /* Display bill */
    LCD_CLRLINE(0);
    LCD_PUTCH('#');
    LCD_PUT_UINT(sl->info.id);
    LCD_PUTCH(' ');
    LCD_PUTCH('R'); LCD_PUTCH('s');
    LCD_PUT_FLOAT(sl->total);
    LCD_CLRLINE(LCD_MAX_ROW-1);
    LCD_PUT_UINT(sl->info.date_dd);
    LCD_PUTCH('/');
    LCD_PUT_UINT(sl->info.date_mm);
    LCD_WR_P(PSTR(" EscPrnDel"));

    /* according to user's wish */
    KBD_RESET_KEY;
    KBD_GETCH;
    if (ASCII_ENTER == keyHitData.KbdData) {
      /* keep direction */
    } else if (ASCII_PRNSCRN == keyHitData.KbdData) {
      menuPrnBill(sl, menuPrnBillNvfHelper);
    } else if ((ASCII_LEFT == keyHitData.KbdData) || (ASCII_UP == keyHitData.KbdData)) {
      ui8_1 = 0;
    } else if ((ASCII_RIGHT == keyHitData.KbdData) || (ASCII_DOWN == keyHitData.KbdData)) {
      ui8_1 = 1;
    } else if (ASCII_ESCAPE == keyHitData.KbdData) {
      break;
    } else if (ASCII_DEL == keyHitData.KbdData) {
      if (0 == menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_DELETE*MENU_PROMPT_LEN), MENU_PROMPT_LEN, 0)) {
	sl->info.is_deleted = 1;
	f_lseek(&Fil, loc);
	f_write(&Fil, (void *)sl, SIZEOF_SALE_EXCEP_ITEMS, &ret_val);
	if (SIZEOF_SALE_EXCEP_ITEMS != ret_val) {
	  LCD_ALERT(PSTR("Save Failed"));
	  break;
	}
      }
    }

    /* */
    if ((0 == ui8_1) && (0 != loc)) { /* go prev */
      assert(loc >= MAX_SIZEOF_1BILL);
      loc -= MAX_SIZEOF_1BILL;
      f_lseek(&Fil, loc);
    } else { /* go next */
      loc += MAX_SIZEOF_1BILL;
      if ((FR_OK != f_lseek(&Fil, loc)) || f_eof(&Fil)) {
	loc -= MAX_SIZEOF_1BILL;
	f_lseek(&Fil, loc);
      }
    }
  }

  /* */
  if (FR_OK != f_chdir("..")) {
    LCD_ALERT(PSTR("UpDir Error"));
  }
  f_mount(NULL, "", 0);
#endif
  return MENU_RET_NOTAGAIN;
}

void
menuPrnBillNvfHelper(uint16_t item_addr, struct item *it, uint16_t it_index)
{
#if MENU_PRINTER_ENABLE
  item_read_bytes(item_addr, (void *)it, ITEM_SIZEOF);
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

  LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 4);
  for (ui16_1=0; ui16_1<max_chars; ui16_1++) {
    ui8_1 = eeprom_read_byte((uint8_t *)(addr+ui16_1));
    arg1.value.str.sptr[ui16_1] = isgraph(ui8_1) ? ui8_1 : ' ';
  }
  LCD_WR_N(arg1.value.str.sptr, LCD_MAX_COL-4);

  MENU_GET_OPT(quest, &arg1, MENU_ITEM_STR, NULL);
#if !UNIT_TEST
  if (0 != menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_CONFI*MENU_PROMPT_LEN), MENU_PROMPT_LEN, 0))
    return;
#endif

  if (MENU_ITEM_STR == arg1.valid) {
    for (ui16_1=0; ui16_1<max_chars; ui16_1++) {
      if (!isgraph(arg1.value.str.sptr[ui16_1])) {
	arg1.value.str.sptr[ui16_1] = ' ';
      } else if ( (ui16_1 > 0) &&
		  ('\\' == arg1.value.str.sptr[ui16_1-1]) &&
		  ('n' == arg1.value.str.sptr[ui16_1]) ) {
	arg1.value.str.sptr[ui16_1] = '\n';
	addr--;
      }
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
#if !UNIT_TEST
  if (0 != menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_CONFI*MENU_PROMPT_LEN), MENU_PROMPT_LEN, 0))
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
#if !UNIT_TEST
  if (0 != menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_CONFI*MENU_PROMPT_LEN), MENU_PROMPT_LEN, 0))
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
#if !UNIT_TEST
  if (0 != menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_CONFI*MENU_PROMPT_LEN), MENU_PROMPT_LEN, 0))
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
#if !UNIT_TEST
  if (0 != menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_CONFI*MENU_PROMPT_LEN), MENU_PROMPT_LEN, 0))
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

void
menuSettingYesNo(uint16_t addr, const uint8_t *quest)
{
  uint8_t val;

  LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 3);
  LCD_PUTCH(':');
  val = eeprom_read_byte((uint8_t *)addr);
  val &= 1;
  val ^= 1;
  LCD_WR_NP((const char *)menu_str2+(4*val), 3);

  arg1.valid = MENU_ITEM_NONE;
  val = menuGetYesNo(quest, MENU_PROMPT_LEN, 0);
#if !UNIT_TEST
  if (0 != menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_CONFI*MENU_PROMPT_LEN), MENU_PROMPT_LEN, 0))
    return;
#endif

  val ^= 1;
  eeprom_update_byte((void *)addr, val);
}
#endif

uint8_t
menuSetDateTime(uint8_t mode)
{
  date_t date;
  timerDateGet(date);
  LCD_CLRLINE(0);
  LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 4);
  LCD_PUT_UINT(date.day);
  LCD_PUTCH('/');
  LCD_PUT_UINT(date.month);
  LCD_PUTCH('/');
  date.year %= 100;
  LCD_PUT_UINT(date.year);
  MENU_GET_OPT(menu_prompt_str+(MENU_PR_DATE*MENU_PROMPT_LEN), &arg1, MENU_ITEM_DATE, NULL);
  if (MENU_ITEM_DATE == arg1.valid) {
    timerDateSet(arg1.value.date);
  }

  s_time_t time;
  timerTimeGet(time);
  LCD_CLRLINE(0);
  LCD_WR_NP((const char *)menu_str1+(MENU_STR1_IDX_OLD*MENU_PROMPT_LEN), 4);
  LCD_PUT_UINT(time.hour);
  LCD_PUTCH(':');
  LCD_PUT_UINT(time.min);
  MENU_GET_OPT(menu_prompt_str+(MENU_PR_TIME*MENU_PROMPT_LEN), &arg2, MENU_ITEM_TIME, NULL);
  if (MENU_ITEM_TIME == arg2.valid) {
    timerTimeSet(arg2.value.time);
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
  case TYPE_YESNO:
    menuSettingYesNo(ui16_1, ((uint8_t *)(SettingVars+(ui8_1-1)))+offsetof(struct setting_vars, name));
    break;
  default:
    assert(0);
    return 0;
  }

  goto menuSettingSetStart;
#endif
}

uint8_t
menuSetHotKey(uint8_t mode)
{
  /* id */
  LCD_CLRLINE(LCD_MAX_ROW-1);
  LCD_WR_NP((const char *)PSTR("Valid Prod Id:"), 14);
  do {
    MENU_GET_OPT(menu_str1+(MENU_STR1_IDX_ITEM*MENU_PROMPT_LEN), &arg1, MENU_ITEM_ID, NULL);
    if ((0 == arg1.value.integer.i16) ||
	(arg1.value.integer.i16 > ITEM_MAX))
      continue;
    if ( (0xFF == itIdxs[arg1.value.integer.i16-1].crc_name3) &&
	 (0xFF == eeprom_read_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_crc_prod_code))+arg1.value.integer.i16-1)) &&
	 (0xFF == eeprom_read_byte((uint8_t *)(offsetof(struct ep_store_layout, unused_itIdxName))+arg1.value.integer.i16-1)) ) {
      continue;
    }
    break;
  } while (1);

  /* now get the key combination */
  do {
    LCD_CLRLINE(LCD_MAX_ROW-1);
    KBD_RESET_KEY;
#if UNIT_TEST
    keypadMultiKeyModeOff = 1;
#endif
    KBD_GETCH;
#if UNIT_TEST
    keypadMultiKeyModeOff = 0;
#endif
    if (keyHitData.KbdDataAvail & kbdWinHit) {
      LCD_WR_P((const char *)PSTR("Win+"));
    }
    if (keyHitData.KbdDataAvail & kbdAltHit) {
      LCD_WR_P((const char *)PSTR("Alt+"));
    }
    LCD_WR_NP((const char *)(keyMapR+(3*keyHitData.KbdData)), 3);
    _delay_ms(500);
  } while ( (keyHitData.KbdData >= 16) ||
	    (0 == (keyHitData.KbdDataAvail & (kbdWinHit | kbdAltHit | kbdHit))) );

  /* assign */
  uint16_t idx = offsetof(struct ep_store_layout, unused_HotKey) +
    (keyHitData.KbdData * sizeof(uint16_t)) +
    (((keyHitData.KbdDataAvail & (kbdWinHit|kbdAltHit)) << 4) * sizeof(uint16_t));
  eeprom_update_word((uint16_t *)idx, arg1.value.integer.i16);
  LCD_ALERT((const char *)menu_str1+(MENU_STR1_IDX_SUCCESS*MENU_PROMPT_LEN));
  _delay_ms(500);

  return 0;
}

static uint8_t
menuCheckDateFromTo(uint8_t mode)
{
  /* if dates not provided assume today */
  {
    date_t date;
    timerDateGet(date);
    if (MENU_ITEM_DATE != arg1.valid) {
      arg1.valid = MENU_ITEM_DATE;
      arg1.value.date.day = date.day;
      arg1.value.date.month = date.month;
      arg1.value.date.year = date.year;
    }
    if (MENU_ITEM_DATE != arg2.valid) {
      arg2.valid = MENU_ITEM_DATE;
      arg2.value.date.day = date.day;
      arg2.value.date.month = date.month;
      arg2.value.date.year = date.year;
    }
  }

  /* check that dates are in order */
  if ( (arg1.value.date.year > arg2.value.date.year) ||
       ( (arg1.value.date.year == arg2.value.date.year) &&
	 (arg1.value.date.month > arg2.value.date.month) ) ||
       ( (arg1.value.date.year == arg2.value.date.year) &&
	 (arg1.value.date.month == arg2.value.date.month) &&
	 (arg1.value.date.day > arg2.value.date.day) ) ) {
    LCD_ALERT(PSTR("Date Wrong Order"));
    return 0;
  }
  if ( !validDate(arg1.value.date) || !validDate(arg2.value.date) ) {
    LCD_ALERT(PSTR("Invalid Date"));
    return 0;
  }

  return ~0;
}

uint8_t
menuDelAllBill(uint8_t mode)
{
#if FF_ENABLE
  date_t date_i, date_l;

  if (RES_OK != disk_inserted()) {
    LCD_ALERT(PSTR_NO_SD);
    return MENU_RET_NOTAGAIN;
  }

  if (0 == menuCheckDateFromTo(mode))
    return 0;
  date_i=arg1.value.date, date_l=arg2.value.date;

  if (MENU_NOCONFIRM != (mode & ~MENU_MODEMASK)) {
    if (0 != menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_DELETE*MENU_PROMPT_LEN), MENU_PROMPT_LEN, 0))
      return 0;
  }

  menuMemset(&FS, 0, sizeof(FS));
  menuMemset(&Fil, 0, sizeof(Fil));
  f_mount(&FS, ".", 1);
  if (FR_OK != f_chdir("billdat")) {
    LCD_ALERT(PSTR("No Bills"));
    f_mount(NULL, "", 0);
    goto menuDelAllBillDelFfBills;
  }

  for (; ; nextDate(&date_i)) {
    /* check loop */
    if ( (date_i.year > date_l.year) ||
	 ((date_i.year == date_l.year) && (date_i.month > date_l.month)) ||
	 ((date_i.year == date_l.year) && (date_i.month == date_l.month) && (date_i.day > date_l.day)) )
      break;

    sprintf_P((char *)bufSS, PSTR("%02d-%02d-%04d.dat"), date_i.day, date_i.month, date_i.year);
    if (FR_OK != f_unlink((char *)bufSS)) {
      continue;
    }
    LCD_BUSY();
  }

  if (FR_OK != f_chdir("..")) {
    LCD_ALERT(PSTR("UpDir Error"));
  }
  f_mount(NULL, "", 0);
#endif

  struct sale *sl;
 menuDelAllBillDelFfBills:
  /* Delete today's bill also */
  sl = (void *) (bufSS + LCD_MAX_COL + LCD_MAX_COL);
  timerDateGet(date_i);
  if ((date_i.year == date_l.year) && (date_i.month == date_l.month) && (date_i.day == date_l.day)) {
    for (uint16_t ui16_1=0, ui16_2=NVF_SALE_START_ADDR; ui16_1<NVF_SALE_MAX_BILLS;
	 ui16_1++, ui16_2 = NVF_NEXT_SALE_RECORD(ui16_2)) {
      bill_read_bytes(ui16_2, (void *)sl, offsetof(struct sale, info));
      if (0xFFFF == (sl->crc_invert ^ sl->crc)) {
	sl->crc = (0 != sl->crc_invert) ? 0xFFFF : 0x0;
	bill_write_bytes(ui16_2, (void *)sl, offsetof(struct sale, info));
      }
    }
  }

  return MENU_RET_NOTAGAIN;
}

#if MENU_DIAG_FUNC
#define DIAG_FLASHMEM_SIZE   16
const uint8_t diagFlashMem[DIAG_FLASHMEM_SIZE] PROGMEM =
  { [ 0 ... (DIAG_FLASHMEM_SIZE - 1) ] = 0 };
#endif

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
  diagStatus |= (0 == menuGetYesNo((const uint8_t *)PSTR("Can see abcd?"), 13, 0)) ? DIAG_LCD : 0;

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
  for (ui16_1=0, ui16_2=NVF_SALE_END_ADDR, ui8_1=0;
       (ui16_1<ITEM_MAX) && (ui8_1<EEPROM_CHECK_MAX); ui16_1++) {
    ui16_2 = NVF_PREV_SALE_RECORD(ui16_2);
    item_read_bytes(ui16_2, bufSS, 4);
    if ((rand() & 0x8) &&
	(0xFFFF != (sl->crc_invert ^ sl->crc))) {
      ui8_1++;
      /* Write read check */
      srand(rand_seed);
      for (ui8_2=0; ui8_2<SIZEOF_SALE_EXCEP_ITEMS; ui8_2++)
	bufSS[ui8_2] = rand();
      item_write_bytes(ui16_2, bufSS, SIZEOF_SALE_EXCEP_ITEMS);
      item_read_bytes(ui16_2, bufSS+SIZEOF_SALE_EXCEP_ITEMS, SIZEOF_SALE_EXCEP_ITEMS);
      for (ui8_2=0; ui8_2<SIZEOF_SALE_EXCEP_ITEMS; ui8_2++)
	if (bufSS[ui8_2] != bufSS[SIZEOF_SALE_EXCEP_ITEMS+ui8_2])
	  break;
      diagStatus |= (ui8_2 >= SIZEOF_SALE_EXCEP_ITEMS) ? DIAG_MEM3 : 0;
    }
  }
#undef EEPROM_CHECK_MAX

  /* Test timer */
  LCD_CLRSCR;
  LCD_WR_NP((const char *)PSTR("Diagnosis Timer"), 15);
  _delay_ms(1000);
  {
    s_time_t time; date_t date;
    for (ui8_1=0; ui8_1<10; ui8_1++) {
      timerDateGet(date);
      LCD_CLRLINE(LCD_MAX_ROW-1);
      LCD_PUT_UINT(date.day);
      LCD_PUTCH(('/'));
      LCD_PUT_UINT(date.month);
      LCD_PUTCH(('/'));
      date.year %= 100;
      LCD_PUT_UINT(date.year);
      timerTimeGet(time);
      LCD_PUT_UINT(time.hour);
      LCD_PUTCH(':');
      LCD_PUT_UINT(time.min);
      LCD_PUTCH(':');
      LCD_PUT_UINT(time.sec);
      LCD_refresh();
      _delay_ms(500);
    }
    diagStatus |= (0 == menuGetYesNo((const uint8_t *)PSTR("Date/Time Corrt?"), 16, 0)) ? DIAG_TIMER : 0;
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
  diagStatus |= (0 == menuGetYesNo((const uint8_t *)PSTR("Did Keypad work?"), 16, 0)) ? DIAG_KEYPAD : 0;

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
  diagStatus |= (0 == menuGetYesNo((const uint8_t *)PSTR("Did Weigh m/c?"), 14, 0)) ? DIAG_WEIGHING_MC : 0;
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
  diagStatus |= (0 == menuGetYesNo((const uint8_t *)PSTR("Did Buzzer Buzz?"), 16, 0)) ? DIAG_BUZZER : 0;

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
  eeprom_read_block(bufSS, (uint8_t *)offsetof(struct ep_store_layout, ShopName), SHOP_NAME_SZ_MAX);
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
    arg2.value.str.sptr = bufSS+LCD_MAX_COL;
    arg2.value.str.len = LCD_MAX_COL;
    menuGetOpt(menu_prompt_str+(pgm_read_byte(menu_prompts+(menu_selected<<1)+1)*MENU_PROMPT_LEN), &arg2, pgm_read_byte(menu_args+(menu_selected<<1)+1), NULL);
    //move(0, 0); printw("arg1.valid:%x arg2.valid:%x", arg1.valid, arg2.valid);
    if ( (arg1.valid == (MENU_ITEM_TYPE_MASK&pgm_read_byte(menu_args+(menu_selected<<1)))) || (MENU_ITEM_OPTIONAL&pgm_read_byte(menu_args+(menu_selected<<1))) ) {
      if ( (arg2.valid == (MENU_ITEM_TYPE_MASK&pgm_read_byte(menu_args+(menu_selected<<1)+1))) || (MENU_ITEM_OPTIONAL&pgm_read_byte(menu_args+(menu_selected<<1)+1)) ) {
	menuRet |= MENU_RET_CALLED;
#if UNIT_TEST_MENU_1
	UNIT_TEST_MENU_1(menu_selected);
#else
	if (0 == (devStatus & DS_DEV_INVALID)) {
	  //printf("call 0x%x\n", pgm_read_dword(menu_handlers+menu_selected));
#if UNIT_TEST
	  menuRet |= (menu_handlers[menu_selected])(pgm_read_byte(menu_mode+menu_selected));
#else
	  menuRet |= ((menu_func_t)(uint16_t)pgm_read_dword((void *)(menu_handlers+menu_selected)))(pgm_read_byte(menu_mode+menu_selected));
#endif
	} else {
	  printf("devStatus:%x\n", devStatus);
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
#if UNIT_TEST
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

/* Set others passwd : only admin can do this
   arg1 : user name
   arg2 : passwd
 */
uint8_t
menuSetUserPasswd(uint8_t mode)
{
#if MENU_USER_ENABLE
  uint8_t ui8_1, ui8_2, ui8_3;
  uint16_t ui16_1;
  assert(MENU_MSUPER == MenuMode);

  /* where to replace it ? */
  for (ui8_2=0, ui16_1=offsetof(struct ep_store_layout, unused_users)+EPS_MAX_UNAME;
       ui8_2<(EPS_MAX_USERS*EPS_MAX_UNAME); ui8_2++, ui16_1++ ) {
    bufSS[(LCD_MAX_COL*3)+ui8_2] = eeprom_read_byte((uint8_t *)ui16_1);
  }
  ui8_2 = menuGetChoice((const uint8_t *)PSTR("Replace?"), bufSS+(LCD_MAX_COL*3), EPS_MAX_UNAME, EPS_MAX_USERS) + 1;
  if (0 != menuGetYesNo((const uint8_t *)menu_str1+(MENU_STR1_IDX_CONFI*MENU_PROMPT_LEN), MENU_PROMPT_LEN, 0)) {
    LCD_ALERT(PSTR("Aborting!"));
    return 0;
  }

  /* Check user name is unique before accepting */
  for (ui8_3=1; ui8_3<=EPS_MAX_USERS; ui8_3++) {
    if (ui8_2 == ui8_3) continue; /* skip choosen name */
    ui16_1 = offsetof(struct ep_store_layout, unused_users) + (((uint16_t)EPS_MAX_UNAME) * ui8_3);
    for (ui8_1=0; ui8_1<EPS_MAX_UNAME; ui8_1++) {
      if (arg1.value.str.sptr[ui8_1] != eeprom_read_byte((uint8_t *)ui16_1+ui8_1))
	break;
    }
    if (ui8_1>=EPS_MAX_UNAME) {
      LCD_ALERT(PSTR("Err:User Exists"));
      return 0;
    }
  }

  /* modify at will */
  for (ui8_3=0, ui16_1=offsetof(struct ep_store_layout, unused_users)+(ui8_2*EPS_MAX_UNAME);
       ui8_3<EPS_MAX_UNAME; ui8_3++, ui16_1++) {
    eeprom_update_byte((uint8_t *)(ui16_1), arg1.value.str.sptr[ui8_3]);
  }
  for (ui8_3=0, ui16_1=0; ui8_3<EPS_MAX_UNAME; ui8_3++) {
    ui16_1 = _crc16_update(ui16_1, arg2.value.str.sptr[ui8_3]);
  }
  eeprom_update_word((uint16_t *)(offsetof(struct ep_store_layout, unused_passwds) + (sizeof(uint16_t)*ui8_2)), ui16_1);
  LCD_ALERT(PSTR("Passwd Updated"));
#endif

  return MENU_RET_NOTAGAIN;
}

/* Set my password, arg1 is old passwd, arg2 is new passwd */
uint8_t
menuSetPasswd(uint8_t mode)
{
#if MENU_USER_ENABLE
  uint16_t crc_old = 0, crc_new = 0;
  uint8_t ui8_2, ui8_4;

  /* */
  assert(0 != LoginUserId);
  assert(LoginUserId <= (EPS_MAX_USERS/*+1*/));
  assert(MENU_MRESET != MenuMode);

  /* Compute CRC on old password, check */
  if (0 != (mode & (~MENU_MODEMASK) & MENU_MVALIDATE)) {
    assert(MENU_ITEM_STR == arg1.valid);
    for (ui8_4=0; ui8_4<arg1.value.str.len; ui8_4++) {
      ui8_2 = (ui8_4<arg1.value.str.len) ? arg1.value.str.sptr[ui8_4] : ' ';
      crc_old = _crc16_update(crc_old, ui8_2);
    }
    //printf(" crc_computed:0x%x\n", crc_old);

    if (eeprom_read_word((uint16_t *)offsetof(struct ep_store_layout, unused_passwds[(LoginUserId-1)])) != crc_old) {
      LCD_ALERT(PSTR("Passwd Wrong!!"));
      return 0;
    }
  }

  /* update mine only */
  assert(MENU_ITEM_STR == arg2.valid);
  for (ui8_4=0; ui8_4<arg2.value.str.len; ui8_4++) {
    ui8_2 = (ui8_4 < arg2.value.str.len) ? arg2.value.str.sptr[ui8_4] : ' ';
    crc_new = _crc16_update(crc_new, ui8_2);
  }
  //printf("updating crc:0x%x\n", crc_new);

  eeprom_update_word((uint16_t *)(offsetof(struct ep_store_layout, unused_passwds)+((LoginUserId-1)*sizeof(uint16_t))), crc_new);
  LCD_ALERT(PSTR("Passwd Updated"));
#endif

  return MENU_RET_NOTAGAIN;
}

/* Logout an user */
uint8_t
menuUserLogout(uint8_t mode)
{
#if MENU_USER_ENABLE
  if ( (mode & MENU_NOCONFIRM) ||
       (0 == menuGetYesNo((const uint8_t *)PSTR("Logout?"), 7, 0)) ) {
    LoginUserId = 0;
    MenuMode = MENU_MRESET;
  }
#endif
  return MENU_RET_NOTAGAIN;
}

/* Login an user */
uint8_t
menuUserLogin(uint8_t mode)
{
#if MENU_USER_ENABLE
  uint16_t crc = 0;
  uint8_t ui2, ui3, ui4;

  assert(MENU_ITEM_STR == arg1.valid);
  assert(MENU_ITEM_STR == arg2.valid);

  for ( ui2=0; ui2<=EPS_MAX_USERS; ui2++ ) {
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
  return 0;

 menuUserLogin_found:
  assert(ui2 <= EPS_MAX_USERS);
  for (ui4=0; ui4<EPS_MAX_UNAME; ui4++) {
    ui3 = arg2.value.str.sptr[ui4];
    /* check isprintable? */
    if (isprint(ui3)) {
      crc = _crc16_update(crc, ui3);
    }
  }

  if (eeprom_read_word((uint16_t *)(offsetof(struct ep_store_layout, unused_passwds) + (ui2*sizeof(uint16_t)))) != crc) {
#if 0
    printf("refcrc:%x crc:%x\n", eeprom_read_word((uint16_t *)(offsetof(struct ep_store_layout, unused_passwds) + (ui2*sizeof(uint16_t)))), crc);
#endif
    LCD_ALERT(PSTR("Wrong Passwd"));
    LoginUserId = 0;
    return 0;
  }

  /* */
  MenuMode = (0 == ui2) ? MENU_MSUPER : MENU_MNORMAL;
  LoginUserId = ui2;
  LCD_ALERT(PSTR("Logged In.."));
#endif

  return MENU_RET_NOTAGAIN;
}

static uint8_t
menuSdLoadItemHelper(uint8_t mode)
{
  static uint16_t id;
  struct item *it = (void *)(bufSS+LCD_MAX_COL+LCD_MAX_COL);
  uint8_t ret;

#if MENU_SDSAVE_EN && FF_ENABLE
  assert (RES_OK == disk_inserted());

  if (MENU_HELPER_INIT == (mode & MENU_MODEMASK)) {
    /* init */
    menuMemset(&FS, 0, sizeof(FS));
    menuMemset(&Fil, 0, sizeof(Fil));

    /* */
    f_mount(&FS, ".", 1);
    if (FR_OK != f_open(&Fil, SD_ITEM_FILE, FA_READ|FA_OPEN_EXISTING)) {
      LCD_ALERT(PSTR("File open error"));
      return 1;
    }
    id = 0;
    return MENU_RET_NOERROR;
  }

  if (MENU_HELPER_QUIT == (mode & MENU_MODEMASK)) {
    /* */
    f_close(&Fil);
    f_mount(NULL, "", 0);
    return MENU_RET_NOERROR;
  }

  UINT ret_size;
  uint32_t ui32_1;

  /* Detect EOF and send unique code */
  if (f_eof(&Fil)) return 0xFF;

  /* prod_code, name, cost, discount, has_vat, Vat, has_tax1, Tax1, has_tax2, Tax2, has_tax3, Tax3, is_reverse_tax, has_weighing_mc, has_common_discount */
  f_read(&Fil, it->prod_code, ITEM_PROD_CODE_BYTEL+1, &ret_size);
  if ('#' == it->prod_code[0]) return 4; /* comment line */
  if ((ITEM_PROD_CODE_BYTEL+1) != ret_size) return 5;
  if (',' != it->prod_code[ITEM_PROD_CODE_BYTEL]) return 2;
  if (0 != menuSdSCheckFix(it->prod_code, ITEM_PROD_CODE_BYTEL)) return 3;

  f_read(&Fil, it->name, ITEM_NAME_BYTEL+1, &ret_size);
  if ((ITEM_NAME_BYTEL+1) != ret_size) return 6;
  if (',' != it->name[ITEM_NAME_BYTEL]) return 7;
  if (0 != menuSdSCheckFix(it->name, ITEM_NAME_BYTEL)) return 8;

  ret = menuSdScanF(&ui32_1, bufSS); it->cost = ui32_1; ret |= menuSdSwallowComma();
  ret |= menuSdScanF(&ui32_1, bufSS); it->discount = ui32_1; ret |= menuSdSwallowComma();
  ret |= menuSdScanD(&ui32_1, bufSS); it->has_vat = ui32_1; ret |= menuSdSwallowComma();
  ret |= menuSdScanF(&ui32_1, bufSS); it->Vat = ui32_1; ret |= menuSdSwallowComma();
  ret |= menuSdScanD(&ui32_1, bufSS); it->has_tax1 = ui32_1; ret |= menuSdSwallowComma();
  ret |= menuSdScanF(&ui32_1, bufSS); it->Tax1 = ui32_1; ret |= menuSdSwallowComma();
  ret |= menuSdScanD(&ui32_1, bufSS); it->has_tax2 = ui32_1; ret |= menuSdSwallowComma();
  ret |= menuSdScanF(&ui32_1, bufSS); it->Tax2 = ui32_1; ret |= menuSdSwallowComma();
  ret |= menuSdScanD(&ui32_1, bufSS); it->has_tax3 = ui32_1; ret |= menuSdSwallowComma();
  ret |= menuSdScanF(&ui32_1, bufSS); it->Tax3 = ui32_1; ret |= menuSdSwallowComma();
  ret |= menuSdScanD(&ui32_1, bufSS); it->is_reverse_tax = ui32_1; ret |= menuSdSwallowComma();
  ret |= menuSdScanD(&ui32_1, bufSS); it->has_weighing_mc = ui32_1; ret |= menuSdSwallowComma();
  ret |= menuSdScanD(&ui32_1, bufSS); it->has_common_discount = ui32_1;

  /* only when no errors */
  it->id = (0 == ret) ? ++id : 0;
#endif

  return (0 == ret) ? MENU_RET_NOERROR : 0xF;
}

#if UNIT_TEST
static int
f_puts_p (const uint8_t* str, FIL* fp)
{
  int len;

  for (len=0; ;len++) {
    uint8_t c = str[0];
    if (0 == c) break;
    f_putc(c, fp);
    str++;
  }

  return len;
}
#else
static int
f_puts_p (const uint8_t* str, FIL* fp)
{
  putbuff pb;

  putc_init(&pb, fp);
  do {
    uint8_t c = pgm_read_byte(str);
    if (0 == c) break;
    putc_bfd(&pb, c);
    str++;
  } while (1);
  return putc_flush(&pb);
}
#endif

static uint8_t
menuSdSaveItemHelper(uint8_t mode)
{
  struct item *it = (void *)(bufSS+LCD_MAX_COL+LCD_MAX_COL);

#if MENU_SDSAVE_EN && FF_ENABLE
  assert (RES_OK == disk_inserted());

  if (MENU_HELPER_INIT == (mode & MENU_MODEMASK)) {
    /* init */
    menuMemset(&FS, 0, sizeof(FS));
    menuMemset(&Fil, 0, sizeof(Fil));

    /* */
    f_mount(&FS, ".", 1);
    if (FR_OK != f_open(&Fil, "out_" SD_ITEM_FILE, FA_WRITE|FA_CREATE_ALWAYS)) {
      LCD_ALERT(PSTR("File open error"));
      return 1;
    }
    f_puts_p(PSTR("## prod_code, name, cost, discount, has_vat, Vat, has_tax1, Tax1, has_tax2, Tax2, has_tax3, Tax3, is_reverse_tax, has_weighing_mc, has_common_discount"), &Fil); f_putc('\r', &Fil); f_putc('\n', &Fil);
    return 0;
  } else if (MENU_HELPER_QUIT == (mode & MENU_MODEMASK)) {
    /* */
    f_close(&Fil);
    f_mount(NULL, "", 0);
    return 0;
  } else {
    /* format :
       prod_code, name, cost, discount, has_vat, Vat, has_tax1, Tax1, has_tax2, Tax2, has_tax3, Tax3, is_reverse_tax, has_weighing_mc, has_common_discount
    */
    UINT  ret_size;
    f_write(&Fil, it->prod_code, ITEM_PROD_CODE_BYTEL, &ret_size); f_putc(',', &Fil);
    assert(ITEM_PROD_CODE_BYTEL == ret_size);
    f_write(&Fil, it->name, ITEM_NAME_BYTEL, &ret_size); f_putc(',', &Fil);
    assert(ITEM_NAME_BYTEL == ret_size);
    menuSdF(it->cost); f_putc(',', &Fil);
    menuSdF(it->discount); f_putc(',', &Fil);
    menuSdD(it->has_vat); f_putc(',', &Fil);
    menuSdF(it->Vat); f_putc(',', &Fil);
    menuSdD(it->has_tax1); f_putc(',', &Fil);
    menuSdF(it->Tax1); f_putc(',', &Fil);
    menuSdD(it->has_tax2); f_putc(',', &Fil);
    menuSdF(it->Tax2); f_putc(',', &Fil);
    menuSdD(it->has_tax3); f_putc(',', &Fil);
    menuSdF(it->Tax3); f_putc(',', &Fil);
    menuSdD(it->is_reverse_tax); f_putc(',', &Fil);
    menuSdD(it->has_weighing_mc); f_putc(',', &Fil);
    menuSdD(it->has_common_discount);
    f_putc('\r', &Fil); f_putc('\n', &Fil);
  }
#endif
  return 0;
}

static uint8_t
menuSdReportItemHelper(uint8_t mode)
{
  struct item *it = (void *)(bufSS+LCD_MAX_COL+LCD_MAX_COL);

  if (MENU_HELPER_INIT == (mode & MENU_MODEMASK)) {
    PRINTER_PSTR(PSTR("  id     name     prod_code    Rs.   Disc.   Vat  Tax1, Tax2, Tax3\r\n"));
    return 0;
  }
  if (MENU_HELPER_QUIT == (mode & MENU_MODEMASK)) {
    return 0;
  }

  /* body */
  menuPrnD(it->id);
  PRINTER_PRINT(' ');
  for (uint8_t ui8_1=0; ui8_1<ITEM_NAME_BYTEL; ui8_1++) {
    PRINTER_PRINT(it->name[ui8_1]);
  }
  PRINTER_PRINT(' ');
  for (uint8_t ui8_1=0; ui8_1<ITEM_PROD_CODE_BYTEL; ui8_1++) {
    PRINTER_PRINT(it->prod_code[ui8_1]);
  }
  PRINTER_PRINT(' ');
  menuPrnF(it->cost);
  PRINTER_PRINT(' ');
  menuPrnF(it->discount);
  PRINTER_PRINT(' ');
  menuPrnF(it->Vat);
  PRINTER_PRINT(' ');
  menuPrnF(it->Tax1);
  PRINTER_PRINT(' ');
  menuPrnF(it->Tax2);
  PRINTER_PRINT(' ');
  menuPrnF(it->Tax3);
  PRINTER_PRINT('\r'); PRINTER_PRINT('\n');

  return 0;
}

uint8_t
menuSdIterItem(uint8_t mode)
{
  uint16_t ui16_1, error = 0;
  uint8_t ui8_1 = 0;
  struct item *it = (void *)(bufSS+LCD_MAX_COL+LCD_MAX_COL);


  if (RES_OK != disk_inserted()) {
    LCD_ALERT(PSTR_SKIPOP);
    return DS_DEV_INVALID;
  }

  /* */
  if (MENU_ITEM_SAVE & (mode & ~MENU_MODEMASK)) {
    ui8_1 = menuSdSaveItemHelper(MENU_HELPER_INIT);
  } else if (MENU_ITEM_LOAD & (mode & ~MENU_MODEMASK)) {
    ui8_1 = menuSdLoadItemHelper(MENU_HELPER_INIT);
  } else if (MENU_ITEM_REPORT & (mode & ~MENU_MODEMASK)) {
    ui8_1 = menuSdReportItemHelper(MENU_HELPER_INIT);
  }
  ui8_1 ? error++ : 0;

  /* delete all items for load */
  if (MENU_ITEM_LOAD & (mode & ~MENU_MODEMASK)) {
    for (ui16_1=0; (0 == ui8_1) && (ui16_1 < ITEM_MAX_ADDR);
	 ui16_1+=(ITEM_SIZEOF>>EEPROM_ADDR_SHIFT)) {
      item_write_bytes(ui16_1+(ITEM_SIZEOF>>EEPROM_ADDR_SHIFT)-1, NULL, 4);
    }
  }

  /* */
  for (ui16_1=0; (0 == ui8_1) && (ui16_1 < ITEM_MAX_ADDR);
       ui16_1+=(ITEM_SIZEOF>>EEPROM_ADDR_SHIFT)) {
    if (MENU_ITEM_LOAD & (mode & ~MENU_MODEMASK)) {
      ui8_1 = menuSdLoadItemHelper(mode);
      if (0xFF == ui8_1) { /* EOF */
	break;
      } else if ((0 == ui8_1 /* no err */) &&
	  (it->id > 0) && (it->id <= ITEM_MAX)) {
	ui16_1 = itemAddr(it->id);
	item_write_bytes(ui16_1, (void *)it, ITEM_SIZEOF);
      } else if (4 == ui8_1) { /* comment line */
	ui8_1 = 0;
      } else if (0 != ui8_1) {
	//printf("ui8_1:%d\n", ui8_1);
	error++;
      }

      ui8_1 = 0; /* ignore errors during load */
      menuSdSwallowUntilNL();
      continue;
    }
    item_read_bytes(ui16_1, (void *)it, ITEM_SIZEOF);
    if (0xFF != (it->unused_crc ^ it->unused_crc_invert))
      continue;
    if (MENU_ITEM_SAVE & (mode & ~MENU_MODEMASK)) {
      ui8_1 = menuSdSaveItemHelper(mode);
    } else if (MENU_ITEM_REPORT & (mode & ~MENU_MODEMASK)) {
      ui8_1 = menuSdReportItemHelper(mode);
    } else {
      assert(0);
    }
    LCD_BUSY();
  }

  /* when all goes well */
  if (MENU_ITEM_SAVE & (mode & ~MENU_MODEMASK)) {
    ui8_1 = menuSdSaveItemHelper(MENU_HELPER_QUIT);
    LCD_ALERT(PSTR("Item Saved.."));
  } else if (MENU_ITEM_LOAD & (mode & ~MENU_MODEMASK)) {
    ui8_1 = menuSdLoadItemHelper(MENU_HELPER_QUIT);
    LCD_ALERT(PSTR("Item Loaded.."));
  } else if (MENU_ITEM_REPORT & (mode & ~MENU_MODEMASK)) {
    ui8_1 = menuSdReportItemHelper(MENU_HELPER_QUIT);
  }
  ui8_1 ? error++ : 0;

  /* */
  if (error) {
    LCD_CLRLINE(1);
    LCD_WR_P(PSTR("#Errors:"));
    LCD_PUT_UINT(error);
    KBD_GETCH;
  }

  return MENU_RET_NOERROR;
}

typedef struct {
  uint16_t num_bills;
  uint16_t n_items;
  uint32_t t_tax;
  uint32_t t_discount;
  uint32_t total;
  uint32_t t_cash_pay;
  date_t   date;
  uint8_t  has_data;
} cumuBillData_t;
//static cumuBillData_t bwiseCumu;

static uint8_t
menuBillRpt1L(uint8_t mode, struct sale *sl)
{
  uint8_t twise = mode & MENU_REPO_WISEMASK;
  uint8_t tbill = mode & (~MENU_MODEMASK & ~MENU_REPO_WISEMASK);
  cumuBillData_t *cumu = (void *)bufSS;

  if (MENU_HELPER_INIT == (mode & MENU_MODEMASK)) {
    PRINTER_PSTR(PSTR("Date  User  BillId  #Items  #Tax #Discount #Total\r\n"));
    menuMemset( bufSS, 0, sizeof(cumuBillData_t) );
    assert(sizeof(cumuBillData_t) <= (LCD_MAX_COL+LCD_MAX_COL));
    return 0;
  }

  /* skip bills of no interest */
  if (MENU_HELPER_QUIT != (mode & MENU_MODEMASK)) {
    if ( (MENU_REPO_VOID == tbill) && (!(sl->info.is_void)) ) {
      return 0;
    } else if ( (MENU_REPO_DUP == tbill) && (!(sl->info.dup_bill_issued)) ) {
      return 0;
    } else if (sl->info.is_void) { /* in MENU_REPO_VALID, MENU_REPO_TAX */
      return 0;
    }

    /* Tally has to acumulate and print once */
    if (MENU_REPO_TALLY == tbill) {
      goto menuBillRpt1LSkipPrint;
    }
  }

  /* print bills only on condition
     MENU_REPO_DAYWISE    : Day wise bill summary
     MENU_REPO_MONWISE    : Month wise bill summary
     MENU_REPO_YEARWISE   : Yearly wise bill summary
     else : Bill-wise
   */
  if (MENU_REPO_BWISE == twise) { /* bill-wise */
    cumu->date.day = sl->info.date_dd;
    cumu->date.month = sl->info.date_mm;
    cumu->date.year = sl->info.date_yy;
    cumu->num_bills = 1;
    cumu->n_items = sl->info.n_items;
    cumu->t_tax = sl->t_tax1 + sl->t_tax2 + sl->t_tax3 + sl->t_vat;
    cumu->t_discount = sl->t_discount;
    cumu->total = sl->total;
    cumu->has_data = 1;
 } else if (MENU_REPO_DAYWISE == twise) {
    if ( sl && (cumu->date.day == sl->info.date_dd) &&
	 (cumu->date.month == sl->info.date_mm) &&
	 (cumu->date.year == sl->info.date_yy) ) {
      goto menuBillRpt1LSkipPrint;
    }
  } else if (MENU_REPO_MONWISE == twise) {
    if ( sl && (cumu->date.month == sl->info.date_mm) &&
	 (cumu->date.year == sl->info.date_yy) ) {
      goto menuBillRpt1LSkipPrint;
    }
  } else if (MENU_REPO_YEARWISE == twise) {
    if (sl && (cumu->date.year == sl->info.date_yy) ) {
      goto menuBillRpt1LSkipPrint;
    }
  }

  /* print the bill date */
  if (!(cumu->has_data)) goto menuBillRpt1LSkipPrint;
  if (twise == MENU_REPO_DAYWISE) {
    menuPrnD(cumu->date.day); PRINTER_PRINT('/');
  }
  if ((twise == MENU_REPO_DAYWISE) ||
      (twise == MENU_REPO_MONWISE)) {
    menuPrnD(cumu->date.month); PRINTER_PRINT('/');
  }
  menuPrnD(cumu->date.year); PRINTER_PRINT(' ');
  if (twise != MENU_REPO_BWISE) {
    menuPrnD(cumu->num_bills);
  } else {
    menuPrnD(sl->info.id);
  }
  PRINTER_PRINT(' ');

  if (MENU_REPO_TALLY == tbill) {
    PRINTER_PSTR(PSTR("#Bills   #TotTax   #TotDisc   #TotCashCollected\r\n"));
  }

  /* */
  menuPrnF(cumu->t_tax);  PRINTER_PRINT(' ');
  menuPrnF(cumu->t_discount);  PRINTER_PRINT(' ');
  menuPrnF(cumu->total);

  /* finish of record */
  PRINTER_PRINT('\r');  PRINTER_PRINT('\n');

  /* tally cash handled seperately */
  if (MENU_REPO_TALLY == tbill) {
    LCD_CLRLINE(0);
    LCD_PUTCH('#');
    LCD_PUT_UINT(cumu->num_bills);
    LCD_WR_P(PSTR(" Tx:"));
    LCD_PUT_UINT(cumu->t_tax);
    LCD_CLRLINE(LCD_MAX_ROW-1);
    LCD_WR_P(PSTR("Cash: "));
    LCD_PUT_UINT(cumu->t_cash_pay);
#if ! UNIT_TEST
    KBD_RESET_KEY;
    KBD_GETCH;
#endif
  }

  /* cleanup data */
  cumu->date.day = 0; cumu->date.month = 0; cumu->date.year = 0;
  cumu->num_bills = cumu->n_items = 0;
  cumu->t_tax = cumu->t_discount = cumu->total = 0;
  cumu->has_data = 0;

 menuBillRpt1LSkipPrint:
  if (MENU_HELPER_QUIT != (mode & MENU_MODEMASK)) {
    cumu->date.day = sl->info.date_dd;
    cumu->date.month = sl->info.date_mm;
    cumu->date.year = sl->info.date_yy;
    cumu->has_data = 1;

    (cumu->num_bills)++;
    cumu->n_items += sl->info.n_items;
    cumu->t_tax += sl->t_tax1 + sl->t_tax2 + sl->t_tax3 + sl->t_vat;
    cumu->t_discount += sl->t_discount;
    cumu->total += sl->total;
    cumu->t_cash_pay += sl->t_cash_pay;

    //printf("cumu: #%d tax:%d disc:%d total:%d\n", cumu->n_items, cumu->t_tax, cumu->t_discount, cumu->total);
    return 0;
  }

  /* Quit : So, print the final statements */
  if (cumu->has_data) {
    PRINTER_PSTR(PSTR("                "));
    menuPrnF(cumu->t_tax); PRINTER_PRINT(' ');
    menuPrnF(cumu->t_discount); PRINTER_PRINT(' ');
    menuPrnF(cumu->total);
  }
  if (MENU_REPO_TALLY == tbill) {
    LCD_CLRLINE(LCD_MAX_ROW-1);
    LCD_WR_P(PSTR("Cash: "));
    LCD_PUT_UINT(cumu->t_cash_pay);
  }

  /* end spaces */
  PRINTER_PRINT('\r'); PRINTER_PRINT('\n');
  PRINTER_PRINT('\r'); PRINTER_PRINT('\n');

  return 0;
}

// not unit tested
static uint8_t
menuItemWiseRpt(uint8_t mode, struct sale *sl)
{
  if (MENU_HELPER_INIT == mode) {
    PRINTER_PSTR(PSTR("Item Wise Report"));
    return 0;
  }
  if (MENU_HELPER_QUIT == mode) {
    return 0;
  }

  /* FIXME: not yet coded */
  assert(0);
  return 0;
}

const uint8_t    wiseStr[] PROGMEM = "Bill Wise Day Wise  Month WiseYear Wise ";
#define WISE_STR_OPT_LEN 10
uint8_t
menuBillReports(uint8_t mode)
{
  uint8_t ui8_1;
  date_t date, date_i, date_l;
  struct sale *sl = (void *)(bufSS+LCD_MAX_COL+LCD_MAX_COL);
  uint8_t rpt;

  /* */
  assert(sizeof(cumuBillData_t) < (LCD_MAX_COL+LCD_MAX_COL));

  /* */
  strcpy_P((char *)bufSS+(LCD_MAX_COL*2), (char *)wiseStr);
  rpt = menuGetChoice((const uint8_t *)PSTR("Collate:"), bufSS+(LCD_MAX_COL*2), WISE_STR_OPT_LEN, 4) & 0x3;
  rpt <<= MENU_REPO_WISE_LSB;
  rpt |= mode & (~MENU_MODEMASK & ~MENU_REPO_WISEMASK);

  /* check and fix from/to date */
  if (0 == menuCheckDateFromTo(mode))
    return 0;
  date=arg1.value.date, date_l=arg2.value.date;

  /* */
  menuPrnHeader();
  PRINTER_PRINT('\r'); PRINTER_PRINT('\n');
  PRINTER_JUSTIFY(PRINTER_JCENTER);
  PRINTER_FONT_ENLARGE(2);
  PRINTER_PRINTN((bufSS+(LCD_MAX_COL*2)), WISE_STR_OPT_LEN);
  if (rpt == MENU_REPO_BWISE) {
    PRINTER_PSTR(PSTR("All Bill Report\r\n"));
  } else if (rpt == MENU_REPO_VOID) {
    PRINTER_PSTR(PSTR("Void Bill Report\r\n"));
  } else if (rpt == MENU_REPO_DUP) {
    PRINTER_PSTR(PSTR("Duplicate Bill Report\r\n"));
  } else if (rpt == MENU_REPO_ITWISE) {
    PRINTER_PSTR(PSTR("Itemwise Bill Report\r\n"));
  } else if (rpt == MENU_REPO_TALLY) {
    PRINTER_PSTR(PSTR("Tally Cash Report\r\n"));
  } else {
    PRINTER_PSTR(PSTR("Tax Report\r\n"));
  }
  ((rpt == MENU_REPO_ITWISE) ? menuItemWiseRpt: menuBillRpt1L)(MENU_HELPER_INIT|rpt, NULL);
  PRINTER_FONT_ENLARGE(1);

  /* */
 menuBillReportsRedo:
#if FF_ENABLE
  if (RES_OK != disk_inserted()) {
    goto menuBillReportsSkipFf;
  }
  menuMemset(&FS, 0, sizeof(FS));
  menuMemset(&Fil, 0, sizeof(Fil));
  f_mount(&FS, ".", 1);
  if (FR_OK != f_chdir("billdat")) {
    goto sdBillsDone;
  }

  for (date_i=date; ; nextDate(&date_i)) {
    /* check loop */
    if ( (date_i.year > date_l.year) ||
	 ((date_i.year == date_l.year) && (date_i.month > date_l.month)) ||
	 ((date_i.year == date_l.year) && (date_i.month == date_l.month) && (date_i.day > date_l.day)) ) {
      break;
    }

    sprintf_P((char *)bufSS, PSTR("%02d-%02d-%04d.dat"), date_i.day, date_i.month, date_i.year);
    if (FR_OK != f_open(&Fil, (char *)bufSS, FA_READ)) {
      continue;
    }

    /* iterate records */
    while (!f_eof(&Fil)) {
      /* Display this item */
      UINT ret_val;
      f_read(&Fil, (void *)sl, SIZEOF_SALE_EXCEP_ITEMS, &ret_val);
      if (SIZEOF_SALE_EXCEP_ITEMS != ret_val) break;
      assert (0xFFFF == (sl->crc_invert ^ sl->crc));

      /* */
      ((rpt == MENU_REPO_ITWISE) ? menuItemWiseRpt: menuBillRpt1L)(rpt, sl);

      /* */
      f_read(&Fil, (void *)sl, MAX_SIZEOF_1BILL-SIZEOF_SALE_EXCEP_ITEMS, &ret_val);
      if ((MAX_SIZEOF_1BILL-SIZEOF_SALE_EXCEP_ITEMS) != ret_val) break;
    }
    f_close(&Fil);
  }

 sdBillsDone:
  /* */
  if (FR_OK != f_chdir("..")) {
    LCD_ALERT(PSTR("UpDir Error"));
  }
  f_mount(NULL, "", 0);
#endif

 menuBillReportsSkipFf:
  /* Look if we need to iterate on today's records */
  timerDateGet(date_i);
  if ( (date_i.day != date_l.day) ||
       (date_i.month != date_l.month) ||
       (date_i.year != date_l.year) )
    goto menuBillReportsFlashBillsDone;
  uint16_t ui16_1, ui16_2, ui16_3;
  ui16_2 = eeprom_read_word((uint16_t *)(offsetof(struct ep_store_layout, unused_todayStartAddr)));
  ui16_3 = eeprom_read_word((uint16_t *)(offsetof(struct ep_store_layout, unused_nextBillAddr)));
  for (ui16_1=0; (ui16_1<NVF_SALE_MAX_BILLS) && (ui16_2 != ui16_3);
       ui16_1++, ui16_2 = NVF_NEXT_SALE_RECORD(ui16_2)) {
    bill_read_bytes(ui16_2, (void *)sl, SIZEOF_SALE_EXCEP_ITEMS);
    assert (0xFFFF == (sl->crc_invert ^ sl->crc));
    /* */
    ((rpt == MENU_REPO_ITWISE) ? menuItemWiseRpt: menuBillRpt1L)(rpt, sl);
  }

 menuBillReportsFlashBillsDone:
  ui8_1 = (((rpt&MENU_REPO_WISEMASK) == MENU_REPO_ITWISE) ? menuItemWiseRpt: menuBillRpt1L)(MENU_HELPER_QUIT|rpt, NULL);
  if (MENU_REPO_REDO == ui8_1)
    goto menuBillReportsRedo;
  menuPrnFooter();

  return MENU_RET_NOTAGAIN;
}

uint8_t
menuUpdateFirmware(uint8_t mode)
{
  /* FIXME: not coded */
  return MENU_RET_NOTAGAIN;
}
