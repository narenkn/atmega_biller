
#define assert(x)

#include "lcd.c"
#include "main.c"

int
main()
{
  LCD_init();
  main_init();

  LCD_bl_on;
  LCD_WR_LINE(0, 0, "Hello World 7");
  _delay_ms(50000);
  LCD_WR_LINE(1, 0, "Hello World 8");
  _delay_ms(50000);

  //Infinite loop
  while(1);

  return 0;
}
