#include "common_incl.c"

int
main (int argc, char *argv[])
{
#if DEBUG
  printf("EEPROM_MAX_ADDRESS %x\n", EEPROM_MAX_ADDRESS);
  printf("SALE_DATA_SIZEOF_NORM:%d\n", SALE_DATA_SIZEOF_NORM);
  printf("ITEM_MAX_ADDR:%x\n", ITEM_MAX_ADDR);
  printf("NVF_SALE_START_ADDR:%x\n", NVF_SALE_START_ADDR);
  printf("NVF_BILL_ADDR(1):%x\n", NVF_BILL_ADDR(1));
  printf("NVF_BILL_ADDR(2):%x\n", NVF_BILL_ADDR(2));
  //  printf("NVF_SALE_END_APPROX_ADDR:%x\n", NVF_SALE_END_APPROX_ADDR);
  printf("NVF_SALE_MAX_BILLS:%d\n", NVF_SALE_MAX_BILLS);
  printf("NVF_SALE_END_ADDR:%x\n", NVF_SALE_END_ADDR);
#endif

  uint32_t ui32_1;
  if ((argc == 1) || (0 == argv[1]))
    ui32_1 = time(NULL);
  else
    ui32_1 = atoi(argv[1]);
  printf("seed : %d\n", ui32_1);
  srand(ui32_1);

  /* */
  assert(16 == sizeof(struct sale_info));
  assert(15 == sizeof(struct sale_item));
  assert(0 == (sizeof(struct item) & 0x3)); /* 4 byte aligned */
  assert(SIZEOF_SALE_EXCEP_ITEMS >= (sizeof(struct sale)-ITEM_SIZEOF));

  /* check NVF_NEXT_SALE_RECORD */
  uint16_t ui16_1, ui16_2, ui16_3, ui16_4;
  ui16_3 = NVF_SALE_START_ADDR;
  ui16_1 = ui16_3;
  for (ui32_1=0, ui16_2=0; ui32_1<NVF_SALE_MAX_BILLS; ui32_1++) {
    assert(ui16_1 >= NVF_SALE_START_ADDR);
    ui16_1 = NVF_NEXT_SALE_RECORD(ui16_1);
    if (ui16_3 != ui16_1) ui16_2++;
  }
  assert ((NVF_SALE_MAX_BILLS-1) == ui16_2);
  assert (ui16_3 == ui16_1);
#if DEBUG
  printf("Check2: ui16_1:%x ui16_3:%x ui16_2:%d\n", ui16_1, ui16_3, ui16_2);
#endif

  /* check NVF_PREV_SALE_RECORD */
  ui16_3 = NVF_SALE_START_ADDR;
  ui16_1 = ui16_3;
  for (ui32_1=0, ui16_2=0; ui32_1<NVF_SALE_MAX_BILLS; ui32_1++) {
    assert(ui16_1 >= NVF_SALE_START_ADDR);
    ui16_1 = NVF_PREV_SALE_RECORD(ui16_1);
    if (ui16_3 != ui16_1) ui16_2++;
  }
  assert ((NVF_SALE_MAX_BILLS-1) == ui16_2);
  assert (ui16_3 == ui16_1);
#if DEBUG
  printf("Check3: ui16_1:%x ui16_3:%x ui16_2:%d\n", ui16_1, ui16_3, ui16_2);
#endif

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
