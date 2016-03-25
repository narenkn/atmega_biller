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

/* Production Code: take care of exporting assertions */
#if 0
#define assert(X)			       \
  if (!(X)) {				       \
    LCD_WR_LINE_NP(1, 0, PSTR(__FILE__), 11);   \
    LCD_PUTCH(':');			       \
    LCD_PUT_UINT16X(__LINE__);		       \
    while (1) {}			       \
  }
#else
#define  assert(X)				\
  if (!(X)) {					\
    LCD_CLRLINE(1);				\
    LCD_PUT_UINT(__LINE__);			\
    LCD_refresh();				\
    while (1) {}				\
  }
#endif

#define ERROR(msg) { /*FIXME: flash error message*/ }

#endif

#endif
