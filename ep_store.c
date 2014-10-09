#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/crc16.h>
#include <avr/eeprom.h>

#include "ep_ds.h"
#include "version.h"
#include "assert.h"
#include "lcd.h"
#include "kbd.h"
#include "ep_store.h"
#include "billing.h"
#include "i2c.h"
#include "uart.h"
#include "a1micro2mm.h"
#include "menu.h"
#include "main.h"

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
