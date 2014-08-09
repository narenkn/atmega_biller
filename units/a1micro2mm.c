#include <stdint.h>
#include <stdio.h>

#include "a1micro2mm.h"

#undef  PRINTER_PRINT
#define PRINTER_PRINT(c)			\
  putchar(c)


void
printerInit(void)
{
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
