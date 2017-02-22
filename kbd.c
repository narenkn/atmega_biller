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
#include "main.h"
#include "menu.h"

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
  1,           2,            3,        ASCII_ALT,
  4,           5,            6,        ASCII_LGUI,
  7,           8,            9,        ASCII_SHIFT,
  ASCII_LEFT,  0,  ASCII_RIGHT,        ASCII_ENTER
};

const uint8_t
keyMapR[] PROGMEM = {
  'N','u','1',  'N','u','2',  'N','u','3',  'A','l','t',
  'N','u','4',  'N','u','5',  'N','u','6',  'W','i','n',
  'N','u','7',  'N','u','8',  'N','u','9',  'S','h','f',
  ' ','<','-',  'N','u','0',  ' ','<','-',  '<','=','|'
};

const uint8_t
keyMapLGui[] PROGMEM = {
  ASCII_F1,    ASCII_F2,     ASCII_F3,    ASCII_UNDEF,
  ASCII_F4,    ASCII_F5,     ASCII_F6,    ASCII_UNDEF,
  ASCII_F7,    ASCII_F8,     ASCII_F9,    ASCII_UNDEF,
  ASCII_F10,   ASCII_F11,    ASCII_F12,   ASCII_PRNSCRN
};

const uint8_t
keyMapAlt[] PROGMEM = {
  ASCII_F1,    ASCII_F2,     ASCII_F3,    ASCII_UNDEF,
  ASCII_F4,    ASCII_F5,     ASCII_F6,    ASCII_UNDEF,
  ASCII_F7,    ASCII_F8,     ASCII_F9,    ASCII_UNDEF,
  ASCII_F10,   ASCII_F11,    ASCII_F12,   ASCII_PRNSCRN
};

volatile keyHitData_t keyHitData;
ps2LineStat_t kbd0, kbd1, kbd2;

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

#define TCNT1_DELAY (0xFFFF - (F_CPU>>11))

void
kbdInit(void)
{
  /* Reset state
     Port C[7:4]   : output  (drive 0)
     Port C[3:0]   : input   (pull high)
     Port E.4 : input   (pull high)
   */
  KBD_NODRIVE;

  /* PS2 IO */
  KBD_IO_INIT;

  /* No data yet */
  keyHitData.count = 0, keyHitData.hbCnt = 0, keyHitData._kbdDataAvail = 0;
  KBD_RESET_KEY;

  /* setup timer 1 */
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0;
  // Set CS10 bit so timer runs at clock speed:
  TCCR1B |= (0x5 << CS10);
  //
  TCNT1 = TCNT1_DELAY;
  TIMSK |= (1 << TOIE1); /* enable Timer1 overflow */

  /* */
  EICRB |= 1<<ISC41 | 0<<ISC40;
  EICRB |= 1<<ISC51 | 0<<ISC50;
  EICRB |= 1<<ISC61 | 0<<ISC60;
  EICRB |= 1<<ISC71 | 0<<ISC70;
  EIMSK |= _BV(INT4) | _BV(INT5) | _BV(INT6) | _BV(INT7);

  /* when sleep get to powerdown mode */
  set_sleep_mode(3);

  /* */
  kbd0.KeyData = 0, kbd0.bitC = 0, kbd0.drC = 0;
  kbd0.kbdStatus = 0, kbd0.kbdTransL = 1;
  kbd1.KeyData = 0, kbd1.bitC = 0, kbd1.drC = 0;
  kbd1.kbdStatus = 0, kbd1.kbdTransL = 1;
  kbd2.KeyData = 0, kbd2.bitC = 0, kbd2.drC = 0;
  kbd2.kbdStatus = 0, kbd2.kbdTransL = 1;
}

void
kbdPushKeyHit(uint8_t key, uint8_t avail)
{
  /* more key-press than we can process */
  if (keyHitData.hbCnt >= sizeof(keyHitData.hitBuf))
    return;

  if ((KBD_NOT_HIT) && (0 != avail)) {
    keyHitData.KbdData = key;
    keyHitData.KbdDataAvail = avail;
  } else if (0 != avail) {
    keyHitData.hbCnt++;
    keyHitData.hitBuf <<= 8; keyHitData.hitBuf |= key;
    keyHitData.availBuf <<= 8; keyHitData.availBuf |= avail;
  } else if ((0 == keyHitData.KbdDataAvail) && (0 != keyHitData.hbCnt)) {
    /* not sure if this is required ... */
    KBD_RESET_KEY;
  }
}

