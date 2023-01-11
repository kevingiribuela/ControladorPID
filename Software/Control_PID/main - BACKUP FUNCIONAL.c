/*
* Control_PID.c
*
* Created: 26/12/2021 17:30:35
* Author : Kevin Giribuela
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>
#include <encoder.h>
#include <LCD_AVR.h>
#include <USART.h>
#include <PWM.h>
#include <ADC.h>

#define Ts	47				// OCR1B register for 325.52Hz interrupt

#define ADQ	9				// Adquisition mode
#define PID 10				// PID mode

#define PID_IMC 11			// Controlador IMC
#define PID_canPC 12		// Controlador cancelacion polo-cero

#define toggle(x,y) x^=(1<<y)

/************************************************VARIABLES************************************************/
/********************************************************************************************************/
// Interrupts flags
volatile unsigned char button=0, turn=0, conversion=0, matlab=0, actualizar=0;
volatile uint16_t contador=0;

// Global vars
unsigned char routine=0, cursor=0, enter=0, modo=0, mode_selection=0, controller_selection=0;
unsigned char intro=0, select=0, ok=0, curA=50, curb=50, curc=50, curRef=50;

// ADC values
unsigned int ADC_H=0, ADC_L=0, muestras=0;

// PID values
float L=0.325, T=0.324, Kp=1, T0=0, Tf=0;		// Modelo de la planta
float Tds=0, Tis=0, Kxs=0;						// Valores de los par�metros del controlador serie
float Tdp=0, Tip=0, Kxp=0;						// Valores de los par�metros del controlador paralelo
float error_anterior=0, error_actual=0, P=0, I=0, D=0, q=1;						// Valores del PID paralelo
float x_pid[2]={0,0}, y_pid[2]={0,0}, a_pid[2]={0,0}, b_pid[2]={0,0};			// Coeficientes del filtro pasa bajos y del PID
float x_ref[3]={2.5,0,0}, y_ref[3]={0,0,0}, b_ref[3]={0,0,0}, a_ref[3]={0,0,0};	// Coeficientes del filtro a la ref y de la ref
float T_samp=0.003074;
float A=0.5, b=0.5, c=0.5;


/************************************************FUNCTIONS***********************************************/
/********************************************************************************************************/
// Timer0 initialization
void TIMER0_init(void);
void TIMER0_off(void);

// Main functions
void mode_off(void);
void dispatch(void);

// PID functions
void PID_parametters(void);
void PID_off(void);
void print_value(unsigned char, unsigned char);
void update_value(unsigned char, unsigned char, unsigned char);
float accion_de_control(void);
void step_on(void);
void step_off(void);


/********************************************************************************************************/
/********************************************************************************************************/

/*******************************************INTERRUPT HANDLERS*******************************************/
/********************************************************************************************************/
// Handler UART RX ready
ISR(USART_RX_vect){
	matlab=USART_read();
}

// Clear OCF0A flag
ISR(TIMER0_COMPA_vect){;}

// ADC complete
ISR(ADC_vect){
	conversion=1;
	contador++;
	if(contador==500){actualizar=1; contador=0;}
}

// Encoder button
ISR(INT1_vect){
	button=1;
}

// Encoder potentiometer
ISR(INT0_vect){
	turn=1;
}
/********************************************************************************************************/
/********************************************************************************************************/

