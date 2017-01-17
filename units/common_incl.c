#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#include <assert.c>

#include "device.h"

#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/boot.h>
#include <util/crc16.h>

uint8_t SREG, PD2, PD3, GICR, INT0, MCUCR, ISC01, ISC00;
uint8_t TIMSK, TCCR2, TCNT2;
#define CS20   0
#define TOIE2  0
#define F_CPU  8000000
#define cli()
#define sei()

#ifndef TEST_KEY_ARR_SIZE
#define TEST_KEY_ARR_SIZE    (BUFSS_SIZE+128)
#endif
#define NO_MAIN

#include "ep_ds.h"
#include "version.h"
#include "lcd.h"
#include "kbd.h"
#include "ep_store.h"
#include "billing.h"
#include "i2c.h"
#include <avr/eeprom.h>
#include "uart.h"
#include "mmc_avr.c"
#include "ff.c"
#include "a1micro2mm.h"
#include "flash.h"
#include "main.h"
#include "menu.h"

//assert(SPM_PAGESIZE == (1<<(FLASH_PAGE_SIZE_LOGN+1)));

/* All Header overrides */
#undef  SD_ITEM_FILE
#define SD_ITEM_FILE "run/items.csv"

#ifndef  __UNITS_KBD_C
#undef  LCD_ALERT
#define LCD_ALERT(str)				\
  LCD_CLRLINE(0); LCD_WR_P(str)

#undef  LCD_ALERT_16N
#define LCD_ALERT_16N(str, n)			\
  LCD_CLRLINE(0);				\
  LCD_WR_P(str);				\
  LCD_PUT_UINT16X(n)
#endif

#define ISR(A) void A()

uint8_t
_crc_ibutton_update(uint8_t crc, uint8_t data)
{
  uint8_t i;

  crc = crc ^ data;
  for (i = 0; i < 8; i++) {
    if (crc & 0x01)
      crc = (crc >> 1) ^ 0x8C;
    else
      crc >>= 1;
  }

  return crc;
}

#define _BV(n) (1<<n)

//#undef  soft_reset
//#define soft_reset()
#undef  WDT_RESET_WAKEUP
#define WDT_RESET_WAKEUP 0

typedef fpos_t FSIZE_t;
#define sprintf_P sprintf

#include "lcd.c"
#include "kbd.c"
#include "ep_store.c"
#include "i2c.c"
#include "uart.c"
#include "a1micro2mm.c"
#include "flash.c"
#include "menu.c"
#include "main.c"

#include <time.h>

uint8_t inp[TEST_KEY_ARR_SIZE], inp2[TEST_KEY_ARR_SIZE];
uint8_t inp3[TEST_KEY_ARR_SIZE], inp4[TEST_KEY_ARR_SIZE];

/* Convert a int to string */
void
int2str(char *str, uint32_t ui, uint32_t *idx)
{
  if (ui > 9) {
    int2str(str, ui/10, idx);
  }
  str[*idx] = '0' + (ui%10);
  (*idx)++;
  str[*idx] = 0;
}

void
common_init()
{
  uint8_t ui8_1, ui8_2;
  uint16_t crc=0;
 
  for (ui8_1=0; ui8_1<(SERIAL_NO_MAX-2); ui8_1++) {
    ui8_2 = '0' + (rand() % 70);
    assert(isgraph(ui8_2));
    crc = _crc16_update(crc, ui8_2);
    eeprom_update_byte((uint8_t *)offsetof(struct ep_store_layout, unused_serial_no)+ui8_1, ui8_2);
  }
  ui8_2 = crc>>8;
  eeprom_update_byte((uint8_t *)offsetof(struct ep_store_layout, unused_serial_no)+ui8_1, ui8_2);
  ui8_1++;
  ui8_2 = crc;
  eeprom_update_byte((uint8_t *)offsetof(struct ep_store_layout, unused_serial_no)+ui8_1, ui8_2);

  /* time */
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  date_t d = {tm.tm_mday, tm.tm_mon+1, 1900+tm.tm_year};
  timerDateSet(d);
  s_time_t _t = {tm.tm_hour, tm.tm_min, 0};
  timerTimeSet(_t);
}
