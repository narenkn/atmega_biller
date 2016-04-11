#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <ctype.h>

#include "lcd.c"
#include "uart.c"

int
main()
{
  LCD_init();
  LCD_bl_on;
  uint8_t ui1=0, ui2;

  LCD_CLRLINE(0);
  LCD_WR_P(PSTR("UART Testing:"));
  LCD_refresh();
  _delay_ms(1000);
  uartInit();

  uartSelect(0);
  _delay_ms(1000);

#if 1
  while (1) {
  uartTransmitByte('H');
  //  _delay_ms(10);
  uartTransmitByte('e');
  //  _delay_ms(10);
  uartTransmitByte('l');
  //  _delay_ms(10);
  uartTransmitByte('l');
  //  _delay_ms(10);
  uartTransmitByte('o');
  //  _delay_ms(10);
  uartTransmitByte(' ');
  }
#endif

#if 0
  set_sleep_mode(SLEEP_MODE_IDLE);
  sei();
  LCD_CLRLINE(1);
  LCD_CLRLINE(0);
  while (1) {
    LCD_cmd(LCD_CMD_CUR_10);
    for (ui1=0; ui1<LCD_MAX_COL; ui1++) {

      sleep_mode();

      ui2 = ReceivedByte;

      if (isgraph(ui2)) {
	LCD_PUTCH(ui2);
      } else {
	LCD_PUTCH('~');
      }
    }
    LCD_cmd(LCD_CMD_CUR_20);
    for (ui1=0; ui1<LCD_MAX_COL; ui1++) {

      sleep_mode();

      ui2 = ReceivedByte;

      if (isgraph(ui2)) {
	LCD_PUTCH(ui2);
      } else {
	LCD_PUTCH('~');
      }
    }
  }
#endif

  //Infinite loop
  while(1);

  return 0;
}
