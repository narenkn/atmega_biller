#ifndef ADC_H
#define ADC_H

#define ADC_ENABLE            ADCSRA |= _BV(ADEN)
#define ADC_DISABLE           ADCSRA &= ~_BV(ADEN)
#define ADC_START_CONVERSION  ADCSRA |= _BV(ADSC)

void adcInit(void);

extern uint16_t batVcc;

#endif
