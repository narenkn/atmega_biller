
#undef LCD_USE_FUNCTIONS
#define LCD_USE_FUNCTIONS 0
#include "lcd.c"

int
main()
{
  LCD_init();

  DDRD |= 0x10 ; LCD_bl_on;
  LCD_WR_LINE(0, 0, "Hello World 7");
  LCD_refresh();
  _delay_ms(1);
  LCD_WR_LINE(1, 0, "Hello World 8");
  LCD_refresh();
  _delay_ms(1);

  //Infinite loop
  uint16_t ui16_1;
  for (ui16_1=0; ; ui16_1+=50) {
    LCD_bl_on;
    _delay_us(ui16_1);
    LCD_bl_off;
    _delay_us(ui16_1);
    if (ui16_1 > 1000)
      ui16_1 = 0;
  }

  return 0;
}
