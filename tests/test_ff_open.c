#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "lcd.c"
#include "mmc_avr.c"
#include "ff.c"		/* Declarations of FatFs API */

#define SD_SECTOR_SIZE (1<<9)

FATFS FatFs1;		/* FatFs work area needed for each volume */
FIL Fil;			/* File object needed for each open file */

uint8_t buf[256];

int
main(void)
{
  UINT bw;
  uint16_t errors = 0;

  _delay_ms(1000);
  LCD_init();

  DDRB  |= 0xB2;
  PORTB |= 0xF2;

  LCD_bl_on;
  LCD_CLRLINE(0);
  LCD_WR_P(PSTR("f_open Testing:"));
  _delay_ms(1000);

  LCD_CLRLINE(1);
  bw = f_mount(&FatFs1, "", 0);		/* Give a work area to the default drive */
  LCD_WR_P(PSTR("File hw.txt "));
  LCD_PUT_UINT16X(bw);

  /* Following needs to be tested
     Options
     1. FA_WRITE|FA_OPEN_APPEND
       a. Create if file not found
       b. Add data if file found
     2. FA_READ|FA_OPEN_EXISTING
       a. Fail if file not found
       b. Pass if file exists
     3. FA_WRITE|FA_OPEN_ALWAYS
       a. Open if file not found
       b. Open, truncate to 0, if file found
  */
  /* */
  f_unlink("hw.txt");
  LCD_CLRLINE(1);
  LCD_WR_P(PSTR("Test 2.a"));
  if (FR_OK == f_open(&Fil, "hw.txt", FA_READ|FA_OPEN_EXISTING)) {
    LCD_ALERT(PSTR("Test 2.a fail"));
    errors++;
    f_close(&Fil);
  } else {
    LCD_CLRLINE(1);
    LCD_WR_P(PSTR("Test 2.a Pass"));
  }

  /* */
  LCD_CLRLINE(1);
  LCD_WR_P(PSTR("Test 3.a"));
  if (f_open(&Fil, "hw.txt", FA_WRITE|FA_OPEN_ALWAYS) == FR_OK) {	/* Create a file */
    LCD_CLRLINE(1);
    LCD_WR_P(PSTR("3.a @ pt1"));
    f_write(&Fil, "Hello World1!\r\n", 15, &bw);	/* Write data to the file */
    f_close(&Fil);				/* Close the file */
    LCD_CLRLINE(1);
    LCD_WR_P(PSTR("3.a pass"));

    /* */
    LCD_CLRLINE(1);
    LCD_WR_P(PSTR("Test 2.b"));
    if (FR_OK == f_open(&Fil, "hw.txt", FA_READ|FA_OPEN_EXISTING)) {
      f_read(&Fil, buf, 64, &bw);
      if ((15 != bw) || (0 != strncmp(buf, "Hello World1!\r\n", 15))) {
	LCD_ALERT(PSTR("2.b fail 1"));
	errors++;
      } else {
	LCD_WR_P(PSTR("2.b pass"));
      }
    } else {
      LCD_ALERT(PSTR("2.b fail 2"));
      errors++;
    }
  } else {
    LCD_ALERT(PSTR("3.a fail:a"));
    errors++;
  }

  /*  */
  LCD_CLRLINE(1);
  LCD_WR_P(PSTR("Test 3.b"));
  if (f_open(&Fil, "hw.txt", FA_WRITE|FA_OPEN_ALWAYS) != FR_OK) {
    LCD_ALERT(PSTR("3.b fail:a"));
    errors++;
  }
  LCD_CLRLINE(1);
  LCD_WR_P(PSTR("3.a @ pt1"));
  if (f_open(&Fil, "hw.txt", FA_READ|FA_OPEN_EXISTING) != FR_OK) {
    LCD_ALERT(PSTR("3.b fail:b"));
    errors++;
  }
  f_read(&Fil, buf, 64, &bw);
  if (0 != bw) {
    LCD_ALERT(PSTR("3.b fail:c"));
    errors++;
  } else {
    LCD_CLRLINE(1);
    LCD_WR_P(PSTR("3.b pass"));
  }
  f_close(&Fil);

  /* delete this file and confirm */
  f_unlink("hw.txt");
  LCD_CLRLINE(1);
  LCD_WR_P(PSTR("Test 1.a"));
  if (FR_OK == f_open(&Fil, "hw.txt", FA_READ|FA_OPEN_EXISTING)) {
    LCD_ALERT(PSTR("Test 1.a fail 1"));
    errors++;
    f_close(&Fil);
  }
  LCD_CLRLINE(1);
  LCD_WR_P(PSTR("Test 1.a"));
  if (f_open(&Fil, "hw.txt", FA_WRITE|FA_OPEN_APPEND) == FR_OK) {
    f_write(&Fil, "Hello World2!\r\n", 15, &bw);
    f_close(&Fil);
  } else {
    LCD_ALERT(PSTR("Test 1.a fail 2"));
    errors++;
  }
  if (f_open(&Fil, "hw.txt", FA_WRITE|FA_OPEN_APPEND) != FR_OK) {
    f_write(&Fil, "Hello World3!\r\n", 15, &bw);
    f_close(&Fil);
  } else {
    LCD_ALERT(PSTR("Test 1.a fail 3"));
    errors++;
  }
  if (FR_OK != f_open(&Fil, "hw.txt", FA_READ|FA_OPEN_EXISTING)) {
    LCD_ALERT(PSTR("Test 1.a fail 4"));
    errors++;
  }
  LCD_WR_P(PSTR("1.a pass"));
  f_read(&Fil, buf, 64, &bw);
  if ((30 != bw) || (0 != strncmp(buf, "Hello World2!\r\nHello World3!\r\n", 30))) {
    LCD_ALERT(PSTR("1.b fail 5"));
    errors++;
  } else {
    LCD_WR_P(PSTR("1.b pass"));
  }
  f_close(&Fil);

  /* end */
  LCD_WR_P(PSTR("Errors: "));
  LCD_PUT_UINT(errors);

  for (;;) ;
}


/*---------------------------------------------------------*/
/* User Provided RTC Function called by FatFs module       */
DWORD
get_fattime(void)
{
  /* Returns current time packed into a DWORD variable */
  return	  ((DWORD)(2014 - 1980) << 25)	/* Year 2013 */
    | ((DWORD)5 << 21)				/* Month 7 */
    | ((DWORD)6 << 16)				/* Mday 28 */
    | ((DWORD)1 << 11)				/* Hour 0 */
    | ((DWORD)19 << 5)				/* Min 0 */
    | ((DWORD)44 >> 1);				/* Sec 0 */
}
