#include "RadioBase.h"
#include "RRadioBase.h"
#include <yaca.h>
#include "libradio/radio.h"
#include "librfm12/rfm12.h"
#include <stdlib.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#ifndef F_CPU
#define F_CPU 2000000UL
#endif
#include <util/delay.h>
#include <string.h>

#define HASH_PRIME 12211009UL // a prime < 2^24

uint16_t voltage;
int16_t temperature;
uint32_t random_seed = 0;
extern uint8_t aes_key[16];
volatile uint16_t ms_timer_count = 0;

void DR(TempStatus()) {
	yc_prepare_ee(YC_EE_TEMPSTATUS_ID);
	RFM12_INT_off();
	yc_send(RadioBase, TempStatus(temperature, voltage));
	RFM12_INT_on();
}

void hash_step(uint32_t *hash_val, uint8_t data) {
	*hash_val = (((*hash_val) << 8) + ((uint32_t) data)) % HASH_PRIME;
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

void DM(Time(uint8_t hour, uint8_t min, uint8_t sec, uint16_t year, uint8_t month, uint8_t day, uint8_t flags)) {
	uint32_t hash_val = 0;

	if(random_seed == 0) {
		hash_step(&hash_val, hour);
		hash_step(&hash_val, min);
		hash_step(&hash_val, sec);
		hash_step(&hash_val, (uint8_t) year);
		hash_step(&hash_val, (uint8_t) (year >> 8));
		hash_step(&hash_val, month);
		hash_step(&hash_val, day);
	}
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

	while(random_seed == 0)
		yc_dispatch_auto();

	srandom(random_seed); // initialize non-standard avr-libc 32-bit random number generator

	RFM12_include(); // dummy call to librfm12 to make linker happy
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

