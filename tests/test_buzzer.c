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

  for (ui2=0; ui2<0xF; ui2++) {
    /* PortD sets */ 
    PORTA |= 0x80;
    _delay_ms(30);
    PORTA &= ~0x80;
    _delay_ms(1000);
  }
}

int
main(void) 
{

  /* sets the direction register of the PORTD */ 
  DDRA |= 0x80;

  /* PortD sets */ 
  PORTA |= 0x80;
  _delay_ms(30);
  PORTA &= ~0x80;
  _delay_ms(1000);

  AppStartPtr();

  DDRA &= ~0x80;

  return 0;
} 
