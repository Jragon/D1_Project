#define PWM_DUTY_MAX 240
#include <avr/io.h>

void init_pwm(void);
void set_pwm_duty(uint8_t x);