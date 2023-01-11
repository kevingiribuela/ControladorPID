#include<avr/io.h>        // AVR header
#include<util/delay.h>    // Delay header

// Sent commands
void LCD_cmd(unsigned char cmd);

// Initialize display
void LCD_Init(void);

// Write data
void LCD_write(unsigned char data);

// Cursor position
void LCD_gotoxy (unsigned char x, unsigned char y);

// Write strings X,Y
void LCD_write_string_xy(const char *msg,unsigned char row, unsigned char pos);
 
// Write strings
void LCD_write_string(const char *str);
 
// Clear display
void LCD_clear(void);

// Blink ON
void LCD_blinkON(void);

// Blink OFF
void LCD_blinkOFF(void);