
#define assert(x)

#include "lcd.c"
#include "kbd.c"

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
  uint16_t ui1;

  LCD_init();
  KbdInit();

  LCD_WriteDirect(LCD_CMD_CUR_10, "Starting!!!", 11);
  _delay_ms(2000);

  LCD_WriteDirect(LCD_CMD_CUR_10, "Press any key!!!", 16);
  LCD_WriteDirect(LCD_CMD_CUR_20, "Shall displayed!", 16);
  KBD_RESET_KEY;
  while (1) {
    KBD_GET_KEY;
    LCD_WriteDirect(LCD_CMD_CUR_20, "Scan Code : ", 11);
    LCD_PUT_UINT8X(KbdData);
    for (ui1=0; ui1<0xFF; ui1++)
      {}
    KBD_RESET_KEY;
  }

  return 0;
}
