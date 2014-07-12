#include <stdint.h>
#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>
#include <util/twi.h>
#include <avr/eeprom.h>

#include "assert.h"
#include "ep_store.h"

struct ep_store_layout EEMEM ep_store;

/*
 * Do all the startup-time peripheral initializations: TWI clock.
 */
void
ep_store_init(void)
{
  assert(64 == ITEM_SIZEOF);
}
