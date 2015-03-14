/*----------------------------------------------------------------------*/
/* Foolproof FatFs sample project for AVR              (C)ChaN, 2013    */
/*----------------------------------------------------------------------*/
#include "ffconf.h"
#undef   _USE_MKFS
#define  _USE_MKFS 1
#include "../ff.c"
#include "mmc_avr.c"

FATFS FatFs1;
FIL Fil;			/* File object needed for each open file */

int main (void)
{
  UINT bw;
  char str[64];

  change_sd(0);
  f_mount(&FatFs1, "", 0);		/* Give a work area to the default drive */
  f_mkfs("", 0, SD_SECTOR_SIZE);
  if (f_open(&Fil, "newfile.txt", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {	/* Create a file */

    f_write(&Fil, "It works!", 10, &bw);	/* Write data to the file */

    f_close(&Fil);								/* Close the file */
  }
  f_mount(NULL, "", 0);

  change_sd(1);
  f_mount(&FatFs1, "", 0);
  f_mkfs("", 0, SD_SECTOR_SIZE);
  if (f_open(&Fil, "newfile.txt", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {	/* Create a file */

    f_write(&Fil, "It works.. in another drive too...!", 37, &bw);	/* Write data to the file */

    f_close(&Fil);								/* Close the file */
  }
  f_mount(NULL, "", 0);

  change_sd(0);
  f_mount(&FatFs1, "", 0);
  if (f_open(&Fil, "newfile.txt", FA_READ) == FR_OK) {	/* Create a file */

    f_read(&Fil, str, 10, &bw);	/* Write data to the file */
    str[10] = 0;
    printf("Obtained string:\'%s\'\n", str);

    f_close(&Fil);								/* Close the file */
  }
  f_mount(NULL, "", 0);

  change_sd(1);
  f_mount(&FatFs1, "", 0);
  if (f_open(&Fil, "newfile.txt", FA_READ) == FR_OK) {	/* Create a file */

    f_read(&Fil, str, 37, &bw);	/* Write data to the file */
    str[37] = 0;
    printf("Obtained string:\'%s\'\n", str);

    f_close(&Fil);								/* Close the file */
  }
  f_mount(NULL, "", 0);

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
