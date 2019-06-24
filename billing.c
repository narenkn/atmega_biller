#include <stdint.h>
#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>
#include <util/twi.h>

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/crc16.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>
#include <util/twi.h>
#include <avr/sleep.h>

#include "ep_ds.h"
#include "version.h"
#include "assert.h"
#include "lcd.h"
#include "kbd.h"
#include "ep_store.h"
#include "billing.h"

/* Save, restore bill */
void
billingInit(void)
{
  assert(8 == sizeof(struct sale_info));
  assert(13 == sizeof(struct sale_item));
  assert(0 == (sizeof(struct item) & 0x3)); /* 4 byte aligned */
  assert(SALE_DATA_EXP_ITEMS_SIZEOF >= (sizeof(struct sale)-ITEM_SIZEOF));
}
