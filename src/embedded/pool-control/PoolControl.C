#include "PoolControl.h"
#include "RPoolControl.h"
#include "sevenseg.h"
#include "calendar.h"
#include <yaca.h>
#ifndef F_CPU
#define F_CPU 2000000UL
#endif
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

/*

Pin configurations
------------------

sevenseg

  PB7 segment 1
  PB6 segment 2
  PB0 segment 3
  PD0 a
  PD1 f
  PC5 b   !!! bit 2
  PD3 e
  PD4 d
  PD5 h
  PD6 c
  PD7 g

PC4 charge pump for opamps
PB1 relay output, active high

*/

#define PUMP_PORT PORTB
#define PUMP_BIT  PB1

#define ADMUX_REF ((1 << REFS1) | (1 << REFS0)) // internal AREF = 2.56 V

#define ADC_PH 0

#define TIMESYNC_TIMEOUT 1500 // timeout in milliseconds after last time update when local clock starts

struct Time {
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
	uint16_t year;
	uint8_t month;
	uint8_t day;
	int16_t sync;
	uint8_t local_clock;
};

Time curtime;
uint8_t pump_from_hour, pump_to_hour;
uint16_t ph_value; // pH * 100
volatile int16_t hms_counter = 0;


// loopdelay_ms: keeps exact time reference in the main loop
void loopdelay_ms(int16_t t) {
	uint16_t i;
	uint8_t cond;

	t *= 2;
	cond = 1;
	while(cond) {
		yc_dispatch_auto();
		if(curtime.sync < hms_counter)
			curtime.local_clock = 1;
		cli();
		cond = hms_counter < t;
		sei();
	}
	cli();
	hms_counter -= t;
	sei();
	curtime.sync -= t;
}

void enter_bootloader_hook() {
	TCCR1B = 0;
	TIMSK = 0;
	cli();
	sevenseg_off();
	yc_bld_reset();
}

void time_changed() {
	if(curtime.min == 0 && curtime.sec == 0) {
		if(curtime.hour == pump_from_hour) {
			set_bit(PUMP_PORT, PUMP_BIT);
		} else if(curtime.hour == pump_to_hour) {
			clear_bit(PUMP_PORT, PUMP_BIT);
		}
	}
}

void time_advance() {
	curtime.sec++;
	if(curtime.sec >= 60) {
		curtime.sec = 0;
		curtime.min++;
	} else {
		goto _ta_return;
	}
	
	if(curtime.min >= 60) {
		curtime.min = 0;
		curtime.hour++;
	} else {
		goto _ta_return;
	}
	
	if(curtime.hour >= 24) {
		curtime.hour = 0;
		curtime.day++;
	} else {
		goto _ta_return;
	}
	
	if(curtime.day > days_in_month(curtime.month, curtime.year)) {
		curtime.day = 1;
		curtime.month++;
	} else {
		goto _ta_return;
	}
	
	if(curtime.month > 12) {
		curtime.month = 1;
		curtime.year++;
	}

_ta_return:
	time_changed();
}

void DM(Time(uint8_t hour, uint8_t min, uint8_t sec, uint16_t year, uint8_t month, uint8_t day, uint8_t flags)) {
	// synchronize current time and reset synchronization half-milliseconds
	curtime.hour = hour;
	curtime.min = min;
	curtime.sec = sec;
	curtime.year = year;
	curtime.month = month;
	curtime.day = day;
	cli();
	curtime.sync = hms_counter + 2 * TIMESYNC_TIMEOUT;
	sei();
	curtime.local_clock = 0;
	time_changed();
}

void DM(SetMode(uint8_t mode)) {
}

uint16_t adc_convert(uint8_t channel) {
	ADMUX = (channel & 0x07) | ADMUX_REF; // select channel (+ keep reference)
	ADCSRA |= (1 << ADSC) | (1 << ADIF); // start conversion, clear int flag
	while(!(ADCSRA & (1 << ADIF))); // wait for conversion
	return ADCW;
}

void measure_ph() {
	uint16_t adc_value = 0;
	uint8_t i;

	for(i = 0; i < 20; i++)
		adc_value += adc_convert(ADC_PH);

	ph_value = (uint16_t) ((((uint32_t) adc_value) * (700 / 20)) / 512);

	yc_prepare_ee(YC_EE_PHSTATUS_ID);
	yc_send(PoolControl, PhStatus(ph_value));
}

void display_ph() {
	if(ph_value > 999)
		sevenseg_display(ph_value / 10, 1);
	else
		sevenseg_display(ph_value, 2);
}

void DR(PhStatus()) {
	display_value();
}


int main() {
	uint8_t n = 0, cond;

	DDRB |= SEVENSEG_SEGMASK | (1 << PB1); // be careful with port B (in use for CAN)
	DDRD = ~(1 << PD2); // PD2 is not ours
	DDRC = (1 << PC5) | (1 << PC4);

	TCCR1B = (1 << WGM12) | (1 << CS10); // CTC (top = OCR1A), prescaler 1
	TIMSK = (1 << OCIE1A);
	OCR1A = 1000; // 2 MHz / 2000 = 1000 Hz

	ADCSRA = (1 << ADEN) | (1 << ADPS2); // prescaler 16, 2 MHz / 16 = 125 kHz ADC clock (max 200 kHz in accordance with datasheet)

	sei();
	sevenseg_display(1001, 0); // "---"
	pump_from_hour = eeprom_read_byte(YC_EE_PUMP_FROM_HOUR);
	pump_to_hour = eeprom_read_byte(YC_EE_PUMP_TO_HOUR);
	curtime.local_clock = 0;
	/* TODO: temp sensor */

	while(1) {
		measure_ph();
		display_ph();
		loopdelay_ms(1000);
		if(curtime.local_clock)
			time_advance();

		//yc_dispatch_auto(); // called in loopdelay_ms()
	}
}

ISR(TIMER1_COMPA_vect) {
	PORTC ^= (1 << PC4);
	sevenseg_render();
	hms_counter++;
}

