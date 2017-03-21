#include <stdint.h>
#include <stdio.h>

#include "a1micro2mm.h"

#undef ASCII_PRINTER_ESC
#undef ASCII_PRINTER_GS

#undef  PRINTER_PRINT
#define PRINTER_PRINT(c)			\
  putc(c, prn_outf)

#undef  PRINTER_PAPER_STATUS
#define PRINTER_PAPER_STATUS			\
  assert(0)
#undef  PRINTER_PRINT_FEED
#define PRINTER_PRINT_FEED(N)			\
  fprintf(prn_outf, "<feed " #N ">\n")
#undef  PRINTER_EMPHASIZE
#define PRINTER_EMPHASIZE(N)			\
  fprintf(prn_outf, "<emphasize " #N ">\n")
#undef  PRINTER_REVERSE
#define PRINTER_REVERSE(N)			\
  fprintf(prn_outf, "<reverse " #N ">\n")
#undef  PRINTER_INITIALIZE
#define PRINTER_INITIALIZE			\
  fprintf(prn_outf, "<printer-init>\n")
#undef  PRINTER_UNDERLINE
#define PRINTER_UNDERLINE(N)			\
  fprintf(prn_outf, "<underline " #N ">\n")
#undef  PRINTER_RX_TIMEOUT
#define PRINTER_RX_TIMEOUT			\
  fprintf(prn_outf, "<timeout>\n")
#undef  PRINTER_WAKE
#define PRINTER_WAKE				\
  fprintf(prn_outf, "<WAKE>\n")
#undef  PRINTER_PRINT_TEST_PAGE
#define PRINTER_PRINT_TEST_PAGE			\
  fprintf(prn_outf, "<print-test-page>\n")
#undef  PRINTER_SLEEP_SET
#define PRINTER_SLEEP_SET(N)			\
  fprintf(prn_outf, "<sleep-set>\n")
#undef  PRINTER_JUSTIFY
#define PRINTER_JUSTIFY(N)			\
  fprintf(prn_outf, "<justify " #N ">\n")
#undef  PRINTER_LINESPACE_DEFAULT
#define PRINTER_LINESPACE_DEFAULT		\
  fprintf(prn_outf, "<linespace-default>\n")
#undef  PRINTER_LINESPACE
#define PRINTER_LINESPACE(N)			\
  fprintf(prn_outf, "<linespace-" #N ">\n")
#undef  PRINTER_MODE_SEL
#define PRINTER_MODE_SEL(N)			\
  fprintf(prn_outf, "<mode-sel " #N ">\n")
#undef  PRINTER_FONT_ENLARGE
#define PRINTER_FONT_ENLARGE(N)			\
  fprintf(prn_outf, "<font-enlarge " #N ">\n")

bool printer_onoff = true;
#undef  PRINTER_TOGGLE_ONOFF
#undef  PRINTER_ONLINE
#define PRINTER_ONLINE				\
  printer_onoff = true; fprintf(prn_outf, "<PRINTER_ONLINE>\n")
#undef  PRINTER_OFFLINE
#define PRINTER_OFFLINE				\
  printer_onoff = false; fprintf(prn_outf, "<PRINTER_OFFLINE>\n")
//#undef  PRINTER_FEED_DOTS
//#define PRINTER_FEED_DOTS(n)			\
//  fprintf(prn_outf, "<feed dots %d>\n", n)
//
//#undef  PRINTER_FEED_LINES
//#define PRINTER_FEED_LINES(n)			\
//  fprintf(prn_outf, "<feed lines %d>\n", n)

FILE *prn_outf = NULL;

void
printerInit(void)
{
  prn_outf = fopen("printer.out", "w");
}

uint8_t
printerStatus(void)
{
  return (printer_onoff?(1<<0):(0<<0));
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
