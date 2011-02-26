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
volatile uint8_t dcf_bit = 0, dcf_ticks = 0, dcf_count = 0, dcf_handle_bit = 0;

void init() {
	set_bit(DDRC, PC0); // set LED output
	clear_bit(DDRD, PD7);
	set_bit(PORTD, PD7); // enable DCF77 pull-up

	TCCR1B = (1 << WGM12) | (1 << CS10); // CTC mode, top = OCR1A, prescaler 1
	OCR1A = 10000;
	TIMSK |= (1 << OCIE1A); // enable CTC interrupt
}

void enter_bootloader_hook() {
	TCCR1B = 0;
	TIMSK = 0;
	cli();
	yc_bld_reset();
}


#define dcf_init_symbol() \
	do { \
		dcf_symbol = 0; \
		dcf_shift = 1; \
		dcf_shift_count = 0; \
		 \
	} while(0)

uint8_t dcf_decode_bcd(uint8_t symbol) {
	return ((symbol & 0x0F) + ((symbol & 0xF0) >> 4) * 10);
}

uint8_t dcf_parity(uint8_t symbol) {
	uint8_t i, parity = 0;
	for(i = 0; i < 8; i++, symbol >>= 1)
		if(symbol & 1)
			parity++;
	return parity % 2;
}

void dcf_dispatch_bit() {
	static uint8_t dcf_symbol = 0, dcf_shift = 1, dcf_shift_count = 0;
	static uint8_t min, hour, day, month, year, date_par = 0;

	if(dcf_bit)
		dcf_symbol |= dcf_shift;
	dcf_shift <<= 1;
	dcf_shift_count++;

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
			min = dcf_decode_bcd(dcf_symbol & ~(1 << 7));
			if(dcf_parity(dcf_symbol))
				dcf_sync_state = DCF_RESET;
			dcf_state = DCF_HOUR;
			dcf_init_symbol();
		}
		break;
	case DCF_HOUR:
		if(dcf_shift_count == 7) {
			hour = dcf_decode_bcd(dcf_symbol & ~(1 << 6));
			if(dcf_parity(dcf_symbol))
				dcf_sync_state = DCF_RESET;
			date_par = 0;
			dcf_state = DCF_DOM;
			dcf_init_symbol();
		}
		break;
	case DCF_DOM:
		if(dcf_shift_count == 6) {
			day = dcf_decode_bcd(dcf_symbol);
			date_par += dcf_parity(dcf_symbol);
			dcf_state = DCF_DOW;
			dcf_init_symbol();
		}
		break;
	case DCF_DOW:
		if(dcf_shift_count == 3) {
			date_par += dcf_parity(dcf_symbol);
			dcf_state = DCF_MONTH;
			dcf_init_symbol();
		}
		break;
	case DCF_MONTH:
		if(dcf_shift_count == 5) {
			month = dcf_decode_bcd(dcf_symbol);
			date_par += dcf_parity(dcf_symbol);
			dcf_state = DCF_YEAR;
			dcf_init_symbol();
		}
		break;
	case DCF_YEAR:
		if(dcf_shift_count == 8) {
			year = dcf_decode_bcd(dcf_symbol);
			date_par += dcf_parity(dcf_symbol);
			dcf_state = DCF_DATEPAR;
			dcf_init_symbol();
		}
		break;
	case DCF_DATEPAR:
		date_par += dcf_parity(dcf_symbol);
		if(date_par % 2 != 0)
			dcf_sync_state = DCF_RESET;
		dcf_state = DCF_BULK;
		dcf_init_symbol();
		break;
	}

	dcf_handle_bit = 0;
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


// tick every 10 ms
ISR(TIMER1_COMPA_vect) {
	static uint8_t bits = 0;

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
		dcf_bit = 0;
		if(dcf_sync_state == DCF_SYNC)
			dcf_handle_bit = 1;
	} else if(dcf_ticks >= 19 && dcf_ticks <= 21) {
		dcf_bit = 1;
		if(dcf_sync_state == DCF_SYNC)
			dcf_handle_bit = 1;
	} else {
		// error
		dcf_sync_state = DCF_RESET;
	}

	if(bit_is_clear(PIND, PD7))
		dcf_ticks = 0;

	if(dcf_count < 250)
		dcf_count++;

	if(dcf_handle_bit && bits == 0) {
		dcf_handle_bit = 0; // don't handle minute marker
		bits++;
	} else if(dcf_handle_bit) {
		bits++;
	}
}

