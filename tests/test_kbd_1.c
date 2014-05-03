
#define assert(...)

#include "lcd.c"

#define LCD_WriteDirect(loc, str, len) do {	\
  uint8_t _ui1;					\
  LCD_cmd(loc);					\
  for (_ui1=0; _ui1<len; _ui1++) {		\
    LCD_wrchar( ((char *)str)[_ui1] );		\
  }						\
} while (0)

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

int
main(void)
{
  uint8_t ui1;

  _delay_ms(100);
  LCD_init();

  PORTD = 0x10;
  LCD_WriteDirect(LCD_CMD_CUR_10, "Press Key:", 13);
  KBD_DRIVE;
  PORTA |= 0xF0;

  while (1) {
    _delay_ms(10);
    ui1 = PINC;
    LCD_cmd(LCD_CMD_CUR_10+11);
    LCD_PUT_UINT8X(ui1);
    _delay_ms(10);
  }

  return 0;
}
