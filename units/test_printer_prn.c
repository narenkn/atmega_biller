#include <stdio.h>
#include <stdint.h>

#define PRINTER_PRINT(a)   putchar(a)
#define PRINTER_PRINT_D menuPrnD
#define PRINTER_PRINT_F menuPrnF

void
menuPrnD(uint32_t var)
{
  if (var>9)
    menuPrnD(var/10);
  PRINTER_PRINT('0'+(var%10));
}

void
menuPrnF(uint32_t var)
{
  menuPrnD(var/100);
  PRINTER_PRINT('.');
  menuPrnD(var%100);
}

int
main()
{
  PRINTER_PRINT_F(0x12345); PRINTER_PRINT('\n');
  PRINTER_PRINT_F(12345); PRINTER_PRINT('\n');
  PRINTER_PRINT_F(0xabcd); PRINTER_PRINT('\n');
  PRINTER_PRINT_F(0); PRINTER_PRINT('\n');
  PRINTER_PRINT_F(-1); PRINTER_PRINT('\n');

  return 0;
}
