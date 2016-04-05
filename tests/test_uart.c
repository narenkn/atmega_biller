#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <ctype.h>

#include "lcd.c"
#include "uart.c"

int
main()
{
  LCD_init();

  LCD_bl_on;
  LCD_CLRLINE(0);
  LCD_WR_P(PSTR("UART Testing:"));
  LCD_refresh();
  _delay_ms(1000);
  uartInit();

  DDRD |= 0x60;
  PORTD |= 0<<5;

  uint8_t ui1, ui2;
  while (1) {
    LCD_CLRLINE(0);
    for (ui1=0; ui1<LCD_MAX_COL; ui1++) {
      ui2 = uartReceiveByte();

      if (isgraph(ui2)) {
	LCD_PUTCH(ui2);
	LCD_refresh();
      } else {
	LCD_PUTCH('N');
	LCD_PUTCH('a');
	LCD_PUTCH('C');
	ui1 += 2;
      }
//      uartTransmitByte('a'+ui1);
//      uartTransmitByte('\r');
//      uartTransmitByte('\n');
      _delay_ms(1000);
    }
  }

  //Infinite loop
  while(1);

  return 0;
}
