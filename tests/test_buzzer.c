#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <main.h>

typedef void (*AppPtr_t)(void) __attribute__ ((noreturn));
AppPtr_t AppStartPtr = (AppPtr_t)(0xFE00>>1);

void buzz(void) __attribute__ ((section (".bootloader")));

void
buzz(void)
{
  uint8_t ui2;

  for (ui2=0; ui2<0xF; ui2++) {
    BUZZER_ON;
    _delay_ms(30);
    BUZZER_OFF;
    _delay_ms(1000);
  }
}

int
main(void) 
{

  /* set direction */
  DDRA |= 0x80;

  /* PortD sets */ 
  BUZZER_ON;
  _delay_ms(30);
  BUZZER_OFF;
  _delay_ms(1000);

  AppStartPtr();

  /* */
  DDRA &= ~0x80;

  return 0;
} 
