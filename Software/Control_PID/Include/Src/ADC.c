#include <avr/io.h>
#include <ADC.h>

void ADC_init(unsigned char mod){	// Rutina de inicializaci�n del ADC en modo Auto trigger por Output on compare match a 125kHz con ajuste a derecha
	// Vref: AVcc
	ADMUX|=(0<<REFS1);
	ADMUX|=(1<<REFS0);
	
	// ADLAR: Right Adjust
	ADMUX|=(0<<ADLAR);
	
	if(mod==ADQ){
		// ADC Channel: ADC2
		ADMUX &=0xF0;
		ADMUX|=2;
	}
	else if(mod==PID){
		// ADC Channel: ADC1
		ADMUX &=0xF0;
		ADMUX|=1;
	}
	
	// ADC Auto Trigger Source: Timer 0 compare match A
	ADCSRB|=(0<<ADTS2);
	ADCSRB|=(1<<ADTS1);
	ADCSRB|=(1<<ADTS0);
	
	// ADC Auto Trigger Enable: ON
	ADCSRA|=(1<<ADATE);
	
	// ADC Interrupt: ON
	ADCSRA|=(1<<ADIE);
	
	// ADC Prescaler: 128
	ADCSRA|=(1<<ADPS2);
	ADCSRA|=(1<<ADPS1);
	ADCSRA|=(1<<ADPS0);
	
	// ADC Digital Input Disable
	DIDR0|=(1<<ADC5D)|(1<<ADC4D)|(1<<ADC3D);
	
	// ADC: ON
	ADCSRA|=(1<<ADEN);
}
void ADC_disable(void){				// Rutina de deshabilitacion del ADC
	ADCSRA&=~(1<<ADEN);
}
int ADC_get(char value){			// Rutina de obtencion del valor de los registros del ADC.
	if(value=='L'){
		return ADCL;
	}
	else if(value=='H'){
		return ADCH;
	}
	else{
		return 0;
	}
}