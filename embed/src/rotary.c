#include "rotary.h"

// rotary encoder timer
ISR(TIMER0_COMPA_vect) {
  static int8_t last;
  int8_t new, diff;
  uint8_t wheel;

  wheel = PIND;
  new = 0;
  if (wheel & _BV(PD2)) new = 3;
  if (wheel & _BV(PD3)) new ^= 1;   // convert gray to binary
  diff = last - new;                // difference last - new
  if (diff & 1) {                   // bit 0 = value (1)
    last = new;                     // store new as next last
    rotaryCount += (diff & 2) - 1;  // bit 1 = direction (+/-)
  }
}

void init_rotary(void) {
  // Configure I/O Ports
  DDRD &= ~_BV(PD2) & ~_BV(PD3);  // Rot. Encoder inputs
  PORTD |= _BV(PD2) | _BV(PD3);   // Rot. Encoder pull-ups

  // Timer 0 for switch scan interrupt:
  TCCR0A = _BV(WGM01);             // CTC Mode, DS Table 14-7
  TCCR0B = _BV(CS01) | _BV(CS00);  // Prescaler: F_CPU / 64
  OCR0A = (uint8_t)(F_CPU / (2 * 64.0 * 1000) - 1);

  TIMSK0 |= _BV(OCIE0A);
}

// read two step encoder
int8_t encodeRotaryCount() {
  int8_t val;

  cli();
  val = rotaryCount;
  rotaryCount &= 1;
  sei();

  return val >> 1;
}
