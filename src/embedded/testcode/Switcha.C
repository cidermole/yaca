#include "Switcha.h"
#include "RSwitcha.h"
#include <yaca.h>
#include <avr/io.h>

void init() {
	DDRD |= (1 << PD4);
}

void DM(SetLed(uint8_t a)) {
	// handle it
	if(a)
		PORTD |= (1 << PD4);
	else
		PORTD &= ~(1 << PD4);
	yc_prepare(0x12345678);
	yc_send(Switcha, LedStatus(a));
}

