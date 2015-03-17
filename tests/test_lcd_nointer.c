
#include "lcd.c"
#include "i2c.c"

#define LCD_WriteDirect(loc, str, len) do {	\
  uint8_t _ui1;					\
  LCD_cmd(loc);					\
  for (_ui1=0; _ui1<len; _ui1++) {		\
    LCD_wrchar( ((char *)str)[_ui1] );		\
  }						\
} while (0)

int
main(void)
{
  LCD_init();

  PORTD = 0x10;
  LCD_WriteDirect(LCD_CMD_CUR_10, "Hello World 7", 13);
  LCD_WriteDirect(LCD_CMD_CUR_20, "Hello World 8", 13);
  _delay_ms(1000);

  LCD_WriteDirect(LCD_CMD_CUR_10, "Hello World 8", 13);
  LCD_WriteDirect(LCD_CMD_CUR_20, "Hello World 9", 13);
  _delay_ms(1000);

  LCD_WriteDirect(LCD_CMD_CUR_10, "Hello World A", 13);
  LCD_WriteDirect(LCD_CMD_CUR_20, "Hello World B", 13);
  _delay_ms(1000);

  LCD_CLRLINE(1);
  LCD_WR_P(PSTR("Hello World 10"));
  LCD_refresh();
  _delay_ms(1000);

  while (1) {}

  PORTD = 0x0;

  return 0;
}
