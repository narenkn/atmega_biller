#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

int
main(void) 
{
  uint8_t ui2;

  /* sets the direction register of the PORTD */ 
  DDRD = 0x60;

  for (ui2=0; ui2<0xF; ui2++) {
    /* PortD sets */ 
    PORTD = 0x40;
    _delay_ms(30);
    PORTD = 0x0;
    _delay_ms(1000);
  }

  DDRD = 0x0;

  return 0;
} 
