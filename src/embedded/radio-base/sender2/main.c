#include <stdint.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include "one-wire.h"
#include "main.h"
#include "flash.h"
#include "../librfm12/rfm12.h"
#include "../libradio/radio.h"


uint16_t voltage;
int16_t temperature;
uint8_t delay_big, delay_small; // big: 256 ms steps, small: 1 ms steps

void ms_timer_on() {
	TCNT0 = 0;
	TCCR0 = (1 << CS02) | (1 << CS00); // timer0 on, prescaler = 1024
}

void ms_timer_off() {
	TCCR0 = 0;
}

uint8_t ms_timer_value() {
	return TCNT0;
}

int main(void) {
	uint8_t i;
	int16_t time_feedback;
	RadioMessage msg, ack;

	MCUCSR = 0;
	ACSR |= (1 << ACD); // disable analog comparator
	msg.can_id = 0;
	memset(&msg.flags, 0, sizeof(msg.flags));
	msg.length = 4;

	for(i = 0; i < 200; i++)
		_delay_ms(10);

	// expand AES key and init radio
	radio_init(__radio_id);

	sei();

	radio_slave_resync();

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
					break;
				} else if(RFM12_PHY_busy()) { // still busy? (jamming)
					RFM12_MAC_receiveCallback(RFM12_MAC_EOF); // signal EOF to reset buffers etc.
					break;
				}
			}
		}


		// XXX: should we not: disable interrupts, set timer, get (=clear) status, then re-enable interrupts?
		RFM12_PHY_timer(delay_big, 8); // delay_big * 2^8 ms
		set_sleep_mode(SLEEP_MODE_PWR_DOWN);
		sleep_mode(); // wait for wakeup timer interrupt

		for(i = 0; i < delay_small; i++)
			_delay_ms(1);
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

