
#define assert(x)

#include "lcd.c"
//#include "kbd.c"
#include "main.c"

int
main(void)
{
  uint8_t KbdData, KbdDataAvail;
#define KBD_RESET_KEY KbdDataAvail = 0
#define KBD_HIT (0 != KbdDataAvail)
  uint16_t ui1, ui2, ui3, ui4, ui5;

  _delay_ms(500);
  LCD_init();
//  KbdInit();
  KbdData = 0;
  KbdDataAvail = 0;
  main_init();

//  SFIOR |= 4;

  LCD_bl_on;
  LCD_WR_LINE(0, 0, "Starting!!!");
  _delay_ms(5000);

  LCD_WR_LINE(0, 0, "Press any key!!!");
  LCD_WR_LINE(1, 0, "Shall displayed!");
  _delay_ms(1000);
  KBD_RESET_KEY;
  while (1) {
    ui1++;
//    KBD_GET_KEY;
//    KbdScan();

    /* C is output, A is input */
    PORTA |= 0xF0;
    PORTC &= ~0x3C;
    for(ui2=0;ui2<4;ui2++) {
      DDRA &= ~(0xF0);
      DDRC &= ~(0x3C);

      DDRC |= (0x20>>ui2);
      for(ui3=0;ui3<4;ui3++) {
	if(!(PINA & (0x80>>ui3))) {
	  KbdData = (ui3*3)+ui2;
	  KbdDataAvail = 1;
	  goto out;
	}
      }
    }
    out:

    if KBD_HIT {
      lcd_buf_p = &(lcd_buf[1][0]);
      LCD_PUT_UINT8X(KbdData);
      KBD_RESET_KEY;
    }
    lcd_buf_p = &(lcd_buf[0][8]);
    LCD_PUT_UINT8X(ui1);

    _delay_ms(1000);
  }

  while(1) {}

  return 0;
}
