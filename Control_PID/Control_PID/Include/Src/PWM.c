#include <avr/io.h>
#include <PWM.h>

void PWM_init(void ){			// Rutina de inicializaci�n del Timer1 en la funci�n Fast PWM
	
	// PWM frequency - TOP Counter1
	ICR1=F_CPU/PWM_freq-1;
	
	// Fast PWM: Clear on compare
	TCCR1A|=(1<<COM1B1)|(0<<COM1B0);
	
	// Fast PWM - TOP: ICR1
	TCCR1B|=(1<<WGM13);
	TCCR1B|=(1<<WGM12);
	TCCR1A|=(1<<WGM11);
	TCCR1A|=(0<<WGM10);
	
	// Prescaler Timer1: no prescaler
	TCCR1B|=(0<<CS12);
	TCCR1B|=(0<<CS11);
	TCCR1B|=(1<<CS10);
	
}

void PWM_on(void){
	TCCR1B|=(0<<CS12);
	TCCR1B|=(0<<CS11);
	TCCR1B|=(1<<CS10);
}

void PWM_duty(float control){	// Rutina de actualizaci�n del ciclo de trabajo del PWM.
	OCR1B=(uint16_t)(control*1024/5);
}