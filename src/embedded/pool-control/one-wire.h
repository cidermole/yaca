/*
 *
 * one-wire driver for DS18S20+
 *
 */

#ifndef ONEWIRE_H
#define ONEWIRE_H

#include <yaca.h>
#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>

#define OW_DDR  DDRC
#define OW_PORT PORTC
#define OW_PIN  PINC
#define OW_BIT  PC3


void ow_pull();

void ow_write(uint8_t byte, uint8_t pull = 0) {
	uint8_t i;
	for(i = 0; i < 8; i++) {
		if(byte & 1) {
			// write 1
			cli();
			set_bit(OW_DDR, OW_BIT); // OW bit = output -> pull low
			_delay_us(2);
			clear_bit(OW_DDR, OW_BIT);
			if(i == 7 && pull) {
				_delay_us(80);
				ow_pull();
			}
			sei();
			_delay_us(80);
		} else {
			// write 0
			cli();
			set_bit(OW_DDR, OW_BIT); // OW bit = output -> pull low
			_delay_us(80);
			clear_bit(OW_DDR, OW_BIT);
			if(i == 7 && pull) {
				ow_pull();
			}
			sei();
			_delay_us(2);
		}
		yc_dispatch_auto();
		byte >>= 1;
	}
}

uint8_t ow_read() {
	uint8_t data = 0, i;
	for(i = 0; i < 8; i++) {
		data >>= 1;
		cli();
		set_bit(OW_DDR, OW_BIT); // OW bit = output -> pull low
		_delay_us(2);
		clear_bit(OW_DDR, OW_BIT);
		_delay_us(12);
		if(bit_is_set(OW_PIN, OW_BIT))
			data |= 0x80;
		sei();
		yc_dispatch_auto();
	}
}

uint8_t ow_check() {
	// reset pulse: low for min. 480 us, max. 960 us
	// 960 us - 448 us (14 SPI bytes = 112 SPI bits * 4 us) - 200 us (INT setup + reserve) = 312 us ~ 300 us
	yc_dispatch_auto();
	cli();
	set_bit(OW_DDR, OW_BIT); // OW bit = output -> pull low
	_delay_us(300);
	// XXX portability! written for ATmega8 (INT0 = PD2)
	if(bit_is_set(PIND, PD2)) { // if no CAN interrupt occured until now, we can safely wait more
		_delay_us(200);
		clear_bit(OW_DDR, OW_BIT);
	}
	sei();
	asm volatile("nop"); // leave time for an interrupt
	clear_bit(OW_DDR, OW_BIT);

	yc_dispatch_auto();
}

void ow_pull() {
	cli();
	set_bit(OW_PORT, OW_BIT);
	set_bit(OW_DDR, OW_BIT);
	sei();
}

void ow_release() {
	cli();
	clear_bit(OW_DDR, OW_BIT);
	clear_bit(OW_PORT, OW_BIT);
	sei();
}

#define OW_SKIP_ROM        0xCC
#define OW_CONVERT_T       0x44
#define OW_READ_SCRATCHPAD 0xBE

#define OW_PULL 1

#endif /* ONEWIRE_H */

