#include <avr/io.h>

#define CLK PINC0			// CLK encoder pin
#define DT PIND2			// DT encoder pin

void encoder_init(void);

char rotation (void);

