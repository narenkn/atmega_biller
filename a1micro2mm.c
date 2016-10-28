#include <stdint.h>
#include <avr/pgmspace.h>

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
  if ('P' != uart0ReceiveByte())
    return 0;
  if ('1' != uart0ReceiveByte())
    return 0;
  if ('V' != uart0ReceiveByte())
    return 0;
  uart0ReceiveByte();
  uart0ReceiveByte();
  if ('T' != uart0ReceiveByte())
    return 0;
  ui8_1 = uart0ReceiveByte() * 10;
  ui8_1 += uart0ReceiveByte();

  return ui8_1;
}

void
PRINTER_FEED_DOTS(uint8_t n)
{
  uart0TransmitByte(ASCII_PRINTER_ESC);
  uart0TransmitByte('J');
  uart0TransmitByte(n);
}

void
PRINTER_FEED_LINES(uint8_t n)
{
  uart0TransmitByte(ASCII_PRINTER_ESC);
  uart0TransmitByte('d');
  uart0TransmitByte(n);
}

void
PRINTER_FONT_ENLARGE(uint8_t N)
{
  uart0TransmitByte(0x1D);
  uart0TransmitByte('!');
  uart0TransmitByte(N);
}

void
PRINTER_BOLD(uint8_t N)
{
  uart0TransmitByte(ASCII_PRINTER_ESC);
  uart0TransmitByte('!');
  uart0TransmitByte(N);
}

void
PRINTER_UNDERLINE(uint8_t N)
{
  uart0TransmitByte(ASCII_PRINTER_ESC);
  uart0TransmitByte('-');
  uart0TransmitByte(N);
}

void
PRINTER_USERCHAR_ENDIS(uint8_t N)
{
  uart0TransmitByte(ASCII_PRINTER_ESC);
  uart0TransmitByte('%');
  uart0TransmitByte(N);
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
