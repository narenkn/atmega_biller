
#define assert(x)

#include "lcd.c"

int
main(void)
{
  uint16_t ui1;

  LCD_init();

  _delay_ms(2);
  LCD_CLRSCR;

  LCD_WR_LINE(0, 0, "Hello World 7");
  for (ui1=0; ui1<0x4FF; ui1++)
    _delay_ms(2);
  LCD_WR_LINE(1, 0, "Hello World 8");
  for (ui1=0; ui1<0x4FF; ui1++)
    _delay_ms(2);

  return 0;
}
