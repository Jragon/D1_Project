#ifndef ROTARY_H
#define ROTARY_H

#include <avr/interrupt.h>
#include <avr/io.h>

volatile int8_t rotaryCount;

ISR(TIMER0_COMPA_vect);
void init_rotary(void);
int8_t encodeRotaryCount();

#endif