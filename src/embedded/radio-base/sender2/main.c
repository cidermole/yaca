#include <stdint.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include "one-wire.h"
#include "main.h"
#include "../librfm12/rfm12_stack.h"
#include "../libradio/radio.h"


uint16_t voltage;
int16_t temperature;
uint8_t delay_big, delay_small; // big: 256 ms steps, small: 1 ms steps

void ms_timer_on() {
	tenms_counter = 0;
	TCNT0 = 0;
	TCCR0 = (1 << CS02) | (1 << CS00); // timer0 on, prescaler = 1024
}

void ms_timer_off() {
	TCCR0 = 0;
}

uint8_t ms_timer_value() {
	return TCNT0;
}

void sync_time(int16_t time_feedback) {
	uint16_t delay;
	int16_t new_delay_small;
	static uint8_t synced_before = 0;

	if(!synced_before) {
		if(time_feedback < 0)
			delay = 60000 - (uint16_t)(-time_feedback);
		else
			delay = (uint16_t) time_feedback;

		// busy-wait the first time
		while(delay > 0)
			_delay_ms(1);

		// set default delay values, approx. 1 min.
		delay_big = 230; // 230 * 256 = 58.880 ms
		delay_small = 220; // + 220 ms + measurements etc. (900 ms) = 60.000 ms
	} else {
		// fine tuning
		new_delay_small = delay_small;
		new_delay_small += time_feedback;

		delay_big = (uint8_t) (((int8_t) delay_big) + (int8_t) (new_delay_small / 256)); // add to or borrow from from delay_big
		new_delay_small %= 256;
		delay_small = (uint8_t) new_delay_small;
	}

	if(!synced_before)
		synced_before = 1;
}

int main(void) {
	uint8_t i;
	int16_t time_feedback;
	RadioMessage msg, ack;

	MCUCSR = 0;
	ACSR |= (1 << ACD); // disable analog comparator
	msg.can_id = 0;
	msg.flags = 0;
	msg.length = 4;

	for(i = 0; i < 100; i++)
		_delay_ms(10);

	radio_init();

	while(1) {
		measure();
		RFM12_PHY_wake();
		RFM12_PHY_modeRX();

		// transmit values
		msg.data[0] = (uint8_t) (temperature >> 8);
		msg.data[1] = (uint8_t) (temperature);
		msg.data[2] = (uint8_t) (voltage >> 8);
		msg.data[3] = (uint8_t) (voltage);

		// try twice
		// worst case: Tx 40 + Dl 10 + Tx 40 + Dl 9 + Rx 40 = 139 ms
		for(i = 0; i < 2; i++) {
			msg.info = 0;
			if(i == 0)
				radio_transmit(RADIO_ID, &msg);
			else
				radio_retransmit(RADIO_ID, &msg);
			while(RFM12_PHY_busy()); // wait for transmission

			ms_timer_on();
			while(ms_timer_value() < RX_DEADLINE && !RFM12_PHY_busy());
			ms_timer_off();

			if(RFM12_PHY_busy()) { // are we receiving?
				ms_timer_on();
				while(ms_timer_value() < RX_TIMEOUT && RFM12_PHY_busy());
				ms_timer_off();

				if(radio_poll_receive()) {
					radio_receive(&ack);
					time_feedback = (((uint16_t) ack.data[1]) << 8) | ack.data[0];
					sync_time(time_feedback);
				} else if(RFM12_PHY_busy()) {
					RFM12_MAC_receiveCallback(RFM12_MAC_EOF); // signal EOF to reset buffers etc.
				}
				break;
			}
		}


		// XXX: should we not: disable interrupts, set timer, get (=clear) status, then re-enable interrupts?
		RFM12_PHY_timer(59, 10); // 59 * 2^10 = 60,416 s
		set_sleep_mode(SLEEP_MODE_PWR_DOWN);
		sleep_mode(); // wait for wakeup timer interrupt
	}
}

uint16_t adc_convert(uint8_t channel) {
	ADMUX = (channel & 0x07) | ADMUX_REF; // select channel (+ keep reference)
	ADCSRA |= (1 << ADSC) | (1 << ADIF); // start conversion, clear int flag
	while(!(ADCSRA & (1 << ADIF))); // wait for conversion
	return ADCW;
}

void measure(void) {
	uint8_t i;

	///////////////////////////////////////////////////////////////

	ADC_init();
	adc_convert(ADC_BATT); // throw away first result
	voltage = 0;
	for(i = 0; i < SAMPLES; i++)
		voltage += adc_convert(ADC_BATT);
	voltage = (uint16_t) ((((uint32_t) voltage) * 557) / (100 * SAMPLES)); // voltage divider 2.2 / 2.7 M
	ADC_deinit();

	///////////////////////////////////////////////////////////////

	ow_check();
	ow_write(OW_SKIP_ROM);
	ow_write(OW_CONVERT_T);
	ow_pull();
	for(i = 0; i < 80; i++) // wait 800 ms for conversion
		_delay_ms(10);
	ow_release();

	ow_check();
	ow_write(OW_SKIP_ROM);
	ow_write(OW_READ_SCRATCHPAD);
	for(i = 0; i < 2; i++)
		((uint8_t *)(&temperature))[i] = ow_read();

#ifdef DS18B20
	temperature = (temperature * 10) / 16;
#else
	temperature *= 5; // 0.5 °C steps
	// TODO: exact temp measurement with remainder
#endif

	///////////////////////////////////////////////////////////////
}

