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

uint8_t otext[16] = "Hello PC";
uint8_t itext[16];

int
main()
{
  _delay_ms(1000);
  LCD_init();
  LCD_bl_on;
  uint8_t ui1=0, ui2;

  LCD_CLRLINE(0);
  LCD_WR_P(PSTR("UART-PC Testing:"));
  _delay_ms(1000);
  uartInit();

  _delay_ms(1000);

#if 1 // Send
  LCD_CLRLINE(0);
  LCD_WR_P(PSTR("Send Testing!"));
  for (ui1=0; otext[ui1]; ui1++) {
    uart1TransmitByte(otext[ui1]);
  }
  uart1TransmitByte('\n');
  LCD_CLRLINE(0);
  LCD_WR_P(PSTR("Sent "));
  LCD_PUT_UINT(ui1);
  LCD_WR_P(PSTR(" bytes"));
#endif

#if 0 // Receive String
  cli();
  LCD_CLRLINE(1);
  LCD_WR_P(PSTR("Receive Test!"));
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

#if 1 // Receive weight
  set_sleep_mode(SLEEP_MODE_IDLE);

  uint32_t weight = uartWeight;
  uint32_t decimal = uartDecimalPlace;
  sei();
  while (1) {
    /* wait for a char */
    LCD_CLRLINE(1);
    LCD_PUT_UINT(weight);
    LCD_PUTCH(' ');
    LCD_PUT_UINT(decimal);
    do {
      sleep_enable();
      sleep_cpu();
      sleep_disable();
    } while ((weight == uartWeight) && (decimal == uartDecimalPlace));
    weight = uartWeight;
    decimal = uartDecimalPlace;
  }
#endif

  //Infinite loop
  while(1);

  return 0;
}
