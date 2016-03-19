#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#define PROGMEM

#include "ep_ds.h"
#include "version.h"
#include "lcd.h"
#include "kbd.h"
#include "ep_store.h"
#include "item.h"
#include "billing.h"
#include "i2c.h"

int
main()
{
  printf("EEPROM_MAX_ADDRESS %x\n", EEPROM_MAX_ADDRESS);
  printf("SALE_DATA_SIZEOF_NORM:%d\n", SALE_DATA_SIZEOF_NORM);
  printf("ITEM_MAX_ADDR:%x exp:%x\n", ITEM_MAX_ADDR, 56*1024);
  printf("EEPROM_SALE_START_ADDR:%x\n", EEPROM_SALE_START_ADDR);
  printf("EEPROM_BILL_ADDR(1):%x\n", EEPROM_BILL_ADDR(1));
  printf("EEPROM_BILL_ADDR(2):%x\n", EEPROM_BILL_ADDR(2));
  printf("EEPROM_SALE_END_APPROX_ADDR:%x\n", EEPROM_SALE_END_APPROX_ADDR);
  printf("EEPROM_SALE_MAX_BILLS:%d\n", EEPROM_SALE_MAX_BILLS);
  printf("EEPROM_SALE_END_ADDR:%x\n", EEPROM_SALE_END_ADDR);

  /* check EEPROM_NEXT_SALE_RECORD */
  uint32_t ui32_1;
  uint16_t ui16_1, ui16_2, ui16_3;
  ui16_3 = EEPROM_SALE_START_ADDR;
  ui16_1 = ui16_3;
  for (ui32_1=0; ui32_1<EEPROM_SALE_MAX_BILLS; ui32_1++) {
    assert(ui16_1 >= EEPROM_SALE_START_ADDR);
    ui16_2 = EEPROM_NEXT_SALE_RECORD(ui16_1);
    if ( ((EEPROM_MAX_ADDRESS+1-ui16_2)<<2)<SALE_DATA_SIZEOF ) {
      printf("Error1: ui16_1:%x ui16_2:%x\n", ui16_1, ui16_2);
      break;
    }
    ui16_1 = ui16_2;
  }
  if (ui16_3 != ui16_1) {
    printf("Error2: ui16_1:%x ui16_3:%x\n", ui16_1, ui16_3);
  }

  /* check EEPROM_PREV_SALE_RECORD */
  ui16_3 = EEPROM_SALE_START_ADDR;
  ui16_1 = ui16_3;
  for (ui32_1=0; ui32_1<EEPROM_SALE_MAX_BILLS; ui32_1++) {
    assert(ui16_1 >= EEPROM_SALE_START_ADDR);
    ui16_2 = EEPROM_PREV_SALE_RECORD(ui16_1);
    if ( ((EEPROM_MAX_ADDRESS+1-ui16_2)<<2)<SALE_DATA_SIZEOF ) {
      printf("Error3: ui16_1:%x ui16_2:%x\n", ui16_1, ui16_2);
      break;
    }
    ui16_1 = ui16_2;
  }
  if (ui16_3 != ui16_1) {
    printf("Error4: ui16_1:%x ui16_3:%x\n", ui16_1, ui16_3);
  }

  return 0;
}
