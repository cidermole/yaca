#include "RTime.h"
#include "Time.h"
#include "libtimesync/timesync.h"
#include "../yaca-serial/calendar.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <yaca.h>

#define CENTURY 2000

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
	DCF_INIT,
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
volatile dcf_state_t dcf_state = DCF_INIT;
volatile uint8_t dcf_bit = 0, dcf_ticks = 0, dcf_count = 0, dcf_handle_bit = 0;
volatile uint8_t dcf_msg = 0;
uint8_t min, hour, day, month, dcf_time_ok = 0;
uint8_t lsec, lmin, lhour, lday, lmonth, dst = 0;
uint16_t year, lyear;
volatile uint32_t timer_local = 0, timer_corr = 0;
volatile uint8_t skip_corr = 0, timer_dms = 0;
int32_t old_time = 0;

volatile uint8_t dbg[8];

void debug_tx(volatile uint8_t *p) {
	yc_send(Time, Debug(p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]));
}

void init() {
	set_bit(DDRC, PC0); // set LED output
	clear_bit(DDRD, PD7);
	set_bit(PORTD, PD7); // enable DCF77 pull-up

	ts_init();

	TCCR1B = (1 << WGM12) | (1 << CS10); // CTC mode, top = OCR1A, prescaler 1
	OCR1A = 2000;
	TIMSK |= (1 << OCIE1A); // enable CTC interrupt
}

void enter_bootloader_hook() {
	TCCR1B = 0;
	TIMSK = 0;
	cli();
	yc_bld_reset();
}


int32_t ms_timer_local() {
	int32_t t;
	uint8_t sr = SREG;
	cli();
	t = timer_local;
	SREG = sr;
	return t;
}

int32_t ms_timer_corr() {
	int32_t t;
	uint8_t sr = SREG;
	cli();
	t = timer_corr;
	SREG = sr;
	return t;
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

uint8_t dcf_dispatch_bit() {
	static uint8_t dcf_symbol = 0, dcf_shift = 1, dcf_shift_count = 0;
	static uint8_t date_par = 0;
	uint8_t rv = 0;

	if(dcf_state == DCF_INIT) {
		dcf_init_symbol();
		dcf_state = DCF_BULK;
	}

	if(dcf_bit)
		dcf_symbol |= dcf_shift;
	dcf_shift <<= 1;
	dcf_shift_count++;

	switch(dcf_state) {
	case DCF_INIT:
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
			year = CENTURY + dcf_decode_bcd(dcf_symbol);
			date_par += dcf_parity(dcf_symbol);
			dcf_state = DCF_DATEPAR;
			dcf_init_symbol();
		}
		break;
	case DCF_DATEPAR:
		date_par += dcf_parity(dcf_symbol);
		if(date_par % 2 != 0)
			dcf_sync_state = DCF_RESET;
		dcf_msg = 0x02;
		dbg[1] = dcf_sync_state;
		dbg[2] = hour;
		dbg[3] = min;
		dbg[4] = day;
		dbg[5] = month;
		dbg[6] = year - 2000;
		dcf_state = DCF_BULK;

		//reported_time = 3600000UL * hour + 60000UL * min + 1000UL + 100UL;
		rv = 1;

/*		diff = ms_timer_corr() - reported_time;
		if(diff < 0)
			diff = -diff;
		if(dcf_time_ok == 0 || (diff > 6000)) {
			cli();
			sec = 58;
			timer_local = reported_time;
			if(dcf_symbol)
				timer_local += 100;
			timer_corr = timer_local;
			old_time = timer_local;
			ts_tick(timer_local % 60000, 1); // reset tick
			sei();
			dcf_msg = 0x03;
		}
		dcf_time_ok = 1;*/
		dcf_init_symbol();
		break;
	}

	dcf_handle_bit = 0;
	return rv;
}

