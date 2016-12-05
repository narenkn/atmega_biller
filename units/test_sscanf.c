#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include "assert.c"

uint8_t menuScanFDotSeen;
void
menuScanF(uint8_t *str, uint32_t *ui32)
{
  if (3 == menuScanFDotSeen) return;

  if ((str[0] >= '0') && (str[0] <= '9')) {
    *ui32 *= 10;
    *ui32 += str[0]-'0';
    if (menuScanFDotSeen) menuScanFDotSeen++;
    menuScanF(str+1, ui32);
  } else if (str[0] == '.') {
    menuScanFDotSeen = 1;
    menuScanF(str+1, ui32);
  }
}

#define SSCANF(str_p, ui32)			\
  ui32 = 0;					\
  menuScanFDotSeen = 0;				\
  menuScanF(str_p, &ui32);			\
  if (menuScanFDotSeen < 2) ui32 *= 10;		\
  if (menuScanFDotSeen < 3) ui32 *= 10

int
main()
{
  uint32_t ui1;

  SSCANF("0123abc", ui1); printf("%u\n", ui1);
  assert(12300 == ui1);
  SSCANF("abc", ui1); printf("%u\n", ui1);
  assert(0 == ui1);
  SSCANF("099.99abc", ui1); printf("%u\n", ui1);
  assert(9999 == ui1);
  SSCANF("0108.12abc", ui1); printf("%u\n", ui1);
  assert(10812 == ui1);
  SSCANF("0.99abc", ui1); printf("%u\n", ui1);
  assert(99 == ui1);
  return 0;
}
