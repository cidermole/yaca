#ifndef SEVENSEG_H
#define SEVENSEG_H

#include <stdint.h>
#include <avr/io.h>

/*

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

*/

#define SEVENSEG_DOT (1 << 5)
#define SEVENSEG_SEGMASK ((1 << PB7) | (1 << PB6) | (1 << PB0))

uint8_t _sevenseg_encode(uint8_t digit);

void sevenseg_display(uint16_t number, uint8_t decimals) {
	uint8_t dig = _sevenseg_encode((uint8_t) number);
	PORTD = dig & ~(1 << PD2); // bit 2 is not ours
	PORTB = (PORTB & ~SEVENSEG_SEGMASK) | (1 << PB6) | (1 << PB0);
	if(dig & (1 << 2))
		PORTC |= (1 << PC5);
	else
		PORTC &= ~(1 << PC5);
}

uint8_t _sevenseg_encode(uint8_t digit) {
	switch(digit) {           // gchd ebfa
		case 0: return 0xA0;  // 1010 0000
		case 1: return 0xBB;  // 1011 1011
		case 2: return 0x62;  // 0110 0010
		case 3: return 0x2A;  // 0010 1010
		case 4: return 0x39;  // 0011 1001
		case 5: return 0x2C;  // 0010 1100
		case 6: return 0x24;  // 0010 0100
		case 7: return 0xBA;  // 1011 1010
		case 8: return 0x20;  // 0010 0000
		case 9: return 0x28;  // 0010 1000
		default: return 0x7F; // 0111 1111
	}
}

#endif /* SEVENSEG_H */

