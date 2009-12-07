#include "MainPower.h"
#include "RMainPower.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#ifndef F_CPU
#define F_CPU 8000000UL
#endif
#include <util/delay.h>
#include <inttypes.h>
#include <yaca.h>

#define OCR_VAL 200

// PC5: green led
// PC4: red led
// PB0: charger: low-freq PWM
// PB1: PWM for step-up regulator (OC1A)

volatile uint8_t ac_power = 0, dc_maybe = 0;
volatile uint16_t ac_status_scaler = 0;
volatile uint8_t charge_pwm_scaler = 0;

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

    yc_send(MainPower, HelloWorld(10));

	while(1) {
		if(ac_power) {
			PORTC |= (1 << PC5); // green on
			PORTC |= (1 << PC4); // red on
		} else {
			DDRB &= ~(1 << PB0); // stop charging if we're not on AC power
			PORTC &= ~(1 << PC5); // green off
			PORTC |= (1 << PC4); // red on
		}
		
		yc_dispatch_auto();
	}
	return 0;
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
			DDRB ^= (1 << PB0);
	}
	
	if(++ac_status_scaler == 3125) { // every 100 ms, update AC status
		ac_status_scaler = 0;
		ac_power = !dc_maybe;
		dc_maybe = 0;
	}
}

