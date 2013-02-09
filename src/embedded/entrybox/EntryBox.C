#include "EntryBox.h"
#include "REntryBox.h"
#include <stdint.h>
#include <yaca.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#ifndef INT32_MAX
#define INT32_MAX 0x7fffffffL
#endif

/*

PB1 (OC1A): charge pump for opamps
PD5: photo camera (N-channel MOSFET gate)
PD6: dummy load (N-channel MOSFET gate)
PD7: motion sensor, pulling vs. GND

ADC0: IBAT (battery current), 6.5 mA resolution
ADC1: VBAT (battery voltage), 15 mV resolution
ADC2: ISOL (solar [charge] current), 3.25 mA resolution

  |
  v IBAT
+ |
-----  12 V lead-acid battery
 ---
  |



coulomb counter
---------------
basic resolution: U = 15 mV, I = 6.5 mA, P = 97.5 uW (microwatt)

ADC low-pass filter tau = 2 ms -> 10 ms changes relevant -> 100 Hz samplerate

12 V 7 Ah capacity fits into 32 bit
1 year's worth of microjoules don't fit

~ 10256 P units equal 1 Watt
1 year's worth of Joules fit. -> energy counting in Joules


*/

#define ADC_IBAT 0
#define ADC_VBAT 1
#define ADC_ISOL 2

#define ADMUX_REF ((1 << REFS1) | (1 << REFS0)) // internal AREF = 2.56 V with external cap

#define DUMMY_OFF_THRESHOLD_V ((uint16_t) (12.5 / 0.015))
#define DUMMY_ON_THRESHOLD_V ((uint16_t) (12.1 / 0.015))

#define JOULE_100_PUNITS 1025641L
#define JOULE_BATTERY_FULL (12 * 7 * 3600UL)
#define MAXIMUM_POWER_OUTPUT 42 // 42: 14 V * 3 A, the expected maximum output power

void enter_bootloader_hook() {
	TCCR1A = 0;
	TCCR1B = 0;
	TCCR2 = 0;
	TIMSK = 0;
	cli();
	yc_bld_reset();
}

void init_timers() {
	OCR1A = 100; // 2 MHz / 100 = 20 kHz toggle -> 10 kHz frequency generator
	TCCR1A = (1 << COM1A0); // toggle OC1A on compare match
	TCCR1B = (1 << WGM12) | (1 << CS10); // CTC mode, top = OCR1A, prescaler = 1

	OCR2 = 156; // 2 MHz / (156 * 128) = ~ 100 Hz
	TCCR2 = (1 << WGM21) | (1 << CS22) | (1 << CS20); // CTC mode, prescaler = 128

	TIMSK = (1 << OCIE2); // enable Timer2 compare match interrupt
}

void init_adc() {
	ADMUX = ADMUX_REF; // Vref = internal 2.56 V with external cap
	ADCSRA = (1 << ADEN) | (1 << ADPS2); // ADC prescaler = 16 -> 2 MHz / 16 = 125 kHz
}

void init_ports() {
	DDRB |= (1 << PB1); // config OC1A as output (charge pump)
	DDRD |= (1 << PD6) | (1 << PD5); // dummy, camera
	PORTD |= (1 << PD7); // PD7: motion relay: input with pullup
}

int16_t adc_convert(uint8_t channel) {
	ADMUX = (channel & 0x07) | ADMUX_REF; // select channel (+ keep reference)
	ADCSRA |= (1 << ADSC) | (1 << ADIF); // start conversion, clear int flag
	while(!(ADCSRA & (1 << ADIF))); // wait for conversion
	return ADCW;
}

int16_t ibat_offset = 512; // Ibat offset from calculations
int16_t ibat;
uint16_t vbat, isol;
int32_t sum_punit = 0;
uint32_t sum_punit_solar = 0;
uint32_t joule_battery = JOULE_BATTERY_FULL;
uint32_t joule_solar = 0;

uint32_t count = 0; // car count

uint8_t dummy_force = 1; // 0: force off, 1: auto, 2: force on

void time_tick();
void dummy_set(uint8_t status);
uint8_t dummy_status();

