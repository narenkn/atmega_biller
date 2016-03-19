#include <stdio.h>
#include <stdint.h>

void
menuScanF(char *str, uint16_t *ui16)
{
  if ((str[0] >= '0') && (str[0] <= '9')) {
    *ui16 *= 10;
    *ui16 += str[0]-'0';
    menuScanF(str+1, ui16);
  }
}

#define SSCANF(str_p, ui16_p) \
  *(ui16_p) = 0;	      \
  menuScanF(str_p, ui16_p)

int
main()
{
  uint16_t ui1;

  SSCANF("0123abc", &ui1); printf("%u\n", ui1);
  SSCANF("abc", &ui1); printf("%u\n", ui1);
  SSCANF("099.99abc", &ui1); printf("%u\n", ui1);
  SSCANF("0108.12abc", &ui1); printf("%u\n", ui1);
  SSCANF("0.99abc", &ui1); printf("%u\n", ui1);
  return 0;
}
