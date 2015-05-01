#ifndef AVR_IO_H
#define AVR_IO_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

volatile uint8_t DDRA, DDRB, DDRC, DDRD;
volatile uint8_t PORTA, PORTB, PORTC, PORTD;
volatile uint8_t PINA, PINB, PINC, PIND;

#ifndef  SPM_PAGESIZE
#if defined (__AVR_ATmega32__)
# define SPM_PAGESIZE     128
#elif defined (__AVR_ATxmega64A1__) || defined (__AVR_ATxmega64A3__) || defined (__AVR_ATxmega64D3__)
# define SPM_PAGESIZE     128
#elif defined (__AVR_ATxmega128A1__) || defined (__AVR_ATxmega128A3__) || defined (__AVR_ATxmega128D3__) || defined (__AVR_ATmega1284P__)
# define SPM_PAGESIZE     256
#elif defined (__AVR_ATxmega256A3__) || defined (__AVR_ATxmega256A3B__) || defined (__AVR_ATxmega256D3__)
# define SPM_PAGESIZE     256
#else
# error "Not a known device"
#endif
#endif

#endif
