#ifndef KBD_H
#define KBD_H

/* Keypad Connections */
#define KBD0_PS2_CLK      ((PINE >> 5)&1)
#define KBD0_PS2_DATA     ((PINE >> 2)&1)
#define KBD1_PS2_CLK      ((PINE >> 6)&1)
#define KBD1_PS2_DATA     ((PINE >> 3)&1)
#define KBD2_PS2_CLK      ((PINE >> 7)&1)
#define KBD2_PS2_DATA     ((PINA >> 6)&1)
#if defined (__AVR_ATmega128__)
# define KBD_IO_INIT						\
  DDRE &= ~((1<<PE2)|(1<<PE5)|(1<<PE3)|(1<<PE6)|(1<<PE7));	\
  DDRA &= ~(1<<PA6)
# define KBD_INT_PIN_DR0			\
  PORTE &= ~0x10; DDRE  |= 0x10  /* drive 0 */
# define KBD_INT_PIN_VAL			\
  (PINE & 0x10)
# define KBD_NODRIVE				\
  PORTC |= 0xF0;  /* pullup */			\
  PORTC &= ~0x0F; /* drive 0 */			\
  PORTE |= 0x10;  /* pullup */			\
  DDRC &= ~0xF0;  /* in */			\
  DDRC |= 0x0F;   /* out */			\
  DDRE &= ~0x10   /* in */
# define KBD_IPIN_PHI				\
  DDRC  &= ~0x0F; /* input */			\
  PORTC |= 0x0F /* pull high */
# define KBD_OPIN_DR0				\
  DDRC  |= 0xF0; /* output */			\
  PORTC &= ~0xF0 /* drive 0 */
# define KBD_OPIN_DR_R0_0			\
  PORTC &= ~0xF0; PORTC |= 0xE0
# define KBD_OPIN_DR_R1_0			\
  PORTC &= ~0xF0; PORTC |= 0xD0
# define KBD_OPIN_DR_R2_0			\
  PORTC &= ~0xF0; PORTC |= 0xB0
# define KBD_OPIN_DR_R3_0			\
  PORTC &= ~0xF0; PORTC |= 0x70
# define KBD_IPIN_ANY_HIT			\
  (0x0F != (PINC & 0x0F))
# define KBD_IPIN_PIN0_HIT			\
  (0 == (PINC & 0x1))
# define KBD_IPIN_PIN1_HIT			\
  (0 == (PINC & 0x2))
# define KBD_IPIN_PIN2_HIT			\
  (0 == (PINC & 0x4))
# define KBD_IPIN_PIN3_HIT			\
  (0 == (PINC & 0x8))
#elif defined (__AVR_ATmega32__)
# define KBD_IO_INIT				\
  DDRD &= ~((1<<PD2)|(1<<PD3))
# define KBD_INT_PIN_DR0			\
  PORTB &= ~0x04; DDRB  |= 0x04  /* drive 0 */
# define KBD_INT_PIN_VAL			\
  (PINB & 0x04)
# define KBD_NODRIVE				\
  PORTA |= 0xF0;  /* pullup */			\
  PORTC &= ~0x3C; /* drive 0 */			\
  PORTB |= 0x04;  /* pullup */			\
  DDRA &= ~0xF0;  /* in */			\
  DDRC |= 0x3C;   /* out */			\
  DDRB &= ~0x04   /* in */
# define KBD_IPIN_PHI				\
  DDRC  &= ~0x3C; /* input */			\
  PORTC |= 0x3C /* pull high */
# define KBD_OPIN_DR0				\
  DDRA  |= 0xF0; /* output */			\
  PORTA &= ~0xF0 /* drive 0 */
# define KBD_OPIN_DR_R0_0			\
  PORTA &= ~0xF0; PORTA |= 0xE0
# define KBD_OPIN_DR_R1_0			\
  PORTA &= ~0xF0; PORTA |= 0xD0
# define KBD_OPIN_DR_R2_0			\
  PORTA &= ~0xF0; PORTA |= 0xB0
# define KBD_OPIN_DR_R3_0			\
  PORTA &= ~0xF0; PORTA |= 0x70
# define KBD_IPIN_ANY_HIT			\
  (0x3C != (PINC & 0x3C))
# define KBD_IPIN_PIN0_HIT			\
  (0 == (PINC & 0x4))
