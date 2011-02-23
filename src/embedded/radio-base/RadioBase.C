#include "RadioBase.h"
#include "RRadioBase.h"
#include <yaca.h>
#include "libradio/radio.h"
#include "libtimesync/timesync.h"
#include "librfm12/rfm12.h"
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#ifndef F_CPU
#define F_CPU 2000000UL
#endif
#include <util/delay.h>
#include <string.h>


uint16_t voltage;
int16_t temperature;
volatile int32_t timer_local = 0, timer_corr = 0;
volatile uint8_t skip_corr = 0;
int32_t old_time = 0, helper_clock_diff = 0;
uint8_t helper_clock_active = 1;

void DR(TempStatus()) {
	yc_prepare_ee(YC_EE_TEMPSTATUS_ID);
	RFM12_INT_master_off();
	yc_send(RadioBase, TempStatus(temperature, voltage));
	RFM12_INT_master_on();
}

extern "C" {
	uint16_t ms_timer();
}
int32_t ms_timer_local();
int32_t ms_timer_corr();

void debug_tx(volatile uint8_t *p) {
	yc_send(RadioBase, Debug(p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]));
}

#define DEVIATION_HARD_RESYNC_MS 10000
#define HELPER_CLOCK_ACTIVE_MS 200

void DM(Time(uint8_t hour, uint8_t min, uint8_t sec, uint16_t year, uint8_t month, uint8_t day, uint8_t flags)) {
	static uint8_t old_day = 0;
	uint8_t debug_msg[2];
	int32_t reported_time = 3600000UL * hour + 60000UL * min + 1000UL * sec;
	int32_t ms_corr = ms_timer_corr(), ms_local = ms_timer_local(), diff;
	int16_t fb;

	// coarse sync
	diff = ms_corr - reported_time;
	if(diff < 0)
		diff = -diff;

	if(diff > DEVIATION_HARD_RESYNC_MS) {
		cli();
		timer_local = reported_time;
		timer_corr = reported_time;
		sei();
		helper_clock_active = 1;
		old_time = reported_time;
		old_day = day;
		ts_tick(reported_time % 60000, 1); // reset tick
		yc_prepare(798);
		debug_tx(debug_msg);
		return;
	}

	helper_clock_diff = reported_time - ms_local;

	if(old_day != day) {
		// daily counter reset at midnight
		cli();
		timer_local = 0;
		timer_corr = 0;
		sei();
		old_time = 0;
		ts_tick(0, 1); // reset tick
	} else {
		fb = ts_slot(ms_local, ms_corr, reported_time);
		debug_msg[0] = ((uint8_t *) (&fb))[1];
		debug_msg[1] = ((uint8_t *) (&fb))[0];

		if(fb < 0)
			fb = -fb;
		helper_clock_active = (fb > HELPER_CLOCK_ACTIVE_MS);

		yc_prepare(799);
		debug_tx(debug_msg);
	}
}


// for libradio-master
uint16_t ms_timer() {
	// always use more accurate time base
	if(helper_clock_active)
		return ms_timer_local() + helper_clock_diff;
	else
		return ms_timer_corr() % 60000;
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

void enter_bootloader_hook() {
    TCCR2 = 0;
    TIMSK = 0;
    cli();
    yc_bld_reset();
}

int main() {
	RadioMessage rmsg;
	int8_t fb;

	TCCR2 = (1 << CS21) | (1 << WGM21); // CTC mode, prescaler = 8
	OCR2 = (uint16_t)((125UL * F_CPU) / 1000000UL); // 8 * 125 = 1000 ticks/s.
	TIMSK = (1 << OCIE2); // enable OC match interrupt
	ts_init();

	sei();

	radio_init(1);

	while(1) {
		if(radio_poll_receive()) {
			radio_receive(&rmsg);
			temperature = rmsg.data[1] | (((int16_t) rmsg.data[0]) << 8);
			voltage = rmsg.data[3] | (((uint16_t) rmsg.data[2]) << 8);
			// XXX: if all else fails, just wait until ACK tx is OK and transmit on CAN afterwards

			yc_status(TempStatus);
		}
		yc_dispatch_auto();

		if(ms_timer_local() != old_time) {
			fb = ts_tick(ms_timer_corr() % 60000, 0);
			if(fb == 1) {
				cli();
				timer_corr++;
				sei();
			} else if(fb == -1) {
				cli();
				skip_corr = 1;
				sei();
			}
			old_time++;
		}
		yc_dispatch_auto();
	}
	return 0;
}

ISR(TIMER2_COMP_vect) {
	if(skip_corr)
		skip_corr = 0;
	else
		timer_corr++;
	timer_local++;
}

