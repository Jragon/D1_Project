#include <avr/io.h>

#define PWM_DUTY_MAX 220

void init_pwm(void);
void set_pwm_duty(uint8_t x);