/**********************************************MAIN PROGRAM**********************************************/
/********************************************************************************************************/
int main(void)
{	
	// Acción de control
	DDRB|= (1<<DDB2);
	
	// LED built-on
	DDRB|=(1<<DDB5);
	
	/****************************************************************/
	// LCD Initialization
	LCD_Init();
	
	// UART Initialization
	USART_Init();
	
	// Encoder Initialization
	encoder_init();
	
	// Timer0 Initialization
	TIMER0_init();

	 LCD_write_string_xy("   Bienvenido   ",1,1);
	_delay_ms(1000);
	LCD_clear();
	
	LCD_write_string_xy("Control III &",1,1);
	LCD_write_string_xy("Arquitectura",2,1);
	_delay_ms(1000);
	
	
	// Global interrupts enable
	sei();
	
	while(1){
		if(mode_selection!=1){
			/* Rutina de selecci�n de modo */
			LCD_clear();
			LCD_write_string_xy("Seleccione modo:",1,0);
			LCD_write_string_xy("PID",2,1);
			LCD_write_string_xy("ADQ",2,13);
			LCD_gotoxy(2,0);
			LCD_write('>');
			cursor=0;
		}
		while(mode_selection!=1){
			// Detecto si se gira el potenciometro del encoder
			if(turn){
				if((rotation())=='+'){
					if(cursor==1){cursor=0;}	// If OVF cursor...
					else{cursor++;}
				}
				else if((rotation())=='-'){
					if(cursor==0){cursor=1;}	// If UDF cursor...
					else{cursor--;}
				}
				// Una vez hecho el giro del potenciometro, muevo el "cursor" del display al modo seleccionado
				if(cursor==0){LCD_write_string_xy(">PID         ADQ",2,0);}
				else{LCD_write_string_xy(" PID        >ADQ",2,0);}
				turn=0;
			}
			// Detecto si se presiona el pulsador del encoder
			if(button&&!(cursor)){modo=PID; button=0; mode_selection=1;}
			else if(button&&cursor){modo=ADQ; button=0; mode_selection=1;}
		}
		/*Rutina de inicilizaci�n del modo adquisidor de datos*/
		if(modo==ADQ){
			OCR1B=0;
			ADC_init(ADQ);
			PWM_init();
			USART_enable();
			
			LCD_clear();
			LCD_write_string_xy("Esperando a",1,0);
			LCD_write_string_xy("MATLAB... >Atras",2,0);
			// Me quedo a la espera de que matlab me envie la se�al de inicio por puerto serie, mientras, observo el pulsador del encoder
			// Si se presiona el boton, se sale del bucle y se vuelve al menu de selecci�n de modo
			matlab=0; // Clear flag
			while(matlab==0){
				if(button){button=0; routine=0; matlab=2; mode_selection=0; modo=0;}
			}
			// Si se recibió la señal deseada, muestreo
			if(matlab==1){
				LCD_clear();
				LCD_write_string_xy("Muestreando...",1,0);
				routine=1;
				matlab=0;
				step_on();	// Aplico un escal�n a la planta
			}
		}
		/*Rutina de inicializaci�n del modo controlador PID*/
		else if(modo==PID){
			LCD_clear();
			LCD_write_string_xy("Controlador:",1,0);
			LCD_write_string_xy("Canc. polo/cero",2,1);
			LCD_gotoxy(2,0);
			LCD_write('>');
			cursor=0;
			controller_selection=0;
			enter=0;
			// Selecciono el controlador que se desea implementar
			while(enter!=1){
				// Detecto si se gira el potenciometro del encoder
				if(turn){
					if((rotation())=='+'){
						if(cursor==2){cursor=0;}	// If OVF cursor...
						else{cursor++;}
					}
					else if((rotation())=='-'){
						if(cursor==0){cursor=2;}	// If UDF cursor...
						else{cursor--;}
					}
					// Una vez hecho el giro del potenciometro, muevo el "cursor" del display al controlador seleccionado
					switch(cursor){
						case 0:
						{
							LCD_write_string_xy("Canc. polo/cero ",2,1);
							LCD_gotoxy(2,0);
							LCD_write('>');
							break;
						}
						case 1:
						{
							LCD_write_string_xy("Enfoque IMC     ",2,1);
							LCD_gotoxy(2,0);
							LCD_write('>');
							break;
						}
						case 2:
						{
							LCD_write_string_xy("Atras           ",2,1);
							LCD_gotoxy(2,0);
							LCD_write('>');
							break;
						}
						default:
						break;
					}
					turn=0;
				}
				// Detecto si se presiona el pulsador del encoder
				if(button&&(cursor==0)){controller_selection=PID_canPC; button=0; enter=1;}
				else if(button&&(cursor==1)){controller_selection=PID_IMC; button=0; enter=1;}
				else if(button&&(cursor==2)){controller_selection=0; button=0; enter=1; routine=0; mode_selection=0; modo=0;}
				else{asm("NOP");}
			}
			// En caso de se haya seleccionado algun controlador, calculo los par�metros, sino, vuelvo al menu de selección de modo
			//toggle(PORTB,PORTB5);
			switch(controller_selection){
				case 0:
					break;
				case PID_canPC:
				{
					// Selección de parámetros A (alpha), b y c
					PID_parametters();
					
					// C�lculo de parámetros del controlador por cancelación polo-cero
					T0=(sqrt(A*A+A)+A)*L/2;
					Kxs=(2*T)/((L+4*T0)*Kp);
					
					break;
				}
				case PID_IMC:
				{
					// Seleccián de parámetros A (alpha), b y c
					PID_parametters();

					// Cálculo de parámetros del controlador IMC
					T0=L*A/(1-A);
					Kxs=T/(L+T0)/Kp;

					break;
				}
			}

			// En caso de haber seleccionado "OK", se inicializa el ADC y el TIMER1 (PWM) para generar la acción de control y se calculan los coeficientes
			if(routine){
				ADC_init(PID);
				PWM_init();
				cursor=0;
				
				// Calculo de parámetros del PID serie
				Tds=L/2;
				Tis=T;

				// Cálculo de parámetros de PID paralelo
				Tip=Tis+Tds;
				Tdp=Tis*Tds/Tip;
				Kxp=Kxs*Tip/Tis;
				Tf=L*A/2;

				// Coeficientes del filtro PID
				a_pid[0]=T_samp+2*Tf;
				a_pid[1]=T_samp-2*Tf;

				b_pid[0]=T_samp;
				b_pid[1]=T_samp;

				// Coeficientes del filtro a la referencia
				b_ref[0]=T_samp*T_samp+b*Tip*2*T_samp+c*Tip*Tdp*4;
				b_ref[1]=2*T_samp*T_samp-8*c*Tip*Tdp;
				b_ref[2]=T_samp*T_samp-b*Tip*2*T_samp+c*Tip*Tdp*4;
							
				a_ref[0]=T_samp*T_samp+Tip*2*T_samp+Tip*Tdp*4;
				a_ref[1]=2*T_samp*T_samp-8*Tip*Tdp;
				a_ref[2]=T_samp*T_samp-Tip*2*T_samp+Tip*Tdp*4;

				// Inicialización de valores en display
				LCD_clear();
				LCD_write_string_xy("Vout=     A=    ",1,0);
				LCD_write_string_xy("Vref=    ",2,0);
				update_value('R',curRef,0);
				update_value('O',0,0);
				update_value('A',curA,0);
			}
		}
		else{asm("NOP");}
		// Bucle de accion
		while (routine){
			dispatch();
		}
	}
}
/********************************************************************************************************/
/********************************************************************************************************/

