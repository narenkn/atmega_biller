#include "common_incl.c"

int
main(void)
{
  uint32_t srand_t;
  time_t t;

  nvfInit();

  srand_t = time(&t);
  srand(srand_t);

  /* Test eeprom_*_block routines */
  for (uint32_t ui32_1=0; ui32_1<10000; ui32_1++) {
    uint16_t addr = rand() & NVF_END_ADDRESS;

    uint16_t len = (rand() % TEST_KEY_ARR_SIZE) + 1;
    for (uint16_t ui1=0; ui1<len; ui1++) {
      inp[ui1] = ' ' + (rand()%('~'-' '));
    }

    bill_write_bytes(addr, inp, len);
    bill_read_bytes(addr, inp2, len);
    assert(0 == strncmp(inp, inp2, len));
  }

  return 0;
}
