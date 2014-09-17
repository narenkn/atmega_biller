#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define assert(x)

#include "lcd.c"

struct { 
  uint8_t kbdData;
  uint8_t count;
} keyHitData;

/* At reset/ idle state
     Port A   : input   (pull high)
     Port C   : output  (drive 0)
     Port B.2 : input   (pull high)
   After key is hit two scans take place
   Scan 1:
     Port A   : input   (pull high)
     Port C   : output  (drive 0)
     Port B.2 : output  (drive 1)
   Scan 2:
     Port A   : output  (drive 0)
     Port C   : input   (pull high)
     Port B.2 : output  (drive 1)
 */
ISR(INT2_vect)
{
  keyHitData.count ++;

  /* get rid of spurious spikes */
  _delay_ms(5);
  if (PINB & 0x04) return;
  _delay_ms(5);
  if (PINB & 0x04) return;
  _delay_ms(5);
  if (PINB & 0x04) return;

  PORTD |= 0x40;  /* Start Buzzer */
//  PORTB &= ~0x04; DDRB  |= 0x04;  /* drive 0 */
  _delay_ms(5);

  /* Scan 1 */
  keyHitData.kbdData = 0x0;
  DDRC  &= ~0x3C; /* input */
  PORTC |= 0x3C; /* pull high */
  DDRA  |= 0xF0; /* output */
  PORTA &= ~0xF0;/* drive 0 */
  _delay_ms(1);
  if (0x3C != (PINC & 0x3C)) {
    keyHitData.kbdData |= (~(PINC >> 2)) & 0xF;
  } else {
    DDRA  &= ~0xF0; DDRA |= 0x20;
    PORTA &= ~0xF0; PORTA |= 0xD0;
    keyHitData.kbdData = 0x10;
    _delay_ms(1);
    if (0x3C != (PINC & 0x3C)) {
      keyHitData.kbdData |= (~(PINC >> 2)) & 0xF;
    } else {
      DDRA  &= ~0xF0; DDRA |= 0x40;
      PORTA &= ~0xF0; PORTA |= 0xB0;
      keyHitData.kbdData = 0x20;
      _delay_ms(1);
      if (0x3C != (PINC & 0x3C)) {
	keyHitData.kbdData |= (~(PINC >> 2)) & 0xF;
      } else {
	DDRA  &= ~0xF0; DDRA |= 0x80;
	PORTA &= ~0xF0;	PORTA |= 0x70;
	keyHitData.kbdData = 0x30;
	_delay_ms(1);
	if (0x3C != (PINC & 0x3C)) {
	  keyHitData.kbdData |= (~(PINC >> 2)) & 0xF;
	} else {
	  keyHitData.kbdData = 0;
	  keyHitData.count = 0;
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

  /* Stop Buzz */
  _delay_ms(25);
  PORTD &= ~0x40; /* stop buzzer */
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

  keyHitData.kbdData = 0;
  
  for (ui8_1=0, ui8_2=0; ; ui8_2++) {
    if (0 != keyHitData.kbdData) {
      LCD_POS(1, 0);
      LCD_PUT_UINT8X(ui8_1);
      LCD_PUTCH(':');
      LCD_PUT_UINT8X(keyHitData.kbdData);
      LCD_refresh();
      keyHitData.kbdData = 0;
      ui8_1++;
    }
    if (0 == ui8_2) {
      ui8_3 = PINC;
      LCD_POS(1, 6);
      LCD_PUTCH(':');
      LCD_PUT_UINT8X(ui8_3);
      LCD_POS(0, 14);
      LCD_PUT_UINT8X(keyHitData.count);
      LCD_refresh();
    }
  }

  return 0;
}

