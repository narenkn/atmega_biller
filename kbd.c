#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "kbd.h"

volatile uint8_t KbdData;
volatile uint8_t KbdDataAvail;

#define KBD_RISE_DELAY(N) \
  _delay_us(N)

const uint8_t
keyChars[] PROGMEM = {
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

uint8_t kbdStatus = 0;
#define ps2ShiftHit (1<<0)
#define ps2CtrlHit  (1<<1)
#define ps2AltHit   (1<<2)
#define ps2CapsHit  (1<<3)
#define LENOF_DR 3
uint8_t kbdDr[LENOF_DR];
uint8_t kbdTransL = 0;
uint8_t _KbdData = 0;

volatile struct { 
  uint8_t kbdData;
  uint8_t _kbdData;
  uint8_t count;
} keyHitData;

void
KbdInit(void)
{
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

  /* No data yet */
  KbdData = 0xFF;
  KbdDataAvail = 0;
  keyHitData.kbdData = keyHitData.count = keyHitData._kbdData = 0;

  /* setup timer 1 */
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0;
  // Set CS10 bit so timer runs at clock speed:
  TCCR1B |= (0x5 << CS10);
  //
  TCNT1 = 0xFFFF - (F_CPU>>11);

  GICR |= (1<<INT2);
}

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

uint8_t
KbdIsShiftPressed(void)
{
  uint8_t shift;

  shift = (kbdStatus & ps2ShiftHit) ? KBD_SHIFT : 0;

  KBD_R1_EN;
  KBD_RISE_DELAY(0x4);
  if (KBD_C3_VAL==0) { shift = KBD_SHIFT; }
  KBD_NODRIVE;

  return shift;
}

const uint8_t
ps2code2ascii[] PROGMEM = {
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, '`', ASCII_UNDEF, /* 0-15 */
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, 'q', '1', ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, 'z', 's', 'a', 'w', '2', ASCII_UNDEF, /* 16-31 */
  ASCII_UNDEF, 'c', 'x', 'd', 'e', '4', '3', ASCII_UNDEF, ' ', ASCII_UNDEF, 'v', 'f', 't', 'r', '5', ASCII_UNDEF, /* 32-47 */
  ASCII_UNDEF, 'n', 'b', 'h', 'g', 'y', '6', ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, 'm', 'j', 'u', '7', '8', ASCII_UNDEF, /* 48-63 */
  ASCII_UNDEF, ',', 'k', 'i', 'o', '0', '9', ASCII_UNDEF, ASCII_UNDEF, '.', '/', 'l', ';', 'p', '-', ASCII_UNDEF, /* 64-79 */
  ASCII_UNDEF, ASCII_UNDEF, '\'', ASCII_UNDEF, '[', '=', ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_ENTER, ']', ASCII_UNDEF, '\\', ASCII_UNDEF, ASCII_UNDEF, /* 80-95 */
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_BACKSPACE, ASCII_UNDEF, ASCII_UNDEF, '1', ASCII_UNDEF, '4', '7', ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, /* 96-111 */
  '0', '.', '2', '5', '6', '8', ASCII_ESCAPE, ASCII_NUMLK, ASCII_UNDEF, '+', '3', '-', '*', '9', ASCII_PRNSCRN, ASCII_UNDEF, /* 112-127 */
};

const uint8_t
ps2code2asciiE0[] PROGMEM = {
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, /* 0-15 */
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, 'Q', '!', ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, 'Z', 'S', 'A', 'W', '@', ASCII_UNDEF, /* 16-31 */
  ASCII_UNDEF, 'C', 'X', 'D', 'E', '$', '#', ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, 'V', 'F', 'T', 'R', '%', ASCII_UNDEF, /* 32-47 */
  ASCII_UNDEF, 'N', 'B', 'H', 'G', 'Y', '^', ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, 'M', 'J', 'U', '&', '*', ASCII_UNDEF, /* 48-63 */
  ASCII_UNDEF, '<', 'K', 'I', 'O', ')', '(', ASCII_UNDEF, ASCII_UNDEF, '>', '/', 'L', ':', 'P', '_', ASCII_UNDEF, /* 64-79 */
  ASCII_UNDEF, ASCII_UNDEF, '"', ASCII_UNDEF, '{', '+', ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_ENTER, '}', ASCII_UNDEF, '|', ASCII_UNDEF, ASCII_UNDEF, /* 80-95 */
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_LEFT, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, /* 96-111 */
  ASCII_UNDEF, ASCII_LEFT, ASCII_DOWN, ASCII_UNDEF, ASCII_RIGHT, ASCII_UP, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, /* 112-127 */
};

ISR(INT0_vect)
{             /* Data come with Clock from Device to MCU together */
  uint8_t KeyData, bitC = 0, drC;
  uint16_t data;

  /* ------------------------------------- */
  for (bitC=0; bitC<11; bitC++) {
    if (0 == bitC) {
      KeyData = 0;
      drC = 0;
      kbdTransL = 2;
    } else if (2 == bitC) {
      /* FIXME: Check parity and blink an LED */
      break;
    } else if (10 == bitC) {
      //    bitC = 0;
      break;
    } else {
      KeyData >>= 1;
      KeyData |= (((uint8_t)KBD_PS2_DATA)<<7);
      data <<= 1;
      data |= KBD_PS2_DATA;
    }
    /* Wait for +ve edge */
    while (0 == KBD_PS2_CLK_NS) {};
    /* Wait for -ve edge */
    while (0 != KBD_PS2_CLK_NS) {};
  }
  LCD_POS(1, 0);
  LCD_PUT_UINT8X(bitC);

  if (drC < LENOF_DR) {
    kbdDr[drC] = KeyData;
  }
  drC++;
  LCD_POS(1, 2);
  LCD_PUT_UINT8X(drC);
 
//  /* --------------------------------------- */
//  if (kbdDr[0] == 0xE0) {
//    kbdTransL = 2;    /* E0 XX */
//    if (kbdDr[1] == 0x12) {    /* E0 12 E0 7C */
//      kbdTransL = 4;
//    }
//    if (kbdDr[1] == 0xF0) {
//      kbdTransL = 3;    /* E0 F0 XX */
//      if (kbdDr[2] == 0x7C) {    /* E0 F0 7C E0 F0 12 */
//	kbdTransL = 6;
//      }
//    }
//  } else if (kbdDr[0] == 0xF0) {
//    kbdTransL = 2;    /* F0 XX */
//    if (2 == drC) { /* Break of normal keys */
//      if ((0x12 == kbdDr[0]) || (0x59 == kbdDr[0]))
//	kbdStatus &= ~ps2ShiftHit;
//      else if (0x14 == kbdDr[0])
//	kbdStatus &= ~ps2CtrlHit;
//      else if (0x11 == kbdDr[0])
//	kbdStatus &= ~ps2AltHit;
//      else {
//	KeyData = pgm_read_byte(&(ps2code2ascii[kbdDr[0]]));
//	if (ASCII_NUMLK == KeyData) {
//	  /* FIXME: Switch TOGGLE the light */
//	} else if (ASCII_UNDEF != KeyData) {
//	  KbdData = KeyData;
//	  KbdDataAvail = 1;
//	}
//      }
//    }
//  } else if (0xE1 == kbdDr[0]) {
//    kbdTransL = 8;
//  } else {
//    /* Make code received, generally no action except for sticky keys */
//    kbdTransL = 1;
//    if ((0x12 == kbdDr[0]) || (0x59 == kbdDr[0]))
//      kbdStatus |= ps2ShiftHit;
//    else if (0x14 == kbdDr[0])
//      kbdStatus |= ps2CtrlHit;
//    else if (0x11 == kbdDr[0])
//      kbdStatus |= ps2AltHit;
//    /* else if (0xFA == kbdDr[0]) ACKNOWLEDGEMENT FROM KBD */
//  }
//
//  /* --------------------------------------- */
//  if (drC == kbdTransL) {
//    if (3 == drC) {
//      KbdData = pgm_read_byte(&(ps2code2ascii[KeyData]));
//      KbdDataAvail = 1;
//    }
//    drC = 0;
//  }
}

void
KbdGetCh()
{
  while (KBD_NOT_HIT) {
    
  }
}
