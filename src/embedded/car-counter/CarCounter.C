#include "CarCounter.h"
#include "RCarCounter.h"
#include <avr/interrupt.h>
#include <yaca.h>

/*

PD7: relay of car-counter pulling vs. GND

*/

uint32_t count = 0;
volatile uint8_t timer_ovf = 0;

void enter_bootloader_hook() {
	// deinit debounce timer
	TCCR0 = 0;
	TIMSK = 0;

	cli();
	yc_bld_reset();
}

void DR(Count()) {
	yc_prepare_ee(YC_EE_COUNT_ID);
	yc_send(CarCounter, Count(count));
}

int main() {
	uint8_t state = 0;

	set_bit(PORTD, PD7); // PD7: input with pullup
	clear_bit(DDRD, PD7);

	// init timer for debounce
	TCCR0 = (1 << CS02); // prescaler 256
	TIMSK |= TOIE1; // enable overflow interrupt. 2 MHz / (256 * 256) => ~ 32 ms ticks for debounce

	sei();

	while(1) {
		if(timer_ovf) {
			switch(state) {
			case 0:
				if(bit_is_clear(PIND, PD7))
					state = 1;
				break;
			case 1: // glitch filter
				if(bit_is_clear(PIND, PD7)) {
					state = 2;
					count++;
					yc_status(Count);
				} else
					state = 0;
				break;
			case 2: // relay on
				if(bit_is_set(PIND, PD7))
					state = 3;
				break;
			case 3: // glitch filter (relay on)
				if(bit_is_set(PIND, PD7))
					state = 0;
				else
					state = 2;
				break;
			default:
				state = 0;
			}
			timer_ovf = 0;
		}
		yc_dispatch_auto();
	}
	return 0;
}

ISR(TIMER0_OVF_vect) {
	timer_ovf = 1;
}

