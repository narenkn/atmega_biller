
#define assert(x)

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "lcd.c"
#include "uart.c"

int
main()
{
  LCD_init();

  LCD_bl_on;
  LCD_WR_LINE(0, 0, "UART Testing:");
  LCD_refresh();
  _delay_ms(1000);
  uart0_init();

  DDRD |= 0x60;
  PORTD |= 2<<5;

  uint8_t ui1, ui2;
  while (1) {
    for (ui1=0; ui1<26; ui1++) {
      ui2 = receiveByte();

      LCD_POS(1, 0);
//      LCD_PUTCH('a'+ui1);
      LCD_PUTCH(ui2);
      LCD_refresh();
      transmitByte(ui2);
      _delay_ms(1000);
    }
  }

  //Infinite loop
  while(1);

  return 0;
}
