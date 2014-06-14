
#define assert(...)

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include "lcd.c"

# define KBD_NODRIVE      DDRA &= ~(0xF0)
# define KBD_DRIVE        DDRA |= 0xF0; DDRC &= ~0x3C; DDRC |= 0x3C
# define KBD_R0_EN        PORTA &= ~0xF0; PORTA |= 0x10
# define KBD_R1_EN        PORTA &= ~0xF0; PORTA |= 0x20
# define KBD_R2_EN        PORTA &= ~0xF0; PORTA |= 0x40
# define KBD_R3_EN        PORTA &= ~0xF0; PORTA |= 0x80
# define KBD_C0_VAL       (PINC & 0x4)
# define KBD_C1_VAL       (PINC & 0x8)
# define KBD_C2_VAL       (PINC & 0x10)
# define KBD_C3_VAL       (PINC & 0x20)

uint8_t
GetKeyPressed()
{
  uint8_t r, c;

  /* initially Port A is input, PORTC is also input */
  DDRA  &= ~0xF0;
  PORTA &= ~0xF0;
  DDRC &= ~(0x3C);
  PORTC |= 0x3C;

  for (c=0; c<4; c++) {
    /* Drive only one bit as 0 */
    DDRA &= ~(0xF0);
    DDRA |= (0x80>>c);

    for (r=0; r<4;  r++) {
      if(!(PINC & (0x20>>r))) {
	return ((r<<2)|c);
      }
    }
  }

  return 0xFF;
}

int
main(void)
{
  uint8_t ui1, ui2;
  uint16_t num_hits;

  _delay_ms(1000);
  LCD_init();
  DDRD |= 0x80;
  LCD_bl_on;

  PORTD = 0x10;
  LCD_WR_LINE(0, 0, "Press Key:");
  LCD_refresh();

  KBD_DRIVE;

  num_hits = 0;
  while (1) {
    /* Drive 1, check for feedback */
    ui1 = GetKeyPressed();
    LCD_POS(1,0);
    LCD_PUT_UINT8X(ui1);
    LCD_refresh();

//    /* Drive 0, check for feedback */
//    PORTA &= ~0xF0;
//    ui1 = PINC;
//    if (0x3C != (ui1&0x3C))
//      num_hits++;
//    else num_hits = 0;

//    /* Buzz if feedback for some time */
//    if (0xFF != ui1) {
//      PORTD = 0x80;
//    } else {
//      PORTD &= ~0x80;
//    }

     /* Wait for some time */
    _delay_ms(100);
  }

  return 0;
}
