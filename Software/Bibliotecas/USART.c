#include <avr/io.h>
#include <USART.h>

void USART_Init(void){
	
	// Mode select: Asynchronous USART
	UCSR0C|=(0<<UMSEL01)+(0<<UMSEL00);
	
	// Parity mode: Disabled
	UCSR0C|=(0<<UPM01)+(0<<UPM00);
	
	// Stop bit: 1 bits
	UCSR0C|=(0<<USBS0);
	
	// Number of data bits: 8 bits
	UCSR0B|=(0<<UCSZ02);
	UCSR0C|=(1<<UCSZ00)+(1<<UCSZ01);
	
	// Clock polarity: Off
	UCSR0A|=(0<<UCPOL0);
	
	// UBRR set at baud rate: 9600 bauds
	UBRR0H=(unsigned char)((MYUBRR)>>8);
	UBRR0L=(unsigned char)(MYUBRR);
	
	// Enable transmitter
	UCSR0B|=(1<<TXEN0);
	
	return;
}
void USART_write(unsigned char data){
	// Wait for empty transmit buffer
	while (!(UCSR0A & (1<<UDRE0)));
	
	// Put data into buffer, sends the data
	UDR0 = data;
	
	return;
}
void USART_write_txt(char* str){
	while (*str){
		USART_write(*str++);
	}
	
	return;
}
