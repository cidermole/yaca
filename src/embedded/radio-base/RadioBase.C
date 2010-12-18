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

