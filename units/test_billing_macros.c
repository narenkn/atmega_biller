#include "test_common.c"

int
main()
{
  printf("EEPROM_MAX_ADDRESS %x\n", EEPROM_MAX_ADDRESS);
  printf("SALE_DATA_SIZEOF_NORM:%d\n", SALE_DATA_SIZEOF_NORM);
  printf("ITEM_MAX_ADDR:%x\n", ITEM_MAX_ADDR);
  printf("EEPROM_SALE_START_ADDR:%x\n", EEPROM_SALE_START_ADDR);
  printf("EEPROM_BILL_ADDR(1):%x\n", EEPROM_BILL_ADDR(1));
  printf("EEPROM_BILL_ADDR(2):%x\n", EEPROM_BILL_ADDR(2));
  printf("EEPROM_SALE_END_APPROX_ADDR:%x\n", EEPROM_SALE_END_APPROX_ADDR);
  printf("EEPROM_SALE_MAX_BILLS:%d\n", EEPROM_SALE_MAX_BILLS);
  printf("EEPROM_SALE_END_ADDR:%x\n", EEPROM_SALE_END_ADDR);

  /* */
  assert(8 == sizeof(struct sale_info));
  assert(13 == sizeof(struct sale_item));
  assert(0 == (sizeof(struct item) & 0x3)); /* 4 byte aligned */
  assert(SALE_DATA_EXP_ITEMS_SIZEOF >= (sizeof(struct sale)-ITEM_SIZEOF));
  assert(0 == (SALE_DATA_EXP_ITEMS_SIZEOF & 0x3)); /* 4 byte aligned */

  /* check EEPROM_NEXT_SALE_RECORD */
  uint32_t ui32_1;
  uint16_t ui16_1, ui16_2, ui16_3, ui16_4;
  ui16_3 = EEPROM_SALE_START_ADDR;
  ui16_1 = ui16_3;
  for (ui32_1=0; ui32_1<EEPROM_SALE_MAX_BILLS; ui32_1++) {
    assert(ui16_1 >= EEPROM_SALE_START_ADDR);
    ui16_2 = EEPROM_NEXT_SALE_RECORD(ui16_1);
    if ( ((EEPROM_MAX_ADDRESS+1-ui16_2)<<2)<SALE_DATA_EXP_ITEMS_SIZEOF ) {
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
    if ( ((EEPROM_MAX_ADDRESS+1-ui16_2)<<2)<SALE_DATA_EXP_ITEMS_SIZEOF ) {
      printf("Error3: ui16_1:%x ui16_2:%x\n", ui16_1, ui16_2);
      break;
    }
    ui16_1 = ui16_2;
  }
  if (ui16_3 != ui16_1) {
    printf("Error4: ui16_1:%x ui16_3:%x\n", ui16_1, ui16_3);
  }

  /* */
  for (ui16_1=0, ui16_4=0, ui16_2=0; ui16_1<ITEM_MAX; ui16_1++,
	 ui16_2 += (ITEM_SIZEOF>>2), ui16_4 = itemNxtAddr(ui16_4)) {
    ui16_3 = itemAddr(ui16_1+1);
    assert(ui16_2 == ui16_3);
    assert((ui16_1+1) == itemId(ui16_3));
    assert(ui16_4 == ui16_3);
  }
  assert(ui16_2 == ITEM_MAX_ADDR);

  return 0;
}
