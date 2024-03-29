#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include "lcd.c"
#include "i2c.c"

#define LCD_uint8x(ch) {			\
  uint8_t ui2_a = (ch>>4) & 0xF;		\
  ui2_a = ((ui2_a>9) ? 'A'-10 : '0') + ui2_a;	\
  LCD_wrchar(ui2_a);				\
  ui2_a = ch & 0xF;				\
  ui2_a = ((ui2_a>9) ? 'A'-10 : '0') + ui2_a;	\
  LCD_wrchar(ui2_a);				\
}

keyHitData_t keyHitData;

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
  uint8_t kbdData = 0;

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

  /* Scan keypad */
  DDRC  &= ~0x3C; /* input */
  PORTC |= 0x3C; /* pull high */
  DDRA  |= 0xF0; /* output */
  PORTA &= ~0xF0;/* drive 0 */
  PORTA |= 0xE0;
  _delay_ms(1);
  if (0x3C != (PINC & 0x3C)) {
    kbdData = 1;
    kbdData += (0 == (PINC & 0x4)) ? 0 :
      (0 == (PINC & 0x8)) ? 0x4 :
      (0 == (PINC & 0x10)) ? 0x8 : 0xC;
  } else {
    PORTA &= ~0xF0; PORTA |= 0xD0;
    _delay_ms(1);
    if (0x3C != (PINC & 0x3C)) {
      kbdData = 2;
      kbdData += (0 == (PINC & 0x4)) ? 0 :
	(0 == (PINC & 0x8)) ? 0x4 :
	(0 == (PINC & 0x10)) ? 0x8 : 0xC;
    } else {
      PORTA &= ~0xF0; PORTA |= 0xB0;
      _delay_ms(1);
      if (0x3C != (PINC & 0x3C)) {
	kbdData = 3;
	kbdData += (0 == (PINC & 0x4)) ? 0 :
	  (0 == (PINC & 0x8)) ? 0x4 :
	  (0 == (PINC & 0x10)) ? 0x8 : 0xC;
      } else {
	PORTA &= ~0xF0;	PORTA |= 0x70;
	_delay_ms(1);
	if (0x3C != (PINC & 0x3C)) {
	  kbdData = 4;
	  kbdData += (0 == (PINC & 0x4)) ? 0 :
	    (0 == (PINC & 0x8)) ? 0x4 :
	    (0 == (PINC & 0x10)) ? 0x8 : 0xC;
	} else {
	  kbdData = 0;
	}
      }
    }
  }

  /* next callback in 0.5s */
  TCNT1 = 0xFFFF - (F_CPU>>11);
  TIMSK |= (1 << TOIE1); /* enable Timer1 overflow */

  /* first time any key is hit */
  if (0 == keyHitData._kbdData) {
    keyHitData._kbdData = kbdData;
    keyHitData.count = 1;
  } else if (keyHitData._kbdData != kbdData) {
    /* diff key, miss any previous key that's pending */
    keyHitData._kbdData = kbdData;
    keyHitData.count = 1;
  } else {
    /* same key was hit */
    keyHitData.count ++;
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
  static uint8_t iter = 0;

  /* key hit */
  if (keyHitData.count) {
    keyHitData.KbdDataAvail = 0;
    if (LCD_WAS_ON && (keyHitData.count < (KCHAR_ROWS<<1))) {
      keyHitData.count--;
      keyHitData.KbdData = (keyHitData._kbdData < 10) ? '0' : ('A'-10);
      keyHitData.KbdData += keyHitData._kbdData;
      keyHitData.KbdDataAvail = 1;
    }
    keyHitData._kbdData = 0;
    keyHitData.count = 0;
    iter = 0;
  }

  /* Wait for 5 secs to disable timer permanently
     assume we need to get call next */
  iter++;
  TCNT1 = 0xFFFF - (F_CPU>>11);
  LCD_bl_on;
  LCD_cmd((LCD_CMD_CUR_10 + 14));
  LCD_uint8x(iter);
  if (iter > 10) {
    TIMSK &= ~(1 << TOIE1); /* disable Timer1 overflow */
    LCD_bl_off;
    iter = 0;
  }
}

#define KBD_GETCH				\
  while (KBD_NOT_HIT) {				\
    /* put the device to sleep */		\
    sleep_enable();				\
    sleep_cpu();				\
    /* some event has to occur to come here */	\
    sleep_disable();				\
  }						\
  LCD_bl_on

/* setup timer 2 : need to get 5 sec pulse
   # cycles to skip : (5*F_CPU)
   # clock div is 1024, so we need to skip : (5*F_CPU)>>10
*/
ISR(TIMER2_OVF_vect)
{
  static uint16_t timer2_beats=0;

  timer2_beats++;
  if (timer2_beats < ((5*F_CPU)>>10))
    return;
  timer2_beats = 0;

  LCD_init();
  LCD_refresh();
}


int
main()
{
  uint8_t ui8_1, ui8_2;

  LCD_init();
  LCD_bl_on;
  LCD_CLRLINE(0);
  LCD_WR_P(PSTR("Kbd Testing"));

  keyHitData.KbdData = keyHitData.KbdDataAvail = 0;
  keyHitData._kbdData = keyHitData.count = 0;

  /* sets the direction register of the PORTD */ 
  DDRD |= 0x60;

  cli();

  /* Reset state
     Port A   : output  (drive 0)
     Port C   : input   (pull high)
     Port B.2 : input   (pull high)
   */
  PORTA |= 0x00;  /* pullup */
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
  TIMSK |= (1 << TOIE1); /* enable Timer1 overflow */

  GICR |= (1<<INT2);

  /* setup timer 2 : need to get 5 sec pulse
     # cycles to skip : (5*F_CPU)
     # clock div is 1024, so we need to skip : (5*F_CPU)>>10
   */
  TCCR2 |= (0x7 << CS20);
  TCNT2 = 0;
  TIMSK |= (1 << TOIE2);

  sei();

  /* alert */
  PORTD |= 0x40;
  _delay_ms(30);
  PORTD &= ~0x40;

  ui8_1=0;
  for (ui8_2=0; ; ui8_2++) {
    KBD_GETCH;
    LCD_CLRLINE(0);
    LCD_WR_P(PSTR("Kbd Testing"));
    LCD_PUTCH(keyHitData.KbdData);
    LCD_CLRLINE(1);
    LCD_PUT_UINT8X(ui8_2);
    LCD_PUTCH(' ');
    LCD_PUT_UINT8X(keyHitData.KbdData);
    LCD_PUTCH(':');
    LCD_PUT_UINT8X(keyHitData.count);
    LCD_PUTCH(':');
    LCD_PUT_UINT8X(keyHitData._kbdData);
    KBD_RESET_KEY;
  }

  return 0;
}
