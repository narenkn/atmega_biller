#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

    // This function is called upon a HARDWARE RESET:
void reset(void) __attribute__((naked)) __attribute__((section(".init3")));

/*! Clear SREG_I on hardware reset. */
void reset(void)
{
     cli();
    // Note that for newer devices (any AVR that has the option to also
    // generate WDT interrupts), the watchdog timer remains active even
    // after a system reset (except a power-on condition), using the fastest
    // prescaler value (approximately 15 ms). It is therefore required
    // to turn off the watchdog early during program startup.
    MCUSR = 0; // clear reset flags
    //    wdt_disable();
}

int
main()
{
  cli();
  DDRD |= 0xFF;

  PORTD |= 0xFF;

  while (1) {}

  return 0;
}

