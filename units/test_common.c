#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#include <assert.c>

#include "device.h"

#ifndef  SPM_PAGESIZE
#if      ATMega32
# define SPM_PAGESIZE     128
#elif    ATxMega64
# define SPM_PAGESIZE     128
#elif  ATxMega128
# define SPM_PAGESIZE     256
#elif  ATxMega256
# define SPM_PAGESIZE     256
#else
# error "Not a known device"
#endif
#endif

#include <avr/pgmspace.h>
#include <avr/boot.h>
#include <avr/eeprom.h>
#include <util/crc16.h>

uint8_t SREG, PD2, PD3, GICR, INT0, MCUCR, ISC01, ISC00;
uint8_t TIMSK, TCCR2;
#define CS20   0
#define TOIE2  0
#define F_CPU  8000000
#define cli()
#define sei()

#ifndef TEST_KEY_ARR_SIZE
#define TEST_KEY_ARR_SIZE    128
#endif
#define NO_MAIN

#include "ep_ds.h"
#include "version.h"
#include "lcd.h"
#include "kbd.h"
#include "ep_store.h"
#include "billing.h"
#include "i2c.h"
#include "uart.h"
#include "a1micro2mm.h"
#include "menu.h"
#include "main.h"

//assert(SPM_PAGESIZE == (1<<(FLASH_PAGE_SIZE_LOGN+1)));

/* All Header overrides */
#undef  SD_ITEM_FILE
#define SD_ITEM_FILE "test_data/items_1.dat"
#define LCD_ALERT(str)				\
  LCD_WR_LINE(0, 0, str);			\
  KBD_RESET_KEY; KBD_GETCH

#define LCD_ALERT_16N(str, n)			\
  LCD_WR_LINE(0, 0, str);			\
  LCD_PUT_UINT16X(n);				\
  KBD_RESET_KEY; KBD_GETCH

#include "lcd.c"
#include "kbd.c"
#include "ep_store.c"
#include "i2c.c"
#include "uart.c"
#include "ff.c"
#include "a1micro2mm.c"
#include "menu.c"
#include "main.c"

#include <time.h>
//******************************************************************
//Function to get RTC date & time in FAT32 format
//  Return format : Year[31:25], Month[24:21], Date[20:16]
//                  Hour[15:11], Min[10:5], Sec[4:0]
//******************************************************************
uint32_t
get_fattime (void)
{
  time_t now_t = time(NULL);
  struct tm *now = localtime(&now_t);

  return (((now->tm_year-80)&0x7F)<<25) |
    ((now->tm_mon&0xF)<<21) |
    ((now->tm_mday&0x1F)<<16) |
    ((now->tm_hour&0x1F)<<11) |
    ((now->tm_min&0x3F)<<5) |
    (now->tm_sec&0x1F);
}

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
