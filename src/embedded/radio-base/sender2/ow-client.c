#include "ow-client.h"
#include <util/delay.h>

void owc_init() {
	set_bit(OWC_PORT, OWC_BIT); // enable pull-up
}

void owc_wait_reset() {
	uint8_t i = 0;

	while(i < 40) {
		loop_until_bit_is_clear(OWC_PIN, OWC_BIT);
		for(i = 0; i < 40; i++) {
			if(bit_is_set(OWC_PIN, OWC_BIT)) // bus needs to be low for > 400 us
				break;
			_delay_us(10);
		}
	}
	loop_until_bit_is_set(OWC_PIN, OWC_BIT); // wait for bus release
	_delay_us(40);

	clear_bit(OWC_PORT, OWC_BIT); // pull bus low for 200 us
	set_bit(OWC_DDR, OWC_BIT);
	_delay_us(200);
	clear_bit(OWC_DDR, OWC_BIT);
	set_bit(OWC_PORT, OWC_BIT); // re-enable pullup

	_delay_us(160); // complete the after-initialization timing to 400 us
}

uint8_t owc_omni(uint8_t byte) {
	uint8_t i, data = 0;

	for(i = 0; i < 8; i++) {
		data >>= 1;
		loop_until_bit_is_clear(OWC_PIN, OWC_BIT); // wait for bit slot
		if(!(byte & 1)) {
			clear_bit(OWC_PORT, OWC_BIT); // pull bus low
			set_bit(OWC_DDR, OWC_BIT);
		}
		_delay_us(30);
		if(bit_is_set(OWC_PIN, OWC_BIT)) // read bit
			data |= 0x80;
		_delay_us(10);
		clear_bit(OWC_DDR, OWC_BIT);
		set_bit(OWC_PORT, OWC_BIT); // re-enable pullup
		_delay_us(1); // some bus rise time
		byte >>= 1;
	}
	return data;
}

uint8_t owc_read() {
	return owc_omni(0xFF);
}

void owc_write(uint8_t byte) {
	owc_omni(byte);
}

