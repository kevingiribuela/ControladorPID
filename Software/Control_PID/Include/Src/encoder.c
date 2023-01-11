	#include <avr/io.h>
	#include <encoder.h>
	
	void encoder_init(void){
		/* PINOUT ENCODER CONFIGURATION */
		// PORTC0 as INPUT - CLK
		DDRC &=~(1<<DDC0);
		
		// PORTD2 as INPUT - DT
		DDRD &=~(1<<DDD2);
	
		// PORTD3 as Pull-Up INPUT - SWT
		DDRD &=~(1<<DDD3);
		PORTD|= (1<<PORTD3);
		/********************************/
		
		
		/* INTERRUPT ENCODER CONFIGURATION */ 
		// Any logical change on INT0 generates an interrupt request.
		EICRA&=~(1<<ISC01);
		EICRA|= (1<<ISC00);
		
		// The falling edge of INT1 generates an interrupt request.
		EICRA|= (1<<ISC11);
		EICRA&=~(1<<ISC10);
		
		// Interrupt INT1/0 enable
		EIMSK|= (1<<INT1);
		EIMSK|= (1<<INT0);
		/***********************************/
	}
	
	char rotation(void ){			// Deteccion de sentido de giro del encoder
		
		char comp1, comp2;
		
		comp2 = (PINC&(1<<CLK))>>CLK;
		comp1 = (PIND&(1<<DT))>>DT;
		
		if(comp1==comp2){return '+';}
		else{return '-';}
	}