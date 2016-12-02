#include <stdint.h>
#include <ctype.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "lcd.c"
#include "i2c.c"
#include "kbd.c"

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
main()
{
  uint8_t ui8_1, ui8_2;

  LCD_init();
  LCD_bl_on;
  LCD_CLRLINE(0);
  LCD_WR_P(PSTR("Kbd Testing"));

  /* */
  kbdInit();
  sei();

  /* alert */
  BUZZER_ON;
  _delay_ms(30);
  BUZZER_OFF;

  /* */
  ui8_1=0;
  LCD_CLRLINE(0);
  LCD_WR_P(PSTR("Kbd Testing:"));
  for (ui8_2=0; ; ui8_2++) {
    LCD_cmd((LCD_CMD_CUR_10|0xD));
    KBD_RESET_KEY;
    KBD_GETCH;
    LCD_CLRLINE(1);
    if (isprint(keyHitData.KbdData)) {
      LCD_PUTCH(keyHitData.KbdData);
    } else {
      LCD_PUTCH('!');
      LCD_PUTCH('@');
    }
    LCD_PUT_UINT8X(ui8_2);
    LCD_PUTCH(' ');
    LCD_PUT_UINT8X(keyHitData.KbdData);
    LCD_PUTCH(':');
    LCD_PUT_UINT8X(keyHitData.KbdDataAvail);
    LCD_PUTCH(':');
    LCD_PUT_UINT8X(keyHitData._kbdData);
    LCD_PUTCH(':');
    LCD_PUT_UINT8X(keyHitData.count);
  }

  return 0;
}
