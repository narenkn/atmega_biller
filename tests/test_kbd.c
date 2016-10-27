#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "lcd.c"
#include "i2c.c"
#include "kbd.c"

volatile uint8_t eeprom_setting0=0;

#undef KBD_GETCH
#define KBD_GETCH				\
  while (KBD_NOT_HIT) {				\
    /* put the device to sleep */		\
    sleep_enable();				\
    sleep_cpu();				\
    /* some event has to occur to come here */	\
    sleep_disable();				\
  }						\
  LCD_bl_on

int
main()
{
  uint8_t ui8_1, ui8_2;

  LCD_init();
  LCD_bl_on;
  LCD_CLRLINE(0);
  LCD_WR_P(PSTR("Kbd Testing"));
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
    LCD_CLRLINE(0);
    LCD_WR_P(PSTR("Kbd Testing:"));
    if (keyHitData.KbdData <= '~') {
      LCD_PUTCH(keyHitData.KbdData);
    } else if (ASCII_LEFT == keyHitData.KbdData) {
      LCD_WR_NP(PSTR("LEFT"), 4);
    } else if (ASCII_RIGHT == keyHitData.KbdData) {
      LCD_WR_NP(PSTR("RIGT"), 4);
    } else if (ASCII_UP == keyHitData.KbdData) {
      LCD_WR_NP(PSTR("UP  "), 4);
    } else if (ASCII_DOWN == keyHitData.KbdData) {
      LCD_WR_NP(PSTR("DOWN"), 4);
    } else if (ASCII_PRNSCRN == keyHitData.KbdData) {
      LCD_WR_NP(PSTR("PRSC"), 4);
    } else if (ASCII_ENTER == keyHitData.KbdData) {
      LCD_WR_NP(PSTR("ENTR"), 4);
    } else {
      LCD_WR_NP(PSTR("UNKN"), 4);
    }
    LCD_refresh();
  }

  return 0;
}
