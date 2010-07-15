#include "PoolControl.h"
#include "RPoolControl.h"
#include "sevenseg.h"
#include "calendar.h"
#ifndef F_CPU
#define F_CPU 2000000UL
#endif
#include "one-wire.h"
#include <yaca.h>
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

PC3 1-wire temp sensor (DS18S20)
PC4 charge pump for opamps
PB1 relay output, active high

*/

#define PUMP_PORT PORTB
#define PUMP_BIT  PB1

#define ADMUX_REF ((1 << REFS1) | (1 << REFS0)) // internal AREF = 2.56 V

#define ADC_PH 0

#define TIMESYNC_TIMEOUT 1500 // timeout in milliseconds after last time update when local clock starts

#define DS18B20 // using DS18B20 with more resolution than DS18S20

#define MEASURE_BUF 64

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
uint16_t ph_value, ph_buffer[MEASURE_BUF]; // pH * 100
int16_t temp_value, temp_buffer[MEASURE_BUF]; // temp * 10
uint32_t ph_sum;
int32_t temp_sum;
uint8_t ph_count = 0, temp_count = 0, temp_buffer_running = 0, ph_buffer_running = 0;
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
	uint8_t oldyr;

	oldyr = curtime.year;

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

	if(oldyr == 0 && curtime.hour >= pump_from_hour && curtime.hour < pump_to_hour)
		set_bit(PUMP_PORT, PUMP_BIT);
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

	if(ph_buffer_running) {
		ph_sum -= ph_buffer[ph_count];
		ph_sum += ph_value;
	}
	ph_buffer[ph_count++] = ph_value;
	if(ph_count == MEASURE_BUF) {
		if(ph_buffer_running == 0) {
			for(i = 0; i < MEASURE_BUF; i++)
				ph_sum += ph_buffer[i];
			ph_buffer_running = 1;
		}
		ph_count = 0;
	}

	if(ph_buffer_running) {
		ph_value = (uint16_t) (ph_sum / MEASURE_BUF);
	}
	yc_prepare_ee(YC_EE_PHSTATUS_ID);
	yc_send(PoolControl, PhStatus(ph_value));
}

void measure_temp() {
	uint8_t data[9], i;

	if(!ow_check()) {
		loopdelay_ms(800);
		return;
	}
	ow_write(OW_SKIP_ROM);
	ow_write(OW_CONVERT_T, OW_PULL);
	loopdelay_ms(800);
	ow_release();
	if(!ow_check())
		return;

	ow_write(OW_SKIP_ROM);
	ow_write(OW_READ_SCRATCHPAD);

	for(i = 0; i < 9; i++)
		data[i] = ow_read();

	temp_value = data[0] | (((int16_t) data[1]) << 8);
#ifdef DS18B20
	temp_value = (temp_value * 10) / 16;
#else
	temp_value *= 5; // 0.5 °C steps
	// TODO: exact temp measurement with remainder
#endif

	if(temp_buffer_running) {
		temp_sum -= temp_buffer[temp_count];
		temp_sum += temp_value;
	}
	temp_buffer[temp_count++] = temp_value;
	if(temp_count == MEASURE_BUF) {
		if(temp_buffer_running == 0) {
			for(i = 0; i < MEASURE_BUF; i++)
				temp_sum += temp_buffer[i];
			temp_buffer_running = 1;
		}
		temp_count = 0;
	}

	if(temp_buffer_running) {
		temp_value = (int16_t) (temp_sum / MEASURE_BUF);
	}

	yc_prepare_ee(YC_EE_TEMPSTATUS_ID);
	yc_send(PoolControl, TempStatus(temp_value));
}

void display_ph() {
	if(ph_value > 999)
		sevenseg_display(ph_value / 10, 1);
	else
		sevenseg_display(ph_value, 2);

/*	if(temp_value > 999)
		sevenseg_display(temp_value / 10, 0);
	else
		sevenseg_display(temp_value, 1); // displays temp instead of pH */
}

void DR(PhStatus()) {
	yc_prepare_ee(YC_EE_PHSTATUS_ID);
	yc_send(PoolControl, PhStatus(ph_value));
}

void DR(TempStatus()) {
	yc_prepare_ee(YC_EE_TEMPSTATUS_ID);
	yc_send(PoolControl, TempStatus(temp_value));
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
	curtime.year = 0;

	while(1) {
		// pH
		measure_ph();
		display_ph();

		// temp
		measure_temp();

		loopdelay_ms(200); // remaining 800 in measure_temp()
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

