//******************************************************
//**** ADC ROUTINES - HEADER FILE **************
//******************************************************
//Controller		: ATmega32 (Clock: 8 Mhz-internal)
//Compiler			: AVR-GCC (winAVR with AVRStudio-4)
//Project Version	: DL_1.0
//Author			: CC Dharmani, Chennai (India)
//			  		  www.dharmanitech.com
//Date				: 10 May 2011
//******************************************************

#ifndef ADC_H
#define ADC_H

#define ADC_ENABLE            ADCSRA |= _BV(ADEN)
#define ADC_DISABLE           ADCSRA &= ~_BV(ADEN)
#define ADC_START_CONVERSION  ADCSRA |= _BV(ADSC)

void adcInit(void);

extern uint16_t batVcc;

#endif
