#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <util/twi.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <ctype.h>

#define BUZZER_ON  PORTA |= 0x80
#define BUZZER_OFF PORTA &= ~0x80

int
main()
{
  DDRA |= 0x80; /* buzzer */
  _delay_ms(1000);

  BUZZER_ON;
  _delay_ms(50);
  BUZZER_OFF;

  /* put the device to sleep */
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_enable();
  sleep_cpu();
  /* some event has to occur to come here */
  sleep_disable();

  while (1) {
    BUZZER_ON;
    _delay_ms(50);
    BUZZER_OFF;
    _delay_ms(1000);
  }

  while (1) {}

  return 0;
}
