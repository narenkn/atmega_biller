#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "uart.c"
#include "lcd.c"
#include "menu.c"

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

int
main(void)
{
  uint8_t ui1;

  LCD_init();

  uartInit();
  uartSelect(0);

  /* Set pin mode & enable pullup */
  DDRD &= ~((1<<PD2)|(1<<PD3));
//  PORTD |= (1<<PD2) | (1<<PD3);
  DDRD |= 0x10;
  PORTD |= 2<<5;

  /* Enable Int0 on falling edge */
  GICR = 1<<INT0;
  MCUCR |= 1<<ISC01 | 0<<ISC00;

  /* Enable Global Interrupts */
  sei();

  PORTD |= 0x10;
  LCD_WriteDirect(LCD_CMD_CUR_10, "Printer: ", 9);
  _delay_ms(1000);

  for (ui1=0; ; ui1++) {
    LCD_cmd((LCD_CMD_CUR_10+9));
    _delay_ms(100);
  }

  return 0;
}
