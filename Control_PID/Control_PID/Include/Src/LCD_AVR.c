#ifndef F_CPU
#define F_CPU 16000000UL	// Clock speed is 16MHz
#endif

#include <avr/io.h>			// AVR header
#include <util/delay.h>		// delay header
#include <LCD_AVR.h>		// LCD header

#define EN PORTD5			// Enable:	PIND5
#define RS PORTD4			// RS:		PIND4

// LCD init
void LCD_Init(void){
	  
	DDRD|=(1<<RS);			// RS -> OUTPUT
	DDRD|=(1<<EN);			// EN  -> OUTPUT

	DDRD|=(1<<PORTD6);		// D4 -> OUTPUT
	DDRD|=(1<<PORTD7);		// D5 -> OUTPUT
	DDRB|=(1<<PORTB0);		// D6 -> OUTPUT
	DDRB|=(1<<PORTB1);		// D7 -> OUTPUT
	
	_delay_ms(40);			// Wait 40mS for power supply stability

	LCD_cmd(0x03);			// Datasheet suggestion
	_delay_ms(10);			// Datasheet suggestion

	LCD_cmd(0x03);			// Datasheet suggestion
	_delay_ms(100);			// Datasheet suggestion

	LCD_cmd(0x03);			// Datasheet suggestion

	LCD_cmd(0x02);			// Display enable
	LCD_cmd(0x28);			// 4bit mode 16X2 LCD
	LCD_cmd(0x0C);			// Display ON, Cursor OFF	
	LCD_cmd(0x01);			// Clear Display
	LCD_cmd(0x06);			// Entry mode set: Increment cursor when write(shift cursor to right)
}

// Prepare data
void LCD_prepare(char nibble,char data){
	/*Prepare high nibble*/
	if(nibble=='H'){
		PORTB=(PORTB&0xFC)|((data>>6)&(0x03));	// PORTB = XXXX XXD7D6
		PORTD=(PORTD&0x3F)|((data<<2)&(0xC0));	// PORTD = D5D4XX XXXX
	}
	/*Prepare low nibble*/
	else if(nibble=='L'){
		PORTB=(PORTB&0xFC)|((data>>2)&(0x03));	// PORTB = XXXX XXD3D2
		PORTD=(PORTD&0x3F)|((data<<6)&(0xC0));	// PORTD = D1D0XX XXXX
	}
}

// Sent commands
void LCD_cmd(unsigned char cmd){

	PORTD&=~(1<<RS);		// Select command register

	// Sent high nibble
	LCD_prepare('H',cmd);	// Prepare high nibble
	PORTD|=(1<<EN);			// Set EN
	_delay_ms(1);
	PORTD&=~(1<<EN);		// Clear EN
	_delay_ms(1);
	
	// Sent low nibble
	LCD_prepare('L',cmd);	// Prepare low nibble
	PORTD|=(1<<EN);			// Set EN
	_delay_ms(1);
	PORTD&=~(1<<EN);		// Clear EN
	_delay_ms(1);
}

// Write data
void LCD_write(unsigned char data){

	PORTD  |= (1<<RS);		// Select data register

	// Sent high nibble
	LCD_prepare('H',data);	// Prepare high nibble
	PORTD  |= (1<<EN);		// Set EN
	_delay_ms(1);
	PORTD &= ~(1<<EN);  	// Clear EN
	_delay_ms(1);

	// Sent low nibble
	LCD_prepare('L',data);	// Prepare low nibble
	PORTD  |= (1<<EN);		// Set EN
	_delay_ms(1);
	PORTD &= ~(1<<EN);		// Clear EN
	_delay_ms(1);
}


// Cursor location
void LCD_gotoxy (unsigned char row,unsigned char pos){            // row=1,2  pos=0,...,15
   	 char location=0;
   	 if(row<=1)
   	 {
	   	 location=(0x80) | ((pos) & 0x0f);  /*Print message on 1st row and desired location*/
	   	 LCD_cmd(location);
   	 }
   	 else
   	 {
	   	 location=(0xC0) | ((pos) & 0x0f);  /*Print message on 2nd row and desired location*/
	   	 LCD_cmd(location);
   	 }
}

// Write string in (x,y) --> x=1,2   y=0,...,15
void LCD_write_string_xy(const char *msg, unsigned char row, unsigned char pos)
 {
	 char location=0;
	 if(row<=1)
	 {
		 location=(0x80) | ((pos) & 0x0f);  /*Print message on 1st row and desired location*/
		 LCD_cmd(location);
	 }
	 else
	 {
		 location=(0xC0) | ((pos) & 0x0f);  /*Print message on 2nd row and desired location*/
		 LCD_cmd(location);
	 }
	 

	 LCD_write_string(msg);
 }  

// Write string
void LCD_write_string(const char *msg){
	while((*msg)!=0)
	{
		LCD_write(*msg);
		msg++;
	}
}

// Clear display
 void LCD_clear(void){
	 LCD_cmd(0x01);
 }
 
// Turn on cursor
 void LCD_blinkON(void){
	 LCD_cmd(0x0F);			// Cursor Off
	 _delay_ms(1);
 }
 
// Turn off cursor
 void LCD_blinkOFF(void){
	 LCD_cmd(0x0C);			// Cursor Off
	 _delay_ms(1);
 }