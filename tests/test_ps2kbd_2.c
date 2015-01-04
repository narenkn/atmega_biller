#include <stdint.h>
#include <ctype.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "lcd.c"
#include "i2c.c"

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

#define eeprom_setting0 1
//#define LENOF_DR    4
//volatile uint8_t kbdDr[LENOF_DR];
//volatile uint8_t KeyData, bitC=0, drC=0;
#include "kbd.c"

int
main(void)
{
  uint8_t ui1;

  LCD_init();
  KbdInit();

  /* Enable Global Interrupts */
  sei();

  LCD_bl_on;
  LCD_WR_LINE_NP(0, 0, PSTR("PS2 Kbd: "), 9);
  LCD_refresh();
  _delay_ms(1000);

  for (ui1=0; ; ui1++) {
    if (KBD_HIT) {
      LCD_POS(0, 9);
      LCD_PUTCH(KbdData);
      KBD_RESET_KEY;
    }
//    LCD_POS(0, 11);
//    LCD_PUT_UINT8X(drC);
    LCD_POS(0, 14);
    LCD_PUT_UINT8X(ui1);
//    LCD_POS(1, 0);
//    LCD_PUT_UINT8X(bitC);
//    LCD_PUT_UINT8X(kbdDr[0]);
//    LCD_PUT_UINT8X(kbdDr[1]);
//    LCD_PUT_UINT8X(kbdDr[2]);
//    LCD_PUT_UINT8X(kbdDr[3]);
    LCD_refresh();
    _delay_ms(500);
  }

  return 0;
}
