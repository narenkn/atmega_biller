#define KBD_H

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "uart.c"

volatile uint8_t KbdData, KbdDataAvail=0;
#define KBD_HIT (0 != KbdDataAvail)
#define KBD0_PS2_CLK      ((PINE >> 5)&1)
#define KBD0_PS2_DATA     ((PINE >> 2)&1)

uint8_t kbdStatus = 0;
#define ps2ShiftHit (1<<0)
#define ps2CtrlHit  (1<<1)
#define ps2AltHit   (1<<2)
#define ps2CapsHit  (1<<3)
#define LENOF_DR 4
uint8_t kbdDr[LENOF_DR];

/* Key definitions */
#define ASCII_UNDEF      0
#define ASCII_ENTER      0xA0
#define ASCII_BACKSPACE  0xA1
#define ASCII_NUMLK      0xA2
#define ASCII_PRNSCRN    0xA3
#define ASCII_DEL        ASCII_BACKSPACE
#define ASCII_LEFT       0xA4
#define ASCII_RIGHT      0xA5
#define ASCII_UP         0xA6
#define ASCII_DOWN       0xA7

const uint8_t
ps2code2ascii[] PROGMEM = {
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, '`', ASCII_UNDEF, /* 0-15 */
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, 'q', '1', ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, 'z', 's', 'a', 'w', '2', ASCII_UNDEF, /* 16-31 */
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
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, 'Q', '!', ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, 'Z', 'S', 'A', 'W', '@', ASCII_UNDEF, /* 16-31 */
  ASCII_UNDEF, 'C', 'X', 'D', 'E', '$', '#', ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, 'V', 'F', 'T', 'R', '%', ASCII_UNDEF, /* 32-47 */
  ASCII_UNDEF, 'N', 'B', 'H', 'G', 'Y', '^', ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, 'M', 'J', 'U', '&', '*', ASCII_UNDEF, /* 48-63 */
  ASCII_UNDEF, '<', 'K', 'I', 'O', ')', '(', ASCII_UNDEF, ASCII_UNDEF, '>', '/', 'L', ':', 'P', '_', ASCII_UNDEF, /* 64-79 */
  ASCII_UNDEF, ASCII_UNDEF, '"', ASCII_UNDEF, '{', '+', ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_ENTER, '}', ASCII_UNDEF, '|', ASCII_UNDEF, ASCII_UNDEF, /* 80-95 */
  ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_LEFT, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, /* 96-111 */
  ASCII_UNDEF, ASCII_LEFT, ASCII_DOWN, ASCII_UNDEF, ASCII_RIGHT, ASCII_UP, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, ASCII_UNDEF, /* 112-127 */
};

volatile uint8_t KeyData, bitC=0, drC=0;
ISR(INT5_vect)
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
    KeyData |= (((uint8_t)KBD0_PS2_DATA)<<7);
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
	KeyData = (kbdStatus & ps2ShiftHit) ?
	  pgm_read_byte(&(ps2code2asciiE0[kbdDr[2]])) :
	  pgm_read_byte(&(ps2code2ascii[kbdDr[2]]));
	if (ASCII_UNDEF != KeyData) {
	  KbdData = KeyData;
	  KbdDataAvail = 1;
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
	  pgm_read_byte(&(ps2code2asciiE0[kbdDr[1]])) :
	  pgm_read_byte(&(ps2code2ascii[kbdDr[1]]));
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
    /* if (0xFA == kbdDr[0]) ACKNOWLEDGEMENT FROM KBD */
  }

  /* Get ready for new trans */
  if (drC == kbdTransL) {
    for (drC=0; drC<LENOF_DR; drC++)
      kbdDr[drC] = 0;
    drC = 0;
    kbdTransL = 1;
  }
}

#include "lcd.c"

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

  /* Set pin mode & enable pullup */
  DDRE &= ~((1<<PE2)|(1<<PE5)|(1<<PE3)|(1<<PE6)|(1<<PE7));
  DDRA &= ~(1<<PA6);

  /* Enable Int0 on falling edge */
  EICRB |= 1<<ISC51 | 0<<ISC50;
  EIMSK |= _BV(INT5);

  /* Enable Global Interrupts */
  sei();

  LCD_bl_on;
  LCD_WriteDirect(LCD_CMD_CUR_10, "Printer: ", 9);
  _delay_ms(1000);

  for (ui1=0; ; ui1++) {
    if (KBD_HIT) {
      LCD_cmd((LCD_CMD_CUR_10+9));
      LCD_wrchar(KbdData);
      KbdDataAvail = 0;
      uart0TransmitByte(KbdData);
      uart0TransmitByte('\r');
      uart0TransmitByte('\n');
    }
    _delay_ms(100);
  }
  LCD_bl_off;

  return 0;
}
