#ifndef F_CPU
#define F_CPU 1000000UL // Clock speed is 16MHz
#endif

#include<avr/io.h>        // AVR header
#include<util/delay.h>    // Delay header

//======================================================
 // Startup LCD library
 //=====================================================

#define EN PORTD5              // Enable:	PIND5
#define RS PORTD4              // RS:		PIND4

// Sent commands
void LCD_cmd(unsigned char cmd);

// Initialize display
void init_LCD(void);

// Write data
void LCD_write(unsigned char data);

// Cursor position
void LCD_gotoxy (unsigned char x, unsigned char y);
 
// Write strings
void LCD_write_string(unsigned char *str);
 