#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#include <assert.c>

#include <avr/pgmspace.h>

#define TEST_KEY_ARR_SIZE 128

#include "lcd.c"
#include "kbd.c"
#include "ep_store.c"
#include "i2c.c"
#include "menu.c"

int
main(void)
{
  uint32_t srand_t;
  time_t t;
  uint8_t b[64] = "The quick brown fox jumps over the lazy dog.";
  uint8_t c[64];

  i2c_init();

  srand_t = time(&t);
  srand(srand_t);

  /* Test eeprom_*_block routines */
  ee24xx_write_bytes(200, b, 45);
  ee24xx_read_bytes(200, c, 45);
  assert(0 == strncmp(b, c, 44));

  return 0;
}
