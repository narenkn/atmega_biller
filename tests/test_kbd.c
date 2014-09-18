#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define assert(x)

#include "lcd.c"

volatile struct { 
  volatile uint8_t kbdData;
  volatile uint8_t count;
  volatile uint8_t pinc;
} keyHitData;

/* At reset/ idle state
     Port A   : output   (pull high)
     Port C   : input  (drive 0)
     Port B.2 : input   (pull high)
   After key is hit two scans take place
   Scan 1:
     Port A   : input   (pull high)
     Port C   : output  (drive 0)
     Port B.2 : output  (drive 1)
   Scan 2: (not yet implemented)
     Port A   : output  (drive 0)
     Port C   : input   (pull high)
     Port B.2 : output  (drive 1)
 */
ISR(INT2_vect)
{
  /* get rid of spurious spikes */
  _delay_ms(5);
  if (PINB & 0x04) return;
  _delay_ms(5);
  if (PINB & 0x04) return;
  _delay_ms(5);
  if (PINB & 0x04) return;

  /* detected a key press */
  PORTD |= 0x40;  /* Start Buzzer */
  PORTB &= ~0x04; DDRB  |= 0x04;  /* drive 0 */

  /* Scan 1 */
  DDRC  &= ~0x3C; /* input */
  PORTC |= 0x3C; /* pull high */
  DDRA  |= 0xF0; /* output */
  PORTA &= ~0xF0;/* drive 0 */
  PORTA |= 0xE0;
  _delay_ms(1);
  keyHitData.pinc = (PINC>>2) & 0xF;
  if (0x3C != (PINC & 0x3C)) {
    keyHitData.kbdData = 0;
    keyHitData.kbdData += (0 == (PINC & 0x4)) ? 0 :
      (0 == (PINC & 0x8)) ? 0x4 :
      (0 == (PINC & 0x10)) ? 0x8 : 0xC;
  } else {
    PORTA &= ~0xF0; PORTA |= 0xD0;
    _delay_ms(1);
    if (0x3C != (PINC & 0x3C)) {
      keyHitData.kbdData = 1;
      keyHitData.kbdData += (0 == (PINC & 0x4)) ? 0 :
	(0 == (PINC & 0x8)) ? 0x4 :
	(0 == (PINC & 0x10)) ? 0x8 : 0xC;
    } else {
      PORTA &= ~0xF0; PORTA |= 0xB0;
      _delay_ms(1);
      if (0x3C != (PINC & 0x3C)) {
	keyHitData.kbdData = 2;
	keyHitData.kbdData += (0 == (PINC & 0x4)) ? 0 :
	  (0 == (PINC & 0x8)) ? 0x4 :
	  (0 == (PINC & 0x10)) ? 0x8 : 0xC;
      } else {
	PORTA &= ~0xF0;	PORTA |= 0x70;
	_delay_ms(1);
	if (0x3C != (PINC & 0x3C)) {
	  keyHitData.kbdData = 3;
	  keyHitData.kbdData += (0 == (PINC & 0x4)) ? 0 :
	    (0 == (PINC & 0x8)) ? 0x4 :
	    (0 == (PINC & 0x10)) ? 0x8 : 0xC;
	} else {
	  keyHitData.kbdData = 0;
	  keyHitData.count --;
	}
      }
    }
  }

  /* Back to idle state */
  PORTA |= 0xF0;  /* pullup */
  PORTC &= ~0x3C; /* drive 0 */
  PORTB |= 0x04;  /* pullup */
  DDRA &= ~0xF0;  /* in */
  DDRC |= 0x3C;   /* out */
  DDRB &= ~0x04;  /* in */
  _delay_ms(2);

  /* debounce */
  while (0 == (PINB & 0x04)) {}
  _delay_ms(5);
  while (0 == (PINB & 0x04)) {}
  _delay_ms(5);
  while (0 == (PINB & 0x04)) {}
  _delay_ms(5);

  /* Stop Buzz */
  PORTD &= ~0x40; /* stop buzzer */
  _delay_ms(25);

  /* */
  keyHitData.count ++;
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
     Port B.2 : input   (pull high)
   */
  PORTA |= 0xF0;  /* pullup */
  PORTC &= ~0x3C; /* drive 0 */
  PORTB |= 0x04;  /* pullup */
  DDRA &= ~0xF0;  /* in */
  DDRC |= 0x3C;   /* out */
  DDRB &= ~0x04;  /* in */

  GICR |= (1<<INT2);
  sei();

  /* alert */
  PORTD |= 0x40;
  _delay_ms(30);
  PORTD &= ~0x40;

  ui8_1=0;
  for (ui8_2=0; ; ui8_2++) {
    if (ui8_1 != keyHitData.count) {
      LCD_POS(1, 0);
      LCD_PUT_UINT8X(keyHitData.count);
      LCD_PUTCH(':');
      LCD_PUT_UINT8X(keyHitData.kbdData);
      LCD_refresh();
      keyHitData.kbdData = 0;
      ui8_1 = keyHitData.count;
    }
    if (0 == ui8_2) {
      LCD_POS(1, 6);
      LCD_PUTCH(':');
      LCD_PUT_UINT8X(keyHitData.pinc);
      LCD_POS(0, 14);
      LCD_PUT_UINT8X(keyHitData.count);
      LCD_refresh();
    }
  }

  return 0;
}
