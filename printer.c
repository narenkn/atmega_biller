#include <stdint.h>
#include <p89v51rd2.h>

#include "printer.h"

void
printer_init(void)
{
  /* Turn ON */
  PRINTER_COMMAND(PRINTER_TOGGLE_ONOFF);
  PRINTER_CMD_ARG(1);

  /* init */
  PRINTER_COMMAND(PRINTER_INITIALIZE);
}

void
printer_prn_date(uint8_t *ymd)
{
    PRINTER_PRINT('0' + (ymd[0]>>4));
    PRINTER_PRINT('0' + (ymd[0]&0xF));
    PRINTER_PRINT('/');
    PRINTER_PRINT('0' + (ymd[1]>>4));
    PRINTER_PRINT('0' + (ymd[1]&0xF));
    //    PRINTER_PRINT('/');
    //    PRINTER_PRINT('0' + (ymd[2]>>4));
    //    PRINTER_PRINT('0' + (ymd[2]&0xF));
}

void
printer_prn_uint16(uint16_t ui1)
{
  uint8_t ui2;

  ui2 = (ui1>>12)&0xF;
  if (ui2) {
    PRINTER_PRINT('0'+ui2);
  }
  ui2 = (ui1>>8)&0xF;
  if (ui2) {
    PRINTER_PRINT('0'+ui2);
  }
  ui2 = (ui1>>4)&0xF;
  if (ui2) {
    PRINTER_PRINT('0'+ui2);
  }
  PRINTER_PRINT('0'+(ui1&0xF));
}

void
printer_prn_str(uint8_t *str)
{
  for (; 0 != str[0]; str+=1) {
    PRINTER_PRINT(str[0]);
  }
}
