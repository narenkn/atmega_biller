#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "kbd.h"

uint8_t KbdData;
uint8_t KbdDataAvail;

#define KBD_RISE_DELAY(N) \
  _delay_ms(N)

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

void
KbdInit(void)
{
  /* IO config */
  DDRA &= ~(0xF0); /* in */
  DDRC |= 0x3C;    /* out */
  PORTA |= 0xF0; /* enable internal pull-ups */
  PORTC &= ~(0x3C0);  /* Drive 0 if enabled */

  /* No data yet */
  KbdData = 0xFF;
  KbdDataAvail = 0;
}

void
KbdScan(void)
{
  static uint8_t gTimer0 = 0;
  uint8_t scan_code = 0, shift = 0;

  if (0 != KbdDataAvail)
    return;

  /* Arrangement of keys
     R1C1 R1C2 R1C3 R1C4
     R2C1 R2C2 R2C3 R2C4
  */

  do {
    KBD_R1_EN;
    KBD_RISE_DELAY(0x4);
    if (KBD_C3_VAL==0) { shift = KBD_SHIFT; }
    if (KBD_C0_VAL==0) { while(KBD_C0_VAL==0) { KBD_RISE_DELAY(0x4); } scan_code = 5; gTimer0 = 0; break; }
    if (KBD_C1_VAL==0) { while(KBD_C1_VAL==0) { KBD_RISE_DELAY(0x4); } scan_code = 6; gTimer0 = 0; break; }
    if (KBD_C2_VAL==0) { while(KBD_C2_VAL==0) { KBD_RISE_DELAY(0x4); } scan_code = 7; gTimer0 = 0; break; }
    KBD_R3_EN;
    KBD_RISE_DELAY(0x4);
    if (KBD_C0_VAL==0) { while(KBD_C0_VAL==0) { KBD_RISE_DELAY(0x4); } scan_code = 12; gTimer0 = 0; break; }
    if (KBD_C1_VAL==0) { while(KBD_C1_VAL==0) { KBD_RISE_DELAY(0x4); } scan_code = 1;  gTimer0 = 0;  break; }
    if (KBD_C2_VAL==0) { while(KBD_C2_VAL==0) { KBD_RISE_DELAY(0x4); } scan_code = 13; gTimer0 = 0; break; }
    if (KBD_C3_VAL==0) { while(KBD_C3_VAL==0) { KBD_RISE_DELAY(0x4); } scan_code = 14; gTimer0 = 0; break; }
    KBD_R0_EN;
    KBD_RISE_DELAY(0x4);
    if (KBD_C0_VAL==0) { while(KBD_C0_VAL==0) { KBD_RISE_DELAY(0x4); } scan_code = 2;  gTimer0 = 0; break; }
    if (KBD_C1_VAL==0) { while(KBD_C1_VAL==0) { KBD_RISE_DELAY(0x4); } scan_code = 3;  gTimer0 = 0; break; }
    if (KBD_C2_VAL==0) { while(KBD_C2_VAL==0) { KBD_RISE_DELAY(0x4); } scan_code = 4;  gTimer0 = 0; break; }
    if (KBD_C3_VAL==0) { while(KBD_C3_VAL==0) { KBD_RISE_DELAY(0x4); } scan_code = 11; gTimer0 = 0; break; }
    KBD_R2_EN;
    KBD_RISE_DELAY(0x4);
    if (KBD_C0_VAL==0) { while(KBD_C0_VAL==0) { KBD_RISE_DELAY(0x4); } scan_code = 8;  gTimer0 = 0; break; }
    if (KBD_C1_VAL==0) { while(KBD_C1_VAL==0) { KBD_RISE_DELAY(0x4); } scan_code = 9;  gTimer0 = 0; break; }
    if (KBD_C2_VAL==0) { while(KBD_C2_VAL==0) { KBD_RISE_DELAY(0x4); } scan_code = 10; gTimer0 = 0; break; }
    if (KBD_C3_VAL==0) { while(KBD_C3_VAL==0) { KBD_RISE_DELAY(0x4); } scan_code = 14; gTimer0 = 0; break; }
  } while (0);
  KBD_NODRIVE;

  /* enough time elapsed after last key hit */
  if ((gTimer0 > 0x8) && (0xFF != KbdData)) {
    uint8_t key_sc = KbdData & 0xF;
    key_sc--;
    KbdDataAvail = 1;
    if (key_sc < KCHAR_ROWS) {
      key_sc *= KCHAR_COLS;
      if (KbdData & KBD_SHIFT) key_sc += KCHAR_SHIFT_SZ;
      key_sc += (KbdData>>4) & 0x7;
      KbdData = pgm_read_byte(&(keyChars[key_sc]));
    } else if (13 == key_sc) {
      KbdData = ASCII_ENTER;
    } else if (10 == key_sc) {
      KbdData = ASCII_PRNSCRN;
    } else if (11 == key_sc) {
      KbdData = ASCII_LEFT;
    } else if (12 == key_sc) {
      KbdData = ASCII_RIGHT;
    } else
      KbdDataAvail = 0;
  } else if (0 != gTimer0) {
    /* No key press */
  } else if ((scan_code-1) == (0xF & (uint8_t)KbdData)) {
    KbdData = (KbdData & 0xF0) + 0x10;
  } else if (0 != scan_code) { /* first time */
    scan_code --;
    KbdData = shift | scan_code;
  }

  /* Don't let timer counter go past a value */
  if (0 ==  (gTimer0 & 0x80))
    gTimer0++;
}

