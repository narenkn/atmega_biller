#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "lcd.c"
#include "ff.c"		/* Declarations of FatFs API */

FATFS FatFs;		/* FatFs work area needed for each volume */
FIL Fil;			/* File object needed for each open file */


int
main(void)
{
  UINT bw;

  f_mount(&FatFs, "", 0);		/* Give a work area to the default drive */

  if (f_open(&Fil, "hw.txt", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {	/* Create a file */
    f_write(&Fil, "Hello World!\r\n", 14, &bw);	/* Write data to the file */
    f_close(&Fil);								/* Close the file */
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
