#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "main.h"
#include "i2c.h"

#ifndef NO_MAIN

void
main_init(void)
{
  /* Set pin mode & enable pullup */
  DDRD &= ~((1<<PD2)|(1<<PD3));
//  PORTD |= (1<<PD2) | (1<<PD3);

  /* PS2 : Int0 falling edge */
  GICR = 1<<INT0;
  MCUCR |= 1<<ISC01 | 0<<ISC00;

  /* For Fat32 */
  DDRB  |= 0xB2;
  PORTB |= 0xF2;

  /* For Buzzer */
  DDRD |= 0x80;
  BUZZER_OFF;

  /* Enable Global Interrupts */
  sei();
}

int
main(void)
{
  return 0;
}

#endif

uint8_t
validDate(uint8_t day, uint8_t month, uint8_t year)
{
  uint8_t max_days_in_month;
  uint32_t ui32_1;

  if (1 == month) {
    ui32_1 = year+1980;
    max_days_in_month = ((0 == (ui32_1%4)) && (0 != (ui32_1%100)))? 29 : 28;
  } else if ( (0 == month) || (2 == month) ||
	    (4 == month) || (6 == month) ||
	    (7 == month) || (9 == month) ||
	    (11 == month) )
    max_days_in_month = 31;
  else
    max_days_in_month = 30;

  return (month<=11) && (day < max_days_in_month);
}