# define KBD_IPIN_PIN1_HIT			\
  (0 == (PINC & 0x8))
# define KBD_IPIN_PIN2_HIT			\
  (0 == (PINC & 0x10))
# define KBD_IPIN_PIN3_HIT			\
  (0 == (PINC & 0x20))
#endif

/* Key definitions */
#define ASCII_UNDEF      0
#define ASCII_DEFINED    ((uint8_t)~ASCII_UNDEF)
#define ASCII_LF         0xA
#define ASCII_ENTER      0xA0
#define ASCII_BACKSPACE  0xA1
#define ASCII_NUMLK      0xA2
#define ASCII_PRNSCRN    0xA3
#define ASCII_DEL        ASCII_BACKSPACE
#define ASCII_LEFT       0xA4
#define ASCII_RIGHT      0xA5
#define ASCII_UP         0xA6
#define ASCII_DOWN       0xA7
#define ASCII_F1         0xA8
#define ASCII_F2         0xA9
#define ASCII_F3         0xAA
#define ASCII_F4         0xAB
#define ASCII_F5         0xAC
#define ASCII_F6         0xAD
#define ASCII_F7         0xAE
#define ASCII_F8         0xAF
#define ASCII_F9         0xB0
#define ASCII_F10        0xB1
#define ASCII_F11        0xB2
#define ASCII_F12        0xB3
#define ASCII_ESCAPE     0xB4
#define ASCII_ALT        0xB5
#define ASCII_CTRL       0xB6
#define ASCII_LGUI       0xB7
#define ASCII_SHIFT      0xB8

#define KBD_RESET_KEY							\
  keyHitData.hbCnt-=keyHitData.hbCnt?1:0, keyHitData.KbdData=(keyHitData.hitBuf>>(keyHitData.hbCnt<<3)), keyHitData.KbdDataAvail=(keyHitData.availBuf>>(keyHitData.hbCnt<<3))

#define kbdAltHit   (1<<0)
#define kbdWinHit   (1<<1)
#define kbdHit      (1<<2)
#define kbdShiftHit (1<<3)
#define kbdCtrlHit  (1<<4)
#define kbdCapsHit  (1<<5)

#define KBD_HIT      (keyHitData.KbdDataAvail & kbdHit)
#define KBD_NOT_HIT  (0x0 == (keyHitData.KbdDataAvail & kbdHit))
extern const uint8_t keyMapR[] PROGMEM;

#if UNIT_TEST
# define KBD_GETCH KbdGetCh()
#else
# define KBD_GETCH				\
  while (KBD_NOT_HIT) {				\
    /* put the device to sleep */		\
    sleep_enable();				\
    sleep_cpu();				\
    /* some event has to occur to come here */	\
    sleep_disable();				\
    /* schedule regular code-check here */	\
    if (0 == (timer2_beats%1000)) {		\
    }						\
    if (menuPendActs & MENU_PEND_LCD_REFRESH) { \
      LCD_init();				\
    }						\
  }						\
  LCD_bl_on
#endif

#define KCHAR_ROWS        10
#define KCHAR_COLS         9
#define KCHAR_SHIFT_SZ     5
#define KBD_SHIFT       0x80

typedef struct s_keyHitData {
  uint8_t KbdData;
  uint8_t KbdDataAvail;
  uint8_t _kbdData;
  uint8_t count;
  uint8_t  hbCnt;
  uint64_t hitBuf;
  uint64_t availBuf;
} keyHitData_t;

extern volatile keyHitData_t keyHitData;
extern volatile uint8_t keypadMultiKeyModeOff;

void    KbdInit(void);
void    KbdScan(void);
extern volatile uint8_t KbdData;
extern volatile uint8_t KbdDataAvail;
extern const uint8_t ps2code2ascii[] PROGMEM;
extern const uint8_t ps2code2asciiE0[] PROGMEM;
extern const uint8_t keyChars[] PROGMEM;

#define LENOF_DR    4
typedef struct {
  uint8_t kbdStatus;
  uint8_t bitC;
  uint8_t drC;
  uint8_t KeyData;
  uint8_t kbdTransL;
  uint8_t kbdDr[LENOF_DR];
} ps2LineStat_t;
extern ps2LineStat_t kbd0, kbd1, kbd2;

#endif
