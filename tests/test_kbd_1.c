
#define assert(...)

#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
//#include "lcd.c"

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

int
main(void)
{
  uint8_t ui1, num_hits;

  _delay_ms(1000);
//  LCD_init();

//  PORTD = 0x10;
//  LCD_WR_LINE(0, 0, "Press Key:");
//  LCD_refresh();

  KBD_DRIVE;
  DDRB |= 1;

  for (ui1=0; ui1<5; ui1++) {
    PORTB = 1;
    _delay_ms(1000);
    PORTB = 0;
    _delay_ms(1000);
  }

  num_hits = 0;
  while (1) {
    _delay_ms(10);

    /* Drive 1, check for feedback */
    PORTA |= 0xF0;
    ui1 = PINC;
    if (0 != (ui1&0x3C))
      num_hits++;
    else num_hits = 0;

    /* Drive 0, check for feedback */
    PORTA &= ~0xF0;
    ui1 = PINC;
    if (0x3C != (ui1&0x3C))
      num_hits++;
    else num_hits = 0;

    /* Buzz if feedback for some time */
    if (num_hits >= 5)
      PORTB = 1;
    else
      PORTB = 0;
    
    /* Wait for some time */
    _delay_ms(1);
  }

  return 0;
}
