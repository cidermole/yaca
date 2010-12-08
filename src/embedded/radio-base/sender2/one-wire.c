#include "one-wire.h"
#include <avr/io.h>
#include <util/delay.h>
#include <inttypes.h>

void ow_write(uint8_t b) {
	uint8_t i;
	for(i = 0; i < 8; i++) {
		if(b & 1) {
			// write 1
			set_bit(OW_DDR, OW_BIT); // OW bit = output -> pull low
			_delay_us(2);
			clear_bit(OW_DDR, OW_BIT);
			_delay_us(80);
		} else {
			// write 0
			set_bit(OW_DDR, OW_BIT); // OW bit = output -> pull low
			_delay_us(80);
			clear_bit(OW_DDR, OW_BIT);
			_delay_us(2);
		}
		b >>= 1;
	}
}

uint8_t ow_read() {
	uint8_t buf = 0;
	uint8_t i;
	for(i = 0; i < 8; i++) {
		buf >>= 1;
		set_bit(OW_DDR, OW_BIT); // OW bit = output -> pull low
		_delay_us(2);
		clear_bit(OW_DDR, OW_BIT);
		_delay_us(12);
		if(bit_is_set(OW_PIN, OW_BIT))
			buf |= 0x80;
		_delay_us(80 - 14);
	}
	return buf;
}

void ow_pull() {
	set_bit(OW_PORT, OW_BIT);
	set_bit(OW_DDR, OW_BIT);
}

void ow_release() {
	clear_bit(OW_DDR, OW_BIT);
	clear_bit(OW_PORT, OW_BIT);
}

uint8_t ow_check() {
	// reset pulse: low for min. 480 us, max. 960 us
	set_bit(OW_DDR, OW_BIT); // OW bit = output -> pull low
	_delay_us(600);
	clear_bit(OW_DDR, OW_BIT);
	_delay_us(600);
	return 1; // FIXME we should check if DS18S20 is there (presence pulse, datasheet page 13)
}
