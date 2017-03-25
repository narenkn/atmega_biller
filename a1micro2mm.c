#include <stdint.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "a1micro2mm.h"
#include "uart.h"

void
printerInit(void)
{
  PRINTER_INITIALIZE;  /* init */
  _delay_ms(100);
  PRINTER_INITIALIZE;  /* init */
  _delay_ms(100);
  PRINTER_INITIALIZE;  /* init */
  PRINTER_SLEEP_SET(30);
  PRINTER_RX_TIMEOUT;

  PRINTER_MODE_SEL(PRINTER_CHAR_FONTA);
}

/* return printer status */
uint8_t
printerStatus(void)
{
  PRINTER_PAPER_STATUS;
  if (0 == (uart0ReceiveByte() & _BV(RXC0)))
    return PRINTER_STATUS_UNCONNECTED;

  return UDR0;
}

void
PRINTER_PSTR(const char *P)
{
  uint8_t ui8_1t, ui8_2t;
  for (ui8_1t=0, ui8_2t=pgm_read_byte(P);
       ui8_2t;
       ui8_2t=pgm_read_byte(((const uint8_t *)P)+ ++ui8_1t)) {
    PRINTER_PRINT(ui8_2t);
  }
}
