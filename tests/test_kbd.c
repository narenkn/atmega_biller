#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define assert(x)

#include "lcd.c"

volatile uint8_t KbdData, pinc;

ISR(INT1_vect)
{
  /* Start Buzz */ 
  PORTD |= 0x40;
  PORTC &= ~0x3C; /* disable pullup */
  _delay_ms(5);
  pinc = PINC;

  /* */
  DDRA  &= ~0xF0; DDRA |= 0x10;
  PORTA &= ~0xF0; PORTA |= 0xE0;
  KbdData = 0x0;
  _delay_ms(1);
  if (0x3C != (PINC & 0x3C)) {
    KbdData |= (~(PINC >> 2)) & 0xF;
  } else {
    DDRA  &= ~0xF0; DDRA |= 0x20;
    PORTA &= ~0xF0; PORTA |= 0xD0;
    KbdData = 0x10;
    _delay_ms(1);
    if (0x3C != (PINC & 0x3C)) {
      KbdData |= (~(PINC >> 2)) & 0xF;
    } else {
      DDRA  &= ~0xF0; DDRA |= 0x40;
      PORTA &= ~0xF0; PORTA |= 0xB0;
      KbdData = 0x20;
      _delay_ms(1);
      if (0x3C != (PINC & 0x3C)) {
	KbdData |= (~(PINC >> 2)) & 0xF;
      } else {
	DDRA  &= ~0xF0; DDRA |= 0x80;
	PORTA &= ~0xF0;	PORTA |= 0x70;
	KbdData = 0x30;
	_delay_ms(1);
	if (0x3C != (PINC & 0x3C)) {
	  KbdData |= (~(PINC >> 2)) & 0xF;
	} else {
	  KbdData = 0x7F;
	}
      }
    }
  }
  KbdData |= (PIND << 4) & 0x80;

  /* */
  PORTA &= ~0xF0;  /* drive 0 */
  DDRA |= 0xF0;  /* out */
  PORTC |= 0x3C; /* enable pullup */
  _delay_ms(1);
  pinc = PINC;
  while (0x3C != (PINC & 0x3C)) { } /* debounce */

  /* Stop Buzz */
  _delay_ms(10);
  PORTD &= ~0x40;
}

int
main()
{
  uint8_t ui8_1, ui8_2, ui8_3;

  LCD_init();
  LCD_bl_on;
  LCD_WR_LINE(0, 0, "Kbd Testing");
  LCD_refresh();

  /* sets the direction register of the PORTD */ 
  DDRD |= 0x60;

  /* Reset state
     Port A   : output  (drive 0)
     Port C   : input   (pull high)
     Port D.3 : input   (pull high)
   */
  PORTA &= ~0xF0;  /* drive 0 */
  PORTC |= 0x3C; /* pullup */
  PORTD |= 0x08;  /* pullup */
  DDRA |= 0xF0;  /* out */
  DDRC &= ~0x3C;   /* in */
  DDRD &= ~0x08;  /* in */

//  MCUCR |= (1<<ISC11);
//  MCUCR &= ~(1<<ISC10);
  MCUCR &= ~0xC;
  MCUCR |= 0x8;
  GICR |= (1<<INT1);
  sei();

  PORTD |= 0x40;
  _delay_ms(30);
  PORTD &= ~0x40;

  KbdData = 0;
  
  for (ui8_1=0, ui8_2=0; ; ui8_2++) {
    if (0 != KbdData) {
      LCD_POS(1, 0);
      LCD_PUT_UINT8X(ui8_1);
      LCD_PUTCH(':');
      LCD_PUT_UINT8X(pinc);
      LCD_PUTCH(':');
      LCD_PUT_UINT8X(KbdData);
      LCD_refresh();
      KbdData = 0;
      ui8_1++;
    }
    if (0 == ui8_2) {
      ui8_3 = PINC;
      LCD_POS(1, 8);
      LCD_PUTCH(':');
      LCD_PUT_UINT8X(ui8_3);
      LCD_refresh();
    }
  }

  return 0;
}