/******************************************FUNCTION DECLARATION******************************************/
/********************************************************************************************************/
void dispatch (void ){
	
	// Si el ADC tiene una conversi�n, actuo
	if(conversion){
		if(modo==PID){
			// Actualizo el ciclo de trabajo del PWM
			PWM_duty(accion_de_control());
			if(actualizar==1){
				update_value('R',curRef,0);
				update_value('O',0,0);
				update_value('A',curA,0);	
				actualizar=0;
			}
		}
		else if(modo==ADQ){
			// Env�o datos por el puerto serie a la PC
			USART_write(ADC&0xFF);
			USART_write(ADC>>8);
			// Si la PC env�a "1", entonces finaliza la transmisi�n, se desactiva el USART y el ADC y se vuelve al inicio.
			if(matlab==1){
				LCD_clear();
				LCD_write_string_xy("Fin de muestreo.",1,0);
				_delay_ms(3000);
				routine=0;
				mode_selection=0;
				modo=0;
				USART_disable();
				ADC_disable();
				step_off();
			}
		}
		conversion=0;	// Clear flag
	}
	// Si se presiona el boton del encoder y me encuentro en el modo PID, detengo el controlador y entro al menú de parámetros
	if(button&&(modo==PID)){
		button=0;
		OCR1B=0;
		P=0; I=0; D=0;

		LCD_clear();
		LCD_write_string_xy(">Vref=      OK",1,0);
		LCD_write_string_xy(" A=      Atras",2,0);
		update_value('R',curRef,1);
		update_value('A',curA,1);

		cursor=0;
		enter=0;
		while(enter!=1){
			if(turn){
				if((rotation())=='+'){
					if(cursor==3){cursor=0;}	// If OVF cursor...
					else{cursor++;}
				}
				else if((rotation())=='-'){
					if(cursor==0){cursor=3;}	// If UDF cursor...
					else{cursor--;}
				}
				switch(cursor){
					case 0:
					{
						LCD_gotoxy(1,0);
						LCD_write('>');

						LCD_gotoxy(1,11);
						LCD_write(' ');
						LCD_gotoxy(2,0);
						LCD_write(' ');
						LCD_gotoxy(2,8);
						LCD_write(' ');
						break;
					}
					case 1:
					{
						LCD_gotoxy(1,11);
						LCD_write('>');

						LCD_gotoxy(1,0);
						LCD_write(' ');
						LCD_gotoxy(2,0);
						LCD_write(' ');
						LCD_gotoxy(2,8);
						LCD_write(' ');
						break;
					}

					case 2:
					{
						LCD_gotoxy(2,0);
						LCD_write('>');

						LCD_gotoxy(1,11);
						LCD_write(' ');
						LCD_gotoxy(1,0);
						LCD_write(' ');
						LCD_gotoxy(2,8);
						LCD_write(' ');
						break;
					}

					case 3:
					{
						LCD_gotoxy(2,8);
						LCD_write('>');

						LCD_gotoxy(1,11);
						LCD_write(' ');
						LCD_gotoxy(1,0);
						LCD_write(' ');
						LCD_gotoxy(2,0);
						LCD_write(' ');
						break;
					}

					default:
						break;
					}
				turn=0;
			}
			if(button&&cursor==0){	// Rutina si se presiona la opción Vref
				button=0;
				LCD_gotoxy(1,7);
				LCD_blinkON();
				intro=0;
				while(intro!=1){
					if(turn){
						if(rotation()=='+'){
							if(curRef==100){curRef=0;}	// If OVF cursor...
							else{curRef++;}
						}
						else if(rotation()=='-'){
							if(curRef==0){curRef=100;}	// If UDF cursor...
							else{curRef--;}
						}
						update_value('R',curRef,1);
						x_ref[0]=(float)curRef*5/100;
						turn=0;
					}
					if(button){button=0; intro=1; LCD_blinkOFF();}
				}
			}
			if(button&&cursor==1){	// Rutina si se presiona la opción OK
				button=0;
				enter=1;

				LCD_clear();
				LCD_write_string_xy("Vout=     A=    ",1,0);
				LCD_write_string_xy("Vref=",2,0);
			}
			if(button&&cursor==2){	// Rutina si se presiona la opción A
				button=0;
				LCD_gotoxy(2,4);
				LCD_blinkON();
				intro=0;
				while(intro!=1){
					if(turn){
						if(rotation()=='+'){
							if(curA==100){curA=0;}	// If OVF cursor...
							else{curA++;}
						}
						else if(rotation()=='-'){
							if(curA==0){curA=100;}	// If UDF cursor...
							else{curA--;}
						}
						update_value('A',curA,1);
						A=(float)curA/100;
						turn=0;
					}
					if(button){button=0; intro=1; LCD_blinkOFF();}
				}
			}
			if(button&&cursor==3){	// Rutina si se presiona la opción Atras
				button=0;
				enter=1;
				routine=0;

				LCD_clear();

				// Reset PID parameters
				x_ref[0]=2.5;
				curA=50;
				curb=50;
			}
		}
	}
}

