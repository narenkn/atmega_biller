#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/crc16.h>
#include <avr/eeprom.h>
#include <util/twi.h>

#include "ep_ds.h"
#include "version.h"
#include "assert.h"
#include "lcd.h"
#include "kbd.h"
#include "ep_store.h"
#include "billing.h"
#include "i2c.h"
#include "uart.h"
#include "a1micro2mm.h"
#include "ff.h"
#include "menu.h"
#include "main.h"


// FIXME: Not unit tested
/* 1 sec has F_CPU clocks
   timer is prescaled to 1024 cycles
   so, for 5 sec delay :
     (F_CPU/1024)*5 ~= 40000 = 0x9C40;
 */
volatile uint8_t timer2_msb = 0;
volatile uint16_t timer2_sleep_delay = 0x9C;
ISR(TIMER2_OVF_vect)
{
  timer2_msb++;
}

volatile uint8_t eeprom_setting0 = 0, eeprom_setting1 = 0;

void
eeprom_setting2ram()
{
  uint8_t ui8_1;

  /* init */
  eeprom_setting0 = eeprom_setting1 = 0;

  ui8_1 = eeprom_read_byte(offsetof(struct ep_store_layout, key_buzz_off));
  if (ui8_1) EEPROM_SETTING0_OFF(BUZZER);
  else EEPROM_SETTING0_ON(BUZZER);
}

void
main_init(void)
{
  /* PS2 keypad */
  DDRD &= ~((1<<PD2)|(1<<PD3));
  //  PORTD |= (1<<PD2) | (1<<PD3); /* not required as we have a pullup on board */
  /* PS2 : Int0 falling edge */
  GICR = 1<<INT0;
  MCUCR |= 1<<ISC01 | 0<<ISC00;

  /* For Fat32 */
  DDRB  |= 0xB2;
  PORTB |= 0xF2;

  /* For Buzzer */
  DDRD |= 0x80;
  BUZZER_OFF;

  /* enable timer 2 for periodic checks */
  TIMSK |= (1 << TOIE2);

  /* setup timer 2 */
  // Set CS10 bit so timer runs at clock speed:
  TCCR2 = 0;
  TCCR2 |= (0x7 << CS20);

  /* */
  uint8_t ui8_1 = eeprom_read_byte((uint8_t *)(offsetof(struct ep_store_layout, idle_wait)));
  ui8_1 %= 60; /* max 60 seconds */
  if (0 == ui8_1) ui8_1 = 5;
  timer2_sleep_delay = ui8_1;
  timer2_sleep_delay *= (F_CPU>>10);

  /* */
  timer2_msb = 0;

  /* */
  eeprom_setting2ram();

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
