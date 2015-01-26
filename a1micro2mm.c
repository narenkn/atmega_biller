#include <stdint.h>

#include "a1micro2mm.h"
#include "uart.h"

void
printerInit(void)
{
  PRINTER_INITIALIZE;  /* init */
  PRINTER_SLEEP_SET(30);
}

void
printerDefineUserChar(uint8_t idx)
{
  /* FIXME */
}

/* return temperature if paper is available */
uint8_t
printerStatus(void)
{
  uint8_t ui8_1;

  PRINTER_PAPER_STATUS;
  PRINTER_PRINT('n'); /* FIXME: not sure if this is right */
  if ('P' != uartReceiveByte())
    return 0;
  if ('1' != uartReceiveByte())
    return 0;
  if ('V' != uartReceiveByte())
    return 0;
  uartReceiveByte();
  uartReceiveByte();
  if ('T' != uartReceiveByte())
    return 0;
  ui8_1 = uartReceiveByte() * 10;
  ui8_1 += uartReceiveByte();

  return ui8_1;
}
