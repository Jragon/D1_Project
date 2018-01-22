#include "adc.h"

ISR(ADC_vect) { adc_read = 1; }

void init_adc() {
  // / by 64 prescalar
  ADCSRA = _BV(ADPS2) | _BV(ADPS1);

  // free running mode
  ADCSRA |= _BV(ADATE) | _BV(ADEN);
  ADCSRB = 0;

  // adc interrupt
  ADCSRA |= _BV(ADIE);

  adc_read = 0;
}