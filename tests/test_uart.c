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

#define BUZZER_ON  PORTA |= 0x80
#define BUZZER_OFF PORTA &= ~0x80

int
main()
{
  LCD_init();
  LCD_bl_on;
  DDRA |= 0x80; /* buzzer */

  uint8_t ui1=0, ui2;

  uartInit();
  _delay_ms(1000);

#if 1 // For Printer
  LCD_CLRLINE(0);
  LCD_WR_P(PSTR("Printer Testing!"));
  LCD_CLRLINE(1);
  LCD_WR_P(PSTR("Sending HW..."));
  for (ui1=10; ui1; ui1--) {
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
    BUZZER_ON;
    _delay_ms(50);
    BUZZER_OFF;
    _delay_ms(1000);
  }
#endif

#if 1
  LCD_CLRLINE(0);
  LCD_WR_P(PSTR("Printer Status!"));
  uint8_t ui8_1, ui8_2;
  while (1) {
    uart0TransmitByte(27);
    uart0TransmitByte('v');
    uart0TransmitByte(0);
    for (ui8_1=8; ui8_1; ui8_1--) {
      if (UCSR0A & _BV(RXC0)) break;
    }
    LCD_CLRLINE(1);
    if (ui8_1) {
      ui8_2 = UDR0;
      LCD_PUT_UINT8X(ui8_2);
    } else {
      LCD_PUTCH('X');
      LCD_PUTCH('x');
    }
    BUZZER_ON;
    _delay_ms(50);
    BUZZER_OFF;
    _delay_ms(1000);
  }
#endif

  //Infinite loop
  while(1);

  return 0;
}
