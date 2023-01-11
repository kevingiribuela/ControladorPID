#include <avr/io.h>

#define FOSC 16000000UL
#define BAUD 9600
#define MYUBRR FOSC/BAUD/16-1

// USART init
void USART_Init(void);

// USART write char
void USART_write(unsigned char data);

// USART read data
unsigned char USART_read(void);

// Disable RX/TX
void USART_disable(void);

// Enable RX/TX
void USART_enable(void);