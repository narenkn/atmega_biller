
#define assert(x)

#include "lcd.c"

int
main()
{
  LCD_init();

  DDRD |= 0x10 ; LCD_bl_on;
  LCD_WR_LINE(0, 0, "Hello World 7");
  LCD_refresh();
  _delay_ms(5000);
  LCD_WR_LINE(1, 0, "Hello World 8");
  LCD_refresh();
  _delay_ms(5000);

  //Infinite loop
  while(1);

  return 0;
}
