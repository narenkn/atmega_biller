#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/crc16.h>
#include <avr/eeprom.h>
#include <util/twi.h>
#include <avr/sleep.h>
#include <avr/wdt.h>

#include "ep_ds.h"
#include "version.h"
#include "assert.h"
#include "lcd.h"
#include "kbd.h"
#include "ep_store.h"
#include "billing.h"
#include "i2c.h"
#if NVFLASH_EN
#include "flash.h"
#endif
#include "uart.h"
#include "a1micro2mm.h"
#if FF_ENABLE
#include "diskio.h"
#include "ff.h"
#endif
#include "main.h"
#include "menu.h"

volatile uint8_t eeprom_setting0;

void
main_init(void)
{
  /* setup timer 2 : need to get 10ms callbacks
     # cycles to skip : (F_CPU/100) : 
     # clock div is 1024, so we need to skip : (F_CPU>>10)/100
     for 8MHz : 
   */
  TCCR2 = (0<<FOC2)|(0<<WGM20)|(0<<COM21)|(0<<COM20)|(1<<WGM21)|(1<<CS22)|(0<<CS21)|(1<<CS20);

  // Reset time
  TCNT2 = 0;

  /* 8MHz - is divided by prescalar 1024 :
       we get 7812 pulses / second
       to get 10ms pulse : 7812 / 100 = 78
       so, OCR2 = 78-1;
   */
  OCR2  = 77;

  // Enable interrupt on compare match
  TIMSK |= (1<<OCIE2);

  /* For Buzzer */
  DDRA |= 0x80;
  eeprom_setting0 = 0;
  /*uint8_t ui8_1 = eeprom_read_byte((uint8_t *)offsetof(struct ep_store_layout, key_buzz));
  if (ui8_1) EEPROM_SETTING0_OFF(BUZZER);
  else */EEPROM_SETTING0_ON(BUZZER);
}

/* is called every 10ms
   to get 10 sec : 10/10m = 1000
 */
volatile uint16_t timer2_beats=0;
ISR(TIMER2_COMP_vect)
{
  timer2_beats++;

  /* FF */
  disk_timerproc();
}

#ifndef NO_MAIN

int
main(void)
{
  /* select power down option */
#if defined (__AVR_ATmega32__)
#elif defined (__AVR_ATxmega64A1__) || defined (__AVR_ATxmega64A3__) || defined (__AVR_ATxmega64D3__)
  #error "Unknown device"
#elif defined (__AVR_ATxmega128A1__) || defined (__AVR_ATxmega128A3__) || defined (__AVR_ATxmega128D3__) || defined (__AVR_ATmega1284P__)
  PRR0 = 0xFF;
  PRR1 = 0xFF;
#elif defined (__AVR_ATxmega256A3__) || defined (__AVR_ATxmega256A3B__) || defined (__AVR_ATxmega256D3__)
  #error "Unknown device"
#endif

  /* */
  wdt_disable();

  LCD_init();

  /* Welcome screen */
  LCD_bl_on;
  LCD_CLRSCR;
  LCD_WR_NP((const char *)PSTR("Welcome..."), 10);
  LCD_CLRLINE(LCD_MAX_ROW-1);
  LCD_WR_NP((const char *)PSTR("  Initializing.."), 16);
  LCD_refresh();
  _delay_ms(500);

  /* All other devices */
  KbdInit();
  ep_store_init();
  i2c_init();
#if NVFLASH_EN
  nvfInit();
#endif
  uartInit();
  printerInit();
  main_init();
  billingInit();
  menuInit();

  /* Check if all devices are ready to go, else give
     error and exit */
  if (0 == (devStatus&DS_DEV_ERROR)) {
    sei();   /* Enable Global Interrupts */

    LCD_cmd(LCD_CMD_CUR_20);
    LCD_WR_NP((const char *)PSTR("   Initialized!"), 15);
    LCD_refresh();
    _delay_ms(1000);
    menuMain();
  } else if (devStatus & DS_DEV_INVALID) {
    LCD_CLRLINE(0);
    LCD_WR_P((void *)PSTR("Invalid Prod Key"));
    LCD_refresh();
    _delay_ms(5000);
  } else {
    /* hardware issues found ....
     FIXME: ASK and run factory tests */
    LCD_CLRLINE(0);
    LCD_WR_NP((const char *)PSTR("Device Error"), 12);
    LCD_refresh();
    _delay_ms(5000);
  }

  /* reach here and you could never get out */
  LCD_CLRLINE(LCD_MAX_ROW-1);
  LCD_WR_NP((const char *)PSTR("Power Off Now"), 13);
  LCD_refresh();
  while (1) {
    KBD_GETCH;
  }

  return 0;
}

#endif

uint8_t
validDate(date_t date)
{
  uint8_t max_days_in_month;

  if (1 == date.month) {
    max_days_in_month = ((0 == (date.year%4)) && (0 != (date.year%100)))? 29 : 28;
  } else if ( (0 == date.month) || (2 == date.month) ||
	      (4 == date.month) || (6 == date.month) ||
	      (7 == date.month) || (9 == date.month) ||
	      (11 == date.month) )
    max_days_in_month = 31;
  else
    max_days_in_month = 30;

  return (date.month<=11) && (date.day < max_days_in_month);
}

void
nextDate(date_t *date)
{
  uint8_t max_days_in_month;

  /* next date */
  (date->day)++;

  /* find max time, keep date */
  if (1 == date->month) {
    max_days_in_month = ((0 == (date->year & 0x3)) && (0 != (date->year%100)))? 29 : 28;
  } else if ( (0 == date->month) || (2 == date->month) ||
	      (4 == date->month) || (6 == date->month) ||
	      (7 == date->month) || (9 == date->month) ||
	      (11 == date->month) )
    max_days_in_month = 31;
  else
    max_days_in_month = 30;
  if (date->day >= max_days_in_month) {
    date->day = 0;
    if (++(date->month) >= 12) {
      date->month = 0;
      (date->year)++;
    }
  }
}

/*
** Translation Table to decode (created by author)
*/
static const uint8_t cd64[] PROGMEM =
  "|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

/*
** decodeblock
**
** decode 4 '6-bit' characters into 3 8-bit binary bytes
*/
static void
b64decode( uint8_t *in, uint8_t *out )
{   
  out[ 0 ] = (in[0] << 2 | in[1] >> 4);
  out[ 1 ] = (in[1] << 4 | in[2] >> 2);
  out[ 2 ] = (((in[2] << 6) & 0xc0) | in[3]);
}

/*
** decode
**
** decode a base64 encoded stream discarding padding, line breaks and noise
*/
static void
decode( uint8_t *in, uint8_t *out )
{
  uint8_t len, in_idx, out_idx;
  uint8_t ui1, v;

  for (len=0, in_idx=out_idx=0; len<12;
       len+=3, in_idx+=4, out_idx+=3) {
    for (ui1=0; ui1<4; ui1++) {
      v = in[in_idx+ui1];
      v = (v < 43 || v > 122) ? (uint8_t) 0 :
	pgm_read_byte (cd64+v-43);
      if (v != 0) {
	v = ((v == (uint8_t)'$') ? 0 : v - 61);
	in[in_idx+ui1] = (uint8_t) (v - 1);
      } else {
	in[in_idx+ui1] = (uint8_t) 0;
      }
    }

    b64decode (in+in_idx, out+out_idx);
  }
}
