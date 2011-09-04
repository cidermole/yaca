#include "EntryBox.h"
#include <yaca.h>
#include <avr/io.h>
#include <avr/interrupt.h>

/*

PB1 (OC1A): charge pump for opamps
PD5: photo camera (N-channel MOSFET gate)
PD6: dummy load (N-channel MOSFET gate)

*/

void enter_bootloader_hook() {
	TCCR1A = 0;
	TCCR1B = 0;
	//TIMSK = 0;
	cli();
	yc_bld_reset();
}

void init_timer() {
	OCR1A = 100; // 2 MHz / 100 = 20 kHz toggle -> 10 kHz frequency generator
	TCCR1A = (1 << COM1A0); // toggle OC1A on compare match
	TCCR1B = (1 << WGM12) | (1 << CS10); // CTC mode, top = OCR1A, prescaler = 1
}

void init_adc() {
	ADMUX = (1 << REFS1) | (1 << REFS0); // Vref = internal 2.56 V with external cap
	ADCSRA = (1 << ADEN) | (1 << ADPS2); // ADC prescaler = 16 -> 2 MHz / 16 = 125 kHz
}

void init_ports() {
	DDRB |= (1 << PB1); // config OC1A as output (charge pump)
	DDRD |= (1 << PD6) | (1 << PD5); // dummy, camera
}

int main() {
	init_ports();
	init_timer();
	init_adc();
	sei();
	while(1) {
		yc_dispatch_auto();
	}
}

