#include "RTime.h"
#include "Time.h"
#include "../yaca-serial/calendar.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <yaca.h>
#include <string.h>

#define CENTURY 2000

/*

PC0: LED (active high)
PD7: DCF77 input (needs pull-up)

*/

#define LED_on() set_bit(PORTC, PC0)
#define LED_off() clear_bit(PORTC, PC0)

#define DCF_Z1_CEST 2 // Z1: CEST
#define DCF_Z2_CET 3 // Z2: CET

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
volatile uint8_t dcf_msg = 0, dcf_minutemarker_valid = 0;
uint8_t min, hour, day, month, dst, dcf_time_ok = 0;
uint8_t lsec, lmin, lhour, lday, lmonth, ldst = 0;
uint16_t year, lyear;
volatile uint32_t timer_local = 0, timer_corr = 0;
volatile uint8_t skip_corr = 0, timer_dms = 0;
int32_t slot_start, last_minute, vts_next = 0;

volatile uint8_t dbg[8];

void debug_tx(volatile uint8_t *p) {
	yc_send(Time, Debug(p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]));
}

void init() {
	set_bit(DDRC, PC0); // set LED output
	clear_bit(DDRD, PD7);
	set_bit(PORTD, PD7); // enable DCF77 pull-up

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

void dcf_dispatch_bit() {
	static uint8_t dcf_symbol = 0, dcf_shift = 1, dcf_shift_count = 0;
	static uint8_t date_par = 0;

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
			if(bit_is_set(dcf_symbol, DCF_Z1_CEST) && bit_is_clear(dcf_symbol, DCF_Z2_CET))
				dst = 1;
			else if(bit_is_set(dcf_symbol, DCF_Z2_CET) && bit_is_clear(dcf_symbol, DCF_Z1_CEST))
				dst = 0;
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

		cli();
		dcf_minutemarker_valid = 1;
		sei();

		dcf_init_symbol();
		break;
	}

	dcf_handle_bit = 0;
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
			ldst = 1;
			lhour = 3;
		}
		
		// CEST ends on the last Sunday of October at 03:00 CEST
		if(ldst == 1 && lmonth == 10 && lhour == 3 && (lday + 7) > 31 && day_of_week(lyear, lmonth, lday) == 0) {
			ldst = 0;
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

void DM(AddTimeOffset(int16_t ms)) {
	cli();
	timer_corr += ms;
	sei();
}

int main() {
	int32_t ct, next_sec = 0;
	int32_t vts_dist = 2270;
	uint8_t old_time_ok = 0;

	init();
	sei();

	while(1) {
		if(ms_timer_local() >= vts_next) {
			vts_next += vts_dist;
			cli();
			timer_corr++;
			sei();
		}

		// TODO: test if this timing is accurate enough (will we be here the right ms? what about high bus load?)
		// TODO: check if this can be united with the if below (ct >= next_sec)

		// midnight counter reset
		ct = ms_timer_corr();
		if(ct >= (3600L * 1000L * 24L)) {
			cli();
			timer_corr = 0;
			ct = 0;
			timer_local = 0;
			vts_next = 0; // or vts_dist; - we either lose or gain an ms
			next_sec = 0;
			sei();
		}

		if(!old_time_ok && dcf_time_ok) {
			next_sec = ct - (ct % 1000) + 1000UL;
			old_time_ok = 1;
		}

		if(ct >= next_sec && dcf_time_ok) {
			advance_time();

			yc_prepare_ee(YC_EE_TIME_ID);
			yc_send(Time, Time(lhour, lmin, lsec, lyear, lmonth, lday, 0));

			next_sec = ct + 1000;
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
			dcf_dispatch_bit();
			dcf_handle_bit = 0;
		}

		yc_dispatch_auto();
	}
	return 0;
}


// tick every ms
ISR(TIMER1_COMPA_vect) {
	static uint8_t bits = 0, last_state = 0;
	int32_t reported_time;

	if(skip_corr)
		skip_corr = 0;
	else
		timer_corr++;
	timer_local++;

	if(!last_state && bit_is_set(PIND, PD7) && dcf_minutemarker_valid && dcf_count >= 199 && dcf_count <= 201) {
		reported_time = 60000UL * ((int32_t) (((int16_t) 60) * hour + (int16_t) min));

		if(dcf_time_ok == 0) {
			timer_local = reported_time;
			vts_next = timer_local;
			timer_corr = timer_local;
			lyear = year;
			lmonth = month;
			lday = day;
			lhour = hour;
			lmin = min;
			lsec = 0;
			dcf_time_ok = 1;
		}

		// sync to minute
		reported_time = timer_corr - reported_time;

		*((int32_t *)(&((uint8_t *)dbg)[1])) = reported_time;

		if(reported_time > 0) { // are we too fast?
			timer_corr--;
		} else if(reported_time < 0) { // are we too slow?
			timer_corr++;
		}

		dcf_minutemarker_valid = 0;
	}
	last_state = bit_is_set(PIND, PD7);

	if(++timer_dms != 10) // continue in this function every 10 ms
		return;

	timer_dms = 0;

	if(bit_is_set(PIND, PD7)) {
		if(dcf_ticks == 0) {
			if(dcf_count >= 199 && dcf_count <= 201) { // minute marker?
				dcf_sync_state = DCF_SYNC;
				dcf_state = DCF_INIT;
				dcf_msg = 0x01;
				bits = 0;
				dcf_minutemarker_valid = 0;
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
		dcf_minutemarker_valid = 0;
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