void
keypadPushHit()
{
  uint8_t key;

  /* swallow the first key when lcd was off */
  if ((!LCD_IS_ON) || (0 == keyHitData.count)) return;

  /* */
  keyHitData.count--;
  keyHitData.count %= KCHAR_COLS;
  keyHitData._kbdData &= 0xF;
  key = pgm_read_byte(keyMap+keyHitData._kbdData);
  if (ASCII_LGUI == key) {
    keyHitData._kbdDataAvail |= kbdWinHit;
    goto keypadPushHitRet;
  } else if (ASCII_SHIFT == key) {
    keyHitData._kbdDataAvail |= kbdShiftHit;
    goto keypadPushHitRet;
  } else if (ASCII_ALT == key) {
    keyHitData._kbdDataAvail |= kbdShiftHit;
    goto keypadPushHitRet;
  } else if (keyHitData._kbdDataAvail & kbdWinHit) {
    key = pgm_read_byte(keyMapLGui+keyHitData._kbdData);
  } else if (keyHitData._kbdDataAvail & kbdAltHit) {
    key = pgm_read_byte(keyMapAlt+keyHitData._kbdData);
  } else if (key < 10) {
    key = pgm_read_byte( keyChars+ ((key*KCHAR_COLS) + keyHitData.count + ((keyHitData._kbdDataAvail & kbdShiftHit)?KCHAR_SHIFT_SZ:0) ) );
  }
  kbdPushKeyHit(key, keyHitData._kbdDataAvail|kbdHit);
  keyHitData._kbdDataAvail = 0;

 keypadPushHitRet:
  keyHitData.count = 0;
}