void debounce_count_tick() {
	static uint8_t state = 0;

	switch(state) {
	case 0:
		if(bit_is_clear(PIND, PD7))
			state = 1;
		break;
	case 1: // glitch filter
		if(bit_is_clear(PIND, PD7)) {
			state = 2;
			count++;
			yc_status(Count);
		} else
			state = 0;
		break;
	case 2: // relay on
		if(bit_is_set(PIND, PD7))
			state = 3;
		break;
	case 3: // glitch filter (relay on)
		if(bit_is_set(PIND, PD7))
			state = 0;
		else
			state = 2;
		break;
	default:
		state = 0;
	}
}

void conversion_tick() {
	static uint8_t time = 0;
	int32_t power;
	/*uint16_t*/ vbat = adc_convert(ADC_VBAT);
	/*int16_t*/ ibat = adc_convert(ADC_IBAT) - ibat_offset;

	power = ((int32_t) vbat) * ibat;
	sum_punit += power;
	sum_punit_solar += (uint32_t) ((uint32_t) vbat) * isol;

	isol = adc_convert(ADC_ISOL) / 2; // 3.25 mA -> 6.5 mA resolution

	if(++time == 100) {
		time_tick();
		time = 0;
	}
}

void time_tick() {
	static uint8_t tick = 0;
	// update joule counter and power
	int16_t djoule = sum_punit / JOULE_100_PUNITS;
	int16_t djoule_solar = sum_punit_solar / JOULE_100_PUNITS;

	// one-time init
	if(tick == 0) {
		djoule = 0;
		djoule_solar = 0;
		sum_punit = 0;
		sum_punit_solar = 0;
	}

	sum_punit -= ((int32_t) djoule) * JOULE_100_PUNITS;
	sum_punit_solar -= ((uint32_t) djoule_solar) * JOULE_100_PUNITS;
	joule_battery += djoule;
	joule_solar += djoule_solar;

	if(joule_battery >= INT32_MAX - MAXIMUM_POWER_OUTPUT)
		joule_battery = 0; // this is probably an unsigned underrun - clear capacity
	else if(joule_battery > JOULE_BATTERY_FULL)
		joule_battery = JOULE_BATTERY_FULL; // battery can never be charged over its full capacity

	dummy_set(((vbat >= DUMMY_OFF_THRESHOLD_V && !dummy_status()) || (vbat >= DUMMY_ON_THRESHOLD_V && dummy_status()) || dummy_force == 2) && dummy_force);

	tick++;
	if(tick % 2 == 0) {
		yc_status(JouleStatus);
		tick = 2;
	} else
		yc_status(PowerStatus);
}

void DR(JouleStatus()) {
	yc_prepare_ee(YC_EE_JOULESTATUS_ID);
	yc_send(EntryBox, JouleStatus(joule_battery, joule_solar));
}

void DR(PowerStatus()) {
	yc_prepare_ee(YC_EE_POWERSTATUS_ID);
	yc_send(EntryBox, PowerStatus((uint16_t) vbat, ibat, (uint16_t) isol));
}

void DR(Count()) {
	yc_prepare_ee(YC_EE_COUNT_ID);
	yc_send(EntryBox, Count(count));
}

void DM(SetCount(uint32_t c)) {
	count = c;
	yc_status(Count);
}

void DM(SetDummy(uint8_t status)) {
	dummy_force = status;
}

void dummy_set(uint8_t status) {
	if(status)
		PORTD |= (1 << PD6);
	else
		PORTD &= ~(1 << PD6);
}

uint8_t dummy_status() {
	return (PORTD & (1 << PD6));
}

volatile uint8_t start_conversion = 0;

int main() {
	init_ports();
	init_timers();
	init_adc();
	sei();
	while(1) {
		if(start_conversion) {
			cli();
			start_conversion--;
			sei();
			debounce_count_tick();
			conversion_tick();
		}
		yc_dispatch_auto();
	}
}

// 100 Hz interrupt
ISR(TIMER2_COMP_vect) {
	start_conversion++;
}

