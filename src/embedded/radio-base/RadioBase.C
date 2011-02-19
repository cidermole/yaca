#include "RadioBase.h"
#include "RRadioBase.h"
#include <yaca.h>
#include "libradio/radio.h"
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
volatile uint16_t ms_timer_count = 0;

void DR(TempStatus()) {
	yc_prepare_ee(YC_EE_TEMPSTATUS_ID);
	RFM12_INT_master_off();
	yc_send(RadioBase, TempStatus(temperature, voltage));
	RFM12_INT_master_on();
}

extern "C" {
	uint16_t ms_timer();
}

void debug_tx(volatile uint8_t *p) {
	yc_send(RadioBase, Debug(p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]));
}

#define MAX_DEVIATION_MS 200
#define SYNCPOINT_SECS 30
#define MAX_OSCCAL_DEVIATION 10
//#define FINE_SYNC_MS 20

void DM(Time(uint8_t hour, uint8_t min, uint8_t sec, uint16_t year, uint8_t month, uint8_t day, uint8_t flags)) {
	uint16_t timer_count = ms_timer(); // % 1000
	uint16_t timer_sub = timer_count % 1000;
	static uint8_t cal = 0;
	uint8_t debug_msg[8];

	// XXX: let's hope OSCCAL will never be 0 (static uint8_t cal = OSCCAL doesn't make compiler happy)
	if(cal == 0)
		cal = OSCCAL;

	if(sec == SYNCPOINT_SECS && (timer_count / 1000 != SYNCPOINT_SECS || timer_sub < (500 - MAX_DEVIATION_MS) || timer_sub > (500 + MAX_DEVIATION_MS))) {
		cli();
		ms_timer_count = SYNCPOINT_SECS * 1000 + 500;
		sei();

		// debugging
		yc_prepare(798);
		//debug_msg[0] = 0x01;
		debug_msg[0] = hour;
		debug_msg[1] = min;
		debug_msg[2] = sec;
		debug_tx(debug_msg);
	} else {
		// fine sync
		if(timer_sub > 500 && OSCCAL > cal - MAX_OSCCAL_DEVIATION) // too fast?
			OSCCAL--;
		else if(timer_sub < 500 && OSCCAL < cal + MAX_OSCCAL_DEVIATION) // too slow?
			OSCCAL++;

		// debugging
		if((timer_sub > 500 && OSCCAL == cal - MAX_OSCCAL_DEVIATION) || (timer_sub < 500 && OSCCAL == cal + MAX_OSCCAL_DEVIATION)) {
			yc_prepare(797);
			debug_msg[0] = 0x02;
			debug_msg[1] = timer_sub > 500;
			debug_tx(debug_msg);
		}
	}
}


// for libradio-master
uint16_t ms_timer() {
	uint16_t t;
	uint8_t sr = SREG;
	cli();
	t = ms_timer_count;
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

	TCCR2 = (1 << CS21) | (1 << WGM21); // CTC mode, prescaler = 8
	OCR2 = (uint16_t)((125UL * F_CPU) / 1000000UL); // 8 * 125 = 1000 ticks/s.
	TIMSK = (1 << OCIE2); // enable OC match interrupt

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
	}
	return 0;
}

//TODO: synchronize ms_timer with CAN bus time
ISR(TIMER2_COMP_vect) {
	if(++ms_timer_count == 60000)
		ms_timer_count = 0;
}

