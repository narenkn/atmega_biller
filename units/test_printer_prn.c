#include <stdio.h>
#include <stdint.h>
#include "common_incl.c"

uint16_t ui16_1;

char golden_str[]="745.65\n123.45\n439.81\n0.0\n42949672.95\n";

int
main()
{
  ui16_1 = 0;

  printerInit();

  PRINTER_PRINT_F(0x12345); PRINTER_PRINT('\n');
  PRINTER_PRINT_F(12345); PRINTER_PRINT('\n');
  PRINTER_PRINT_F(0xabcd); PRINTER_PRINT('\n');
  PRINTER_PRINT_F(0); PRINTER_PRINT('\n');
  PRINTER_PRINT_F(-1); PRINTER_PRINT('\n');

  printerCompareStatus(golden_str);

  return 0;
}
