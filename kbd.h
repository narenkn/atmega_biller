#ifndef KBD_H
#define KBD_H

/* Keypad Connections */
#define KBD_PS2_CLK      ((PIND >> 2)&1)
#define KBD_PS2_CLK_NS   (PIND & 0x4)
#define KBD_PS2_DATA     ((PIND >> 3)&1)
#if 1
# define KBD_NODRIVE      DDRC &= ~(0x3C)
# define KBD_DRIVE        DDRC |= 0x3C; DDRA &= ~0xF0/*; PORTA |= 0xF0*/
# define KBD_R0_EN        PORTC &= ~0x3C; PORTC |= 0x04
# define KBD_R1_EN        PORTC &= ~0x3C; PORTC |= 0x08
# define KBD_R2_EN        PORTC &= ~0x3C; PORTC |= 0x10
# define KBD_R3_EN        PORTC &= ~0x3C; PORTC |= 0x20
# define KBD_C0_VAL       (PINA & 0x10)
# define KBD_C1_VAL       (PINA & 0x20)
# define KBD_C2_VAL       (PINA & 0x40)
# define KBD_C3_VAL       (PINA & 0x80)
#else
# define KBD_NODRIVE      DDRA &= ~(0xF0)
# define KBD_DRIVE        DDRA |= 0xF0; DDRC &= ~0x3C; PORTC &= ~0x3C
# define KBD_R0_EN        PORTA &= ~0xF0; PORTA |= 0x10
# define KBD_R1_EN        PORTA &= ~0xF0; PORTA |= 0x20
# define KBD_R2_EN        PORTA &= ~0xF0; PORTA |= 0x40
# define KBD_R3_EN        PORTA &= ~0xF0; PORTA |= 0x80
# define KBD_C0_VAL       (PINC & 0x4)
# define KBD_C1_VAL       (PINC & 0x8)
# define KBD_C2_VAL       (PINC & 0x10)
# define KBD_C3_VAL       (PINC & 0x20)
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
#define ASCII_F2         0xA8
#define ASCII_ESCAPE     0xA9

#define KBD_RESET_KEY          \
  keyHitData.KbdData=0, keyHitData._kbdData=0, keyHitData.count=0, keyHitData.KbdDataAvail=0

#define KBD_HIT      (0x0 != keyHitData.KbdDataAvail)
#define KBD_NOT_HIT  (0x0 == keyHitData.KbdDataAvail)

#ifdef UNIT_TEST
# define KBD_GETCH KbdGetCh()
#else
# define KBD_GETCH				\
  while (KBD_NOT_HIT) {				\
    /* put the device to sleep */		\
    sleep_enable();				\
    sleep_cpu();				\
    /* some event has to occur to come here */	\
    sleep_disable();				\
  }						\
  LCD_bl_on
#endif

#define KCHAR_ROWS        10
#define KCHAR_COLS         9
#define KCHAR_SHIFT_SZ     5
#define KBD_SHIFT       0x80

typedef volatile struct {
  uint8_t KbdData;
  uint8_t KbdDataAvail;
  uint8_t _kbdData;
  uint8_t count;
} keyHitData_t;

extern keyHitData_t keyHitData;

void    KbdInit(void);
void    KbdScan(void);
uint8_t KbdIsShiftPressed(void);
extern volatile uint8_t KbdData;
extern volatile uint8_t KbdDataAvail;
extern const uint8_t ps2code2ascii[] PROGMEM;
extern const uint8_t ps2code2asciiE0[] PROGMEM;
extern const uint8_t keyChars[] PROGMEM;

#endif
