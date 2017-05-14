#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <string.h>

#include "lcd.c"
#include "spi.c"
#include "mmc_avr.c"
#include "ff.c"		/* Declarations of FatFs API */
#include "main.c"

#define SD_SECTOR_SIZE (1<<9)

FATFS FatFs1;		/* FatFs work area needed for each volume */
FIL Fil;			/* File object needed for each open file */

int
main(void)
{
  UINT bw;

  _delay_ms(1000);
  LCD_init();
  main_init();
  spiInit();

  sei();

  LCD_bl_on;
  LCD_CLRLINE(0);
  LCD_PUTCH((RES_OK == disk_inserted()) ? 'a' : 'x');
  LCD_WR_P(PSTR("Fat32 Testing:"));
  _delay_ms(1000);

  memset(&FatFs1, 0, sizeof(FatFs1));
  memset(&Fil, 0, sizeof(Fil));

  LCD_CLRLINE(1);
  LCD_WR_P(PSTR("File hw.txt m"));
  bw = f_mount(&FatFs1, "", 0);		/* Give a work area to the default drive */
  _delay_ms(1000);
  LCD_CLRLINE(1);
  LCD_WR_P(PSTR("File hw.txt "));
  LCD_PUT_UINT16X(bw);
  _delay_ms(1000);

  bw = f_open(&Fil, "hw.txt", FA_WRITE);
  LCD_CLRLINE(1);
  LCD_WR_P(PSTR("Open: "));
  LCD_PUT_UINT(bw);
  _delay_ms(1000);
  if (bw == FR_OK) {	/* Create a file */
    LCD_CLRLINE(1);
    LCD_WR_P(PSTR("File hw.txt o"));
    f_write(&Fil, "Hello World1!\r\n", 15, &bw);	/* Write data to the file */
    f_close(&Fil);				/* Close the file */
    LCD_CLRLINE(1);
    LCD_WR_P(PSTR("File hw.txt pass"));
  } else {
    LCD_CLRLINE(1);
    LCD_WR_P(PSTR("File hw.txt fail"));
  }

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
