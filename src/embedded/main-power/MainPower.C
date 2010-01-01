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


#define ST_IDLE    0
#define ST_BOOST   1
#define ST_CHARGE  2
#define ST_CHG_TOP 3


#define MILLIAMP_TO_PWM(a) (a * 255 / 100)

// TODO: soft-start (with reduced OCR_VAL) -- blew 2 A fuse without warning :-)

// PC5: red led
// PC4: green led

// PB0: charger: low-freq PWM (const. I on/off, I ~ 100 mA, we are consuming 33 mA with both LEDs)
// we are charging if we pull PB0 to 0. *do not* set PB0 to 1 (connected to base of transistor)

// PB1: PWM for step-up regulator (OC1A)

volatile uint8_t ac_power = 0, dc_maybe = 0, status_update = 0;
volatile uint16_t ac_status_scaler = 0;
volatile uint8_t charge_pwm_scaler = 0, charge_pwm = 0;
uint8_t state = ST_IDLE;
// PORTABILITY. seconds type 32-bit int overflows after 136 years, seems OK
uint32_t seconds = 0;
uint32_t chg_target = 0;
uint32_t lost_charge = 0;
uint16_t u_in, i_in;
uint8_t fifths = 0;
uint8_t no_topping = 0;

void transmit_status();

void delay_ms(uint16_t t) {
	uint16_t i;
	for(i = 0; i < t; i++)
		_delay_ms(1);
}

void red_led(uint8_t i) {
	if(i)
		PORTC |= (1 << PC5);
	else
		PORTC &= ~(1 << PC5);
}

void red_led_toggle() {
	PORTC ^= (1 << PC5);
}

void green_led(uint8_t i) {
	if(i)
		PORTC |= (1 << PC4);
	else
		PORTC &= ~(1 << PC4);
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

void transition_idle() {
	transmit_status();
	charge_pwm = MILLIAMP_TO_PWM(35); // we consume 33 mA from battery at all times, 2 mA reserved
	red_led(0);
	green_led(1);
	state = ST_IDLE;
}

void transition_boost() {
	charge_pwm = 0; // if boosting, we have no AC - don't waste our own power
	red_led(1);
	green_led(0);
	state = ST_BOOST;
}

void transition_charge() {
	charge_pwm = MILLIAMP_TO_PWM(100); // charge with (100 - 33) = 67 mA
	yc_prepare(2008);
	yc_send(MainPower, Debug1(lost_charge));
	if((lost_charge >> 16) < 22) { // if discharge was less than 10 %, omit the topping
		no_topping = 1;
	} else {
		no_topping = 0;
	}
	chg_target = seconds + (lost_charge >> 16) * 459;
	yc_prepare(2009);
	yc_send(MainPower, Debug2(chg_target));
	red_led(1);
	green_led(1);
	state = ST_CHARGE;
}

void transition_chg_top() {
	if(no_topping) {
		transition_idle();
		return;
	}

	// low charging current is supposedly bad for batteries, so topping charge with the full current (C/28 anyways...)
	charge_pwm = MILLIAMP_TO_PWM(100); // charge with (100 - 33) = 67 mA
	chg_target = seconds + 1200; // 20 min * 67 mA = 22 mAh (1,2 %)
	green_led(1);
	// red led is blinking
	state = ST_CHG_TOP;
}

// recalculate_charge(): we were charging and lost power... recalculate lost_charge
void recalculate_charge() {
	lost_charge = (((chg_target - seconds) * 1713) / 3072) * 256;
}

void transmit_status() {
	uint8_t t_high = 0;
	uint16_t t = 0;
	
	if(state == ST_CHARGE || state == ST_CHG_TOP) {
		t_high = (chg_target - seconds) >> 16;
		t = (chg_target - seconds) & 0xFFFF;
	} else if(state == ST_BOOST) {
		t_high = lost_charge >> 16;
		t = lost_charge & 0xFFFF;
	}
	
	yc_prepare_ee(YC_EE_POWERSTATUS_ID);
	yc_send(MainPower, PowerStatus(state, u_in, i_in, t_high, t));
}

int main() {
	uint8_t tenth_count = 0;

	DDRC |= (1 << PC5) | (1 << PC4);
	DDRB |= (1 << PB1) | (1 << PB0);
	ACSR = (1 << ACBG); // Analog Comp: + is bandgap ref. = 1,23 V

	ICR1 = 256;
	TCCR1A = (1 << WGM11); // 8-bit
	TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10); // prescaler 1, fast pwm
	TIMSK |= (1 << TOIE1);
	OCR1A = OCR_VAL;

	sei();
	
	ac_power = 1; // optimism rules!
	adc_init();
	
	transition_idle();
	
	while(1) {
		switch(state) {
		case ST_IDLE:
			if(!ac_power)
				transition_boost();
			break;

		case ST_BOOST:
			if(ac_power) {
				transition_charge();
			}
			break;

		case ST_CHARGE:
			if(!ac_power) {
				recalculate_charge();
				transition_boost();
				break;
			}
			if(seconds > chg_target)
				transition_chg_top();
			break;

		case ST_CHG_TOP:
			if(!ac_power) {
				transition_boost();
				break;
			}
			if(seconds > chg_target)
				transition_idle();
			break;

		default:
			break;
		}
		
		if(status_update) { // every 100 ms
			if(++tenth_count == 10) {
				seconds++;
				tenth_count = 0;
				
				if(state == ST_CHG_TOP) {
					red_led_toggle();
				}
			}
			
			u_in = adc_convert(ADC_U_IN);
			i_in = adc_convert(ADC_I_IN);
			
			if(state == ST_BOOST)
				lost_charge += i_in;
			
			if(++fifths == 2) {
				if(state != ST_IDLE)
					transmit_status();
				fifths = 0;
			}
			status_update = 0;
		}
		
		yc_dispatch_auto();
	}
	return 0;
}

void DM(Charge(uint8_t t_high, uint16_t t)) {
	transition_charge();
	chg_target = seconds + (((uint32_t) t_high) << 16) + t;
	transmit_status();
}

void DR(PowerStatus()) {
	transmit_status();
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
	
	if(ac_power) { // software PWM
		charge_pwm_scaler++;
		
		if(charge_pwm_scaler == 0) // 256 -> every 8.2 ms (122 Hz)
			DDRB |= (1 << PB0);
		
		if(charge_pwm_scaler == charge_pwm)
			DDRB &= ~(1 << PB0);
	} else {
		DDRB &= ~(1 << PB0);
	}
	
	if(++ac_status_scaler == 3125) { // every 100 ms, update AC status
		ac_status_scaler = 0;
		ac_power = !dc_maybe;
		dc_maybe = 0;
		status_update = 1;
	}
}

