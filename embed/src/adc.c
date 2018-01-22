#include "adc.h"

ISR(ADC_vect) {
  adc_ready = 1;
  ADC_INT_OFF;
}

void init_adc() {
  // / by 64 prescalar
  ADCSRA = _BV(ADPS2) | _BV(ADPS1);

  // free running mode
  ADCSRA |= _BV(ADATE) | _BV(ADEN);
  ADCSRB = 0;

  // adc interrupt
  ADC_INT_ON;

  adc_ready = 0;
}

uint16_t adc_read() {
  ADC_INT_ON;
  return ADC;
}