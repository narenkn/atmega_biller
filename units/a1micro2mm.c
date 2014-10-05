#include <stdint.h>
#include <stdio.h>

#include "a1micro2mm.h"

#undef  PRINTER_PRINT
#define PRINTER_PRINT(c)			\
  putc(c, prn_outf)

#undef  PRINTER_FEED_DOTS
#define PRINTER_FEED_DOTS(n)			\
  fprintf(prn_outf, "<feed dots %d>\n", n)

#undef  PRINTER_FEED_LINES
#define PRINTER_FEED_LINES(n)			\
  fprintf(prn_outf, "<feed lines %d>\n", n)

#undef PRINTER_FONT_ENLARGE
#define PRINTER_FONT_ENLARGE(N)			\
  fprintf(prn_outf, "<size %d>\n", N)

#undef  PRINTER_BOLD
#define PRINTER_BOLD(N)				\
  fprintf(prn_outf, "<bold %d>\n", N)

#undef  PRINTER_UNDERLINE
#define PRINTER_UNDERLINE(N)			\
  fprintf(prn_outf, "<underline %d>\n", N)

#undef  PRINTER_JUSTIFY
#define PRINTER_JUSTIFY(X)                        \
  fprintf(prn_outf, "<justify %d>\n", X)

#undef  PRINTER_USERCHAR_ENDIS
#define PRINTER_USERCHAR_ENDIS(N)		\
  assert(0)

FILE *prn_outf = NULL;

void
printerInit(void)
{
  prn_outf = fopen("stdout.log", "w");
}

void
printerDefineUserChar(uint8_t idx)
{
  /* FIXME */
  assert(0);
}

/* return temperature if paper is available */
uint8_t
printerStatus(void)
{
  return PRINTER_STATUS_ONLINE;
}
