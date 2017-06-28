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

  /* Enable Global Interrupts */
  sei();

  LCD_bl_on;
  DDRA |= 0x80;
  BUZZER_ON;
  _delay_ms(500);
  BUZZER_OFF;

  kbdInit();
  for (ui1=0; ; ui1++) {
    KBD_RESET_KEY;
    KBD_GETCH;
    if (KBD_HIT) {
      LCD_CLRLINE(0);
      LCD_WR_P(PSTR("PS2 Kbd: "));
      LCD_PUTCH(keyHitData.KbdData);
      LCD_PUTCH(':');
      _delay_ms(500);
    }
    LCD_PUT_UINT8X(kbd2.drC);
    LCD_PUT_UINT8X(ui1);
    LCD_CLRLINE(1);
#if 0
    LCD_PUT_UINT8X(kbd2.bitC);
    LCD_PUT_UINT8X(kbd2.kbdTransL);
    LCD_PUT_UINT8X(kbd2.kbdDr[0]);
    LCD_PUT_UINT8X(kbd2.kbdDr[1]);
    LCD_PUT_UINT8X(kbd2.kbdDr[2]);
    LCD_PUT_UINT8X(kbd2.kbdDr[3]);
#endif
    LCD_PUT_UINT8X(keyHitData.KbdDataAvail);
    LCD_PUT_UINT8X(keyHitData.hbCnt);
    LCD_PUT_UINT16X(keyHitData.hitBuf);
  }

  return 0;
}
