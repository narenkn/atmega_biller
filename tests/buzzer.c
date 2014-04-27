#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

int
main(void) 
{
  uint8_t ui2;

  /* sets the direction register of the PORTD */ 
  DDRD = 0x80;

  for (ui2=0; ui2<0xF; ui2++) {
    /* PortD sets */ 
    PORTD = 0x80;
    _delay_ms(5);
    PORTD = 0x0;
    _delay_ms(5);
  }

  DDRD = 0x0;

  return 0;
} 
