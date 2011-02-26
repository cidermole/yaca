#include "Time.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <yaca.h>

/*

PC0: LED (active high)
PD7: DCF77 input (needs pull-up)

*/

#define LED_on() set_bit(PORTC, PC0)
#define LED_off() clear_bit(PORTC, PC0)

typedef enum {
	DCF_RESET,
	DCF_SYNC
} dcf_sync_state_t;

typedef enum {
	DCF_BULK,
	DCF_STATUS,
	DCF_MINUTE,
	DCF_HOUR,
	DCF_DOM,
	DCF_DOW,
	DCF_MONTH,
	DCF_YEAR,
	DCF_DATEPAR
} dcf_state_t;

volatile dcf_sync_state_t dcf_sync_state = DCF_RESET;
volatile dcf_state_t dcf_state = DCF_BULK;
volatile uint8_t dcf_bit = 0, dcf_ticks = 0, dcf_count = 0;

void init() {
	set_bit(DDRC, PC0); // set LED output
	clear_bit(DDRD, PD7);
	set_bit(PORTD, PD7); // enable DCF77 pull-up

	TCCR1B = (1 << WGM12) | (1 << CS10); // CTC mode, top = OCR1A, prescaler 1
	TIMSK |= (1 << OCIE1A); // enable CTC interrupt
}

void enter_bootloader_hook() {
	TCCR1B = 0;
	TIMSK = 0;
	cli();
	yc_bld_reset();
}

int main() {
	init();

	while(1) {
		if(bit_is_set(PIND, PD7))
			LED_on();
		else
			LED_off();
		yc_dispatch_auto();
	}
	return 0;
}

#define dcf_init_symbol() \
	do { \
		dcf_symbol = 0; \
		dcf_shift = 1; \
		dcf_shift_count = 0; \
		 \
	} while(0)

// tick every 10 ms
ISR(TIMER1_COMPA_vect) {
	static uint16_t dcf_symbol = 0, dcf_shift = 1, dcf_shift_count = 0;
	uint8_t handle_bit = 0, bit = 0, bits = 0;

	if(bit_is_set(PIND, PD7)) {
		if(dcf_ticks == 0) {
			if(dcf_count >= 198 && dcf_count <= 202) { // minute marker?
				dcf_sync_state = DCF_SYNC;
				dcf_state = DCF_BULK;
				bits = 0;
			}
			dcf_count = 0;
		}
		dcf_ticks++;
	} else if(dcf_ticks >= 9 && dcf_ticks <= 11) {
		bit = 0;
		handle_bit = 1;
	} else if(dcf_ticks >= 19 && dcf_ticks <= 21) {
		bit = 1;
		handle_bit = 1;
	} else {
		// error
		dcf_sync_state = DCF_RESET;
		dcf_init_symbol();
	}

	if(bit_is_clear(PIND, PD7))
		dcf_ticks = 0;

	if(dcf_count < 250)
		dcf_count++;

	if(handle_bit && dcf_sync_state == DCF_SYNC) {
		if(bits) { // don't process bit 0 (minute marker)
			if(bit)
				dcf_symbol |= dcf_shift;
			dcf_shift <<= 1;
			dcf_shift_count++;
		}

		switch(dcf_state) {
		case DCF_BULK:
			if(dcf_shift_count == 14) {
				dcf_state = DCF_STATUS;
				dcf_init_symbol();
			}
			break;
		case DCF_STATUS:
			if(dcf_shift_count == 6) {
				dcf_state = DCF_MINUTE;
				dcf_init_symbol();
			}
			break;
		case DCF_MINUTE:
			if(dcf_shift_count == 8) {
				// TODO: decode
				dcf_state = DCF_HOUR;
				dcf_init_symbol();
			}
			break;
		case DCF_HOUR:
			if(dcf_shift_count == 7) {
				// TODO: decode
				dcf_state = DCF_DOM;
				dcf_init_symbol();
			}
			break;
		case DCF_DOM:
			if(dcf_shift_count == 6) {
				// TODO: decode
				dcf_state = DCF_DOW;
				dcf_init_symbol();
			}
			break;
		case DCF_DOW:
			if(dcf_shift_count == 3) {
				// TODO: decode
				dcf_state = DCF_MONTH;
				dcf_init_symbol();
			}
			break;
		case DCF_MONTH:
			if(dcf_shift_count == 5) {
				// TODO: decode
				dcf_state = DCF_YEAR;
				dcf_init_symbol();
			}
			break;
		case DCF_YEAR:
			if(dcf_shift_count == 8) {
				// TODO: decode
				dcf_state = DCF_DATEPAR;
				dcf_init_symbol();
			}
			break;
		case DCF_DATEPAR:
			// TODO: decode
			dcf_state = DCF_DATEPAR;
			dcf_init_symbol();
			break;
		}

		bits++;
	}
}

