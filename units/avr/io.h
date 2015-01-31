#ifndef AVR_IO_H
#define AVR_IO_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

volatile uint8_t DDRA, DDRB, DDRC, DDRD;
volatile uint8_t PORTA, PORTB, PORTC, PORTD;
volatile uint8_t PINA, PINB, PINC, PIND;

#ifndef  SPM_PAGESIZE
#if      ATMega32
# define SPM_PAGESIZE     128
#elif    ATxMega64
# define SPM_PAGESIZE     128
#elif  ATxMega128
# define SPM_PAGESIZE     256
#elif  ATxMega256
# define SPM_PAGESIZE     256
#else
# error "Not a known device"
#endif
#endif

#endif
