#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#define assert(x)

#include "lcd.c"
#include "i2c.c"
#include "kbd.c"

volatile uint8_t eeprom_setting0=0;

int
main()
{
  uint8_t ui8_1, ui8_2, ui8_3;

  LCD_init();
  LCD_bl_on;
  LCD_WR_LINE(0, 0, "Kbd Testing");
  LCD_refresh();
  KbdInit();

  /* sets the direction register of the PORTD */ 
  DDRD |= 0x80;
  BUZZER_OFF;
  EEPROM_SETTING0_ON(BUZZER);

  sei();

  /* */
  ui8_1=0;
  for (ui8_2=0; ; ui8_2++) {
    KBD_RESET_KEY;
    KBD_GETCH;
    LCD_WR_LINE(0, 0, "Kbd Testing");
    LCD_POS(0, 12);
    if (keyHitData.KbdData <= '~') {
      LCD_PUTCH(keyHitData.KbdData);
    } else if (ASCII_LEFT == keyHitData.KbdData) {
      LCD_WR_LINE_NP(0, 11, PSTR("LEFT"), 4);
    } else if (ASCII_RIGHT == keyHitData.KbdData) {
      LCD_WR_LINE_NP(0, 11, PSTR("RIGT"), 4);
    } else if (ASCII_UP == keyHitData.KbdData) {
      LCD_WR_LINE_NP(0, 11, PSTR("UP  "), 4);
    } else if (ASCII_DOWN == keyHitData.KbdData) {
      LCD_WR_LINE_NP(0, 11, PSTR("DOWN"), 4);
    } else if (ASCII_PRNSCRN == keyHitData.KbdData) {
      LCD_WR_LINE_NP(0, 11, PSTR("PRSC"), 4);
    } else if (ASCII_ENTER == keyHitData.KbdData) {
      LCD_WR_LINE_NP(0, 11, PSTR("ENTR"), 4);
    } else {
      LCD_WR_LINE_NP(0, 11, PSTR("UNKN"), 4);
    }
    LCD_POS(1, 0);
    LCD_refresh();
  }

  return 0;
}
