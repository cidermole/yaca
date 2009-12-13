#include "MainPower.h"
#include "RMainPower.h"
#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#ifndef F_CPU
#define F_CPU 8000000UL
#endif
#include <util/delay.h>
#include <inttypes.h>
#include <yaca.h>

#define OCR_VAL 200

#define ADC_U_OUT 0
#define ADC_I_IN  1
#define ADC_U_IN  2

#define ADMUX_REF ((1 << REFS1) | (1 << REFS0)) // AREF = 2.56 V

// TODO: soft-start (with reduced OCR_VAL) -- blew 2 A fuse without warning :-)

// PC5: red led
// PC4: green led
// PB0: charger: low-freq PWM
// PB1: PWM for step-up regulator (OC1A)

volatile uint8_t ac_power = 0, dc_maybe = 0, status_update = 0;
volatile uint16_t ac_status_scaler = 0;
volatile uint8_t charge_pwm_scaler = 0;


void delay_ms(uint16_t t) {
	uint16_t i;
	for(i = 0; i < t; i++)
		_delay_ms(1);
}

uint16_t adc_convert(uint8_t channel) {
	ADMUX = (channel & 0x07) | ADMUX_REF; // select channel (+ keep reference)
	ADCSRA |= (1 << ADSC) | (1 << ADIF); // start conversion, clear int flag
	while(!(ADCSRA & (1 << ADIF))); // wait for conversion

	return ADCW;
}

void adc_init() {
	ADMUX = (1 << REFS1) | (1 << REFS0);
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1); // enable ADC, prescaler = 64 (8 MHz / 64 = 125 kHz, 13 cycles per conversion ~ 100 us conversion)
	delay_ms(100); // wait for ADC to warm up
	adc_convert(0); // dummy conversion
}

int main() {
	DDRC |= (1 << PC5) | (1 << PC4);
	DDRB |= (1 << PB1) | (1 << PB0);
	ACSR = (1 << ACBG); // Analog Comp: + is bandgap ref. = 1,23 V

	ICR1 = 256;
	TCCR1A = (1 << WGM11); // 8-bit
	TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10); // prescaler 1, fast pwm
	TIMSK |= (1 << TOIE1);
	OCR1A = OCR_VAL;

	sei();
	
	adc_init();

	while(1) {
		if(ac_power) {
			PORTC |= (1 << PC4); // green on
			PORTC |= (1 << PC5); // red on
		} else {
			DDRB &= ~(1 << PB0); // stop charging if we're not on AC power
			PORTC &= ~(1 << PC4); // green off
			PORTC |= (1 << PC5); // red on
		}
		
		if(status_update) { // every 100 ms
			yc_prepare(22);
			yc_send(MainPower, PowerStatus(ac_power, adc_convert(ADC_U_IN), adc_convert(ADC_I_IN)));
			status_update = 0;
		}
		
		yc_dispatch_auto();
	}
	return 0;
}

void enter_bootloader_hook() {
	cli();
	TCCR1A = 0;
	TCCR1B = 0;
	TCNT1 = 0;
	TIMSK = 0;
	DDRB &= ~(1 << PB0); // stop charging
	PORTC &= ~(1 << PC4); // green off
	PORTC &= ~(1 << PC5); // red off
	yc_bld_reset();
}

ISR(TIMER1_OVF_vect) {
	if(ACSR & (1 << ACO)) {
		TCCR1A |= (1 << COM1A1);
		dc_maybe = 1;
	} else {
		TCCR1A &= ~(1 << COM1A1);
	}
	
	if(++charge_pwm_scaler == 0) { // 256 -> every 8.2 ms (122 Hz)
		if(ac_power)
			DDRB |= (1 << PB0); // was ^= for PWM
	}
	
	if(++ac_status_scaler == 3125) { // every 100 ms, update AC status
		ac_status_scaler = 0;
		ac_power = !dc_maybe;
		dc_maybe = 0;
		status_update = 1;
	}
}

