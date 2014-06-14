
#define assert(x)

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


int
main(void)
{
  UINT bw;

  _delay_ms(1000);
  LCD_init();

  DDRB  |= 0xB2;
  PORTB |= 0xF2;

  LCD_bl_on;
  LCD_WR_LINE(0, 0, "Fat32 Testing:");
  LCD_refresh();
  _delay_ms(1000);

  LCD_WR_LINE(1, 0, "File hw.txt m");
  LCD_refresh();
  bw = f_mount(&FatFs1, "", 0);		/* Give a work area to the default drive */
  LCD_WR_LINE(1, 0, "File hw.txt ");
  LCD_POS(1, 12);
  LCD_PUT_UINT16X(bw);
  LCD_refresh();

  if (f_open(&Fil, "hw.txt", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {	/* Create a file */
    LCD_WR_LINE(1, 0, "File hw.txt o");
    LCD_refresh();
    f_write(&Fil, "Hello World1!\r\n", 15, &bw);	/* Write data to the file */
    f_close(&Fil);				/* Close the file */
    LCD_WR_LINE(1, 0, "File hw.txt pass");
    LCD_refresh();
  } else {
    LCD_WR_LINE(1, 0, "File hw.txt fail");
    LCD_refresh();
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
