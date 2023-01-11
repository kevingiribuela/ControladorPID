#include <avr/io.h>

#define FOSC 16000000UL
#define BAUD 9600
#define MYUBRR FOSC/16UL/BAUD-1

// USART init
void USART_Init(void);
// USART write char
void USART_write(unsigned char data);
// USART write text
void USART_write_txt(char* str);