#include <avr/interrupt.h>
#include <avr/io.h>

#define ADC_START ADCSRA |= _BV(ADSC)

volatile uint8_t adc_read;

void init_adc();