#include <avr/io.h>
#include <USART.h>

// UART Init
void USART_Init(void){
	
	// Mode select: Asynchronous USART
	UCSR0C|=(0<<UMSEL01)|(0<<UMSEL00);
	
	// Parity mode: Disabled
	UCSR0C|=(0<<UPM01)|(0<<UPM00);
	
	// Stop bit: 1 bits
	UCSR0C|=(0<<USBS0);
	
	// Number of data bits: 8 bits
	UCSR0B|=(0<<UCSZ02);
	UCSR0C|=(1<<UCSZ00)|(1<<UCSZ01);
	
	// Clock polarity: Off
	UCSR0A|=(0<<UCPOL0);
	
	// UBRR set at baud rate: 9600 bauds
	UBRR0H=(unsigned char)((MYUBRR)>>8);
	UBRR0L=(unsigned char)(MYUBRR);
	
	// Receive Interrupt enable 
	UCSR0B|=(1<<RXCIE0);
}

// Write TX buffer
void USART_write(unsigned char data){
	// Wait for empty transmit buffer
	while (!(UCSR0A & (1<<UDRE0)));
	
	// Put data into buffer, sends the data
	UDR0 = data;
	return;
}

// Read RX buffer
unsigned char USART_read(void){
	/* Get and return received data from buffer */
	return UDR0;
}

// Enable USART
void USART_enable(void){
	
	UCSR0B|=(1<<TXEN0)|(1<<RXEN0);
}

// Disable USART
void USART_disable(void){
	
	UCSR0B&=~((1<<RXEN0)|(1<<TXEN0));
}