#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define assert(x)

#include "lcd.c"

volatile struct { 
  uint8_t kbdData;
  uint8_t _kbdData;
  uint8_t count;
} keyHitData;

volatile uint8_t KbdData, KbdDataAvail;

const uint8_t
keyChars[] = {
  /* KCHAR_ROWS x KCHAR_COLS */
  '0', ' ', '.', ',', ')', '+', '?', '_', ':',
  '1', 'a', 'b', 'c', '!', 'A', 'B', 'C', '~',
  '2', 'd', 'e', 'f', '@', 'D', 'E', 'F', '{',
  '3', 'g', 'h', 'i', '#', 'G', 'H', 'I', '}',
  '4', 'j', 'k', 'l', '$', 'J', 'K', 'L', '[',
  '5', 'm', 'n', 'o', '%', 'M', 'N', 'O', ']',
  '6', 'p', 'q', 'r', '^', 'P', 'Q', 'R', '|',
  '7', 's', 't', 'u', '&', 'S', 'T', 'U', '/',
  '8', 'v', 'w', 'x', '*', 'V', 'W', 'X', '<',
  '9', 'y', 'z', '(', '-', 'Y', 'Z', '=', '>',
};

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
  TIMSK &= ~(1 << TOIE1); /* disable Timer1 overflow */
  PORTD |= 0x40;  /* Start Buzzer */
  PORTB &= ~0x04; DDRB  |= 0x04;  /* drive 0 */

  /* Scan 1 */
  DDRC  &= ~0x3C; /* input */
  PORTC |= 0x3C; /* pull high */
  DDRA  |= 0xF0; /* output */
  PORTA &= ~0xF0;/* drive 0 */
  PORTA |= 0xE0;
  _delay_ms(1);
  if (0x3C != (PINC & 0x3C)) {
    keyHitData.kbdData = 1;
    keyHitData.kbdData += (0 == (PINC & 0x4)) ? 0 :
      (0 == (PINC & 0x8)) ? 0x4 :
      (0 == (PINC & 0x10)) ? 0x8 : 0xC;
  } else {
    PORTA &= ~0xF0; PORTA |= 0xD0;
    _delay_ms(1);
    if (0x3C != (PINC & 0x3C)) {
      keyHitData.kbdData = 2;
      keyHitData.kbdData += (0 == (PINC & 0x4)) ? 0 :
	(0 == (PINC & 0x8)) ? 0x4 :
	(0 == (PINC & 0x10)) ? 0x8 : 0xC;
    } else {
      PORTA &= ~0xF0; PORTA |= 0xB0;
      _delay_ms(1);
      if (0x3C != (PINC & 0x3C)) {
	keyHitData.kbdData = 3;
	keyHitData.kbdData += (0 == (PINC & 0x4)) ? 0 :
	  (0 == (PINC & 0x8)) ? 0x4 :
	  (0 == (PINC & 0x10)) ? 0x8 : 0xC;
      } else {
	PORTA &= ~0xF0;	PORTA |= 0x70;
	_delay_ms(1);
	if (0x3C != (PINC & 0x3C)) {
	  keyHitData.kbdData = 4;
	  keyHitData.kbdData += (0 == (PINC & 0x4)) ? 0 :
	    (0 == (PINC & 0x8)) ? 0x4 :
	    (0 == (PINC & 0x10)) ? 0x8 : 0xC;
	} else {
	  keyHitData.kbdData = 0;
	}
      }
    }
  }

  /* when previous key hit was not yet consumed
     miss this key...
   */
  if ((0 == KbdDataAvail) && (0 != keyHitData.kbdData)) {
    TCNT1 = 0xFFFF - (F_CPU>>11); /* next callback in 0.5s */
    TIMSK = (1 << TOIE1); /* enable Timer1 overflow */

    if (0 == keyHitData._kbdData) {
      /* first time this key is hit */
      keyHitData._kbdData = keyHitData.kbdData;
      keyHitData.count = 1;
      keyHitData.kbdData = 0;
    } else if (keyHitData._kbdData != keyHitData.kbdData) {
      /* diff key was hit */
      assert(keyHitData.count > 0);
      keyHitData.count--;
      KbdData = keyChars[keyHitData._kbdData - 1 + keyHitData.count]; /* FIXME : mul with KCHAR_COLS */
      KbdDataAvail = 1;
      keyHitData._kbdData = keyHitData.kbdData;
      keyHitData.kbdData = 0;
      keyHitData.count = 1;
    } else {
      /* same key was hit */
      keyHitData.count ++;
      keyHitData._kbdData = keyHitData.kbdData;
      keyHitData.kbdData = 0;
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
}

/* need call at every 500 ms
   clock is at 8MHz, timer1 clock is prescaled by 1024
   (F_CPU/2*1024)
 */
ISR(TIMER1_OVF_vect)
{
  TIMSK &= ~(1 << TOIE1); /* disable Timer1 overflow */

  /* key hit stopped */
  assert(keyHitData.count > 0);
  keyHitData.count--;
  KbdData = keyChars[keyHitData._kbdData - 1 + keyHitData.count]; /* FIXME : mul with KCHAR_COLS */
  KbdDataAvail = 1;
  keyHitData._kbdData = 0;
  keyHitData.kbdData = 0;
  keyHitData.count = 0;
}

int
main()
{
  uint8_t ui8_1, ui8_2, ui8_3;

  LCD_init();
  LCD_bl_on;
  LCD_WR_LINE(0, 0, "Kbd Testing");
  LCD_refresh();

  KbdData = 0;
  KbdDataAvail = 0;
  keyHitData._kbdData = keyHitData.kbdData = keyHitData.count = 0;

  /* sets the direction register of the PORTD */ 
  DDRD |= 0x60;

  cli();

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

  /* setup timer 1 */
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0;
  // Set CS10 bit so timer runs at clock speed:
  TCCR1B |= (0x5 << CS10);
  //
  TCNT1 = 0xFFFF - (F_CPU>>11);

  GICR |= (1<<INT2);
  sei();

  /* alert */
  PORTD |= 0x40;
  _delay_ms(30);
  PORTD &= ~0x40;

  ui8_1=0;
  for (ui8_2=0; ; ui8_2++) {
    if (KbdDataAvail) {
      LCD_POS(0, 12);
      LCD_PUTCH(KbdData);
      KbdDataAvail = 0;
    }
    if (0 == ui8_2) {
      LCD_POS(1, 0);
      LCD_PUT_UINT8X(keyHitData.kbdData);
      LCD_PUTCH(':');
      LCD_PUT_UINT8X(keyHitData.count);
      LCD_PUTCH(':');
      LCD_PUT_UINT8X(keyHitData._kbdData);
      LCD_refresh();
    }
  }

  return 0;
}
