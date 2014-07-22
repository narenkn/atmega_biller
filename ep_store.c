#include <stdint.h>
#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>
#include <util/twi.h>
#include <avr/eeprom.h>

#include "assert.h"
#include "ep_store.h"

struct ep_store_layout EEMEM ep_store;

//#if 0 != (ITEM_SIZEOF&3)
//#error "should be multiple of 4"
//#endif
//#if 0 != (EP_STORE_LAYOUT_SIZEOF&3)
//#error "should be multiple of 4"
//#endif
//#if 0 != (EEPROM_MAX_DEVICES&3)
//#error "should be multiple of 4"
//#endif

/*
 * Do all the startup-time peripheral initializations: TWI clock.
 */
void
ep_store_init(void)
{
}
