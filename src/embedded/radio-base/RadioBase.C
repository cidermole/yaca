#include "RadioBase.h"
#include "RRadioBase.h"
#include "librfm12/rfm12.h"
#include <yaca.h>
#include <avr/io.h>

uint8_t _n = 0;

void DR(Wtf()) {
	yc_prepare(406);
	yc_send(RadioBase, Wtf(_n++));
}

void init() {
	DDRB |= (1 << PB1);
	PORTB |= (1 << PB1); // disable CS of RFM12

	RFM12_PHY_init();
}

