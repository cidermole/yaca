#include "RadioBase.h"
#include "RRadioBase.h"
#include <yaca.h>
#include "libradio/radio.h"
#include "librfm12/rfm12.h"
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
// TODO FIXME F_CPU should not be needed here
#ifndef F_CPU
#define F_CPU 2000000UL
#endif
#include <util/delay.h>
#include <util/atomic.h>
#include <string.h>


uint16_t voltage;
int16_t temperature;

volatile uint8_t dbg_used;
volatile uint8_t dbg_mem[8];
volatile uint16_t timer_local = 0;

void DR(TempStatus()) {
	yc_prepare_ee(YC_EE_TEMPSTATUS_ID);
	RFM12_INT_master_off();
	yc_send(RadioBase, TempStatus(temperature, voltage));
	RFM12_INT_master_on();
}

extern "C" {
	uint16_t ms_timer();
}

uint16_t ms_timer() {
	uint16_t t;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		t = timer_local;
	return t;
}

void debug_tx(volatile uint8_t *p) {
	RFM12_INT_master_off();
	yc_send(RadioBase, Debug(p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]));
	RFM12_INT_master_on();
}

void DM(Time(uint8_t hour, uint8_t min, uint8_t sec, uint16_t year, uint8_t month, uint8_t day, uint8_t flags)) {
	uint16_t t = 1000 * sec;
	ATOMIC_BLOCK(ATOMIC_FORCEON)
		timer_local = t;
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
			yc_status(TempStatus);
		}
		yc_dispatch_auto();

/*
		if(dbg_used) {
			yc_prepare(792);
			debug_tx(dbg_mem);
			dbg_used = 0;
		}
*/

		yc_dispatch_auto();
	}
	return 0;
}

ISR(TIMER2_COMP_vect) {
	timer_local++;
}

