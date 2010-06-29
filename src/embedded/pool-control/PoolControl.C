#include "PoolControl.h"
#include "sevenseg.h"
#include <yaca.h>
#include <util/delay.h>

/*

Pin configurations
------------------

7 seg display, 3 digits, all outputs active low

     a
    ___
 f |   | b
   |_g_|
 e |   | c
   |___| * h (dot)
     d

PB7 segment 1
PB6 segment 2
PB0 segment 3
PD0 a
PD1 f
PC5 b   !!! bit 2
PD3 e
PD4 d
PD5 h
PD6 c
PD7 g

=================================================

PC4 charge pump for opamps
PB1 relay output, active high

*/

void delay_ms(uint16_t t) {
	uint16_t i;
	for(i = 0; i < t; i++) {
		_delay_ms(1);
		yc_dispatch_auto();
	}
}

int main() {
	uint8_t n = 0;

	DDRB |= SEVENSEG_SEGMASK | (1 << PB1); // be careful with port B (in use for CAN)
	DDRD = ~(1 << PD2); // PD2 is not ours
	DDRC = (1 << PC5) | (1 << PC4);

	while(1) {
		sevenseg_display(n, 0);
		if(++n == 11)
			n = 0;
		delay_ms(1000);
	}
}