uint8_t
KbdIsShiftPressed(void)
{
  uint8_t shift;

  shift = ps2ShiftHit ? KBD_SHIFT : 0;

  KBD_R1_EN;
  KBD_RISE_DELAY(0x4);
  if (KBD_C3_VAL==0) { shift = KBD_SHIFT; }
  KBD_NODRIVE;

  return shift;
}

const uint8_t
ps2code2ascii[] PROGMEM = {
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, '`', ASCII_UNDEF, /* 0-15 */
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, '1', ASCII_UNDEF, 'q', ASCII_UNDEF, 'z', 's', 'a', 'w', '2', ASCII_UNDEF, /* 16-31 */
  ASCII_UNDEF, 'c', 'x', 'd', 'e', '4', '3', ASCII_UNDEF, ' ', ASCII_UNDEF, 'v', 'f', 't', 'r', '5', ASCII_UNDEF, /* 32-47 */
  ASCII_UNDEF, 'n', 'b', 'h', 'g', 'y', '6', ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, 'm', 'j', 'u', '7', '8', ASCII_UNDEF, /* 48-63 */
  ASCII_UNDEF, ',', 'k', 'i', 'o', '0', '9', ASCII_UNDEF, ASCII_UNDEF, '.', '/', 'l', ';', 'p', '-', ASCII_UNDEF, /* 64-79 */
  ASCII_UNDEF, ASCII_UNDEF, '\'', ASCII_UNDEF, '[', '=', ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_ENTER, ']', ASCII_UNDEF, '\\', ASCII_UNDEF, ASCII_UNDEF, /* 80-95 */
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_BACKSPACE, ASCII_UNDEF, ASCII_UNDEF, '1', ASCII_UNDEF, '4', '7', ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, /* 96-111 */
  '0', '.', '2', '5', '6', '8', ASCII_UNDEF, ASCII_NUMLK, ASCII_UNDEF, '+', '3', '-', '*', '9', ASCII_PRNSCRN, ASCII_UNDEF, /* 112-127 */
};

const uint8_t
ps2code2asciiE0[] PROGMEM = {
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, /* 0-15 */
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, /* 16-31 */
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, /* 32-47 */
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, /* 48-63 */
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, '/', ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, /* 64-79 */
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_ENTER, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, /* 80-95 */
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_LEFT, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, /* 96-111 */
  ASCII_UNDEF, ASCII_LEFT, ASCII_DOWN, ASCII_UNDEF, ASCII_RIGHT, ASCII_UP, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, /* 112-127 */
};

uint8_t kbdStatus = 0;
#define ps2ShiftHit (1<<0)
#define ps2CtrlHit  (1<<1)
#define ps2AltHit   (1<<2)
#define ps2CapsHit  (1<<3)
#define LENOF_DR 3
uint8_t kbdDr[LENOF_DR];
uint8_t kbdTransL = 0;

ISR(INT0_vect)
{             /* Data come with Clock from Device to MCU together */
  static uint8_t KeyData, bitC = 0, drC;

  /* ------------------------------------- */
  bitC++;
  if (1 == bitC) {
    KeyData = 0;
    drC = 0;
    kbdTransL = 2;
  } else if (10 == bitC) {
    /* FIXME: Check parity and blink an LED */
  } else if (11 == bitC) {
    bitC = 0;
  } else {
    KeyData >>= 1;
    KeyData |= (((uint8_t)KBD_PS2_DATA)<<7);
  }

  if (0 != bitC)
    return;
  else if (drC < LENOF_DR) {
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
      }
    }
  } else if (kbdDr[0] == 0xF0) {
    kbdTransL = 2;    /* F0 XX */
    if (2 == drC) { /* Break of normal keys */
      if ((0x12 == kbdDr[0]) || (0x59 == kbdDr[0]))
	kbdStatus &= ~ps2ShiftHit;
      else if (0x14 == kbdDr[0])
	kbdStatus &= ~ps2CtrlHit;
      else if (0x11 == kbdDr[0])
	kbdStatus &= ~ps2AltHit;
      else {
	KeyData = pgm_read_byte(&(ps2code2ascii[kbdDr[0]]));
	if (ASCII_NUMLK == KeyData) {
	  /* FIXME: Switch TOGGLE the light */
	} else if (ASCII_UNDEF != KeyData) {
	  KbdData = KeyData;
	  KbdDataAvail = 1;
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
    /* else if (0xFA == kbdDr[0]) ACKNOWLEDGEMENT FROM KBD */
  }

  /* --------------------------------------- */
  if (drC == kbdTransL) {
    if (3 == drC) {
      KbdData = pgm_read_byte(&(ps2code2ascii[KeyData]));
      KbdDataAvail = 1;
    }
    drC = 0;
  }
}
