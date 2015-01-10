#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#include <assert.c>

#include <avr/pgmspace.h>

#define TEST_KEY_ARR_SIZE 128

#include "lcd.h"
#include "i2c.h"

#include "lcd.c"
#include "kbd.c"
#include "i2c.c"
#include "ep_store.c"

int
main(void)
{
  uint8_t a;
  uint8_t b[64] = "The quick brown fox jumps over the lazy dog.";
  uint8_t c[64];
  uint8_t x;
  time_t t;
  uint32_t srand_t;

  ep_store_init();

  srand_t = time(&t);
  srand(srand_t);

  /* Test eeprom_*_byte routines */
  for (a = 1; a; a++) {
    eeprom_update_byte((uint8_t *)a, rand());
  }
  srand(srand_t);
  for (a = 1; a; a++) {
    x = eeprom_read_byte((uint8_t *)a);
    assert ((rand() & 0xFF) == x);
  }

  /* Test eeprom_*_block routines */
  eeprom_update_block(b, 100, 45);
  eeprom_read_block(c, 100, 45);
  assert(0 == strncmp(b, c, 44));

  return 0;
}
