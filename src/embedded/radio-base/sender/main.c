#include <stdlib.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include "global.h"
#include "rf12.h"
#include "one-wire.h"
#include "main.h"


int main(void) {
	uint8_t i;

	MCUCSR = 0;

	ACSR |= (1 << ACD); // disable analog comparator
	INT0_init();

	for(i = 0; i < 100; i++)
		_delay_ms(10);

	rf12_init();
	rf12_setfreq(RFM12FREQ868(868));
	rf12_setbandwidth(4, 1, 4);       // 200kHz bandwidth, -6dB gain, DRSSI threshold: -79dBm
	rf12_setbaud(9600);
	rf12_setpower(0, 6);              // 1mW output power, 120kHz frequency shift

	while(1) {
		send();

/*		for(i = 0; i < 100; i++) // XXX debug (wait a second to prevent endless send loop when sleep is not working)
			_delay_ms(10);*/

		rf12_timer(59, 10); // 59 * 2^10 = 60,416 s
		rf12_trans(0x0000);
		INT0_on();
		sei();
		set_sleep_mode(SLEEP_MODE_PWR_DOWN);
		sleep_mode(); // wait for wakeup timer interrupt
		INT0_off();
		cli();
	}
}

uint16_t adc_convert(uint8_t channel) {
	ADMUX = (channel & 0x07) | ADMUX_REF; // select channel (+ keep reference)
	ADCSRA |= (1 << ADSC) | (1 << ADIF); // start conversion, clear int flag
	while(!(ADCSRA & (1 << ADIF))); // wait for conversion
	return ADCW;
}

void send(void) {
	uint8_t text[]="\0\0\4.... C ..... V\r\n";
	uint8_t hamming[] = {0x15,0x02,0x49,0x5E,0x64,0x73,0x38,0x2F,0xD0,0xC7,0x8C,0x9B,0xA1,0xB6,0xFD,0xEA};
	uint8_t out[80], i;
	uint16_t temperature, voltage = 0;

	///////////////////////////////////////////////////////////////

	ADC_init();
	adc_convert(ADC_BATT); // throw away first result
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

	text[3] = (temperature / 100) % 10 + '0';
	text[4] = (temperature / 10) % 10 + '0';
	text[6] = (temperature) % 10 + '0';

	text[10] = (voltage / 1000) % 10 + '0';
	text[12] = (voltage / 100) % 10 + '0';
	text[13] = (voltage / 10) % 10 + '0';
	text[14] = (voltage) % 10 + '0';

	for(i = 0; i < sizeof(text)*2-2; i++) {
		if(i & 1)
			out[i] = hamming[text[i/2] & 0x0f];
		else
			out[i] = hamming[text[i/2] >> 4];
	}
	out[i] = 0xAA;
	out[i+1] = 0xAA;
	out[i+2] = 0xAA;

	rf12_on();
	rf12_txdata(out,sizeof(text)*2+1);
	rf12_off();
}

ISR(INT0_vect) {
}

