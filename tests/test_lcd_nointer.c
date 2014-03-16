
#include "lcd.c"

#define LCD_WriteDirect(loc, str, len) do {	\
  uint8_t _ui1;					\
  LCD_cmd(loc);					\
  for (_ui1=0; _ui1<len; _ui1++) {		\
    LCD_wrchar( (((char *)str)+len)[0] );	\
  }						\
  } while (0)

int
main(void)
{
  uint8_t ui1;

  LCD_init();
  _delay_ms(2);

  LCD_WriteDirect(LCD_CMD_CUR_10, "Hello World 7", 13);
  LCD_WriteDirect(LCD_CMD_CUR_20, "Hello World 8", 13);
  _delay_ms(2000);

  LCD_WriteDirect(LCD_CMD_CUR_10, "Hello World 8", 13);
  LCD_WriteDirect(LCD_CMD_CUR_20, "Hello World 9", 13);
  _delay_ms(2000);

  LCD_WriteDirect(LCD_CMD_CUR_10, "Hello World A", 13);
  LCD_WriteDirect(LCD_CMD_CUR_20, "Hello World B", 13);
  _delay_ms(2000);
}
