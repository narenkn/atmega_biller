#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/crc16.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>

#include "ep_ds.h"
#include "version.h"
#include "assert.h"
#include "lcd.h"
#include "kbd.h"
#include "ep_store.h"
#include "billing.h"
#include "i2c.h"
#include "uart.h"
#include "a1micro2mm.h"
#include "menu.h"
#include "main.h"

#define KBD_RISE_DELAY(N)			\
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

const uint8_t
keyMap[] PROGMEM = {
  1,           2,            3,        ASCII_ESCAPE,
  4,           5,            6,        ASCII_LGUI,
  7,           8,            9,        ASCII_SHIFT,
  ASCII_LEFT,  0,  ASCII_RIGHT,        ASCII_ENTER
};

const uint8_t
keyMapLGui[] PROGMEM = {
  ASCII_F1,    ASCII_F2,     ASCII_F3,    ASCII_DEL,
  ASCII_F4,    ASCII_F5,     ASCII_F6,    ASCII_UP,
  ASCII_F7,    ASCII_F8,     ASCII_F9,    ASCII_DOWN,
  ASCII_F10,   ASCII_F11,    ASCII_F12,   ASCII_PRNSCRN
};

keyHitData_t keyHitData;

#if defined (__AVR_ATmega32__)
/* Known device */
#elif defined (__AVR_ATxmega64A1__) || defined (__AVR_ATxmega64A3__) || defined (__AVR_ATxmega64D3__)
# error "Not a known device"
#elif defined (__AVR_ATxmega128A1__) || defined (__AVR_ATxmega128A3__) || defined (__AVR_ATxmega128D3__) || defined (__AVR_ATmega1284P__)
#define TIMSK  TIMSK1
#define MCUCR  EICRA
#define GICR   EIMSK
#elif defined (__AVR_ATxmega256A3__) || defined (__AVR_ATxmega256A3B__) || defined (__AVR_ATxmega256D3__)
# error "Not a known device"
#endif

