#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/crc16.h>
#include <avr/eeprom.h>
#include <util/twi.h>

#include "lcd.c"

int
main()
{
  LCD_init();
  LCD_bl_on;
  LCD_CLRSCR;
  LCD_WR_P(PSTR("Test VCC!"));
  _delay_ms(1000);

  while (1) {
    // Read 1.1V reference against AVcc
    // set the reference to Vcc and the measurement to the
    // internal 1.1V reference
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega128__) || defined(__AVR_ATmega256__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
#elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
#else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
#endif  

    ADCSRA |= _BV(ADEN);
    _delay_ms(200); // Wait for Vref to settle
    ADCSRA |= _BV(ADSC); // Start conversion
    while (ADCSRA & _BV(ADSC)) {}; // measuring

    uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
    uint16_t result = ADCH; // unlocks both
    result <<= 8;
    result |= low;

    uint32_t vcc = 1259520; // = 1.23*1024*1000
    vcc /= result;

    LCD_CLRLINE(1);
    LCD_PUT_UINT(vcc);
    LCD_PUTCH('V');
    _delay_ms(5000);
  }

  return 0;
}
