#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "main.h"

void
main_init(void)
{
  /* Set pin mode & enable pullup */
  DDRD &= ~((1<<PD2)|(1<<PD3));
//  PORTD |= (1<<PD2) | (1<<PD3);

  /* Enable Int0 on falling edge */
  GICR = 1<<INT0;
  MCUCR |= 1<<ISC01 | 0<<ISC00;

  /* For Fat32 */
  DDRB  |= 0xB2;
  PORTB |= 0xF2;

  /* Enable Global Interrupts */
  sei();
}

#ifndef NO_MAIN

int
main(void)
{
  return 0;
}

#endif
