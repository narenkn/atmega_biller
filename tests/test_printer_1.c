#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "uart.c"
#include "lcd.c"
#include "i2c.c"
#include "kbd.c"

#define LCD_WriteDirect(loc, str, len) do {	\
  uint8_t _ui1;					\
  LCD_cmd(loc);					\
  for (_ui1=0; _ui1<len; _ui1++) {		\
    LCD_wrchar( ((char *)str)[_ui1] );		\
  }						\
} while (0)

#define LCD_uint8x(ch) {			\
  uint8_t ui2_a = (ch>>4) & 0xF;		\
  ui2_a = ((ui2_a>9) ? 'A'-10 : '0') + ui2_a;	\
  LCD_wrchar(ui2_a);				\
  ui2_a = ch & 0xF;				\
  ui2_a = ((ui2_a>9) ? 'A'-10 : '0') + ui2_a;	\
  LCD_wrchar(ui2_a);				\
}

volatile uint16_t timer2_beats;
volatile uint8_t eeprom_setting0, eeprom_setting1;
volatile uint8_t menuPendActs;

int
main(void)
{
  uint8_t ui1;

  LCD_init();
  uartInit();
  kbdInit();

  /* Enable Global Interrupts */
  sei();

  LCD_bl_on;
  LCD_CLRLINE(0);
  LCD_WR_P(PSTR("Live Prn Test: "));
  _delay_ms(1000);

  for (ui1=0; ; ui1++) {
    KBD_RESET_KEY;
    KBD_GETCH;

    LCD_CLRLINE(1);
    if (isprint(keyHitData.KbdData)) {
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
    _delay_ms(100);
  }
  LCD_bl_off;

  return 0;
}
