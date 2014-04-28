
#define assert(x)

#include "lcd.c"
#include "kbd.c"
#include "main.c"

int
main(void)
{
  uint16_t ui1;

  LCD_init();
  KbdInit();
  main_init();

  LCD_bl_on;
  LCD_WR_LINE(0, 0, "Starting!!!");
  _delay_ms(5000);

  LCD_WR_LINE(0, 0, "Press any key!!!");
  LCD_WR_LINE(1, 0, "Shall displayed!");
  KBD_RESET_KEY;
  while (1) {
    ui1++;
//    KBD_GET_KEY;
    KbdScan();
    if KBD_HIT {
      LCD_WR_LINE(1, 0, "Scan Code : ");
      lcd_buf_p = &(lcd_buf[1][12]);
      LCD_PUT_UINT8X(KbdData);
      _delay_ms(10000);
      KBD_RESET_KEY;
    }
    lcd_buf_p = &(lcd_buf[0][8]);
    LCD_PUT_UINT8X(ui1);

    _delay_us(10000);
  }

  while(1) {}

  return 0;
}
