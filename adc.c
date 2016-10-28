//******************************************************
//     **** ADC ROUTINES - SOURCE FILE *****
//******************************************************
//Controller		: ATmega32 (Clock: 8 Mhz-internal)
//Compiler			: AVR-GCC (winAVR with AVRStudio-4)
//Project Version	: DL_1.0
//Author			: CC Dharmani, Chennai (India)
//			  		  www.dharmanitech.com
//Date				: 10 May 2011
//******************************************************

#include <stdint.h>
#include <avr/io.h>
#include "adc.h"

void
adcInit(void)
{
  /* internal reference voltage of 2.56 with cap @ VREF */
  ADMUX = _BV(REFS0) | _BV(ADLAR);
  ADMUX |= 0x1E; /* V_bg */
  /* intr & clock div */
  ADCSRA = _BV(ADIE) | 2;
  /* pins en */
  DDRF &= ~_BV(PF1);

  ADC_ENABLE;
}

uint16_t batVcc;

/* ADC End of Conversion interrupt */
ISR(ADC_vect)
{
  uint8_t adc_data = ADCH;

  batVcc = 123; /* 1.23 V_bg */
  batVcc *= 255;
  batVcc /= adc_data;
}
