#include <stdint.h>

#include "printer.h"

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
void
printerStatus(void)
{
  uint8_t ui8_1;

  PRINTER_PRINT(ASCII_ESCAPE);
  PRINTER_PRINT('v');
  PRINTER_PRINT(N);
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
