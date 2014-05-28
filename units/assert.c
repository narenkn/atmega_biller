#include "assert.h"

#define  EXPECT_FIFO_SZ  8
char* expect_assert[EXPECT_FIFO_SZ];

void
assert_init()
{
  expect_assert[0] = NULL;
}

void
add_expect_assert(char* str)
{
  uint8_t ui1;

  for (ui1=0; NULL!=expect_assert[ui1]; ui1++);

  expect_assert[ui1] = str;
  expect_assert[ui1+1] = NULL;
}

void
expect_else_assert(const char *s, const char *f, const uint32_t l)
{
  uint8_t ui1;
  if ((NULL != expect_assert[0]) && (0 == strcmp(s, expect_assert[0]))) {
    printf("Expect Assert Obtained :'%s', from '%s:%0d'", s, f, l);
    for (ui1=0; expect_assert[ui1]; ui1++) {
      expect_assert[ui1] = expect_assert[ui1+1];
    }
  } else {
    printf("ASSERT ERROR '%s' '%s' :%d\n", s, f, l);
  }
}
