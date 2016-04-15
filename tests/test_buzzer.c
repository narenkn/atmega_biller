#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

typedef void (*AppPtr_t)(void) __attribute__ ((noreturn));
AppPtr_t AppStartPtr = (AppPtr_t)(0x7E00>>1);

void buzz(void) __attribute__ ((section (".bootloader")));

void
buzz(void)
{
  uint8_t ui2;

  /* sets the direction register of the PORTD */ 
  DDRD = 0x80;

  for (ui2=0; ui2<0xF; ui2++) {
    /* PortD sets */ 
    PORTD = 0x80;
    _delay_ms(30);
    PORTD = 0x0;
    _delay_ms(1000);
  }

  DDRD = 0x0;
}

int
main(void) 
{
  AppStartPtr();

  return 0;
} 
