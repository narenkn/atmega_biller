#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/crc16.h>
#include <avr/eeprom.h>
#include <util/twi.h>

#include "lcd.c"
#include "kbd.c"

volatile uint8_t eeprom_setting0, eeprom_setting1;
static menu_arg_t arg1, arg2;
uint8_t    menu_error;
uint8_t bufSS[BUFSS_SIZE];

/* Helper routine to obtain input from user */
void
menuGetOpt(const uint8_t *prompt, menu_arg_t *arg, uint8_t opt, menuGetOptHelper helper)
{
  uint8_t item_type = (opt & MENU_ITEM_TYPE_MASK);
  uint32_t val = 0;
  uint8_t ui8_1, ui8_2;
  uint8_t sbuf[LCD_MAX_COL], *buf, buf_idx;

  if (MENU_ITEM_NONE == opt) return;

  /* init */
  if (MENU_ITEM_STR == item_type) {
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
    LCD_WR_NP((const char *)prompt, MENU_PROMPT_LEN);
    LCD_PUTCH('?');
    for (ui8_1=MENU_PROMPT_LEN+1,
	   ui8_2=((LCD_MAX_COL-MENU_PROMPT_LEN-1)<=buf_idx) ? 0 : buf_idx-MENU_PROMPT_LEN-2;
	 (ui8_1<LCD_MAX_COL) && (ui8_2<buf_idx); ui8_1++) {
      ui8_2++;
      LCD_PUTCH(buf[ui8_2]);
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
    LCD_WR_NP((const char *)quest, size);
    LCD_PUTCH(':');
    LCD_WR_P((const char *)menu_str2+((ret)*3));
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

int
main()
{
  LCD_init();
  KbdInit();

  DDRD |= 0x10 ; LCD_bl_on;

  sei();

  LCD_WR_NP((const char *)PSTR("Hello World 7"), 13);
  _delay_ms(1000);
  LCD_CLRLINE(1);
  LCD_WR_NP((const char *)PSTR("Hello World 8"), 13);
  _delay_ms(1000);

  arg1.value.str.sptr = bufSS;
  arg1.value.str.len = 5;
  menuGetOpt((const uint8_t *)PSTR("What 5"), &arg1, MENU_ITEM_STR, NULL);
  LCD_CLRSCR;
  LCD_WR_N((uint8_t *)arg1.value.str.sptr, 5);
  KBD_GETCH;

  arg1.value.str.sptr = bufSS;
  arg1.value.str.len = 8;
  menuGetOpt((const uint8_t *)PSTR("What 8"), &arg1, MENU_ITEM_STR, NULL);
  LCD_CLRSCR;
  LCD_WR_N((uint8_t *)arg1.value.str.sptr, 8);
  KBD_GETCH;

  arg1.value.str.sptr = bufSS;
  arg1.value.str.len = 10;
  menuGetOpt((const uint8_t *)PSTR("What 10"), &arg1, MENU_ITEM_STR, NULL);
  LCD_CLRSCR;
  LCD_WR_N((uint8_t *)arg1.value.str.sptr, 10);
  KBD_GETCH;

  arg1.value.str.sptr = bufSS;
  arg1.value.str.len = 20;
  menuGetOpt((const uint8_t *)PSTR("What 20"), &arg1, MENU_ITEM_STR, NULL);
  LCD_CLRLINE(0);
  LCD_WR_N((uint8_t *)arg1.value.str.sptr, LCD_MAX_COL);
  LCD_CLRLINE(LCD_MAX_ROW-1);
  LCD_WR_N((uint8_t *)arg1.value.str.sptr+LCD_MAX_COL, 20-LCD_MAX_COL);
  KBD_GETCH;

  while (1) {}

  return 0;
}