void
KbdInit(void)
{
  /* Reset state
     Port A   : output  (drive 0)
     Port C   : input   (pull high)
     Port B.2 : input   (pull high)
   */
  KBD_NODRIVE;

  /* PS2 IO */
  KBD_IO_INIT;

  /* No data yet */
  KBD_RESET_KEY;

  /* setup timer 1 */
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0;
  // Set CS10 bit so timer runs at clock speed:
  TCCR1B |= (0x5 << CS10);
  //
  TCNT1 = 0xFFFF - (F_CPU>>11);
  TIMSK |= (1 << TOIE1); /* enable Timer1 overflow */

  /* */
  EICRB |= 1<<ISC51 | 0<<ISC50;
  EICRB |= 1<<ISC61 | 0<<ISC60;
  EICRB |= 1<<ISC71 | 0<<ISC70;
  MCUSR |= 0x80;

  /* when sleep get to powerdown mode */
  set_sleep_mode(2);

  /* */
  kbdStatus = 0, bitC = 0, drC = 0;
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
  uint8_t kbdData = 0;

  /* get rid of spurious spikes */
  _delay_ms(5);
  if KBD_INT_PIN_VAL return;
  _delay_ms(5);
  if KBD_INT_PIN_VAL return;

  /* detected a key press */
  if (EEPROM_SETTING0(BUZZER))
    BUZZER_ON;
  KBD_INT_PIN_DR0;

  /* Scan keypad */
  KBD_IPIN_PHI;
  KBD_OPIN_DR0;
  KBD_OPIN_DR_R0_0;
  _delay_ms(1);
  if (KBD_IPIN_ANY_HIT) {
    kbdData = 1;
    kbdData += KBD_IPIN_PIN0_HIT ? 0 :
      KBD_IPIN_PIN1_HIT ? 0x4 :
      KBD_IPIN_PIN2_HIT ? 0x8 : 0xC;
  } else {
    KBD_OPIN_DR_R1_0;
    _delay_ms(1);
    if (KBD_IPIN_ANY_HIT) {
      kbdData = 2;
      kbdData += KBD_IPIN_PIN0_HIT ? 0 :
	KBD_IPIN_PIN1_HIT ? 0x4 :
	KBD_IPIN_PIN2_HIT ? 0x8 : 0xC;
    } else {
	KBD_OPIN_DR_R2_0;
      _delay_ms(1);
      if (KBD_IPIN_ANY_HIT) {
	kbdData = 3;
	kbdData += KBD_IPIN_PIN0_HIT ? 0 :
	  KBD_IPIN_PIN1_HIT ? 0x4 :
	  KBD_IPIN_PIN2_HIT ? 0x8 : 0xC;
      } else {
	KBD_OPIN_DR_R3_0;
	_delay_ms(1);
	if (KBD_IPIN_ANY_HIT) {
	  kbdData = 4;
	  kbdData += KBD_IPIN_PIN0_HIT ? 0 :
	    KBD_IPIN_PIN1_HIT ? 0x4 :
	    KBD_IPIN_PIN2_HIT ? 0x8 : 0xC;
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
  KBD_NODRIVE;
  _delay_ms(2);

  /* debounce */
  while (0 == KBD_INT_PIN_VAL) {}
  _delay_ms(5);
  while (0 == KBD_INT_PIN_VAL) {}
  _delay_ms(5);

  /* action before last */
  BUZZER_OFF;
  _delay_ms(5);
}

/* need call at every 500 ms
   clock is at 8MHz, timer1 clock is prescaled by 1024
   (F_CPU/2*1024)
 */
static uint8_t timer1_iter = 0;
ISR(TIMER1_OVF_vect)
{
  /* key hit */
  if ((keyHitData.count) && KBD_NOT_HIT) {
    keyHitData.KbdData = 0;
    if ( LCD_WAS_ON && (keyHitData.count < (KCHAR_ROWS<<1)) ) {
      keyHitData.count--;
      keyHitData.count %= KCHAR_COLS;
      keyHitData._kbdData--;
      keyHitData._kbdData &= 0xF;
      keyHitData.KbdData = pgm_read_byte(keyMap+keyHitData._kbdData);
      keyHitData.KbdDataAvail |= KBD_HIT_BIT;
      if (keyHitData.KbdDataAvail & KBD_LWIN_BIT) {
	keyHitData.KbdData = pgm_read_byte(keyMapLGui+keyHitData._kbdData);
	keyHitData.KbdDataAvail |= KBD_HIT_BIT;
      } else if (keyHitData.KbdData < 10) {
	keyHitData.KbdData = pgm_read_byte(keyChars+((keyHitData.KbdData*KCHAR_COLS) + keyHitData.count + ((keyHitData.KbdDataAvail & KBD_SHIFT_BIT)*5) ));
      } else if (ASCII_LGUI == keyHitData.KbdData) {
	keyHitData.KbdDataAvail |= KBD_LWIN_BIT;
	keyHitData.KbdDataAvail &= ~KBD_HIT_BIT; /* disable this key now */
      } else if (ASCII_SHIFT == keyHitData.KbdData) {
	keyHitData.KbdDataAvail |= KBD_SHIFT_BIT;
	keyHitData.KbdDataAvail &= ~KBD_HIT_BIT; /* disable this key now */
      }
    }
    keyHitData._kbdData = 0;
    keyHitData.count = 0;
    timer1_iter = 0;
  } else if (keyHitData.KbdDataAvail & KBD_HIT_BIT) {
    timer1_iter = 0;
  }

  /* Wait for 5 secs to disable timer permanently
     assume we need to get call next */
  timer1_iter++;
  TCNT1 = 0xFFFF - (F_CPU>>11);
  LCD_bl_on;
//  LCD_cmd(LCD_CMD_CUR_10 + 14);
//  LCD_uint8x(timer1_iter);
  if (timer1_iter > 10) {
    TIMSK &= ~(1 << TOIE1); /* disable Timer1 overflow */
    LCD_bl_off;
    timer1_iter = 0;
  }
}

#define PS2CODE2ASCII_MASK  0x7F

const uint8_t
ps2code2ascii[] PROGMEM = {
  ASCII_UNDEF, ASCII_F9, ASCII_UNDEF, ASCII_F5, ASCII_F3, ASCII_F1, ASCII_F2, ASCII_F12, ASCII_UNDEF, ASCII_F10, ASCII_F8, ASCII_F6, ASCII_F4, ASCII_UNDEF, '`', ASCII_UNDEF, /* 0-15 */
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, 'q', '1', ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, 'z', 's', 'a', 'w', '2', ASCII_UNDEF, /* 16-31 */
  ASCII_UNDEF, 'c', 'x', 'd', 'e', '4', '3', ASCII_UNDEF, ' ', ASCII_UNDEF, 'v', 'f', 't', 'r', '5', ASCII_UNDEF, /* 32-47 */
  ASCII_UNDEF, 'n', 'b', 'h', 'g', 'y', '6', ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, 'm', 'j', 'u', '7', '8', ASCII_UNDEF, /* 48-63 */
  ASCII_UNDEF, ',', 'k', 'i', 'o', '0', '9', ASCII_UNDEF, ASCII_UNDEF, '.', '/', 'l', ';', 'p', '-', ASCII_UNDEF, /* 64-79 */
  ASCII_UNDEF, ASCII_UNDEF, '\'', ASCII_UNDEF, '[', '=', ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_ENTER, ']', ASCII_UNDEF, '\\', ASCII_UNDEF, ASCII_UNDEF, /* 80-95 */
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_BACKSPACE, ASCII_UNDEF, ASCII_UNDEF, '1', ASCII_UNDEF, '4', '7', ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, /* 96-111 */
  '0', '.', '2', '5', '6', '8', ASCII_ESCAPE, ASCII_NUMLK, ASCII_F11, '+', '3', '-', '*', '9', ASCII_PRNSCRN, ASCII_UNDEF, /* 112-127 */
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
  ASCII_UNDEF, ASCII_DEL, ASCII_DOWN, ASCII_UNDEF, ASCII_RIGHT, ASCII_UP, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, /* 112-127 */
};

uint8_t kbdStatus;
#define ps2ShiftHit (1<<0)
#define ps2CtrlHit  (1<<1)
#define ps2AltHit   (1<<2)
#define ps2CapsHit  (1<<3)

#define LENOF_DR    4
uint8_t kbdDr[LENOF_DR];
uint8_t KeyData, bitC, drC;
ISR(INT0_vect)
{             /* Data come with Clock from Device to MCU together */
  static uint8_t kbdTransL = 1;
  /* ------------------------------------- */
  bitC++;
  if (1 == bitC) {
    KeyData = 0;
  } else if (10 == bitC) {
    /* FIXME: Check parity and blink an LED */
  } else if (bitC >= 11) {
    bitC = 0;
  } else {
    KeyData >>= 1;
    KeyData |= (((uint8_t)KBD_PS2_DATA)<<7);
  }

  if (0 != bitC)
    return;
  if (drC < LENOF_DR) {
    kbdDr[drC] = KeyData;
  }
  drC++;
 
  /* --------------------------------------- */
  if (kbdDr[0] == 0xE0) {
    kbdTransL = 2;    /* E0 XX */
    if (kbdDr[1] == 0x12) {    /* E0 12 E0 7C */
      kbdTransL = 4;
    }
    if (kbdDr[1] == 0xF0) {
      kbdTransL = 3;    /* E0 F0 XX */
      if (kbdDr[2] == 0x7C) {    /* E0 F0 7C E0 F0 12 */
	kbdTransL = 6;
      } else {
	KeyData = pgm_read_byte(&(ps2code2asciiE0[kbdDr[2]&PS2CODE2ASCII_MASK]));
	if ((ASCII_UNDEF != KeyData) && KBD_NOT_HIT && LCD_WAS_ON) {
	  keyHitData.KbdData = KeyData;
	  keyHitData.KbdDataAvail |= KBD_HIT_BIT;
	  drC = 3; /* make sure == kbdTransL */
	}
      }
    }
  } else if (kbdDr[0] == 0xF0) {
    kbdTransL = 2;    /* F0 XX */
    if (2 == drC) { /* Break of normal keys */
      if ((0x12 == kbdDr[1]) || (0x59 == kbdDr[1]))
	kbdStatus &= ~ps2ShiftHit;
      else if (0x14 == kbdDr[1])
	kbdStatus &= ~ps2CtrlHit;
      else if (0x11 == kbdDr[1])
	kbdStatus &= ~ps2AltHit;
      else {
	KeyData = (kbdStatus & ps2ShiftHit) ?
	  pgm_read_byte(&(ps2code2asciiE0[kbdDr[1]&PS2CODE2ASCII_MASK])) :
	  pgm_read_byte(&(ps2code2ascii[kbdDr[1]&PS2CODE2ASCII_MASK]));
	/* temporary fix for code not in table */
	if (ASCII_UNDEF == KeyData) {
	  if (0x83 == kbdDr[1])
	    KeyData = ASCII_F7;
	}
	if (ASCII_NUMLK == KeyData) {
	  /* FIXME: Switch TOGGLE the light */
	} else if ((ASCII_UNDEF != KeyData) && KBD_NOT_HIT && LCD_WAS_ON) {
	  keyHitData.KbdData = KeyData;
	  keyHitData.KbdDataAvail |= KBD_HIT_BIT;
	}
      }
    }
  } else if (0xE1 == kbdDr[0]) {
    kbdTransL = 8;
  } else {
    /* Make code received, generally no action except for sticky keys */
    kbdTransL = 1;
    if ((0x12 == kbdDr[0]) || (0x59 == kbdDr[0]))
      kbdStatus |= ps2ShiftHit;
    else if (0x14 == kbdDr[0])
      kbdStatus |= ps2CtrlHit;
    else if (0x11 == kbdDr[0])
      kbdStatus |= ps2AltHit;
    /* if (0xFA == kbdDr[0]) ACKNOWLEDGEMENT FROM KBD */
  }

  /* Get ready for new trans */
  if (drC == kbdTransL) {
    if ( ((2 == kbdTransL) && (0xF0 == kbdDr[0])) || /* skip make codes */
	 ((3 == kbdTransL) && (0xF0 == kbdDr[1]) && (0xE0 == (kbdDr[0]&0xF0))) ) {
      kbdTransL = 1;
      TCNT1 = 0xFFFF - (F_CPU>>11);
      TIMSK |= (1 << TOIE1); /* enable Timer1 overflow */
      timer1_iter = 0;
    }
    for (drC=0; drC<LENOF_DR; drC++)
      kbdDr[drC] = 0;
    drC = 0;
  }
}
