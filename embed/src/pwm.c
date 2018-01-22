#include "pwm.h"

// from Klaus
void init_pwm(void) {
  DDRD |= _BV(PD6); /* PWM out */
  DDRD |= _BV(PD7); /* inv. PWM out */

  TCCR2A = _BV(WGM20) | /* fast PWM/MAX */
           _BV(WGM21) | /* fast PWM/MAX */
           _BV(COM2A1); /* A output */
  TCCR2B = _BV(CS20);   /* no prescaling */
}

void set_pwm_duty(uint8_t x) { OCR2A = x > PWM_DUTY_MAX ? PWM_DUTY_MAX : x; }
