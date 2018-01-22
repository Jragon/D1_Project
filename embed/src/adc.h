#include <avr/interrupt.h>
#include <avr/io.h>

#define ADC_START ADCSRA |= _BV(ADSC)
#define ADC_INT_ON ADCSRA |= _BV(ADIE)
#define ADC_INT_OFF ADCSRA &= ~_BV(ADIE)

volatile uint8_t adc_ready;

void init_adc();
uint16_t adc_read();