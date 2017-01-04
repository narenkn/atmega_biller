#include <stdio.h>
#include <assert.h>

#define ITEM_SIZEOF   64
#define ITEM_MAX_ADDR ((uint16_t)(ITEM_MAX*(ITEM_SIZEOF>>2)))
#define NVF_SALE_START_ADDR     ((uint16_t)(((ITEM_MAX_ADDR-1)>>7)+(((ITEM_MAX_ADDR-1)&((1<<8)-1))?1:0)))

int
main()
{
#define ITEM_MAX 0
  //printf("ITEM_MAX:%d NVF_SALE_START_ADDR:%d\n", ITEM_MAX, NVF_SALE_START_ADDR);
  assert(0 == NVF_SALE_START_ADDR);
#undef ITEM_MAX
#define ITEM_MAX 1
  //printf("ITEM_MAX:%d NVF_SALE_START_ADDR:%d\n", ITEM_MAX, NVF_SALE_START_ADDR);
  assert(1 == NVF_SALE_START_ADDR);
#undef ITEM_MAX
#define ITEM_MAX 2
  //printf("ITEM_MAX:%d NVF_SALE_START_ADDR:%d\n", ITEM_MAX, NVF_SALE_START_ADDR);
  assert(1 == NVF_SALE_START_ADDR);
#undef ITEM_MAX
#define ITEM_MAX 8
  //printf("ITEM_MAX:%d NVF_SALE_START_ADDR:%d\n", ITEM_MAX, NVF_SALE_START_ADDR);
  assert(1 == NVF_SALE_START_ADDR);
#undef ITEM_MAX
#define ITEM_MAX 9
  //printf("ITEM_MAX:%d NVF_SALE_START_ADDR:%d\n", ITEM_MAX, NVF_SALE_START_ADDR);
  assert(2 == NVF_SALE_START_ADDR);
#undef ITEM_MAX
  return 0;
}
