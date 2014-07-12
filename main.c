#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "main.h"
#include "i2c.h"

void
main_init(void)
{
  /* Set pin mode & enable pullup */
  DDRD &= ~((1<<PD2)|(1<<PD3));
//  PORTD |= (1<<PD2) | (1<<PD3);

  /* PS2 : Int0 falling edge */
  GICR = 1<<INT0;
  MCUCR |= 1<<ISC01 | 0<<ISC00;

  /* For Fat32 */
  DDRB  |= 0xB2;
  PORTB |= 0xF2;

  /* For Buzzer */
  DDRD |= 0x80;
  BUZZER_OFF;

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

//******************************************************************
//Function to get RTC date & time in FAT32 format
//  Return format : Year[31:25], Month[24:21], Date[20:16]
//                  Hour[15:11], Min[10:5], Sec[4:0]
//******************************************************************   
uint32_t
get_fattime (void)
{
  uint8_t ui1, buf[3];
  uint32_t dtFat;

  /* init with 1/1/1981 */
  dtFat = 0x02108000;

  /* Process date */
  timerDateGet(buf);

  dtFat = (buf[2] & 0xf0) >> 4;
  dtFat = (dtFat * 10) + (buf[2] & 0x0f);
  dtFat = dtFat+20;

  ui1 = (buf[1] & 0xf0) >> 4;
  ui1 = (ui1 * 10) + (buf[1] & 0x0f);
  dtFat <<= 4;
  dtFat |= ui1;

  ui1 = (buf[0] & 0xf0) >> 4;
  ui1 = (ui1 * 10) + (buf[0] & 0x0f);
  dtFat <<= 5;
  dtFat |= ui1;

  /* Process time */
  timerTimeGet(buf);

  ui1 = (buf[0] & 0xf0) >> 4;
  ui1 = (ui1 * 10) + (buf[0] & 0x0f);
  dtFat <<= 5;
  dtFat |= ui1;

  ui1 = (buf[1] & 0xf0) >> 4;
  ui1 = (ui1 * 10) + (buf[1] & 0x0f);
  dtFat <<= 6;
  dtFat |= ui1;

  /* FAT32 fromat accepts dates with 2sec resolution
     (e.g. value 5 => 10sec) */
  ui1 = (buf[2] & 0xf0) >> 4;
  ui1 = (ui1 * 10) + (buf[2] & 0x0f);
  ui1 >>= 1;
  dtFat <<= 5;
  dtFat |= ui1;

  return dtFat;
}
