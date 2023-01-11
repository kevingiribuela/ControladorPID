#include<avr/io.h>        // AVR header
#include<util/delay.h>    // delay header
#include <LCD_AVR.h>        // LCD header

// LCD init
void init_LCD(void){
	  
	DDRD|=(1<<RS);			// RS -> OUTPUT
	DDRD|=(1<<EN);			// EN  -> OUTPUT

	DDRD|=(1<<PORTD6);		// D4 -> OUTPUT
	DDRD|=(1<<PORTD7);		// D5 -> OUTPUT
	DDRB|=(1<<PORTB0);		// D6 -> OUTPUT
	DDRB|=(1<<PORTB1);		// D7 -> OUTPUT

	LCD_cmd(0x28);			// 4bit mode 16X2 LCD
	_delay_ms(1);

	LCD_cmd(0x01);			// Clear display screen
	_delay_ms(1);

	LCD_cmd(0x02);			// Return home
	_delay_ms(1);

	LCD_cmd(0x06);			//  Increment cursor (shift cursor to right)
	_delay_ms(1);

	LCD_cmd(0x80);			// Force cursor to beginning of first line
	_delay_ms(1);

	return;
}

// Cursor location
void LCD_gotoxy (unsigned char x,unsigned char y){            // y=1,2  x=1,...,16
   unsigned char inicio[]={0x80,0xC0};
   LCD_cmd(inicio[y-1]+x-1);
   _delay_ms(10);
}
   
// Sent commands
void LCD_cmd(unsigned char cmd){
 /*Sent high nibble*/
PORTB=(PORTB&0xFC)|((cmd>>6)&(0x03));	// PORTB = XXXX XXD7D6
PORTD=(PORTD&0x3F)|((cmd<<2)&(0xC0));	// PORTD = D5D4XX XXXX

PORTD&=~(1<<RS);	// Select command register
PORTD|=(1<<EN);		// Set EN
_delay_ms(10);
PORTD  &= ~(1<<EN);	// Clear EN

_delay_ms(1);

 /*Sent low nibble*/
PORTB=(PORTB&0xFC)|((cmd>>2)&(0x03));	// PORTB = XXXX XXD3D2
PORTD=(PORTD&0x3F)|((cmd<<6)&(0xC0));	// PORTD = D1D0XX XXXX

PORTD&=~(1<<RS);	// Select command register
PORTD|=(1<<EN);		// Set EN
_delay_ms(10);
PORTD  &= ~(1<<EN);	// Clear EN

return;
}

 
// Write data
void LCD_write(unsigned char data){
	
 /*Sent high nibble*/
PORTB=(PORTB&0xFC)|((data>>6)&(0x03));	// PORTB = XXXX XXD7D6
PORTD=(PORTD&0x3F)|((data<<2)&(0xC0));	// PORTD = D5D4XX XXXX

PORTD  |= (1<<RS);	// Select data register
PORTD  |= (1<<EN);	// Set EN
_delay_ms(10);		
PORTD &= ~(1<<EN);  // Clear EN

_delay_ms(1);

 /*Sent low nibble*/
 PORTB=(PORTB&0xFC)|((data>>2)&(0x03));	// PORTB = XXXX XXD3D2
 PORTD=(PORTD&0x3F)|((data<<6)&(0xC0));	// PORTD = D1D0XX XXXX
 
 PORTD  |= (1<<RS);	// Select data register
 PORTD  |= (1<<EN);	// Set EN
 _delay_ms(10);
 PORTD &= ~(1<<EN);  // Clear EN

return ;
}

// Write string
void LCD_write_string(unsigned char *str){
	
	unsigned char i=0;		// Auxiliar variable
		
    while (str[i] != 0x00)
	{
		LCD_write ( str[i++]);	// Write a single character
	}
	return;
 }

 