/* At reset/ idle state
     Port C[7:4]   : input   (pull high)
     Port C[3:0]   : output  (drive 0)
     Port E.4      : input   (pull high)
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
ISR(INT4_vect)
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
  TCNT1 = TCNT1_DELAY;
  TIMSK |= (1 << TOIE1); /* enable Timer1 overflow */

  /* first time any key is hit */
  if (0 != kbdData) {
    kbdData--;
    if (0 == keyHitData.count) {
      keyHitData._kbdData = kbdData;
      keyHitData.count = 1;
    } else if (keyHitData._kbdData != kbdData) {
      /* diff key, miss any previous key that's pending */
      keypadPushHit();
      keyHitData._kbdData = kbdData;
      keyHitData.count = 1;
    } else {
      /* same key was hit */
      keyHitData.count ++;
    }
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
ISR(TIMER1_OVF_vect)
{
  BUZZER_ON;
  _delay_ms(10);
  BUZZER_OFF;
  /* key hit */
  keypadPushHit();
  kbdPushKeyHit(0, 0);
  /* */
  TCNT1 = TCNT1_DELAY;
  TIMSK |= (1 << TOIE1); /* enable Timer1 overflow */
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


static void
ps2Intr(ps2LineStat_t *kbd)
{
  uint8_t key = ASCII_UNDEF;

  /* Data come with Clock from Device to MCU together */
  /* ------------------------------------- */
  kbd->bitC++;
  if (1 == kbd->bitC) {
    kbd->KeyData = 0;
  } else if (10 == kbd->bitC) {
    /* FIXME: Check parity and blink an LED */
  } else if (kbd->bitC >= 11) {
    kbd->bitC = 0;
  } else {
    kbd->KeyData >>= 1;
    kbd->KeyData |= (((uint8_t)KBD0_PS2_DATA)<<7);
  }

  if (0 != kbd->bitC)
    return;
  if (kbd->drC < LENOF_DR) {
    kbd->kbdDr[kbd->drC] = kbd->KeyData;
  }
  kbd->drC++;
 
  /* --------------------------------------- */
  if (kbd->kbdDr[0] == 0xE0) {
    kbd->kbdTransL = 2;    /* E0 XX */
    if (kbd->kbdDr[1] == 0x12) {    /* E0 12 E0 7C */
      kbd->kbdTransL = 4;
    } else if (kbd->kbdDr[1] == 0x14) {
      kbd->kbdStatus |= kbdCtrlHit;
    } else if (kbd->kbdDr[1] == 0x11) {
      kbd->kbdStatus |= kbdAltHit;
    } else if ((kbd->kbdDr[1] == 0x1F) || (kbd->kbdDr[1] == 0x27)) {
      kbd->kbdStatus |= kbdWinHit;
    } else if (kbd->kbdDr[1] == 0xF0) {
      kbd->kbdTransL = 3;    /* E0 F0 XX */
      if (kbd->kbdDr[2] == 0x7C) {    /* E0 F0 7C E0 F0 12 */
	kbd->kbdTransL = 6;
	key = ASCII_PRNSCRN;
      } else if (kbd->kbdDr[2] == 0x14) {
	kbd->kbdStatus &= ~kbdCtrlHit;
      } else if (kbd->kbdDr[2] == 0x11) {
	kbd->kbdStatus &= ~kbdAltHit;
      } else if ((kbd->kbdDr[2] == 0x1F) || (kbd->kbdDr[2] == 0x27)) {
	kbd->kbdStatus &= ~kbdWinHit;
      } else {
	key = pgm_read_byte(&(ps2code2asciiE0[kbd->kbdDr[2]&PS2CODE2ASCII_MASK]));
      }
    }
  } else if (kbd->kbdDr[0] == 0xF0) {
    kbd->kbdTransL = 2;    /* F0 XX */
    if (2 == kbd->drC) { /* Break of normal keys */
      if ((0x12 == kbd->kbdDr[1]) || (0x59 == kbd->kbdDr[1]))
	kbd->kbdStatus &= ~kbdShiftHit;
      else if (0x14 == kbd->kbdDr[1])
	kbd->kbdStatus &= ~kbdCtrlHit;
      else if (0x11 == kbd->kbdDr[1])
	kbd->kbdStatus &= ~kbdAltHit;
      else {
	key = (kbd->kbdStatus & kbdShiftHit) ?
	  pgm_read_byte(&(ps2code2asciiE0[kbd->kbdDr[1]&PS2CODE2ASCII_MASK])) :
	  pgm_read_byte(&(ps2code2ascii[kbd->kbdDr[1]&PS2CODE2ASCII_MASK]));
	/* temporary fix for code not in table */
	if (ASCII_UNDEF == key) {
	  if (0x83 == kbd->kbdDr[1])
	    key = ASCII_F7;
	}
      }
    }
  } else if (0xE1 == kbd->kbdDr[0]) {
    kbd->kbdTransL = 8;
  } else {
    /* Make code received, generally no action except for sticky keys */
    kbd->kbdTransL = 1;
    if ((0x12 == kbd->kbdDr[0]) || (0x59 == kbd->kbdDr[0]))
      kbd->kbdStatus |= kbdShiftHit;
    else if (0x14 == kbd->kbdDr[0])
      kbd->kbdStatus |= kbdCtrlHit;
    else if (0x11 == kbd->kbdDr[0])
      kbd->kbdStatus |= kbdAltHit;
    /* if (0xFA == kbd->kbdDr[0]) ACKNOWLEDGEMENT FROM KBD */
  }

  /* Get ready for new trans skip make codes */
  if (kbd->drC >= kbd->kbdTransL) {
    /* */
    if (ASCII_NUMLK == key) {
      /* FIXME: Switch TOGGLE the light */
    } else if ((ASCII_UNDEF != key) && KBD_NOT_HIT && LCD_IS_ON) {
      kbdPushKeyHit(key, kbd->kbdStatus);
      /* */
      TCNT1 = TCNT1_DELAY;
      TIMSK |= (1 << TOIE1); /* enable Timer1 overflow */
    }
    kbd->kbdTransL = 1;
    for (; (kbd->drC<LENOF_DR) && (--(kbd->drC)); )
      kbd->kbdDr[kbd->drC] = 0;
    kbd->drC = 0;
  }
}

ISR(INT5_vect)
{
  ps2Intr(&kbd0);
}
ISR(INT6_vect)
{
  ps2Intr(&kbd1);
}
ISR(INT7_vect)
{
  ps2Intr(&kbd2);
}