void TIMER0_init(void){
	// Timer 0 mode: CTC
	TCCR0A|=(0<<WGM00);
	TCCR0A|=(1<<WGM01);
	TCCR0B|=(0<<WGM02);
	
	// Seteo el registro OCR0A para que al llegar a 47 (325Hz) active una interrupci�n
	OCR0A=Ts;
	
	// Interrupt on compare: ON
	TIMSK0|=(1<<OCIE0A);
	
	// Prescaler Timer0: 1024.
	TCCR0B|=(1<<CS02)|(0<<CS01)|(1<<CS00);
}
void mode_off(void){
	USART_disable();
	ADC_disable();
}

/******PID functions*******/
void PID_parametters(void ){
	//select=0--> Vref	 select=1--> A		select=2--> b		select=3--> c		select=4--> Atras		select=5-->OK
	LCD_clear();
	LCD_write_string_xy("Parametros: ",1,0);
	print_value('R',curRef);

	//Asigno valores por defecto
	x_ref[0]=(float)curRef*5/100;
	A=(float)curA/100;
	b=(float)curb/100;
	c=(float)curc/100;

	ok=0;
	select=0;
	while(ok!=1){
		if(turn){					// Detecto si se giro el encoder
			if(rotation()=='+'){
				if(select==5){select=0;}	// If OVF cursor...
				else{select++;}
			}
			else if(rotation()=='-'){
				if(select==0){select=5;}	// If UDF cursor...
				else{select--;}
			}
			LCD_write_string_xy("                ",2,0);
			switch(select){
				case 0:
					print_value('R',curRef);
					x_ref[0]=(float)curRef*5/100;
					break;
				case 1:
					print_value('A',curA);
					A=(float)curA/100;
					break;
				case 2:
					print_value('b',curb);
					b=(float)curb/100;
					break;
				case 3:
					print_value('c',curc);
					c=(float)curc/100;
					break;
				case 4:
					LCD_write_string_xy(">Atras",2,0);
					break;
				case 5:
					LCD_write_string_xy(">OK",2,0);
					break;
				default:
					break;
			}
			turn=0;					//  Clear flag
		}
		
		if(button&&select==0){		// Si se presiona el pulsador y el par�metro R se encuentra en pantalla...
			button=0;
			LCD_blinkON();
			intro=0;
			while(intro!=1){
				if(turn){
					if(rotation()=='+'){
						if(curRef==100){curRef=0;}	// If OVF cursor...
						else{curRef++;}
					}
					else if(rotation()=='-'){
						if(curRef==0){curRef=100;}	// If UDF cursor...
						else{curRef--;}
					}
					print_value('R',curRef);
					x_ref[0]=(float)curRef*5/100;
					turn=0;
				}
				if(button){button=0; intro=1; LCD_blinkOFF();}
			}
		}
		if(button&&select==1){		// Si se presiona el pulsador y el par�metro A se encuentra en pantalla...
			button=0;
			LCD_blinkON();
			intro=0;
			while(intro!=1){
				if(turn){
					if(rotation()=='+'){
						if(curA==100){curA=0;}	// If OVF cursor...
						else{curA++;}
					}
					else if(rotation()=='-'){
						if(curA==0){curA=100;}	// If UDF cursor...
						else{curA--;}
					}
					print_value('A',curA);
					A=(float)curA/100;
					turn=0;
				}
				if(button){button=0; intro=1; LCD_blinkOFF();}
			}
		}
		if(button&&select==2){		// Si se presiona el pulsador y el par�metro b se encuentra en pantalla...
			button=0;
			LCD_blinkON();
			intro=0;
			while(intro!=1){
				if(turn){
					if((rotation())=='+'){
						if(curb==100){curb=0;}	// If OVF cur...
						else{curb++;}
					}
					else if((rotation())=='-'){
						if(curb==0){curb=100;}	// If UDF cur...
						else{curb--;}
					}
					print_value('b',curb);
					b=(float)curb/100;
					turn=0;
				}
				if(button){button=0; intro=1; LCD_blinkOFF();}
			}
		}
		if(button&&select==3){		// Si se presiona el pulsador y el par�metro c se encuentra en pantalla...
			button=0;
			LCD_blinkON();
			intro=0;
			while(intro!=1){
				if(turn){
					if((rotation())=='+'){
						if(curc==100){curc=0;}	// If OVF cur...
						else{curc++;}
					}
					else if((rotation())=='-'){
						if(curc==0){curc=100;}	// If UDF cur...
						else{curc--;}
					}
					print_value('c',curc);
					c=(float)curc/100;
					turn=0;
				}
				if(button){button=0; intro=1; LCD_blinkOFF();}
			}
		}
		if(button&&select==4){		// Si se presiona el pulsador y el par�metro Atras se encuentra en pantalla...
			button=0;
			ok=1;
			routine=0;
		}
		if(button&&select==5){		// Si se presiona el pulsador y el par�metro OK se encuentra en pantalla...
			button=0;
			ok=1;
			routine=1;
		}
	}
	return;
}
void print_value(unsigned char value_print, unsigned char cursor){	// Rutina de escritura de par�metros en display: 0.00 hasta 1.00
	int cursor_aux=cursor;
	unsigned char aux, i;
	if(value_print=='R'){
		cursor_aux=5*cursor_aux;
	}
	LCD_gotoxy(2,0);
	LCD_write('>');
	LCD_gotoxy(2,1);
	LCD_write(value_print);
	LCD_gotoxy(2,2);
	LCD_write('=');
	for(i=0;i<3;i++){
		aux=cursor_aux%10;
		cursor_aux=cursor_aux/10;
		if(i==2){
			LCD_gotoxy(2,4);
			LCD_write(',');
			LCD_gotoxy(2,3);
		}
		else{
			LCD_gotoxy(2,6-i);
		}
		LCD_write(48+aux);
		if(i==2){
			LCD_gotoxy(2,7);
		}
	}
	return;
}
void update_value(unsigned char value_print, unsigned char cursor, unsigned char mod){	// Rutina de escritura de par�metros en display: 0.00 hasta 1.00
	uint16_t cursor_aux=cursor;
	unsigned char aux, i, fila, columna;
	if((value_print=='R')&&(mod==0)){
		cursor_aux=5*cursor_aux;
		fila=2;
		columna=8;
	}
	else if((value_print=='R')&&(mod=1)){
		cursor_aux=5*cursor_aux;
		fila=1;
		columna=9;
	}
	else if((value_print=='A')&&(mod==0)){
		fila=1;
		columna=15;
	}
	else if((value_print=='A')&&(mod==1)){
		fila=2;
		columna=6;
	}
	else{
		fila=1;
		columna=8;
		cursor_aux=ADC;
		cursor_aux=(int)(((float)cursor_aux*5/1024)*100);
		
	}
	for(i=0;i<3;i++){
		aux=cursor_aux%10;
		cursor_aux=cursor_aux/10;
		if(i==2){
			LCD_gotoxy(fila,columna-2);
			LCD_write(',');
			LCD_gotoxy(fila,columna-3);
		}
		else{
			LCD_gotoxy(fila,columna-i);
		}
		LCD_write(48+aux);
	}
	return;
}
float accion_de_control(void){										// Rutina de c�lculo de acci�n de control
	// Filtro a la referencia
	y_ref[0]=1/a_ref[0]*(b_ref[0]*x_ref[0]+b_ref[1]*x_ref[1]+b_ref[2]*x_ref[2]-a_ref[1]*y_ref[1]-a_ref[2]*y_ref[2]);

	// Actualizo valores del filtro a la referencia
	y_ref[2]=y_ref[1];
	y_ref[1]=y_ref[0];
	
	x_ref[2]=x_ref[1];
	x_ref[1]=x_ref[0];
	
	// Calculo del error y parámetros del PID paralelo
	error_actual=y_ref[0]-((float)ADC*5/1024);		// Obtenci�n del error
	P=error_actual*Kxp;								// Proporcional
	I+=T_samp*error_actual*q/Tip*Kxp;				// Integral
	D=(error_actual-error_anterior)/T_samp*Tdp*Kxp;	// Derivativo		
			
	x_pid[0]=P+I+D;
	error_anterior=error_actual;					// Actualizo el error
	
// Filtro para el PID paralelo
	y_pid[0]=1/a_pid[0]*(b_pid[0]*x_pid[0]+b_pid[1]*x_pid[1]-a_pid[1]*y_pid[1]);	
	
// Actualizo valores del PID
	y_pid[1]=y_pid[0];
	x_pid[1]=x_pid[0];


// Retorno de valores
	if(y_pid[0]>5){			// Saturacion
		y_pid[0]=5;
		q=0;
	}
	else if(y_pid[0]<0){	// Saturacion
		y_pid[0]=0;
		q=0;
	}
	else{
		q=1;
	}
	
	return y_pid[0];
	
}
void step_on(void){
	OCR1B=1023;
}
void step_off(void){
	OCR1B=0;
}
/**************************/