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
	
	yc_prepare_ee(YC_EE_LEDSTATUS_ID);
	yc_send(Switcha, LedStatus(a));
}

void DR(LedStatus()) {
	yc_prepare_ee(YC_EE_LEDSTATUS_ID);
	yc_send(Switcha, LedStatus((PORTD & (1 << PD4)) ? 1 : 0));
}