// TODO: dst
void advance_time() {
	lsec++;
	if(lsec >= 60) {
		lsec = 0;
		lmin++;
	} else {
		return;
	}
	
	if(lmin >= 60) {
		lmin = 0;
		lhour++;
		
		// CEST starts on the last Sunday of March at 02:00 CET
		if(lmonth == 3 && lhour == 2 && (lday + 7) > 31 && day_of_week(lyear, lmonth, lday) == 0) {
			dst = 1;
			lhour = 3;
		}
		
		// CEST ends on the last Sunday of October at 03:00 CEST
		if(dst == 1 && lmonth == 10 && lhour == 3 && (lday + 7) > 31 && day_of_week(lyear, lmonth, lday) == 0) {
			dst = 0;
			lhour = 2;
		}
	} else {
		return;
	}
	
	if(lhour >= 24) {
		lhour = 0;
		lday++;
	} else {
		return;
	}
	
	if(lday > days_in_month(lmonth, lyear)) {
		lday = 1;
		lmonth++;
	} else {
		return;
	}
	
	if(lmonth > 12) {
		lmonth = 1;
		lyear++;
	}
}

int main() {
	uint8_t last_state = bit_is_set(PIND, PD7);
	int16_t fb;
	int8_t tfb;
	int32_t reported_time, ct, last_sec = 0, diff;
	uint8_t dcf_minute = 0;

	init();
	sei();

	while(1) {
		// sync seconds
		if(!last_state && bit_is_set(PIND, PD7) && dcf_time_ok) {
			ct = ms_timer_corr();
			if(ct - last_sec > 900) { // spike filter

				if(dcf_minute) {
					reported_time = 3600000UL * hour + 60000UL * min;
					diff = ms_timer_corr() - reported_time;
					if(diff < 0)
						diff = -diff;
					if(dcf_time_ok == 0 || (diff > 6000)) {
						cli();
						timer_local = reported_time;
						timer_corr = timer_local;
						old_time = timer_local;
						ts_tick(timer_local % 60000, 1); // reset tick
						lyear = year;
						sei();
						lmonth = month;
						lday = day;
						lhour = hour;
						lmin = min;
						lsec = 0;
						dcf_msg = 0x03;
					}
					dcf_time_ok = 1;
					dcf_minute = 0;
				}
				reported_time = 3600000UL * hour + 60000UL * min + 1000UL * lsec;

				fb = ts_slot(ms_timer_local(), ct, reported_time);
				dbg[0] = ((uint8_t *) (&fb))[1];
				dbg[1] = ((uint8_t *) (&fb))[0];
				dbg[2] = lsec;
				dbg[3] = 0x00;
				yc_prepare(798);
				debug_tx(dbg);
				yc_dispatch_auto();

				advance_time();
			}
			last_sec = ct;
		}
		last_state = bit_is_set(PIND, PD7);

		if(ms_timer_local() != old_time) {
			tfb = ts_tick(ms_timer_corr() % 60000, 0);
			if(tfb == 1) {
				cli();
				timer_corr++;
				sei();
			} else if(tfb == -1) {
				cli();
				skip_corr = 1;
				sei();
			}
			old_time++;
		}

		if(bit_is_set(PIND, PD7))
			LED_on();
		else
			LED_off();

		if(dcf_msg) {
			yc_prepare(799);
			dbg[0] = dcf_msg;
			debug_tx(dbg);
			dcf_msg = 0;
		}

		if(dcf_handle_bit) {
			dcf_minute = dcf_dispatch_bit();
			dcf_handle_bit = 0;
		}

		yc_dispatch_auto();
	}
	return 0;
}


// tick every ms
ISR(TIMER1_COMPA_vect) {
	static uint8_t bits = 0;

	if(skip_corr)
		skip_corr = 0;
	else
		timer_corr++;
	timer_local++;

	if(++timer_dms != 10) // continue in this function every 10 ms
		return;

	timer_dms = 0;

	if(bit_is_set(PIND, PD7)) {
		if(dcf_ticks == 0) {
			if(dcf_count >= 198 && dcf_count <= 202) { // minute marker?
				dcf_sync_state = DCF_SYNC;
				dcf_state = DCF_INIT;
				dcf_msg = 0x01;
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
	} else if(dcf_ticks != 0) {
		// error
		dcf_sync_state = DCF_RESET;
		dcf_msg = 0xFF;
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

#include "../yaca-serial/calendar.c"

