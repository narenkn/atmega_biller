/*----------------------------------------------------------------------*/
/* Foolproof FatFs sample project for AVR              (C)ChaN, 2013    */
/*----------------------------------------------------------------------*/
#include "ffconf.h"
#include "ff.c"
#include "mmc_avr.c"

FATFS FatFs1;		/* FatFs work area needed for each volume */
FIL Fil;			/* File object needed for each open file */

int main (void)
{
  UINT bw;
  char str[16];

  f_mount(&FatFs1, "", 0);		/* Give a work area to the default drive */
  bw = f_mkfs("0:", 0, SD_SECTOR_SIZE);

  if (f_open(&Fil, "newfile.txt", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {	/* Create a file */

    f_write(&Fil, "It works!\n", 11, &bw);	/* Write data to the file */

    f_close(&Fil);								/* Close the file */
  }

  if (f_open(&Fil, "newfile.txt", FA_READ) == FR_OK) {	/* Create a file */

    f_read(&Fil, str, 11, &bw);	/* Write data to the file */
    str[12] = 0;
    printf("Obtained string:\'%s\'\n", str);

    f_close(&Fil);								/* Close the file */
  }

  return 0;
}

/*---------------------------------------------------------*/
/* User Provided RTC Function called by FatFs module       */

DWORD get_fattime (void)
{
  /* Returns current time packed into a DWORD variable */
  return	  ((DWORD)(2013 - 1980) << 25)	/* Year 2013 */
    | ((DWORD)7 << 21)				/* Month 7 */
    | ((DWORD)28 << 16)				/* Mday 28 */
    | ((DWORD)0 << 11)				/* Hour 0 */
    | ((DWORD)0 << 5)				/* Min 0 */
    | ((DWORD)0 >> 1);				/* Sec 0 */
}
