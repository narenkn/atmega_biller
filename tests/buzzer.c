#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

int
main(void) 
{
  /* sets the direction register of the PORTD */ 
  DDRD = 0x80;

  while (1) {
    /* PortD sets */ 
    PORTD = 0x80;
    _delay_ms(5);
    PORTD = 0x0;
    _delay_ms(5);
  }

  return 0;
} 
