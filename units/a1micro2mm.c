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
  prn_outf = fopen("printer.out", "w");
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

uint32_t
printerCompareStatus(char *golden)
{
  char buf[512];
  uint32_t golden_size = strlen(golden), file_size=0, this_sz, ret=0;
  char *gp;

  assert(golden_size);
  assert(NULL != prn_outf);

  fclose(prn_outf);

  prn_outf = fopen("printer.out", "r");
  gp = golden;
  do {
    this_sz = fread(buf, 1, 512, prn_outf);
    file_size += this_sz;
    ret += strncmp(buf, gp, this_sz);
    gp += this_sz;
  } while (this_sz);
  assert(0 == ret);
  assert(file_size == golden_size);

  fclose(prn_outf);

  return ret;
}

void
PRINTER_PSTR(const char *P)
{
  for (;P[0]; P++) {
    putc(P[0], prn_outf);
  }
}
