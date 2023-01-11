#include <avr/io.h>

#define PID 10
#define ADQ 9

void ADC_init(unsigned char);

int ADC_get(char value);

void ADC_disable(void);
