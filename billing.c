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
  assert(6 == sizeof(struct sale_info));
  assert(10 == sizeof(struct sale_item));
  assert(241 == SALE_SIZEOF);
  assert(SALE_SIZEOF <= BUFSS_SIZE);
}
