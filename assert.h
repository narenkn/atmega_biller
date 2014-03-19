#ifndef _ASSERT_H
#define _ASSERT_H

#ifdef  UNIT_TEST

#define assert(X)				\
  if (!(X)) {					\
    expect_else_assert(#X, __FILE__, __LINE__);	\
  }

#define ERROR(msg) fprintf(stderr, msg)

void assert_init();
void add_expect_assert(char* str);
void expect_else_assert(const char *s, const char *f, const uint32_t l);

#else

#if 0
#define assert(X)			       \
  if (!(X)) {				       \
    LCD_WR_LINE_N(1, 0, __FILE__, 12);	       \
    LCD_wrchar('0' + (((__LINE__/10)/10)%10)); \
    LCD_wrchar('0' + ((__LINE__/10)%10));      \
    LCD_wrchar('0' + (__LINE__%10));	       \
  }
#else
#define  assert(X)
#endif

#define ERROR(msg) { /*FIXME: flash error message*/ }

#endif

#endif
