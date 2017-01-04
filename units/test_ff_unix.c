#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#include <assert.c>

#include <avr/pgmspace.h>

#define TEST_KEY_ARR_SIZE (BUFSS_SIZE+128)

#include "lcd.h"
#include "lcd.c"
#include "kbd.c"
#include "ep_store.c"
#include "i2c.c"
#include "ff.c"

int
main(void)
{
  FATFS FS;
  FIL   fp;

  f_mount(&FS, ".", 0);
  f_mkdir("dir1");
  fp.is_active = 0;
  f_open(&fp, "dir1/f1", FA_WRITE);
  f_close(&fp);
  f_unlink("dir1");

  return 0;
}
