#include "PoolControl.h"
#include "sevenseg.h"
#include <yaca.h>
#ifndef F_CPU
#define F_CPU 2000000UL
#endif
#include <util/delay.h>
#include <avr/interrupt.h>

/*

Pin configurations
------------------

7 seg display, 3 digits, all outputs active low

     a
    ___
 f |   | b
   |_g_|
 e |   | c
   |___| * h (dot)
     d

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

=================================================

PC4 charge pump for opamps
PB1 relay output, active high

*/

#define ADMUX_REF ((1 << REFS1) | (1 << REFS0)) // internal AREF = 2.56 V

#define ADC_PH 0

void delay_ms(uint16_t t) {
	uint16_t i;
	for(i = 0; i < t; i++) {
		_delay_ms(1);
		yc_dispatch_auto();
	}
}

void enter_bootloader_hook() {
	TCCR1B = 0;
	TIMSK = 0;
	cli();
	sevenseg_off();
	yc_bld_reset();
}

void DM(Time(uint8_t hour, uint8_t min, uint8_t sec, uint16_t year, uint8_t month, uint8_t day, uint8_t flags)) {
/*	if(hour > 12)
		hour -= 12;
	sevenseg_display((uint16_t)hour * 100 + min, 2);*/
}

uint16_t adc_convert(uint8_t channel) {
	ADMUX = (channel & 0x07) | ADMUX_REF; // select channel (+ keep reference)
	ADCSRA |= (1 << ADSC) | (1 << ADIF); // start conversion, clear int flag
	while(!(ADCSRA & (1 << ADIF))); // wait for conversion
	return ADCW;
}

int main() {
	uint8_t n = 0;

	DDRB |= SEVENSEG_SEGMASK | (1 << PB1); // be careful with port B (in use for CAN)
	DDRD = ~(1 << PD2); // PD2 is not ours
	DDRC = (1 << PC5) | (1 << PC4);

	TCCR1B = (1 << WGM12) | (1 << CS10); // CTC (top = OCR1A), prescaler 1
	TIMSK = (1 << OCIE1A);
	OCR1A = 1000; // 2 MHz / 2000 = 1000 Hz

	ADCSRA = (1 << ADEN) | (1 << ADPS2); // prescaler 16, 2 MHz / 16 = 125 kHz ADC clock (max 200 kHz in accordance with datasheet)

	sei();
	sevenseg_display(1001, 0); // "---"

	while(1) {
		sevenseg_display(adc_convert(ADC_PH), 0);
		delay_ms(1000);
		yc_dispatch_auto();
	}
}

ISR(TIMER1_COMPA_vect) {
	PORTC ^= (1 << PC4);
	sevenseg_render();
}

