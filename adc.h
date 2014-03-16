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


#define ADC_ENABLE 					ADCSRA |= (1<<ADEN)
#define ADC_DISABLE 				ADCSRA &= 0x7F
#define ADC_START_CONVERSION		ADCSRA |= (1<<ADSC)

uint8_t temperature[7];
uint8_t voltage[7];	

void ADC_init(void);
unsigned int ADC_read(void);
void readTemperature(uint8_t);
void readVoltage(uint8_t);


#endif
