#include <stdint.h>
#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>
#include <util/twi.h>

#include "assert.h"
#include "billing.h"

/*
 */
void
billing_init(void)
{
  assert(6 == SALE_INFO_SIZEOF);
  assert(10 == SALE_ITEM_SIZEOF);
  assert(350 == SALE_SIZEOF);
  assert(SALE_SIZEOF <= BUFSS_SIZE);
}
