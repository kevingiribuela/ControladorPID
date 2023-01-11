#include <avr/io.h>

#define PWM_freq 15625U
#define F_CPU 16000000UL

void PWM_init(void);

void PWM_duty(float control);

void PWM_off(void);

void PWM_on(void);