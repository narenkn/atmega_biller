/*----------------------------------------------------------------------*/
/* Foolproof FatFs sample project for AVR              (C)ChaN, 2013    */
/*----------------------------------------------------------------------*/
#include "time.h"
#include "ffconf.h"
#undef   _USE_MKFS
#define  _USE_MKFS 1
#include "../ff.c"
#include "mmc_avr.c"
#include "assert.c"

FATFS FatFs1;
FIL Fil;			/* File object needed for each open file */

/* Intent of this test: validate the following
   1. f_chdir, f_mkdir
     1.a : FR_OK : if dir exists
     1.b : f_chdir("..")
   2. f_open
     2.a : FA_WRITE|FA_OPEN_APPEND : check appends to file
   3. f_putc
     3.a : ascii gets written
   4. Delete just one bill
   //5. menuSdSwallowComma()
   //6. menuSdSwallowUntilNL()
 */

BYTE work[SD_SECTOR_SIZE*4];

int main (int argc, char *argv[])
{
  UINT bw;
  char str[64];

  uint32_t ui32_1;
  if ((argc == 1) || (0 == argv[1]))
    ui32_1 = time(NULL);
  else
    ui32_1 = atoi(argv[1]);
  printf("seed : %d\n", ui32_1);
  srand(ui32_1);

  /* Test for : 1, 3 */
  change_sd(0);
  f_mount(&FatFs1, "", 0);		/* Give a work area to the default drive */
  f_mkfs("C", FM_FAT, SD_SECTOR_SIZE, work, SD_SECTOR_SIZE);
  assert(FR_OK == f_mkdir("billdat"));
  assert(FR_OK == f_chdir("billdat"));
  if (f_open(&Fil, "newfile.txt", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {	/* Create a file */

    f_write(&Fil, "It works!", 9, &bw);	/* Write data to the file */

    f_putc('k', &Fil);
    f_putc('n', &Fil);
    f_putc('n', &Fil);

    f_close(&Fil);								/* Close the file */
  } else {
    printf("Error(%d): Can't open newfile.txt\n", __LINE__);
  }
  assert(FR_OK == f_chdir(".."));

  if (f_open(&Fil, "billdat\\newfile.txt", FA_READ) == FR_OK) {	/* Create a file */

    f_read(&Fil, str, 12, &bw);
    if (0 != strncmp("It works!knn", str, 12)) {
      printf("Error(%d): Obtained string:\'%s\'\n", __LINE__, str);
    }

    f_close(&Fil);								/* Close the file */
  } else {
    printf("Error(%d): Can't open billdat\\newfile.txt", __LINE__);
  }

  /* Test for : 2 */
  if (f_open(&Fil, "billdat\\newfile.txt", FA_WRITE | FA_OPEN_APPEND) == FR_OK) {	/* Create a file */

    f_write(&Fil, "It works!", 9, &bw);	/* Write data to the file */

    f_putc('k', &Fil);
    f_putc('n', &Fil);
    f_putc('n', &Fil);

    f_close(&Fil);								/* Close the file */
  } else {
    printf("Error(%d): Can't open billdat\\newfile.txt\n", __LINE__);
  }
  assert(FR_OK == f_chdir(".."));

  if (f_open(&Fil, "billdat\\newfile.txt", FA_READ) == FR_OK) {	/* Create a file */

    f_read(&Fil, str, 24, &bw);
    if (0 != strncmp("It works!knnIt works!knn", str, 24)) {
      printf("Error(%d): Obtained string:\'%s\'\n", __LINE__, str);
    }
    str[24] = 0;

    f_close(&Fil);								/* Close the file */
  } else {
    printf("Error(%d): Can't open billdat\\newfile.txt\n", __LINE__);
  }

  /* 4. Delete just one bill */
#if 0
	sl->crc = sl->crc_invert = 0xFFFF;
	if (FR_OK == f_lseek(&Fil, loc)) {
	  if (FR_OK == f_write(&Fil, (void *)sl, SIZEOF_SALE_EXCEP_ITEMS, &ret_val)) {
	    if (SIZEOF_SALE_EXCEP_ITEMS != ret_val) {
	      LCD_ALERT(PSTR("Save Failed"));
	      break;
	    }
	  }
	}
#else
#error "Not yet testing 4."
#endif

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
