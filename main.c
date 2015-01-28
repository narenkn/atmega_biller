#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/crc16.h>
#include <avr/eeprom.h>
#include <util/twi.h>
#include <avr/sleep.h>

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
#if FF_ENABLE
#include "ff.h"
#endif
#include "menu.h"
#include "main.h"


volatile uint8_t eeprom_setting0 = 0, eeprom_setting1 = 0;

void
eeprom_setting2ram()
{
  uint8_t ui8_1;

  /* init */
  eeprom_setting0 = eeprom_setting1 = 0;

  ui8_1 = eeprom_read_byte((uint8_t *)offsetof(struct ep_store_layout, key_buzz));
  if (ui8_1) EEPROM_SETTING0_OFF(BUZZER);
  else EEPROM_SETTING0_ON(BUZZER);
}

void
main_init(void)
{
  /* For Fat32 */
  DDRB  |= 0xB2;
  PORTB |= 0xF2;

  /* For Buzzer */
  DDRD |= 0x80;
  BUZZER_OFF;
  EEPROM_SETTING0_ON(BUZZER);

  /* setup timer 2 : need to get 5 sec pulse
     # cycles to skip : (5*F_CPU)
     # clock div is 1024, so we need to skip : (5*F_CPU)>>10
   */
//  TCCR2 |= (0x7 << CS20);
//  TCNT2 = 0;
//  TIMSK |= (1 << TOIE2);

  /* */
  eeprom_setting2ram();

  /* clear display buffer */
  LCD_CLRSCR;

  /* Enable Global Interrupts */
  sei();
}

/* setup timer 2 : need to get 5 sec pulse
   # cycles to skip : (5*F_CPU)
   # clock div is 1024, so we need to skip : (5*F_CPU)>>10
*/
ISR(TIMER2_OVF_vect)
{
  static uint16_t timer2_beats=0;

  timer2_beats++;
  if (timer2_beats < ((5*F_CPU)>>10))
    return;
  timer2_beats = 0;

  LCD_init();
  LCD_refresh();
}

#ifndef NO_MAIN

int
main(void)
{
  LCD_init();

  /* Welcome screen */
  LCD_bl_on;
  LCD_CLRSCR;
  LCD_WR_NP((const uint8_t *)PSTR("Welcome..."), 10);
  LCD_CLRLINE(LCD_MAX_ROW-1);
  LCD_WR_NP((const uint8_t *)PSTR("  Initializing.."), 1);
  LCD_refresh();
  _delay_ms(500);

  /* All other devices */
  KbdInit();
  ep_store_init();
  i2c_init();
  uartInit();
  printerInit();
  main_init();
  menuInit();

  /* Check if all devices are ready to go, else give
     error and exit */
  if (0 == (devStatus&DS_DEV_ERROR)) {
    LCD_cmd(LCD_CMD_CUR_20);
    LCD_WR_NP((const uint8_t *)PSTR("   Initialized!"), 15);
    LCD_refresh();
    _delay_ms(1000);
    menuMain();
  } else if (devStatus & DS_DEV_INVALID) {
    LCD_CLRLINE(0);
    LCD_WR_NP((const uint8_t *)PSTR("Invalid Prod Key"), 15);
    LCD_refresh();
    _delay_ms(5000);
  } else {
    /* hardware issues found ....
     FIXME: ASK and run factory tests */
  }

  /* reach here and you could never get out */
  LCD_CLRSCR;
  LCD_WR_NP((const uint8_t *)PSTR("Power Off Now"), 13);
  LCD_refresh();
  while (1) {
    KBD_GETCH;
  }

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
