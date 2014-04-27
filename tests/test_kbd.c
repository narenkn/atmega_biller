
#define assert(x)

#include "lcd.c"
#include "kbd.c"

int
main(void)
{
  uint16_t ui1;

  LCD_init();
  KbdInit();

  LCD_busy;
  LCD_CLRSCR;

  LCD_WR_LINE(0, 0, "Starting!!!");
  for (ui1=0; ui1<0x3F; ui1++)
    LCD_busy;

  LCD_WR_LINE(0, 0, "Press any key!!!");
  LCD_WR_LINE(1, 0, "Shall displayed!");
  KBD_RESET_KEY;
  while (1) {
    KBD_GET_KEY;
    LCD_POS(1, 0);
    LCD_WR("Scan Code : ");
    LCD_PUT_UINT8X(KbdData);
    for (ui1=0; ui1<0xFF; ui1++)
      {}
    KBD_RESET_KEY;
  }

  return 0;
}
