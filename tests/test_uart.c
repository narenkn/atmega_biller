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

  _delay_ms(1000);

#if 1 // For Printer
  LCD_CLRLINE(0);
  LCD_WR_P(PSTR("Printer Testing!"));
  LCD_CLRLINE(1);
  LCD_WR_P(PSTR("Sending HW..."));
  while (1) {
    uart0TransmitByte('H');
    //  _delay_ms(10);
    uart0TransmitByte('e');
    //  _delay_ms(10);
    uart0TransmitByte('l');
    //  _delay_ms(10);
    uart0TransmitByte('l');
    //  _delay_ms(10);
    uart0TransmitByte('o');
    //  _delay_ms(10);
    uart0TransmitByte(' ');
    //  _delay_ms(10);
    uart0TransmitByte('W');
    //  _delay_ms(10);
    uart0TransmitByte('o');
    //  _delay_ms(10);
    uart0TransmitByte('r');
    //  _delay_ms(10);
    uart0TransmitByte('l');
    //  _delay_ms(10);
    uart0TransmitByte('d');
    //  _delay_ms(10);
    uart0TransmitByte('!');
    //  _delay_ms(10);
    uart0TransmitByte('\n');
  }
#endif

#if 1 // Weighing machine
  set_sleep_mode(SLEEP_MODE_IDLE);
  cli();
  LCD_CLRLINE(1);
  LCD_CLRLINE(0);
  while (1) {
    LCD_cmd(LCD_CMD_CUR_10);
    for (ui1=0; ui1<LCD_MAX_COL; ui1++) {

      while(!(UCSR1A & _BV(RXC1)));
      ui2 = UDR1;

      if (isgraph(ui2)) {
	LCD_PUTCH(ui2);
      } else {
	LCD_PUTCH('~');
      }
    }
    LCD_cmd(LCD_CMD_CUR_20);
    for (ui1=0; ui1<LCD_MAX_COL; ui1++) {

      while(!(UCSR1A & _BV(RXC1)));
      ui2 = UDR1;

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
